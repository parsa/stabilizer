#include <stdint.h>

int phi_switch(int x, int y) {
  int result;
  switch ((x + y) & 3) {
    case 0: result = x - y; break;
    case 1: result = y - x; break;
    case 2: result = x + y; break;
    default: result = x ^ y; break;
  }
  return result + 1;
}

