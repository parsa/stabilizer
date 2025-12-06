#include <stdexcept>

int nested_throw(int x) {
  try {
    if (x < 0) {
      try {
        throw std::runtime_error("neg");
      } catch (...) {
        throw;
      }
    }
    return x;
  } catch (const std::exception&) {
    return -5;
  }
}

