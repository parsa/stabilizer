#include <stdint.h>

int callbr_switch(int cond, int x) {
  asm goto(
      "test %0, %0\n\t"
      "je %l[target]\n\t"
      :
      : "r"(cond)
      : "cc"
      : target);
  goto after;

target:
  switch (x & 3) {
    case 0: return 1;
    case 1: return 2;
    default: return 3;
  }

after:
  switch (x) {
    case 5: return 50;
    case 7: return 70;
    default: return -1;
  }
}

