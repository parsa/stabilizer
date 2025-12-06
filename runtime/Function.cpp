#include "Function.h"
#include "FunctionLocation.h"

#include <dlfcn.h>

/**
 * Free the current function location and stack pad table
 */
Function::~Function() {
    if(_current != NULL) {
        _current->release();
    }
    
    if(_stackPad != NULL) {
        getDataHeap()->free(_stackPad);
    }
}

/**
 * Copy the code and relocation table for this function.  Use the pre-assembled
 * code/table chunk if the function has already been relocated.
 * 
 * \arg target The destination of the copy.
 */
    // Patch the magic number for the helper function call
    extern "C" void* stabilizer_get_current_relocation_table_base();
    namespace {
        void patchMagic(void* target, size_t size) {
            uint64_t magic = 0x57ab1122334457abULL;
            uint64_t helperAddr = reinterpret_cast<uint64_t>(stabilizer_get_current_relocation_table_base);
            static int patchLogCount = 0;
            constexpr int patchLogLimit = 16;
            int patchesThisCall = 0;
            
            // We scan the copied code for the magic number
            // The magic number is part of a movabs instruction (10 bytes), so it's 8 bytes aligned or unaligned
            uint8_t* code = static_cast<uint8_t*>(target);
            
                // Use a pattern matching approach that is robust to alignment
            for(size_t i = 0; i < size - 8; i++) {
            if(code[i] == 0xab && code[i+1] == 0x57 && code[i+2] == 0x44 && code[i+3] == 0x33 && 
               code[i+4] == 0x22 && code[i+5] == 0x11 && code[i+6] == 0xab && code[i+7] == 0x57) {
                
                uint64_t* targetPtr = reinterpret_cast<uint64_t*>(&code[i]);
                *targetPtr = helperAddr;
                patchesThisCall++;
                
                if(patchLogCount < patchLogLimit) {
                    DEBUG("Patched magic at offset %zu -> helper %p", i, reinterpret_cast<void*>(helperAddr));
                    ++patchLogCount;
                }
                
                i += 7; // Skip the bytes we just patched
            }
            }
            
            (void)patchesThisCall; // Suppress unused variable warning in release builds
        }
    }

    Function::Function(void* codeBase, void* codeLimit, void* tableBase, size_t tableSize, bool tableAdjacent, void** tableSlot, uint8_t* stackPad) :
        _code(codeBase, computeCodeSize(codeBase, codeLimit)), _table(tableBase, tableSize) {
        
        this->_tableAdjacent = tableAdjacent;
        this->_stackPad = stackPad;
        this->_tableSlot = tableSlot;
        this->_current = NULL;
        Dl_info ctorInfo{};
        const char* ctorName = "<unknown>";
        if(dladdr(codeBase, &ctorInfo) && ctorInfo.dli_sname) {
            ctorName = ctorInfo.dli_sname;
        }

        DEBUG("Function ctor %s code=%p table=%p size=%zu stackPad=%p slot=%p adjacent=%d",
              ctorName,
              codeBase,
              tableBase,
              tableSize,
              stackPad,
              tableSlot,
              tableAdjacent ? 1 : 0);

        // Make the function header writable
        if(mprotect(_code.pageBase(), _code.pageSize(), PROT_READ | PROT_WRITE | PROT_EXEC)) {
            perror("Unable make code writable");
            abort();
        }
        
        _header = (FunctionHeader*)_code.base();
        
        // Patch the code in place (for execution before relocation)
        patchMagic(_code.base(), _code.size());
        flush_icache(_code.base(), _code.size());
        
        // Save raw bytes AFTER patching - this ensures _savedHeader contains
        // the patched magic numbers. Otherwise, if magic is at offset 60-67
        // and _savedHeader is 64 bytes, restoration would create a partial
        // magic (bytes 60-63 unpatched, 64-67 patched) that can't be detected.
        memcpy(_savedHeader, _code.base(), sizeof(_savedHeader));
    }

void Function::copyTo(void* target) {
    // Minimized debug output to reduce stack usage in signal handler
    uint8_t* newTableBase = nullptr;
    if(_current == NULL) {
        // Copy the code from the original function
        memcpy(target, _code.base(), _code.size());

        // Patch in the saved header, since the original has been overwritten
        memcpy(target, _savedHeader, sizeof(_savedHeader));
        
        // If there is a stack pad table, move it to a random location
        if(_stackPad != NULL && _table.base() != NULL && _table.size() >= sizeof(uintptr_t)) {
            uintptr_t* table = reinterpret_cast<uintptr_t*>(_table.base());
            size_t entryCount = _table.size() / sizeof(uintptr_t);
            for(size_t i = 0; i < entryCount; ++i) {
                DEBUG("relocation table entry %zu for %p: %p (stackPad=%p)",
                      i,
                      _code.base(),
                      reinterpret_cast<void*>(table[i]),
                      _stackPad);
                if(table[i] == reinterpret_cast<uintptr_t>(_stackPad)) {
                    uint8_t* newPad = static_cast<uint8_t*>(getDataHeap()->malloc(1));
                    DEBUG("stack pad relocation for %p entry %zu: %p -> %p",
                          _code.base(),
                          i,
                          reinterpret_cast<void*>(table[i]),
                          newPad);
                    _stackPad = newPad;
                    table[i] = reinterpret_cast<uintptr_t>(_stackPad);
                    DEBUG("updated table[%zu] for %p to %p",
                          i,
                          _code.base(),
                          reinterpret_cast<void*>(table[i]));
                }
            }
        }

        // Copy the relocation table, if needed
        if(_tableAdjacent && _table.base() != NULL && _table.size() > 0) {
            // Preserve contiguous code+table layout so PC-relative loads survive relocation.
            uint8_t* a = (uint8_t*)target;
            newTableBase = &a[_code.size()];
            memcpy(newTableBase, _table.base(), _table.size());
        }
    } else {
        memcpy(target, _current->_memory.base(), getAllocationSize());
        if(_tableAdjacent && _table.size() > 0) {
            uint8_t* a = (uint8_t*)target;
            newTableBase = &a[_code.size()];
        }
    }
    if(newTableBase != nullptr) {
        DEBUG("relocated table entry[0] for %p: %p",
              _code.base(),
              reinterpret_cast<void**>(newTableBase)[0]);
    }
    if(_tableSlot != nullptr && _table.size() > 0) {
        void* updatedBase = newTableBase != nullptr ? newTableBase : _table.base();
        *_tableSlot = updatedBase;
        DEBUG("Updated table slot %p -> %p", _tableSlot, updatedBase);
    }
    
    patchMagic(target, _code.size());
    flush_icache(target, _code.size());
}

/**
 * Create a new FunctionLocation for this Function.
 * \arg relocation The ID for the current relocation phase.
 * \returns Whether or not a new location was created
 */
FunctionLocation* Function::relocate() {
    FunctionLocation* oldLocation = _current;
    _current = new FunctionLocation(this);
    Dl_info relInfo{};
    const char* relName = "<unknown>";
    if(dladdr(_code.base(), &relInfo) && relInfo.dli_sname) {
        relName = relInfo.dli_sname;
    }

    DEBUG("relocate %s %p -> %p (table=%p size=%zu adjacent=%d)",
          relName,
          _code.base(),
          _current->getBase(),
          _table.base(),
          _table.size(),
          _tableAdjacent ? 1 : 0);
    _current->activate();

    // Fill the stack pad table with random bytes
    randomizeStackPad();
    
    return oldLocation;
}

void Function::randomizeStackPad() {
    if(_stackPad != NULL) {
        *_stackPad = getRandomByte();
    }
}
