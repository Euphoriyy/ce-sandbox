#pragma once

#include <cstdint>
#include <sys/util.h>

#include "pixels.h"

#define CUR_POS x, y

#define UP x, y - 1
#define DOWN x, y + 1
#define LEFT x - 1, y
#define RIGHT x + 1, y

#define UP_LEFT x - 1, y - 1
#define UP_RIGHT x + 1, y - 1
#define DOWN_LEFT x - 1, y + 1
#define DOWN_RIGHT x + 1, y + 1

void updateSand(uint8_t x, uint8_t y);
void updateWater(uint8_t x, uint8_t y);
void updateDirt(uint8_t x, uint8_t y);
void updateAcid(uint8_t x, uint8_t y);
void updateAvatarVerticalPos();
void update();
