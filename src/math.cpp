#include "../include/math.h"

uint8_t gcd(uint24_t a, uint24_t b) { return !b ? a : gcd(b, a % b); }
