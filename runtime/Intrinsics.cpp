#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <string.h>

extern "C" {
    float powif(float b, int e) {
        return powf(b, (float)e);
    }

    // ---------------------------------------------------------------------
    // Memory intrinsic wrappers
    //
    // `pass/LowerIntrinsics.cpp` replaces LLVM intrinsics with libcalls while
    // preserving the intrinsic function type. That type is overloaded by the
    // length integer width (i8/i16/i32/i64), and may or may not include extra
    // parameters like alignment and volatility depending on LLVM version.
    //
    // These wrappers intentionally:
    // - accept the length using the correct integer width (avoids ABI issues)
    // - accept (and ignore) the extra parameters when present
    // ---------------------------------------------------------------------

    void memcpy_i8(void* dst, const void* src, uint8_t len, uint32_t, bool) {
        memcpy(dst, src, (size_t)len);
    }

    void memcpy_i16(void* dst, const void* src, uint16_t len, uint32_t, bool) {
        memcpy(dst, src, (size_t)len);
    }

    void memcpy_i32(void* dst, const void* src, uint32_t len, uint32_t, bool) {
        memcpy(dst, src, (size_t)len);
    }

    void memcpy_i64(void* dst, const void* src, uint64_t len, uint32_t, bool) {
        memcpy(dst, src, (size_t)len);
    }

    void memmove_i8(void* dst, const void* src, uint8_t len, uint32_t, bool) {
        memmove(dst, src, (size_t)len);
    }

    void memmove_i16(void* dst, const void* src, uint16_t len, uint32_t, bool) {
        memmove(dst, src, (size_t)len);
    }

    void memmove_i32(void* dst, const void* src, uint32_t len, uint32_t, bool) {
        memmove(dst, src, (size_t)len);
    }

    void memmove_i64(void* dst, const void* src, uint64_t len, uint32_t, bool) {
        memmove(dst, src, (size_t)len);
    }

    void memset_i8(void* p, uint8_t val, uint8_t len, uint32_t, bool) {
        memset(p, val, (size_t)len);
    }

    void memset_i16(void* p, uint8_t val, uint16_t len, uint32_t, bool) {
        memset(p, val, (size_t)len);
    }
    
    void memset_i32(void* p, uint8_t val, uint32_t len, uint32_t align, bool isvolatile) {
        memset(p, val, len);
    }

    void memset_i64(void* p, uint8_t val, uint64_t len, uint32_t align, bool isvolatile) {
        memset(p, val, len);
    }
}
