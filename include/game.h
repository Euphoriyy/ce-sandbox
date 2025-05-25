#pragma once

#include "math.h"
#include "pixels.h"

struct KeyState
{
    struct Keys
    {
        bool enter = false, del = false, yequ = false, graph = false, second = false, zoom = false,
             window = false, trace = false, graphVar = false;
    };
    Keys prev, cur;
};

struct Cursor
{
    Vector2 pos;
    uint8_t color = 255;
    uint8_t size;
    uint8_t paletteIndex = 0;
};

struct Timing
{
    uint16_t frame = 0;
    uint8_t frametime = 0;
};

struct GameState
{
    bool isDrawing = false, isErasing = false, isPaused = false, enableFloor = true,
         circleBrush = true;
    uint8_t brushSize = 1;
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
constexpr uint8_t materialShades[][shadeCount] = {{0, 1},     {245, 213, 0}, {26, 27, 28},
                                                  {64, 0, 0}, {148, 0, 181}, {109, 77, 110}};
const uint8_t palette[] = {Material::Sand, Material::Water, Material::Dirt, Material::Stone,
                           Material::Acid};
const uint8_t paletteLen = sizeof(palette);

extern Cursor cursor;
extern Timing timing;
extern KeyState keyState;
extern GameState gameState;

void clearCursor();
