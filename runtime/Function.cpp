#include "Function.h"
#include "FunctionLocation.h"

#if defined(__x86_64__)
#include <elf.h>
#include <limits>
#endif

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
void Function::copyTo(void* target) {
    void* source = NULL;

    if(_current == NULL) {
        source = _code.base();

        // Copy the code from the original function
        memcpy(target, _code.base(), _code.size());

        // Patch in the saved header, since the original has been overwritten
        *(FunctionHeader*)target = _savedHeader;

        // If there is a stack pad table, move it to a random location
        if(_stackPad != NULL && _table.base() != NULL && _table.size() >= sizeof(uintptr_t)) {
            uintptr_t* table = (uintptr_t*)_table.base();
            size_t n = _table.size() / sizeof(uintptr_t);
            for(size_t i = 0; i < n; i++) {
                if(table[i] == (uintptr_t)_stackPad) {
                    _stackPad = (uint8_t*)getDataHeap()->malloc(1);
                    table[i] = (uintptr_t)_stackPad;
                }
            }
        }

        // Copy the relocation table, if needed
        if(_tableAdjacent) {
            uint8_t* a = (uint8_t*)target;
            memcpy(&a[_code.size()], _table.base(), _table.size());
        }
    } else {
        source = _current->_memory.base();
        memcpy(target, source, getAllocationSize());
    }

    // After copying the code to a new location, patch any pc-relative
    // references (e.g., constant pools) so they still point to the intended
    // external targets from the new location.
    if(source != NULL) {
        applyTextRelocs(source, target);
    }
}

/**
 * Create a new FunctionLocation for this Function.
 * \arg relocation The ID for the current relocation phase.
 * \returns Whether or not a new location was created
 */
FunctionLocation* Function::relocate() {
    FunctionLocation* oldLocation = _current;
    _current = new FunctionLocation(this);
    _current->activate();

    // Fill the stack pad table with random bytes
    if(_stackPad != NULL) {
        // Update random stack pad
        *_stackPad = getRandomByte();
    }
    
    return oldLocation;
}

void Function::applyTextRelocs(void* source, void* dest) {
#if defined(__x86_64__)
    if(_textRelocs.empty()) {
        return;
    }

    intptr_t delta = (uint8_t*)dest - (uint8_t*)source;

    // If the function didn't move, nothing to do.
    if(delta == 0) {
        return;
    }

    uint8_t* src = (uint8_t*)source;
    uint8_t* dst = (uint8_t*)dest;

    // Anything within the copied allocation should move with the function,
    // and therefore should NOT be re-targeted back to the original address.
    uintptr_t internal_begin = (uintptr_t)src;
    uintptr_t internal_end = (uintptr_t)src + _code.size() + (_tableAdjacent ? _table.size() : 0);

    for(const TextReloc& r : _textRelocs) {
        size_t fieldSize = 0;
        switch(r.type) {
            case R_X86_64_PC8:
                fieldSize = 1;
                break;
            case R_X86_64_PC16:
                fieldSize = 2;
                break;
            case R_X86_64_PC32:
            case R_X86_64_PLT32:
            case R_X86_64_GOTPCREL:
            case R_X86_64_GOTPCRELX:
            case R_X86_64_REX_GOTPCRELX:
            case R_X86_64_GOTPC32:
            case R_X86_64_TLSGD:
            case R_X86_64_TLSLD:
            case R_X86_64_GOTTPOFF:
            case R_X86_64_GOTPC32_TLSDESC:
                fieldSize = 4;
                break;
            case R_X86_64_PC64:
            case R_X86_64_GOTPCREL64:
            case R_X86_64_GOTPC64:
                fieldSize = 8;
                break;
            default:
                fieldSize = 0;
                break;
        }

        if(fieldSize == 0) {
            continue;
        }

        if(r.offset + fieldSize > _code.size()) {
            continue;
        }

        uint8_t* oldP = src + r.offset;
        uint8_t* newP = dst + r.offset;

        int64_t oldVal = 0;
        if(fieldSize == 8) {
            int64_t v = 0;
            memcpy(&v, newP, sizeof(v));
            oldVal = v;
        } else if(fieldSize == 4) {
            int32_t v = 0;
            memcpy(&v, newP, sizeof(v));
            oldVal = v;
        } else if(fieldSize == 2) {
            int16_t v = 0;
            memcpy(&v, newP, sizeof(v));
            oldVal = v;
        } else if(fieldSize == 1) {
            int8_t v = 0;
            memcpy(&v, newP, sizeof(v));
            oldVal = v;
        }

        switch(r.type) {
            case R_X86_64_PC8:
            case R_X86_64_PC16:
            case R_X86_64_PC32:
            case R_X86_64_PLT32:
            case R_X86_64_PC64: {
                // Relocation semantics: S + A - P
                // Derive S from the already-relocated field value.
                intptr_t S = (intptr_t)oldVal + (intptr_t)oldP - (intptr_t)r.addend;

                // If the target is within our copied allocation, leave it alone
                // so it continues to resolve into the relocated copy (including
                // the adjacent relocation table).
                if((uintptr_t)S >= internal_begin && (uintptr_t)S < internal_end) {
                    break;
                }

                int64_t newVal64 = (int64_t)oldVal - (int64_t)delta;

                if(fieldSize == 8) {
                    int64_t newVal = (int64_t)newVal64;
                    memcpy(newP, &newVal, sizeof(newVal));
                } else if(fieldSize == 4) {
                    if(newVal64 < std::numeric_limits<int32_t>::min() || newVal64 > std::numeric_limits<int32_t>::max()) {
                        ABORT("Text relocation overflow (PC32/PLT32): func=%p src=%p dst=%p off=%zu old=%ld delta=%ld",
                            _code.base(), source, dest, r.offset, (long)oldVal, (long)delta);
                    }
                    int32_t newVal = (int32_t)newVal64;
                    memcpy(newP, &newVal, sizeof(newVal));
                } else if(fieldSize == 2) {
                    if(newVal64 < std::numeric_limits<int16_t>::min() || newVal64 > std::numeric_limits<int16_t>::max()) {
                        ABORT("Text relocation overflow (PC16): func=%p src=%p dst=%p off=%zu old=%ld delta=%ld",
                            _code.base(), source, dest, r.offset, (long)oldVal, (long)delta);
                    }
                    int16_t newVal = (int16_t)newVal64;
                    memcpy(newP, &newVal, sizeof(newVal));
                } else if(fieldSize == 1) {
                    if(newVal64 < std::numeric_limits<int8_t>::min() || newVal64 > std::numeric_limits<int8_t>::max()) {
                        ABORT("Text relocation overflow (PC8): func=%p src=%p dst=%p off=%zu old=%ld delta=%ld",
                            _code.base(), source, dest, r.offset, (long)oldVal, (long)delta);
                    }
                    int8_t newVal = (int8_t)newVal64;
                    memcpy(newP, &newVal, sizeof(newVal));
                }
                break;
            }

            case R_X86_64_GOTPCREL:
            case R_X86_64_GOTPCRELX:
            case R_X86_64_REX_GOTPCRELX:
            case R_X86_64_GOTPCREL64:
            case R_X86_64_GOTPC32:
            case R_X86_64_GOTPC64:
            case R_X86_64_TLSGD:
            case R_X86_64_TLSLD:
            case R_X86_64_GOTTPOFF:
            case R_X86_64_GOTPC32_TLSDESC: {
                int64_t newVal64 = (int64_t)oldVal - (int64_t)delta;
                if(fieldSize == 8) {
                    int64_t newVal = (int64_t)newVal64;
                    memcpy(newP, &newVal, sizeof(newVal));
                } else {
                    if(newVal64 < std::numeric_limits<int32_t>::min() || newVal64 > std::numeric_limits<int32_t>::max()) {
                        ABORT("Text relocation overflow (PC-relative 32): func=%p src=%p dst=%p off=%zu old=%ld delta=%ld",
                            _code.base(), source, dest, r.offset, (long)oldVal, (long)delta);
                    }
                    int32_t newVal = (int32_t)newVal64;
                    memcpy(newP, &newVal, sizeof(newVal));
                }
                break;
            }

            default:
                // Unknown relocation type: leave untouched.
                break;
        }
    }
#else
    (void)source;
    (void)dest;
#endif
}
