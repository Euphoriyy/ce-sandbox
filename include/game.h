#pragma once

#include "math.h"
#include "pixels.h"

struct KeyState
{
    struct Keys
    {
        bool enter = false, del = false, yequ = false, graph = false, second = false, zoom = false,
             window = false, trace = false, graphVar = false, apps = false, sto = false, on = false,
             ln = false, decPnt = false, power = false;
    };
    Keys prev, cur;
};

constexpr uint8_t MAX_SELECTED_POINTS = 2;

struct Cursor
{
    enum Sprite
    {
        Brush,
        Eraser,
        Hand,
        Pinching,
        SpriteCount
    };
    enum SpriteSize
    {
        Normal,
        Large,
        SizeCount
    };
    Vector2 prevPos, pos;
    uint8_t color = 255;
    uint8_t size;
    uint8_t paletteIndex = 0;
    bool spriteCursor = false, largeSprite = false, pinching = false, movedThisFrame = false;
    uint24_t pinchedPixel;
    Vector2 selectedPoints[MAX_SELECTED_POINTS];
    gfx_rletsprite_t *sprites[SpriteCount][SizeCount];
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
        Right,
        Left,
        OrientationCount
    };
    enum Sprite
    {
        Standing,
        Walking,
        Jumping,
        Hanging,
        SpriteCount
    };
    static Vector2_24 defaultPos;
    Vector2_24 pos = defaultPos;
    Orientation orientation = Orientation::Right;
    bool spawned = false;
    Sprite spriteState = Sprite::Standing;
    uint8_t speed = 2;
    uint24_t frameAtLastJump = 0;
    gfx_rletsprite_t *sprites[SpriteCount][OrientationCount];
};

enum MatterState
{
    Solid,
    Liquid,
    Gas
};

struct MaterialProperties
{
    const char name[7];
    MatterState stateOfMatter;
    bool isFlammable = false;
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
    Fire,
    MaterialCount
};

const MaterialProperties materialProperties[MaterialCount] = {
    {"AIR", Gas, false},    {"SAND", Solid, false},  {"WATER", Liquid, false},
    {"DIRT", Solid, false}, {"STONE", Solid, false}, {"ACID", Liquid, false},
    {"STEAM", Gas, false},  {"WOOD", Solid, true},   {"FIRE", Gas, false}};

const uint8_t SHADE_COUNT = 3;
constexpr uint8_t materialShades[MaterialCount][SHADE_COUNT] = {
    {0, 1},       {2, 1, 0},   {5, 4, 3},   {6, 7, 8},   {9, 0, 10},
    {11, 12, 13}, {14, 15, 0}, {16, 17, 0}, {18, 19, 20}};

const uint8_t palette[] = {Material::Sand, Material::Water, Material::Dirt,  Material::Stone,
                           Material::Wood, Material::Acid,  Material::Steam, Material::Fire};
const uint8_t PALETTE_LEN = sizeof(palette);

inline bool isSolid(uint8_t mat) { return materialProperties[mat].stateOfMatter == Solid; }

extern Cursor cursor;
extern KeyState keyState;
extern GameState gameState;
extern Avatar avatar;

void initCursorSprites();
void initAvatarSprites();
void resetAvatar();
void clearCursor();
void clearAvatar();
bool avatarCanMoveHorizontally(int8_t offset);
bool avatarIsGrounded();
void initSelectedPoints();
void clearPointOutline(Vector2 pos);
bool cursorIntersectingAvatar();

inline bool pointIsSelected(uint8_t index)
{
    return cursor.selectedPoints[index].x != 0xFF && cursor.selectedPoints[index].y != 0xFF;
}
