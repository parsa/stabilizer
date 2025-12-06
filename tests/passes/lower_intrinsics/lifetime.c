int consume(int value) {
  int buffer[8];
  buffer[0] = value;
  for (int i = 1; i < 8; ++i) {
    buffer[i] = buffer[i - 1] + value;
  }
  return buffer[7];
}

