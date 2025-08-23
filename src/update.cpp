#include "../include/update.h"

#include <sys/util.h>

static inline void innerUpdateLoop(uint24_t rowIdx, uint8_t x, uint8_t y)
{
    uint24_t idx = rowIdx + x;

    if (!pixelData.pixels[idx] || pixelData.updatedFlags[idx] ||
        timing.frame - pixelData.lastUpdate[idx] >= 2 || idx == cursor.pinchedPixel)
        return;

    uint8_t mat = getPixel(x, y);
    switch (mat)
    {
        case Material::Sand:
            updateSand(x, y);
            break;
        case Material::Water:
            updateWater(x, y);
            break;
        case Material::Dirt:
            updateDirt(x, y);
            break;
        case Material::Stone:
            updateStone(x, y);
            break;
        case Material::Acid:
            updateAcid(x, y);
            break;
        case Material::Steam:
            updateSteam(x, y);
            break;
        case Material::Fire:
            updateFire(x, y);
            break;
    }
}

void update()
{
    for (uint8_t y = HEIGHT - 1; y != 0xFF; --y)
    {
        // Skip updating rows that are inactive or weren't updated within the last 2 frames
        if (!pixelData.activeRows[y] || timing.frame - pixelData.lastUpdateByRow[y] >= 2)
            continue;

        bool flip = randInt(0, 1);
        uint24_t rowIdx = IDX(0, y);

        if (flip)
        {
            for (uint8_t x = 0; x < WIDTH; ++x)
            {
                innerUpdateLoop(rowIdx, x, y);
            }
        }
        else
        {
            for (uint8_t x = WIDTH - 1; x != 0xFF; --x)
            {
                innerUpdateLoop(rowIdx, x, y);
            }
        }
    }

    if (avatar.spawned)
        updateAvatarVerticalPos();
}

void updateAvatarVerticalPos()
{
    bool positionModified = false;
    Vector2_24 newAvatarPos = avatar.pos;

    uint8_t scaledX = pixelData.divByScaleFactor[avatar.pos.x + HALF_OF_AVATAR_WIDTH];

    // Lift avatar if intersecting a solid material
    uint8_t nonintersectingCount = 0;
    for (uint8_t y = avatar.pos.y; y > GUI_HEIGHT; --y)
    {
        uint8_t scaledY = pixelData.divByScaleFactor[y + HALF_OF_AVATAR_HEIGHT + 4];
        uint8_t intersectingMat = pixelData.activeRows[scaledY] ? getPixel(scaledX, scaledY) : 0;
        if (intersectingMat && isSolid(intersectingMat))
        {
            --newAvatarPos.y;
            positionModified = true;
        }
        else if (nonintersectingCount++ >= AVATAR_HEIGHT)
        {
            break;
        }
    }

    uint8_t belowMat =
        getPixel(scaledX, pixelData.divByScaleFactor[avatar.pos.y + HALF_OF_AVATAR_HEIGHT]);

    // Fall avatar if there is no solid material directly below
    if ((!belowMat || !isSolid(belowMat)) && avatar.pos.y < GFX_LCD_HEIGHT - AVATAR_HEIGHT + 1)
    {
        newAvatarPos.y += 2;
        positionModified = true;
    }

    if (positionModified)
    {
        if (newAvatarPos.y - avatar.pos.y != 0)
            clearAvatar();
        avatar.pos = newAvatarPos;
    }
}

void updateSand(uint8_t x, uint8_t y)
{
    if (gameState.enableFloor && y == HEIGHT - 1)
        return;

    if (!getPixel(DOWN))
    {
        switchMat(CUR_POS, DOWN, Material::Sand);
    }
    else if (!getPixel(DOWN_LEFT) && x > 0)
    {
        switchMat(CUR_POS, DOWN_LEFT, Material::Sand);
    }
    else if (!getPixel(DOWN_RIGHT) && x + 1 < WIDTH)
    {
        switchMat(CUR_POS, DOWN_RIGHT, Material::Sand);
    }
    else if (getPixel(DOWN) == Material::Water)
    {
        switchMat(CUR_POS, DOWN, Material::Sand, Material::Water);
    }
}

void updateWater(uint8_t x, uint8_t y)
{
    if (gameState.enableFloor && y == HEIGHT - 1)
        return;

    if (!getPixel(DOWN))
    {
        switchMat(CUR_POS, DOWN, Material::Water);
    }
    else if (!getPixel(DOWN_LEFT) && x > 0)
    {
        switchMat(CUR_POS, DOWN_LEFT, Material::Water);
    }
    else if (!getPixel(DOWN_RIGHT) && x + 1 < WIDTH)
    {
        switchMat(CUR_POS, DOWN_RIGHT, Material::Water);
    }
    else if (!getPixel(LEFT) && x > 0)
    {
        switchMat(CUR_POS, LEFT, Material::Water);
    }
    else if (!getPixel(RIGHT) && x + 1 < WIDTH)
    {
        switchMat(CUR_POS, RIGHT, Material::Water);
    }
}

void updateDirt(uint8_t x, uint8_t y)
{
    if (gameState.enableFloor && y == HEIGHT - 1)
        return;

    if (!getPixel(DOWN))
    {
        switchMat(CUR_POS, DOWN, Material::Dirt);
    }
    else if (!getPixel(DOWN_LEFT) && x > 0)
    {
        switchMat(CUR_POS, DOWN_LEFT, Material::Dirt);
    }
    else if (!getPixel(DOWN_RIGHT) && x + 1 < WIDTH)
    {
        switchMat(CUR_POS, DOWN_RIGHT, Material::Dirt);
    }
    else if (getPixel(DOWN) == Material::Water)
    {
        switchMat(CUR_POS, DOWN, Material::Dirt, Material::Water);
    }
}

void updateStone(uint8_t x, uint8_t y)
{
    if (gameState.enableFloor && y == HEIGHT - 1)
        return;

    if (!getPixel(DOWN))
    {
        switchMat(CUR_POS, DOWN, Material::Stone);
    }
    else if (!getPixel(DOWN_LEFT) && x > 0)
    {
        switchMat(CUR_POS, DOWN_LEFT, Material::Stone);
    }
    else if (!getPixel(DOWN_RIGHT) && x + 1 < WIDTH)
    {
        switchMat(CUR_POS, DOWN_RIGHT, Material::Stone);
    }
    else if (getPixel(DOWN) == Material::Water)
    {
        switchMat(CUR_POS, DOWN, Material::Stone, Material::Water);
    }
}

void updateAcid(uint8_t x, uint8_t y)
{
    if (gameState.enableFloor && y == HEIGHT - 1)
        return;

    if (!getPixel(DOWN))
    {
        switchMat(CUR_POS, DOWN, Material::Acid);
    }
    else if (!getPixel(DOWN_LEFT) && x > 0)
    {
        switchMat(CUR_POS, DOWN_LEFT, Material::Acid);
    }
    else if (!getPixel(DOWN_RIGHT) && x + 1 < WIDTH)
    {
        switchMat(CUR_POS, DOWN_RIGHT, Material::Acid);
    }
    else if (!getPixel(LEFT) && x > 0)
    {
        switchMat(CUR_POS, LEFT, Material::Acid);
    }
    else if (!getPixel(RIGHT) && x + 1 < WIDTH)
    {
        switchMat(CUR_POS, RIGHT, Material::Acid);
    }

    // Corrode nearby materials
    if (randInt(0, 9) == 0)
    {
        if (getPixel(UP) && getPixel(UP) != Material::Acid)
        {
            switchMat(CUR_POS, UP, Material::Acid);
        }
        else if (getPixel(DOWN) && getPixel(DOWN) != Material::Acid)
        {
            switchMat(CUR_POS, DOWN, Material::Acid);
        }
        else if (getPixel(DOWN_LEFT) && getPixel(DOWN_LEFT) != Material::Acid && x > 0)
        {
            switchMat(CUR_POS, DOWN_LEFT, Material::Acid);
        }
        else if (getPixel(DOWN_RIGHT) && getPixel(DOWN_RIGHT) != Material::Acid && x + 1 < WIDTH)
        {
            switchMat(CUR_POS, DOWN_RIGHT, Material::Acid);
        }
        else if (getPixel(LEFT) && getPixel(LEFT) != Material::Acid)
        {
            switchMat(CUR_POS, LEFT, Material::Acid);
        }
        else if (getPixel(RIGHT) && getPixel(RIGHT) != Material::Acid)
        {
            switchMat(CUR_POS, RIGHT, Material::Acid);
        }
    }

    // Dissipate
    if (randInt(0, 24) == 0)
        setPixel(CUR_POS, 0);
}

void updateSteam(uint8_t x, uint8_t y)
{
    if (y == 0)
        return;

    uint8_t above = getPixel(UP);

    if (!above)
    {
        switchMat(CUR_POS, UP, Material::Steam);
        return;
    }

    if (x > 0 && !getPixel(UP_LEFT))
    {
        switchMat(CUR_POS, UP_LEFT, Material::Steam);
        return;
    }

    if (x + 1 < WIDTH && !getPixel(UP_RIGHT))
    {
        switchMat(CUR_POS, UP_RIGHT, Material::Steam);
        return;
    }

    if (above == Material::Water)
    {
        switchMat(CUR_POS, UP, Material::Steam, above);
        return;
    }

    if (getPixel(DOWN) == Material::Water)
    {
        setPixel(DOWN, Material::Steam);
        return;
    }

    bool leftFirst = y & 1;

    if (leftFirst)
    {
        if (x > 0 && !getPixel(LEFT))
        {
            switchMat(CUR_POS, LEFT, Material::Steam);
            return;
        }
        else if (x + 1 < WIDTH && !getPixel(RIGHT))
        {
            switchMat(CUR_POS, RIGHT, Material::Steam);
            return;
        }
    }
    else
    {
        if (x + 1 < WIDTH && !getPixel(RIGHT))
        {
            switchMat(CUR_POS, RIGHT, Material::Steam);
            return;
        }
        else if (x > 0 && !getPixel(LEFT))
        {
            switchMat(CUR_POS, LEFT, Material::Steam);
            return;
        }
    }
}

void updateFire(uint8_t x, uint8_t y)
{
    if (y == 0)
        setPixel(CUR_POS, Material::Empty);

    uint8_t lifetime = getLifetime(CUR_POS);

    // Burned out
    if (lifetime == 0)
    {
        setPixel(CUR_POS, Material::Empty);
        return;
    }

    // Update lifetime
    --lifetime;

    // Try to rise
    if (!getPixel(UP))
    {
        switchMat(CUR_POS, UP, Material::Fire, 0, lifetime);
        return;
    }
    if (x > 0 && !getPixel(UP_LEFT))
    {
        switchMat(CUR_POS, UP_LEFT, Material::Fire, 0, lifetime);
        return;
    }
    if (x + 1 < WIDTH && !getPixel(UP_RIGHT))
    {
        switchMat(CUR_POS, UP_RIGHT, Material::Fire, 0, lifetime);
        return;
    }

    // Try to spread sideways
    bool leftFirst = y & 1;

    if (leftFirst)
    {
        if (x > 0 && !getPixel(LEFT))
        {
            switchMat(CUR_POS, LEFT, Material::Fire, 0, lifetime);
            return;
        }
        else if (x + 1 < WIDTH && !getPixel(RIGHT))
        {
            switchMat(CUR_POS, RIGHT, Material::Fire, 0, lifetime);
            return;
        }
    }
    else
    {
        if (x + 1 < WIDTH && !getPixel(RIGHT))
        {
            switchMat(CUR_POS, RIGHT, Material::Fire, 0, lifetime);
            return;
        }
        else if (x > 0 && !getPixel(LEFT))
        {
            switchMat(CUR_POS, LEFT, Material::Fire, 0, lifetime);
            return;
        }
    }

    // Ignite flammable neighbors
    static const int8_t offsets[4][2] = {{0, -1}, {-1, 0}, {1, 0}, {0, 1}};
    for (uint8_t i = 0; i < 4; i++)
    {
        int nx = x + offsets[i][0];
        int ny = y + offsets[i][1];
        if (nx < 0 || nx >= WIDTH || ny < 0 || ny >= HEIGHT)
            continue;

        uint8_t neighbor = getPixel(nx, ny);
        if (materialProperties[neighbor].isFlammable)
        {
            setPixel(nx, ny, Material::Fire, randInt(3, 10)); // Random lifetime 3-10
        }
        else if (neighbor == Material::Water)
        {
            setPixel(nx, ny, Material::Steam);
        }
    }
}
