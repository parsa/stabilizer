#include <stddef.h>

void volatile_copy(volatile char *dst, const volatile char *src, size_t n) {
  __builtin_memcpy((void *)dst, (const void *)src, n);
}

