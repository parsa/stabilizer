#if !defined(RUNTIME_CODEWINDOW_H)
#define RUNTIME_CODEWINDOW_H

#include <cstdint>

// Configure the address window used for allocating relocated executable code.
// The window is expressed as a half-open interval [lo, hi).
void stabilizer_set_code_window(uintptr_t lo, uintptr_t hi);

// Returns true if a window is configured.
bool stabilizer_get_code_window(uintptr_t* lo, uintptr_t* hi);

#endif

