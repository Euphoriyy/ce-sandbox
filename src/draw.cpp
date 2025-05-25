#include "../include/draw.h"

void drawSquare(Vector2 pos, uint8_t size, uint8_t mat)
{
    uint8_t oddOffset = size & 1;
    for (int8_t dy = -size / 2; dy < size / 2 + oddOffset; ++dy)
    {
        for (int8_t dx = -size / 2; dx < size / 2 + oddOffset; ++dx)
        {
            uint8_t nx = pos.x + dx, ny = pos.y + dy;
            if (IN_BOUNDS(nx, ny) && !getPixel(nx, ny))
                setPixel(nx, ny, mat);
        }
    }
}

void drawCircle(Vector2 pos, uint8_t size, uint8_t mat)
{
    uint8_t radius = size / 2;
    uint8_t oddOffset = size & 1;
    for (int8_t dy = -radius; dy <= radius + oddOffset; ++dy)
    {
        for (int8_t dx = -radius; dx <= radius + oddOffset; ++dx)
        {
            // Check if (dx, dy) is within the circle
            if (dx * dx + dy * dy <= radius * (radius + oddOffset))
            {
                uint8_t nx = pos.x + dx, ny = pos.y + dy;
                if (IN_BOUNDS(nx, ny) && !getPixel(nx, ny))
                {
                    setPixel(nx, ny, mat);
                }
            }
        }
    }
}
