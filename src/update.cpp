#include "../include/update.h"

void update()
{
    for (uint8_t y = HEIGHT - 1, x; y != 0xFF; --y)
    {
        // Skip updating rows that are inactive or weren't updated within the last 2 frames
        if (!pixelData.activeRows[y] || timing.frame - pixelData.lastUpdateByRow[y] >= 2)
            continue;

        bool flip = randInt(0, 1);
        uint24_t rowIdx = IDX(0, y);

        for (x = (flip ? 0 : WIDTH - 1); flip ? x < WIDTH : x != 0xFF; x += (flip ? 1 : -1))
        {
            if (!pixelData.pixels[rowIdx + x] ||
                timing.frame - pixelData.lastUpdate[rowIdx + x] >= 2)
                continue;

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
                case Material::Acid:
                    updateAcid(x, y);
                    break;
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
        uint8_t scaledY = scaledY = pixelData.divByScaleFactor[y + HALF_OF_AVATAR_HEIGHT + 4];
        uint8_t intersectingMat = pixelData.activeRows[scaledY] ? getPixel(scaledX, scaledY) : 0;
        if (intersectingMat && intersectingMat != Material::Water &&
            intersectingMat != Material::Acid)
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
    if ((!belowMat || belowMat == Material::Water || belowMat == Material::Acid) &&
        avatar.pos.y < GFX_LCD_HEIGHT - AVATAR_HEIGHT + 1)
    {
        newAvatarPos.y += 2;
        positionModified = true;
    }

    if (positionModified)
    {
        if (newAvatarPos.y - avatar.pos.y > 1)
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
