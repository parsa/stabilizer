#include <stddef.h>

void shift_bytes(char *dst, const char *src, size_t len) {
  __builtin_memmove(dst, src, len);
}

