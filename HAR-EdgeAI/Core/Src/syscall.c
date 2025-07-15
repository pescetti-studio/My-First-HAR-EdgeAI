int _write(int file, char *ptr, int len) {
  // Redirect to UART, or just return len to ignore
  return len;
}

int _read(int file, char *ptr, int len) {
  return 0;
}

int _close(int file) {
  return -1;
}

int _fstat(int file, void *st) {
  return 0;
}

int _isatty(int file) {
  return 1;
}

int _lseek(int file, int ptr, int dir) {
  return 0;
}
