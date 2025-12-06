#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <string.h>

extern "C" {
    float powif(float b, int e) {
        return powf(b, (float)e);
    }

    double powid(double b, int e) {
        return pow(b, (double)e);
    }

    long double powil(long double b, int e) {
        return powl(b, (long double)e);
    }

    void memset_i8(void* p, uint8_t val, uint8_t len, uint32_t align, bool isvolatile) {
        memset(p, val, len);
    }

    void memset_i16(void* p, uint8_t val, uint16_t len, uint32_t align, bool isvolatile) {
        memset(p, val, len);
    }

    void memset_i32(void* p, uint8_t val, uint32_t len, uint32_t align, bool isvolatile) {
        memset(p, val, len);
    }

    void memset_i64(void* p, uint8_t val, uint64_t len, uint32_t align, bool isvolatile) {
        memset(p, val, len);
    }
}
