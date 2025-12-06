#include <stdexcept>

int may_throw_pos(int x) {
  if (x == 0) {
    throw std::runtime_error("zero");
  }
  return x;
}

int may_throw_neg(int x) {
  if (x < 0) {
    throw std::logic_error("neg");
  }
  return may_throw_pos(-x);
}

