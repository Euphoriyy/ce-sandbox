#include "../include/pixels.h"

Pixels pixelData;

void makeDirty(uint8_t x, uint8_t y)
{
    if (IN_BOUNDS(x, y))
    {
        pixelData.dirtyFlags[IDX(x, y)] = true;
        pixelData.dirtyRows[y] = true;
    }
}
