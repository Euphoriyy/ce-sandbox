#pragma once

#include <graphx.h>

#include "game.h"

extern const uint8_t GUI_HEIGHT;
extern const uint8_t SCALE_FACTOR;
extern const uint8_t WIDTH;
extern const uint8_t HEIGHT;
extern const uint16_t TOTAL_PIXELS;

extern uint8_t pixels[];
extern bool activeFlags[];
extern bool activeRows[];
extern uint16_t activeCount;
extern uint16_t lastUpdate[];
extern uint16_t yOffsets[];

#define IDX(x, y) (yOffsets[y] + (x))
#define IN_BOUNDS(x, y) ((x) < WIDTH && (y) < HEIGHT)

uint8_t getPixel(uint8_t x, uint8_t y);
void setPixel(uint8_t x, uint8_t y, uint8_t color);
