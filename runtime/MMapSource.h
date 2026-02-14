#if !defined(RUNTIME_MMAPSOURCE_H)
#define RUNTIME_MMAPSOURCE_H

#include "Util.h"
#include "CodeWindow.h"

#if defined(__linux__) && defined(__x86_64__)
#include <errno.h>
#endif

template<int Prot, int Flags> class MMapSource {
private:
    bool _exhausted32;

#if defined(__linux__) && defined(__x86_64__)
    inline uintptr_t rand_u64() {
        static RandomNumberGenerator rng;
        uint64_t hi = (uint32_t)rng.next();
        uint64_t lo = (uint32_t)rng.next();
        return (uintptr_t)((hi << 32) | lo);
    }

    inline void* mmapInWindow(uintptr_t lo, uintptr_t hi, size_t sz) {
        // Require a non-empty interval with space for sz.
        if(hi <= lo) {
            return NULL;
        }
        if(sz > (size_t)(hi - lo)) {
            return NULL;
        }

        // Align bounds to pages.
        const uintptr_t pageMask = ~((uintptr_t)Alignment - 1u);
        lo = (lo + (Alignment - 1u)) & pageMask; // round up
        hi = hi & pageMask;                      // round down

        if(hi <= lo || (uintptr_t)(hi - lo) < (uintptr_t)sz) {
            return NULL;
        }

        uintptr_t maxBase = hi - (uintptr_t)sz;
        uintptr_t spanPages = (maxBase - lo) / (uintptr_t)Alignment;

#if defined(MAP_FIXED_NOREPLACE)
        static bool fixed_noreplace_works = true;
#endif

        // Try a bounded number of random placements within the window.
        for(size_t attempt = 0; attempt < 256; attempt++) {
            uintptr_t page = (spanPages == 0) ? 0 : (rand_u64() % (spanPages + 1u));
            uintptr_t addr = lo + page * (uintptr_t)Alignment;

#if defined(MAP_FIXED_NOREPLACE)
            if(fixed_noreplace_works) {
                void* p = mmap((void*)addr, sz, Prot, (Flags & ~MAP_32BIT) | MAP_FIXED_NOREPLACE, -1, 0);
                if(p != MAP_FAILED) {
                    return p;
                }
                if(errno == EINVAL) {
                    fixed_noreplace_works = false;
                }
                continue;
            }
#endif

            // Fallback: use an address hint, then verify it landed in-range.
            void* p = mmap((void*)addr, sz, Prot, (Flags & ~MAP_32BIT), -1, 0);
            if(p == MAP_FAILED) {
                continue;
            }

            uintptr_t got = (uintptr_t)p;
            if(got < lo || got > maxBase) {
                munmap(p, sz);
                continue;
            }
            return p;
        }

        return NULL;
    }
#endif
    
public:
    enum { Alignment = PAGESIZE };

    MMapSource() {
        _exhausted32 = false;
    }
    
    inline void* malloc(size_t sz) {
        void* ptr;

#if defined(__linux__) && defined(__x86_64__)
        // For executable allocations (relocated code), try to allocate within
        // the configured window near the original text so 32-bit pc-relative
        // relocations remain representable even for PIE binaries.
        if(Prot & PROT_EXEC) {
            uintptr_t lo = 0;
            uintptr_t hi = 0;
            if(stabilizer_get_code_window(&lo, &hi)) {
                ptr = mmapInWindow(lo, hi, sz);
                if(ptr != NULL) {
                    return ptr;
                }
            }
        }
#endif

        if(Flags & MAP_32BIT) {
            // If we haven't exhausted the 32 bit pages
            if(!_exhausted32) {
                ptr = mmap(NULL, sz, Prot, Flags, -1, 0);
                
                if(ptr != MAP_FAILED) {
                    return ptr;
                } else {
                    _exhausted32 = true;
                }
            }
        }
        
        // Try the map without the MAP_32BIT flag set
        ptr = mmap(NULL, sz, Prot, Flags & ~MAP_32BIT, -1, 0);
        
        if(ptr == MAP_FAILED) {
            ptr = NULL;
        }
        
        return ptr;
    }
};

#endif
