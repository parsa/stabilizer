// Test that array constants are materialized into global variables
#include <stdint.h>

int test_array_constant(int idx) {
  // Large array constant that should be materialized
  int arr[8] = {1, 2, 3, 4, 5, 6, 7, 8};
  return arr[idx];
}

