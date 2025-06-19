#include "../include/math.h"

uint8_t gcd(uint24_t a, uint24_t b)
{
    while (b)
    {
        a %= b;
        swap(a, b);
    }
    return a;
}
