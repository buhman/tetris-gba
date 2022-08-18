static unsigned char base10_symbols[10] = "0123456789";

static int n_digits[] = {
  [0] = 9,
  [1] = 99,
  [2] = 999,
  [3] = 9999,
  [4] = 99999,
  [5] = 999999,
  [6] = 9999999,
  [7] = 99999999,
};

static inline int get_shift(unsigned int n)
{
  for (int i = 7; i > 0; i--) {
    if (n <= n_digits[i])
      return i;
  }
  return 0;
}

static inline void
uint_to_base10(void * dst, unsigned int mask, unsigned int num, int n)
{
  int offset = n - get_shift(num);

  if (n == 0) return;

  do {
    ((unsigned short *)dst)[offset + (--n)] = mask | (unsigned short)(base10_symbols[num % 10]);
    num /= 10;
  } while (n > 0 && num != 0);
}
