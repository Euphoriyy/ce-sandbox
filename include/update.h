#pragma once

#include <cstdint>
#include <sys/util.h>

#include "pixels.h"

void updateSand(uint8_t x, uint8_t y);
void updateWater(uint8_t x, uint8_t y);
void updateDirt(uint8_t x, uint8_t y);
void updateAcid(uint8_t x, uint8_t y);
void updateAvatarVerticalPos();
void update();
