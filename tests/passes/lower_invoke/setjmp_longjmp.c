#include <setjmp.h>

int call_setjmp(jmp_buf env, int (*fn)(void)) {
  if (setjmp(env) == 0) {
    return fn();
  } else {
    return -1;
  }
}

