#pragma once

#include "math.h"
#include "pixels.h"

struct Keys
{
    bool enter = false, del = false, yequ = false, graph = false, second = false, zoom = false,
         window = false, trace = false;
};

struct KeyState
{
    Keys prev;
    Keys cur;
};

struct Cursor
{
    Vector2 pos;
    uint8_t color = 255;
    uint8_t size;
    uint8_t paletteIndex = 0;
};

const uint8_t SAND = 245;
const uint8_t WATER = 28;
const uint8_t STONE = 148;
const uint8_t ACID = 109;
const uint8_t palette[] = {SAND, WATER, STONE, ACID};
const uint8_t paletteLen = sizeof(palette);

extern Cursor cursor;
extern uint16_t frame;
extern uint8_t frametime;
extern KeyState keyState;
extern bool isDrawing, isErasing, isPaused, enableFloor;
extern uint8_t brushSize;
