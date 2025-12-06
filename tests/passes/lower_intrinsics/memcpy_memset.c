#include <stddef.h>
#include <string.h>

void do_copy(char *dst, const char *src) {
  __builtin_memcpy(dst, src, 32);
}

void do_fill(char *dst, int value) {
  __builtin_memset(dst, value, 64);
}

