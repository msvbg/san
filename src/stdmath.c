#include "std.h"

inline int sanstd_absi(int n) {
  if (n < 0) { return -n; }
  return n;
}

inline int sanstd_squarei(int n) {
  return n * n;
}

inline int sanstd_sqrti(int n) {
  return (int)round(sqrt((double)n));
}

inline int sanstd_factoriali(int n) {
  if (n < 3) { return n; }
  int acc = 1;
  for (; n > 1; --n) {
      acc *= n;
  }
  return acc;
}
