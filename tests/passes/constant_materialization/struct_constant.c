// Test that struct constants are materialized into global variables
#include <stdint.h>

struct Point {
  int x;
  int y;
  int z;
};

int test_struct_constant(int field) {
  // Struct constant that should be materialized
  struct Point p = {10, 20, 30};
  if (field == 0) return p.x;
  if (field == 1) return p.y;
  return p.z;
}

