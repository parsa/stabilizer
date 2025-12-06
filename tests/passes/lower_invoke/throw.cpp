#include <stdexcept>

int may_throw(int x) {
  if (x < 0) {
    throw std::runtime_error("negative");
  }
  return x;
}

