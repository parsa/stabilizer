#include <cmath>
#include <csignal>
#include <cstdint>
#include <cstdlib>
#include <set>
#include <vector>

#include <dlfcn.h>
#include <execinfo.h>
#include <sys/time.h>

#include "Function.h"
#include "FunctionLocation.h"
#include "Debug.h"
#include "Heap.h"
#include "Context.h"

using namespace std;
 
extern "C" int stabilizer_main(int argc, char **argv);

int main(int argc, char** argv);

void onTrap(int sig, siginfo_t* info, void*);
void onTimer(int sig, siginfo_t* info, void*);
void onFault(int sig, siginfo_t* info, void*);

void setTimer(int msec);
void setHandler(int sig, void(*fn)(int, siginfo_t*, void*));
static void randomizeStackPads();

typedef void(*ctor_t)();

set<Function*> functions;
set<Function*> live_functions;
set<uint8_t*> stack_pads;
vector<ctor_t> constructors;

bool rerandomizing = false;
size_t interval = 500;

void** topFrame = NULL;

static Function* lookupFunction(void* symbol) {
    if(symbol == nullptr) {
        return nullptr;
    }
    for(Function* f : functions) {
        if(f != nullptr && f->getCodeBase() == symbol) {
            return f;
        }
    }
    return nullptr;
}

static Function* findFunctionContaining(void* addr) {
    for(Function* f : functions) {
        uintptr_t base = (uintptr_t)f->getCodeBase();
        if((uintptr_t)addr >= base && (uintptr_t)addr < base + f->getCodeSize()) {
            return f;
        }
    }
    return nullptr;
}

/**
 * Entry point for a program run with Stabilizer.  The program's existing
 * main function has been renamed 'stabilizer_main' by the compiler pass.
 * 
 * 1. Save the current top of the stack
 * 2. Set signal handlers for debug traps, timers, and segfaults for error handling
 * 3. Place a trap instruction at the start of each randomizable function to trigger relocation on-demand
 * 4. Set the re-randomization timer
 * 5. Call module constructors
 * 6. Invoke stabilizer_main
 */
int main(int argc, char **argv) {
    DEBUG("Initializing Stabilizer");
    
    topFrame = (void**)__builtin_frame_address(0);
    DEBUG("Stack top is at %p", topFrame);
    
    // Register signal handlers
    setHandler(Trap::TrapSignal, onTrap);
    setHandler(SIGALRM, onTimer);
    setHandler(SIGSEGV, onFault);
    DEBUG("Signal handlers installed");

    // Call all constructors
    for(vector<ctor_t>::iterator i = constructors.begin(); i != constructors.end(); i++) {
        (*i)();
    }
    DEBUG("Finished with program constructors");

    // Lazily relocate functions now that constructors have run
    for(set<Function*>::iterator iter = functions.begin(); iter != functions.end(); iter++) {
        Function* f = *iter;
        f->setTrap();
    }
    DEBUG("Trapped all functions");

    setTimer(interval);
    
    // Call the old main function
    int r = stabilizer_main(argc, argv);
    DEBUG("Shutting down");
    
    return r;
}

static void* moduleBase = nullptr;

extern "C" {
    void stabilizer_register_function(void* codeBase, void* codeLimit, void* tableBase, size_t tableSize, bool adjacent, void** tableSlot, uint8_t* stackPad) {
        Dl_info info;
        const char* name = nullptr;
        if(dladdr(codeBase, &info) && info.dli_sname) {
            name = info.dli_sname;
        }
        if(moduleBase == nullptr) {
            Dl_info moduleInfo;
            if(dladdr((void*)&stabilizer_main, &moduleInfo)) {
                moduleBase = moduleInfo.dli_fbase;
            }
        }
        uintptr_t offset = 0;
        if(moduleBase != nullptr) {
            offset = (uintptr_t)codeBase - (uintptr_t)moduleBase;
        }
        DEBUG("register_function %s code=[%p,%p) (offset=0x%zx) table=%p size=%zu adjacent=%d slot=%p stackPad=%p",
              name ? name : "<unknown>",
              codeBase,
              codeLimit,
              (size_t)offset,
              tableBase,
              tableSize,
              adjacent ? 1 : 0,
              tableSlot,
              stackPad);
        if(tableBase != nullptr && tableSize >= sizeof(uintptr_t)) {
            uintptr_t* tableEntries = reinterpret_cast<uintptr_t*>(tableBase);
            size_t entryCount = tableSize / sizeof(uintptr_t);
            for(size_t i = 0; i < entryCount; ++i) {
                DEBUG("  table[%zu] = %p", i, reinterpret_cast<void*>(tableEntries[i]));
            }
        }
        Function* f = new Function(codeBase, codeLimit, tableBase, tableSize, adjacent, tableSlot, stackPad);
        functions.insert(f);
    }

    void stabilizer_register_constructor(ctor_t ctor) {
        constructors.push_back(ctor);
    }
    
    void stabilizer_register_stack_pad(uint8_t* pad) {
        stack_pads.insert(pad);
    }

    void* stabilizer_malloc(size_t sz) {
        return getDataHeap()->malloc(sz);
    }
    
    void* stabilizer_calloc(size_t n, size_t sz) {
        return getDataHeap()->calloc(n, sz);
    }

    void* stabilizer_realloc(void *p, size_t sz) {
        return getDataHeap()->realloc(p, sz);
    }

    void stabilizer_free(void *p) {
        if(getDataHeap()->getSize(p) == 0) {
            free(p);
        } else {
            getDataHeap()->free(p);
        }
    }

    void* stabilizer_get_active_code_location(void* symbol) {
        Function* f = lookupFunction(symbol);
        if(f == nullptr) {
            return nullptr;
        }
        if(FunctionLocation* location = f->getCurrentLocation()) {
            return location->getBase();
        }
        return f->getCodeBase();
    }

    void* stabilizer_get_relocation_table(void* symbol) {
        Function* f = lookupFunction(symbol);
        if(f == nullptr) {
            return nullptr;
        }
        if(FunctionLocation* location = f->getCurrentLocation()) {
            return location->getTableBase();
        }
        return f->getOriginalTableBase();
    }

    void* stabilizer_get_current_relocation_table_base() {
        void* returnAddr = __builtin_return_address(0);
        // DEBUG("helper called from %p", returnAddr);
        FunctionLocation* location = FunctionLocation::locate(returnAddr);
        
        if(location != nullptr) {
            void* table = location->getTableBase();
            // DEBUG("helper: location found, table=%p", table);
            return table;
        }
        
        // Fallback: check if running from original code
        Function* f = findFunctionContaining(returnAddr);
        if(f != nullptr) {
            DEBUG("Running from original code at %p", returnAddr);
            return f->getOriginalTableBase();
        }

        fprintf(stderr, " [libstabilizer.cpp:%d] helper miss: %p\n", __LINE__, returnAddr);
        return nullptr;
    }

    void reportDoubleFreeError() {
        ABORT("Double free error");
    }
}

    void onTrap(int sig, siginfo_t* info, void* p) {
    ucontext_t* uc = (ucontext_t*)p;
    Context c(p);

    // Back up over the trap instruction
    c.ip() = (void*)((uintptr_t)c.ip() - Trap::TrapAdjust);

    // Extract the trapped function (stored next to the trap instruction)
    FunctionHeader* h = (FunctionHeader*)c.ip();
    Function* f = h->getFunction();
    
    // Sanity check: verify the Function pointer looks valid
    if(f == nullptr) {
        ABORT("Trapped at %p but Function pointer is NULL!", c.ip());
    }
    
    // If the trap was placed to trigger a re-randomization
    if(rerandomizing) {
        DEBUG("Re-randomization started after trap on %p", c.ip());
        live_functions.clear();
        
        // Mark all on-stack function locations as used
        Stack s = c.stack();
        while(s.fp() != topFrame) {
            FunctionLocation::mark(s.ret());
            s++;
        }
        
        // Mark the current instruction pointer as used
        FunctionLocation::mark((void*)c.ip());
        
        // Mark the top return address on the stack as used
        FunctionLocation::mark(*(void**)c.sp());
        
        // Collect unused function locations
        FunctionLocation::sweep();
        
        rerandomizing = false;
        setTimer(interval);
    }

    // Relocate the function
    FunctionLocation* oldLocation = f->relocate();
    live_functions.insert(f);
    if(oldLocation != NULL) {
        oldLocation->release();
    }

    void* newBase = f->getCurrentLocation()->getBase();
    c.ip() = newBase;
}

void onTimer(int sig, siginfo_t* info, void* p) {
    Context c(p);
    
    randomizeStackPads();

    if(functions.size() == 0) {
        setTimer(interval);
    } else {
        DEBUG("Placing traps");
        for(set<Function*>::iterator iter = live_functions.begin(); iter != live_functions.end(); iter++) {
            Function* f = *iter;
            if(c.ip() == f->getCodeBase()) {
                DEBUG("Forwarding from trap at %p", c.ip());
                c.ip() = f->getCurrentLocation()->getBase();
            }
            f->setTrap();
        }
        
        live_functions.clear();
    }
    
    rerandomizing = true;
}

void onFault(int sig, siginfo_t* info, void* p) {
    Context c(p);
    
    // Try to find which function the fault is in
    void* faultAddr = c.ip();
    FunctionLocation* loc = FunctionLocation::locate(faultAddr);
    Function* func = findFunctionContaining(faultAddr);
    
    fprintf(stderr, " [libstabilizer.cpp:%d] Fault details:\n", __LINE__);
    fprintf(stderr, "   IP: %p\n", faultAddr);
    fprintf(stderr, "   SP: %p\n", c.sp());
    fprintf(stderr, "   FP: %p\n", c.fp());
    fprintf(stderr, "   Accessing: %p\n", info->si_addr);
    fprintf(stderr, "   In relocated function: %s\n", loc ? "yes" : "no");
    fprintf(stderr, "   In original function: %s\n", func ? "yes" : "no");
    if(loc) {
        fprintf(stderr, "   Relocated base: %p\n", loc->getBase());
    }
    if(func) {
        fprintf(stderr, "   Original base: %p\n", func->getCodeBase());
    }
    
    void** sp = (void**)c.sp();
    if(sp != nullptr) {
        fprintf(stderr, "   Stack top: %p %p %p %p\n",
                sp[0], sp[1], sp[2], sp[3]);
    }
    void* rax = c.rax();
    void* rdi = c.rdi();
    fprintf(stderr, "   Registers: RAX=%p RDI=%p\n", rax, rdi);
    
    // Dump stack walk
    fprintf(stderr, "   Stack walk:\n");
    Stack s = c.stack();
    for(int i = 0; i < 10 && s.fp() != nullptr && s.fp() != topFrame; i++) {
        void* retAddr = s.ret();
        FunctionLocation* retLoc = FunctionLocation::locate(retAddr);
        fprintf(stderr, "     [%d] ret=%p %s\n", i, retAddr, 
                retLoc ? "(relocated)" : "");
        s++;
    }
    
    ABORT("Fault at %p, accessing address %p", c.ip(), info->si_addr);
}

void setTimer(int msec) {
    struct itimerval timer;

    timer.it_value.tv_sec = (msec - msec % 1000) / 1000;
    timer.it_value.tv_usec = 1000 * (msec % 1000);
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 0;
    
    setitimer(ITIMER_REAL, &timer, 0);
}

static stack_t altstack;
static bool altstack_initialized = false;

void setupAltStack() {
    if(altstack_initialized) return;
    
    // Allocate 64KB alternate signal stack
    altstack.ss_size = 64 * 1024;
    altstack.ss_sp = malloc(altstack.ss_size);
    if(altstack.ss_sp == nullptr) {
        perror("malloc altstack");
        abort();
    }
    altstack.ss_flags = 0;
    
    if(sigaltstack(&altstack, nullptr) != 0) {
        perror("sigaltstack");
        abort();
    }
    
    altstack_initialized = true;
}

void setHandler(int sig, void(*fn)(int, siginfo_t*, void*)) {
    setupAltStack();
    
    struct sigaction sa{};
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = (void(*)(int, siginfo_t*, void*))fn;
    sa.sa_flags = SA_SIGINFO | SA_NODEFER | SA_ONSTACK;
    sigaction(sig, &sa, NULL);
}

static void randomizeStackPads() {
    if(!stack_pads.empty()) {
        DEBUG("Re-randomizing stack pads");
        for(set<uint8_t*>::iterator iter = stack_pads.begin(); iter != stack_pads.end(); iter++) {
            uint8_t* pad = *iter;
            if(pad != NULL) {
                *pad = getRandomByte();
            }
        }
    }

    bool functionPads = false;
    for(set<Function*>::iterator iter = functions.begin(); iter != functions.end(); iter++) {
        Function* f = *iter;
        if(f != NULL) {
            f->randomizeStackPad();
            functionPads = true;
        }
    }

    if(functionPads) {
        DEBUG("Re-randomizing function stack pads");
    }
}
