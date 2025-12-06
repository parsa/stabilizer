#include <math.h>

float needs_warning(float x) {
  return __builtin_fabsf(x);
}

