#if !defined(RUNTIME_FUNCTION_H)
#define RUNTIME_FUNCTION_H

#include <string.h>
#include <sys/mman.h>

#include "Util.h"
#include "Jump.h"
#include "Trap.h"
#include "Heap.h"
#include "MemRange.h"

struct Function;
struct FunctionLocation;

struct FunctionHeader {
private:
    union {
        uint8_t _jmp[sizeof(Jump)];
        uint8_t _trap[sizeof(Trap)];
    };
    
    Function* _f;
    
public:
    FunctionHeader(Function* f) : _f(f) {}
    FunctionHeader() {} // No-op constructor
    
    void setFunction(Function* f) {
        _f = f;
    }
    
    void jumpTo(void* target) {
        new(_jmp) Jump(target);
    }
    
    void trap() {
        new(_trap) Trap();
    }
    
    Function* getFunction() {
        return _f;
    }
};

struct Function {
private:
    friend class FunctionLocation;
    
    MemRange _code;
    MemRange _table;
    FunctionHeader* _header;
    uint8_t _savedHeader[64]; // Save raw bytes to preserve padding and straddling instructions
    
    bool _tableAdjacent;    //< If true, the relocation table should be placed next to the function
    
    uint8_t* _stackPad;		//< The address of the stack pad value for this function
    void** _tableSlot;
    
    FunctionLocation* _current;
    
    static size_t computeCodeSize(void* codeBase, void* codeLimit) {
        uintptr_t baseAddr = reinterpret_cast<uintptr_t>(codeBase);
        uintptr_t limitAddr = reinterpret_cast<uintptr_t>(codeLimit);
        if(limitAddr >= baseAddr) {
            return limitAddr - baseAddr;
        }
        return baseAddr - limitAddr;
    }

    /**
     * \brief Place a jump instruction to forward calls to this function
     * \arg target The destination of the jump instruction
     */
    inline void forward(void* target) {
        _header->setFunction(this);
        _header->jumpTo(target);
        flush_icache(_header, sizeof(FunctionHeader));
    }
    
    void copyTo(void* target);
    
public:
    /**
     * \brief Allocate Function objects on the randomized heap
     * \arg sz The object size
     */
    void* operator new(size_t sz) {
        return getDataHeap()->malloc(sz);
    }
    
    /**
     * \brief Free allocated memory to the randomized heap
     * \arg p The object base pointer
     */
    void operator delete(void* p) {
        getDataHeap()->free(p);
    }
    
    /**
    * \brief Create a new runtime representation of a function
    * \arg codeBase The address of the function
    * \arg codeLimit The top of the function
    * \arg tableBase The address of the function's relocation table
    * \arg tableSize The size of the function's relocation table
    * \arg tableAdjacent If true, the relocation table should be placed immediately after the function
	* \arg stackPad The address of this function's stack pad size
    */
    Function(void* codeBase, void* codeLimit, void* tableBase, size_t tableSize, bool tableAdjacent, void** tableSlot, uint8_t* stackPad);
    
    /**
     * \brief Free all code locations when deleted
     */
    ~Function();
    
    FunctionLocation* relocate();
    void randomizeStackPad();
    
    /**
     * \brief Place a trap instruction at the beginning of this function
     */
    inline void setTrap() {
        _header->setFunction(this);
        _header->trap();
    }
    
    inline void* getCodeBase() {
        return _code.base();
    }
    
    inline size_t getCodeSize() {
        return _code.size();
    }

    inline bool hasRelocationTable() const {
        return _table.size() > 0;
    }
    
    inline size_t getTableSize() const {
        return _table.size();
    }

    inline void* getOriginalTableBase() {
        if(_table.size() == 0) {
            return nullptr;
        }
        if(_tableSlot != nullptr) {
            return *_tableSlot;
        }
        // For the original code (before relocation), the table is in the data section.
        // Even if _tableAdjacent is true, it only applies to the relocated copy on the heap.
        // The original table is always at _table.base().
        return _table.base();
    }
    
    inline size_t getAllocationSize() {
        if(_tableAdjacent) {
            return _code.size() + _table.size();
        } else {
            return _code.size();
        }
    }
    
    inline FunctionLocation* getCurrentLocation() {
        return _current;
    }
};

#endif
