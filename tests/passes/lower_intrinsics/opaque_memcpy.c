#include <stddef.h>

void copy_int_array(int *dst, const int *src, int count) {
  __builtin_memcpy(dst, src, sizeof(int) * (size_t)count);
}

