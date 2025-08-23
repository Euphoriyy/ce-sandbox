#pragma once

#include "pixels.h"

#include <cstdint>

#define CUR_POS x, y

#define UP x, y - 1
#define DOWN x, y + 1
#define LEFT x - 1, y
#define RIGHT x + 1, y

#define UP_LEFT x - 1, y - 1
#define UP_RIGHT x + 1, y - 1
#define DOWN_LEFT x - 1, y + 1
#define DOWN_RIGHT x + 1, y + 1

inline void switchMat(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t mat0,
                      uint8_t mat1 = 0, uint8_t lifetime0 = 0xF, uint8_t lifetime1 = 0xF,
                      uint8_t misc0 = 0, uint8_t misc1 = 0)
{
    setPixel(x1, y1, mat0, lifetime0, misc0);
    setPixel(x0, y0, mat1, lifetime1, misc1);

    if (!mat1)
        pixelData.updatedFlags[IDX(x1, y1)] = true;
}

void updateSand(uint8_t x, uint8_t y);
void updateWater(uint8_t x, uint8_t y);
void updateDirt(uint8_t x, uint8_t y);
void updateStone(uint8_t x, uint8_t y);
void updateAcid(uint8_t x, uint8_t y);
void updateSteam(uint8_t x, uint8_t y);
void updateFire(uint8_t x, uint8_t y);
void updateAvatarVerticalPos();
void update();
