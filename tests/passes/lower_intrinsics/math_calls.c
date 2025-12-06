#include <math.h>

float chained_math(float x) {
  float root = __builtin_sqrtf(fabsf(x) + 1.0f);
  float logged = __builtin_logf(root + 2.0f);
  float expd = __builtin_expf(logged);
  float powed = __builtin_powf(expd, 3.0f);
  float powi = __builtin_powif(powed, 2);
  return powi + __builtin_log10f(expd + 1.0f);
}

