// Test that vector constants are materialized into global variables
#include <stdint.h>

typedef int v4si __attribute__((vector_size(16)));

int test_vector_constant(int idx) {
  // Vector constant that should be materialized
  v4si vec = {1, 2, 3, 4};
  return vec[idx];
}

