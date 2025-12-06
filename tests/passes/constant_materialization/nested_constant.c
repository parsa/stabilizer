// Test that nested constant expressions are materialized
#include <stdint.h>

struct Nested {
  int arr[4];
  int val;
};

int test_nested_constant(int idx) {
  // Nested struct with array that should be materialized
  struct Nested n = {{1, 2, 3, 4}, 42};
  if (idx < 4) {
    return n.arr[idx];
  }
  return n.val;
}

