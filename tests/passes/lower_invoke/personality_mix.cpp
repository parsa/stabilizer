#include <stdexcept>

extern "C" int personality_mix_fn(int x) {
  if (x == 0) {
    throw std::runtime_error("zero");
  }
  if (x == 1) {
    throw std::logic_error("one");
  }
  return x;
}

