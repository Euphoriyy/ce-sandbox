#include "../include/pixels.h"

Pixels pixelData;

uint8_t getPixel(uint8_t x, uint8_t y) { return IN_BOUNDS(x, y) ? pixelData.pixels[IDX(x, y)] : 0; }

void setPixel(uint8_t x, uint8_t y, uint8_t mat)
{
    if (IN_BOUNDS(x, y))
    {
        uint8_t prevMat = getPixel(x, y);
        if (mat != prevMat)
        {
            // If the mat is being set to something else, adjust the active count
            if (prevMat == 0 && mat != 0)
                ++pixelData.activeCount; // Increment active count if the new mat is non-zero
            else if (prevMat != 0 && mat == 0)
                --pixelData.activeCount; // Decrement active count if it is zero
            makeDirty(x, y);
            pixelData.pixels[IDX(x, y)] = mat;
        }

        if (mat != 0)
        {
            pixelData.activeFlags[IDX(x, y)] = true;
            pixelData.activeRows[y] = true;
        }
        else
        {
            pixelData.activeFlags[IDX(x, y)] = false;
            bool rowActive = false;
            for (int j = 0; j < WIDTH; ++j)
            {
                if (pixelData.activeFlags[IDX(j, y)])
                    rowActive = true;
            }
            pixelData.activeRows[y] = rowActive;
        }
        // When a pixel is updated, change the updated status of itself and its adjacent pixels
        for (int8_t dy = -1; dy <= 1; ++dy)
        {
            for (int8_t dx = -1; dx <= 1; ++dx)
            {
                uint8_t nx = x + dx, ny = y + dy;
                if (IN_BOUNDS(nx, ny))
                    pixelData.lastUpdate[IDX(nx, ny)] = timing.frame;
            }
        }
    }
}

void makeDirty(uint8_t x, uint8_t y)
{
    if (IN_BOUNDS(x, y))
    {
        pixelData.dirtyFlags[IDX(x, y)] = true;
        pixelData.dirtyRows[y] = true;
    }
}
