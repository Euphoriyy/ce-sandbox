#pragma once

#include <graphx.h>

#include "game.h"
#include "gfx/gfx.h"

const uint8_t GUI_HEIGHT = 12;
const uint8_t SCALE_FACTOR = 5;
const uint8_t WIDTH = GFX_LCD_WIDTH / SCALE_FACTOR;
const uint8_t HEIGHT = (GFX_LCD_HEIGHT - GUI_HEIGHT) / SCALE_FACTOR;
const uint16_t TOTAL_PIXELS = WIDTH * HEIGHT;

struct Pixels
{
    uint8_t pixels[TOTAL_PIXELS] = {0};
    bool activeFlags[TOTAL_PIXELS] = {0};
    bool activeRows[HEIGHT] = {0};
    bool dirtyFlags[TOTAL_PIXELS] = {0};
    bool dirtyRows[HEIGHT] = {0};
    uint16_t activeCount = 0;
    uint16_t lastUpdate[TOTAL_PIXELS];
    uint16_t yOffsets[HEIGHT];
    uint8_t bgColorCells[TOTAL_PIXELS];
};

extern Pixels pixelData;

#define IDX(x, y) (pixelData.yOffsets[y] + (x))
#define IN_BOUNDS(x, y) ((x) < WIDTH && (y) < HEIGHT)

uint8_t getPixel(uint8_t x, uint8_t y);
void makeDirty(uint8_t x, uint8_t y);
void setPixel(uint8_t x, uint8_t y, uint8_t color);
