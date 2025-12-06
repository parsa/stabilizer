#include <stdint.h>

int sparse_switch(int x) {
  switch (x) {
    case 10: return 100;
    case 42: return 200;
    case 999: return 300;
    default: return -5;
  }
}

