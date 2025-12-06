// Test that inline array constants used directly in instructions are materialized
#include <stdint.h>

int test_inline_array(int idx) {
  // This should create a literal constant array that's embedded in the instruction
  // We'll use it in a way that forces it to be a constant expression
  static const int arr[8] = {1, 2, 3, 4, 5, 6, 7, 8};
  return arr[idx];
}

// Test with a function that uses a constant array directly in a comparison
int test_array_in_expr(int val) {
  static const int lookup[4] = {10, 20, 30, 40};
  // Force the array to be used as a constant expression
  if (val < 4) {
    return lookup[val];
  }
  return 0;
}

