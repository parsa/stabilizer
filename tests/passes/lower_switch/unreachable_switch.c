#include <stdint.h>

int unreachable_switch(int x) {
  switch (x) {
    case 1: return 10;
    case 2: return 20;
  }
  __builtin_unreachable();
}

