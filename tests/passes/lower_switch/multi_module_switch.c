int first_switch(int v) {
  switch (v) {
    case 1: return 11;
    case 2: return 22;
    default: return -2;
  }
}

static int helper(int v) {
  switch (v & 7) {
    case 0: return 100;
    case 6: return 600;
    default: return 999;
  }
}

int second_switch(int v) {
  return helper(v) + ((v > 0) ? first_switch(v) : 0);
}

