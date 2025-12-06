#include <math.h>

typedef float vec4f __attribute__((vector_size(16)));

vec4f vector_ops(vec4f v) {
  return __builtin_elementwise_sqrt(v) + __builtin_elementwise_log(v + (vec4f){1.0f, 1.0f, 1.0f, 1.0f});
}

