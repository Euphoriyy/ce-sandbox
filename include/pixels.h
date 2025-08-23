#pragma once

#include "game.h"
#include "timing.h"

#include <graphx.h>
#include <string.h>

const uint8_t GUI_HEIGHT = 12;
const uint8_t SCALE_FACTOR = 5;
const uint8_t WIDTH = GFX_LCD_WIDTH / SCALE_FACTOR;
const uint8_t HEIGHT = (GFX_LCD_HEIGHT - GUI_HEIGHT) / SCALE_FACTOR;
const uint24_t TOTAL_PIXELS = WIDTH * HEIGHT;

struct Pixels
{
    // Pixel materials
    uint8_t pixels[TOTAL_PIXELS] = {0};
    // Specific per-pixel colors
    uint8_t colors[TOTAL_PIXELS] = {0};
    // Custom per-pixel properties, lower 4 bits for lifetime and upper 4 bits for misc.
    uint8_t props[TOTAL_PIXELS] = {0};
    uint24_t activeCount = 0;
    bool activeRows[HEIGHT] = {0};
    bool dirtyFlags[TOTAL_PIXELS] = {0};
    bool dirtyRows[HEIGHT] = {0};
    bool updatedFlags[TOTAL_PIXELS] = {0};
    uint24_t lastUpdate[TOTAL_PIXELS];
    uint24_t lastUpdateByRow[HEIGHT];
    uint24_t yOffsets[HEIGHT];
    uint8_t divByScaleFactor[GFX_LCD_WIDTH + SCALE_FACTOR];
    uint8_t bgColorCells[TOTAL_PIXELS];
};

const uint8_t LIFETIME_MASK = 0x0F;
const uint8_t MISC_MASK = 0xF0;

extern Pixels pixelData;

void makeDirty(uint8_t x, uint8_t y);

inline uint24_t IDX(uint8_t x, uint8_t y) { return pixelData.yOffsets[y] + x; }

inline bool IN_BOUNDS(uint8_t x, uint8_t y) { return x < WIDTH && y < HEIGHT; }

inline bool IN_BOUNDS(uint24_t idx) { return idx < TOTAL_PIXELS; }

inline uint8_t getPixel(uint8_t x, uint8_t y)
{
    return IN_BOUNDS(x, y) ? pixelData.pixels[IDX(x, y)] : 0;
}

inline uint8_t getPixel(uint24_t idx) { return IN_BOUNDS(idx) ? pixelData.pixels[idx] : 0; }

inline uint8_t getColor(uint8_t x, uint8_t y)
{
    return IN_BOUNDS(x, y) ? pixelData.colors[IDX(x, y)] : 0;
}

inline uint8_t getColor(uint24_t idx) { return IN_BOUNDS(idx) ? pixelData.colors[idx] : 0; }

inline uint8_t getLifetime(uint8_t x, uint8_t y)
{
    return IN_BOUNDS(x, y) ? pixelData.props[IDX(x, y)] & LIFETIME_MASK : 0;
}

inline uint8_t getLifetime(uint24_t idx)
{
    return IN_BOUNDS(idx) ? pixelData.props[idx] & LIFETIME_MASK : 0;
}

inline uint8_t getMisc(uint8_t x, uint8_t y)
{
    return IN_BOUNDS(x, y) ? (pixelData.props[IDX(x, y)] & MISC_MASK) >> 4 : 0;
}

inline uint8_t getMisc(uint24_t idx)
{
    return IN_BOUNDS(idx) ? (pixelData.props[idx] & MISC_MASK) >> 4 : 0;
}

inline void setLifetime(uint24_t idx, uint8_t lifetime)
{
    pixelData.props[idx] &= ~LIFETIME_MASK;
    pixelData.props[idx] |= lifetime & LIFETIME_MASK;
}

inline void setMisc(uint24_t idx, uint8_t misc)
{
    pixelData.props[idx] &= ~MISC_MASK;
    pixelData.props[idx] |= (misc & MISC_MASK) >> 4;
}

inline void setPixel(uint8_t x, uint8_t y, uint8_t mat, uint8_t lifetime = 0xF, uint8_t misc = 0)
{
    if (!IN_BOUNDS(x, y))
        return;

    uint24_t idx = IDX(x, y);
    uint8_t prevMat = pixelData.pixels[idx];

    if (mat == prevMat)
        return;

    // Adjust the active count
    if (!prevMat && mat)
    {
        if (++pixelData.activeCount == 1) // Increment active count if the new mat is non-zero
        {
            // If the new pixel count is 1, rerender the background
            memset(pixelData.dirtyFlags, 1, TOTAL_PIXELS);
            memset(pixelData.dirtyRows, 1, HEIGHT);
        }
        setLifetime(idx, lifetime);
        setMisc(idx, misc);
    }
    else if (prevMat && !mat)
    {
        --pixelData.activeCount; // Decrement active count if it is zero
        pixelData.colors[idx] = 0;
        pixelData.props[idx] = 0;
    }

    // Change the material and mark pixel as dirty
    pixelData.pixels[idx] = mat;
    pixelData.dirtyFlags[idx] = true;
    pixelData.dirtyRows[y] = true;

    if (mat)
    {
        pixelData.activeRows[y] = true;
    }
    else
    {
        // Check if the row is otherwise active
        bool rowActive = false;
        uint24_t rowIdx = idx - x;
#pragma unroll
        for (uint8_t j = 0; j < WIDTH; ++j)
        {
            if (j == x)
                continue;

            if (pixelData.pixels[rowIdx + j])
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
