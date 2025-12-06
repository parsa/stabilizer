#include <stdexcept>

int cleanup_example(int x) {
  int result = x;
  try {
    if (x < 0) {
      throw std::runtime_error("neg");
    }
    return x;
  } catch (...) {
    result = -x;
    throw;
  }
}

