#include "CodeWindow.h"

static uintptr_t g_code_window_lo = 0;
static uintptr_t g_code_window_hi = 0;
static bool g_code_window_set = false;

void stabilizer_set_code_window(uintptr_t lo, uintptr_t hi) {
    g_code_window_lo = lo;
    g_code_window_hi = hi;
    g_code_window_set = (hi > lo);
}

bool stabilizer_get_code_window(uintptr_t* lo, uintptr_t* hi) {
    if(!g_code_window_set) {
        return false;
    }
    if(lo != nullptr) {
        *lo = g_code_window_lo;
    }
    if(hi != nullptr) {
        *hi = g_code_window_hi;
    }
    return true;
}

