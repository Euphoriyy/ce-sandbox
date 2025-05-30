#pragma once

#include <graphx.h>

#include "game.h"
#include "gfx/gfx.h"
#include "timing.h"

const uint8_t GUI_HEIGHT = 12;
const uint8_t SCALE_FACTOR = 5;
const uint8_t WIDTH = GFX_LCD_WIDTH / SCALE_FACTOR;
const uint8_t HEIGHT = (GFX_LCD_HEIGHT - GUI_HEIGHT) / SCALE_FACTOR;
const uint24_t TOTAL_PIXELS = WIDTH * HEIGHT;

struct Pixels
{
    uint8_t pixels[TOTAL_PIXELS] = {0};
    bool activeFlags[TOTAL_PIXELS] = {0};
    bool activeRows[HEIGHT] = {0};
    bool dirtyFlags[TOTAL_PIXELS] = {0};
    bool dirtyRows[HEIGHT] = {0};
    uint24_t activeCount = 0;
    uint24_t lastUpdate[TOTAL_PIXELS];
    uint24_t lastUpdateByRow[HEIGHT];
    uint24_t yOffsets[HEIGHT];
    uint8_t bgColorCells[TOTAL_PIXELS];
};

extern Pixels pixelData;

void makeDirty(uint8_t x, uint8_t y);

inline uint24_t IDX(uint8_t x, uint8_t y) { return pixelData.yOffsets[y] + x; }

inline bool IN_BOUNDS(uint8_t x, uint8_t y) { return x < WIDTH && y < HEIGHT; }

inline bool IN_BOUNDS(uint24_t idx) { return idx >= 0 && idx < TOTAL_PIXELS; }

inline uint8_t getPixel(uint8_t x, uint8_t y)
{
    return IN_BOUNDS(x, y) ? pixelData.pixels[IDX(x, y)] : 0;
}

inline uint8_t getPixel(uint24_t idx) { return IN_BOUNDS(idx) ? pixelData.pixels[idx] : 0; }

inline void setPixel(uint8_t x, uint8_t y, uint8_t mat)
{
    if (!IN_BOUNDS(x, y))
        return;

    uint24_t idx = IDX(x, y);
    uint8_t prevMat = getPixel(idx);

    if (mat == prevMat)
        return;

    // Adjust the active count
    if (!prevMat && mat)
        ++pixelData.activeCount; // Increment active count if the new mat is non-zero
    else if (prevMat && !mat)
        --pixelData.activeCount; // Decrement active count if it is zero

    makeDirty(x, y);
    pixelData.pixels[idx] = mat;

    if (mat)
    {
        pixelData.activeFlags[idx] = true;
        pixelData.activeRows[y] = true;
    }
    else
    {
        pixelData.activeFlags[idx] = false;

        // Check if the row is otherwise active
        bool rowActive = false;
        uint24_t rowIdx = IDX(0, y);
#pragma unroll
        for (uint8_t j = 0; j < WIDTH; ++j)
        {
            if (j == x)
                continue;

            if (pixelData.activeFlags[rowIdx + j])
            {
                rowActive = true;
                break;
            }
        }
        pixelData.activeRows[y] = rowActive;
    }

    // Change updated status of pixel and row
    pixelData.lastUpdate[idx] = timing.frame;
    pixelData.lastUpdateByRow[y] = timing.frame;

    // Change updated status of adjacent pixels & rows
    if (IN_BOUNDS(x - 1, y))
        pixelData.lastUpdate[idx - 1] = timing.frame;
    if (IN_BOUNDS(x + 1, y))
        pixelData.lastUpdate[idx + 1] = timing.frame;
    if (IN_BOUNDS(x, y - 1))
    {
        pixelData.lastUpdate[idx - WIDTH] = timing.frame;
        pixelData.lastUpdateByRow[y - 1] = timing.frame;
    }
    if (IN_BOUNDS(x, y + 1))
    {
        pixelData.lastUpdate[idx + WIDTH] = timing.frame;
        pixelData.lastUpdateByRow[y + 1] = timing.frame;
    }
}
