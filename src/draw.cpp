#include "../include/draw.h"

void drawSquare(Vector2 pos, uint8_t size, uint8_t mat)
{
    for (int8_t dy = -size / 2; dy < size / 2 + (size & 1); ++dy)
    {
        for (int8_t dx = -size / 2; dx < size / 2 + (size & 1); ++dx)
        {
            uint8_t nx = pos.x + dx, ny = pos.y + dy;
            if (IN_BOUNDS(nx, ny) && !getPixel(nx, ny))
                setPixel(nx, ny, mat);
        }
    }
}
