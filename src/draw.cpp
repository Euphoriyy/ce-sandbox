#include "../include/draw.h"

void drawSquare(Vector2 pos, uint8_t size, uint8_t mat)
{
    uint8_t oddOffset = size & 1;
    for (int8_t dy = -size / 2; dy < size / 2 + oddOffset; ++dy)
    {
        for (int8_t dx = -size / 2; dx < size / 2 + oddOffset; ++dx)
        {
            uint8_t nx = pos.x + dx, ny = pos.y + dy;
            // Depending on whether the material is not empty, check if the pixel is vacant or not
            if (IN_BOUNDS(nx, ny) && (mat ? !getPixel(nx, ny) : getPixel(nx, ny)))
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
                // Depending on whether the material is not empty, check if the pixel is vacant or
                // not
                if (IN_BOUNDS(nx, ny) && (mat ? !getPixel(nx, ny) : getPixel(nx, ny)))
                {
                    setPixel(nx, ny, mat);
                }
            }
        }
    }
}

// Bresenham's Line Algorithm
void drawLine(Vector2 pos1, Vector2 pos2, uint8_t thickness, uint8_t mat) {
    int8_t dx = abs(pos2.x - pos1.x);
    int8_t sx = pos1.x < pos2.x ? 1 : -1;
    int8_t dy = -abs(pos2.y - pos1.y);
    int8_t sy = pos1.y < pos2.y ? 1 : -1;
    int24_t error = dx + dy;

    while (true) {
        uint8_t oddOffset = thickness & 1;
        for (int8_t dy = -thickness / 2; dy < thickness / 2 + oddOffset; ++dy)
        {
            for (int8_t dx = -thickness / 2; dx < thickness / 2 + oddOffset; ++dx)
            {
                setPixel(pos1.x + dx, pos1.y + dy, mat);
            }
        }
        int24_t e2 = error * 2;
        if (e2 >= dy) {
            if (pos1.x == pos2.x)
                break;
            error += dy;
            pos1.x += sx;
        }
        if (e2 <= dx) {
            if (pos1.y == pos2.y)
                break;
            error += dx;
            pos1.y += sy;
        }
    }
}
