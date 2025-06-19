#pragma once

#include <cstdint>

inline void swap(uint24_t &a, uint24_t &b)
{
    uint24_t temp = a;
    a = b;
    b = temp;
}

uint8_t gcd(uint24_t a, uint24_t b);

struct Vector2
{
    uint8_t x, y;
};

struct Vector2_24
{
    uint24_t x, y;
};
