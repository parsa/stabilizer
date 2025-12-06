#include <stdint.h>

int64_t int64_switch(int64_t value) {
  switch (value) {
    case 0: return 1000;
    case 1234567890123LL: return 2000;
    default: return -1000;
  }
}

