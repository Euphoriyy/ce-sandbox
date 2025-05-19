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

enum Material
{
    Empty,
    Sand,
    Water,
    Dirt,
    Stone,
    Acid
};

const uint8_t shadeCount = 3;
constexpr uint8_t materialShades[][shadeCount] = {{0},         {245, 213, 0}, {26, 27, 28},
                                                  {65, 64, 0}, {148, 0, 181}, {109, 77, 110}};
const uint8_t palette[] = {Material::Sand, Material::Water, Material::Dirt, Material::Stone,
                           Material::Acid};
const uint8_t paletteLen = sizeof(palette);

extern Cursor cursor;
extern uint16_t frame;
extern uint8_t frametime;
extern KeyState keyState;
extern bool isDrawing, isErasing, isPaused, enableFloor;
extern uint8_t brushSize;
