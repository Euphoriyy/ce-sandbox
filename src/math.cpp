#include "../include/math.h"

uint8_t gcd(uint16_t a, uint16_t b) { return !b ? a : gcd(b, a % b); }
