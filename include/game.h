#pragma once

#include "math.h"
#include "pixels.h"

struct KeyState
{
    struct Keys
    {
        bool enter = false, del = false, yequ = false, graph = false, second = false, zoom = false,
             window = false, trace = false, graphVar = false, apps = false, sto = false, on = false;
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

struct GameState
{
    bool isDrawing = false, isErasing = false, isPaused = false, enableFloor = true,
         circleBrush = true;
    uint8_t brushSize = 1;
};

const uint8_t AVATAR_WIDTH = 16, AVATAR_HEIGHT = 32;
constexpr uint8_t HALF_OF_AVATAR_WIDTH = AVATAR_WIDTH / 2,
                  HALF_OF_AVATAR_HEIGHT = AVATAR_HEIGHT / 2;

struct Avatar
{
    enum Orientation
    {
        Left,
        Right
    };
    static Vector2_24 defaultPos;
    Vector2_24 pos = defaultPos;
    Orientation orientation = Avatar::Orientation::Right;
    bool spawned = false, switchSprite = false;
    uint8_t speed = 2;
    uint24_t frameAtLastJump = 0;
    gfx_rletsprite_t *sprite0, *flippedSprite0, *sprite1, *flippedSprite1;
};

enum Material
{
    Empty,
    Sand,
    Water,
    Dirt,
    Stone,
    Acid,
    Steam,
    Wood,
};

const uint8_t shadeCount = 3;
constexpr uint8_t materialShades[][shadeCount] = {{0, 1},     {2, 1, 0},    {5, 4, 3},   {6, 7, 8},
                                                  {9, 0, 10}, {11, 12, 13}, {14, 15, 0}, {16, 17}};
const uint8_t palette[] = {Material::Sand, Material::Water, Material::Dirt, Material::Stone,
                           Material::Wood, Material::Acid,  Material::Steam};
const uint8_t paletteLen = sizeof(palette);

inline bool isSolid(uint8_t mat)
{
    return mat != Material::Water && mat != Material::Acid && mat != Material::Steam;
}

extern Cursor cursor;
extern KeyState keyState;
extern GameState gameState;
extern Avatar avatar;

void initAvatarSprites();
void resetAvatar();
void clearCursor();
void clearAvatar();
bool avatarCanMoveHorizontally(int8_t offset);
bool avatarIsGrounded();
