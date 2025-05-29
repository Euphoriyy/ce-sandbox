#include "../include/update.h"

void update()
{
    for (uint8_t y = HEIGHT - 1, x; y != 0xFF; --y)
    {
        // Skip updating rows that are inactive or weren't updated within the last 5 frames
        if (!pixelData.activeRows[y] || timing.frame - pixelData.lastUpdateByRow[y] >= 5)
            continue;

        bool flip = randInt(0, 1);
        for (x = (flip ? 0 : WIDTH - 1); flip ? x < WIDTH : x != 0xFF; x += (flip ? 1 : -1))
        {
            if (!pixelData.activeFlags[IDX(x, y)])
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

    // Lift avatar if intersecting a solid material
    for (uint8_t y = avatar.pos.y; y > GUI_HEIGHT; --y)
    {
        uint8_t intersectingMat = getPixel((avatar.pos.x + avatarWidth / 2) / SCALE_FACTOR,
                                           (y + avatarHeight / 2 + 4) / SCALE_FACTOR);
        if (intersectingMat && intersectingMat != Material::Water &&
            intersectingMat != Material::Acid)
        {
            --avatar.pos.y;
            positionModified = true;
        }
    }

    uint8_t belowMat = getPixel((avatar.pos.x + avatarWidth / 2) / SCALE_FACTOR,
                                (avatar.pos.y + avatarHeight / 2) / SCALE_FACTOR);

    // Fall avatar if there is no solid material directly below
    if ((!belowMat || belowMat == Material::Water || belowMat == Material::Acid) &&
        avatar.pos.y < GFX_LCD_HEIGHT - avatarHeight + 1)
    {
        avatar.pos.y += 2;
        positionModified = true;
    }

    if (positionModified)
        clearAvatar();
}

void updateSand(uint8_t x, uint8_t y)
{
    if (gameState.enableFloor && y == HEIGHT - 1)
        return;
    // If down is empty
    if (!getPixel(DOWN))
    {
        setPixel(CUR_POS, 0);
        setPixel(DOWN, Material::Sand);
    }
    else if (!getPixel(DOWN_LEFT) && x > 0)
    {
        setPixel(CUR_POS, 0);
        setPixel(DOWN_LEFT, Material::Sand);
    }
    else if (!getPixel(DOWN_RIGHT) && x + 1 < WIDTH)
    {
        setPixel(CUR_POS, 0);
        setPixel(DOWN_RIGHT, Material::Sand);
    }
    else if (getPixel(DOWN) == Material::Water)
    {
        setPixel(CUR_POS, Material::Water);
        setPixel(DOWN, Material::Sand);
    }
}

void updateWater(uint8_t x, uint8_t y)
{
    if (gameState.enableFloor && y == HEIGHT - 1)
        return;
    // If down is empty
    if (!getPixel(DOWN))
    {
        setPixel(CUR_POS, 0);
        setPixel(DOWN, Material::Water);
    }
    else if (x > 0 && !getPixel(DOWN_LEFT))
    {
        setPixel(CUR_POS, 0);
        setPixel(DOWN_LEFT, Material::Water);
    }
    else if (x + 1 < WIDTH && !getPixel(DOWN_RIGHT))
    {
        setPixel(CUR_POS, 0);
        setPixel(DOWN_RIGHT, Material::Water);
    }
    else if (x > 0 && !getPixel(LEFT))
    {
        setPixel(CUR_POS, 0);
        setPixel(LEFT, Material::Water);
    }
    else if (x + 1 < WIDTH && !getPixel(RIGHT))
    {
        setPixel(CUR_POS, 0);
        setPixel(RIGHT, Material::Water);
    }
}

void updateDirt(uint8_t x, uint8_t y)
{
    if (gameState.enableFloor && y == HEIGHT - 1)
        return;
    // If down is empty
    if (!getPixel(DOWN))
    {
        setPixel(CUR_POS, 0);
        setPixel(DOWN, Material::Dirt);
    }
    else if (!getPixel(DOWN_LEFT) && x > 0)
    {
        setPixel(CUR_POS, 0);
        setPixel(DOWN_LEFT, Material::Dirt);
    }
    else if (!getPixel(DOWN_RIGHT) && x + 1 < WIDTH)
    {
        setPixel(CUR_POS, 0);
        setPixel(DOWN_RIGHT, Material::Dirt);
    }
    else if (getPixel(DOWN) == Material::Water)
    {
        setPixel(x, y, Material::Water);
        setPixel(DOWN, Material::Dirt);
    }
}

void updateAcid(uint8_t x, uint8_t y)
{
    if (gameState.enableFloor && y == HEIGHT - 1)
        return;
    // If down is empty
    if (!getPixel(DOWN))
    {
        setPixel(CUR_POS, 0);
        setPixel(DOWN, Material::Acid);
    }
    else if (x > 0 && !getPixel(DOWN_LEFT))
    {
        setPixel(CUR_POS, 0);
        setPixel(DOWN_LEFT, Material::Acid);
    }
    else if (x + 1 < WIDTH && !getPixel(DOWN_RIGHT))
    {
        setPixel(CUR_POS, 0);
        setPixel(DOWN_RIGHT, Material::Acid);
    }
    else if (x > 0 && !getPixel(LEFT))
    {
        setPixel(CUR_POS, 0);
        setPixel(LEFT, Material::Acid);
    }
    else if (x + 1 < WIDTH && !getPixel(RIGHT))
    {
        setPixel(CUR_POS, 0);
        setPixel(RIGHT, Material::Acid);
    }

    // Corrode nearby materials
    if (randInt(0, 9) == 0)
    {
        if (getPixel(UP) && getPixel(UP) != Material::Acid)
        {
            setPixel(CUR_POS, 0);
            setPixel(UP, Material::Acid);
        }
        else if (getPixel(DOWN) && getPixel(DOWN) != Material::Acid)
        {
            setPixel(CUR_POS, 0);
            setPixel(DOWN, Material::Acid);
        }
        else if (getPixel(DOWN_LEFT) && getPixel(DOWN_LEFT) != Material::Acid && x > 0)
        {
            setPixel(CUR_POS, 0);
            setPixel(DOWN_LEFT, Material::Acid);
        }
        else if (getPixel(DOWN_RIGHT) && getPixel(DOWN_RIGHT) != Material::Acid && x + 1 < WIDTH)
        {
            setPixel(CUR_POS, 0);
            setPixel(DOWN_RIGHT, Material::Acid);
        }
        else if (getPixel(LEFT) && getPixel(LEFT) != Material::Acid)
        {
            setPixel(CUR_POS, 0);
            setPixel(LEFT, Material::Acid);
        }
        else if (getPixel(RIGHT) && getPixel(RIGHT) != Material::Acid)
        {
            setPixel(CUR_POS, 0);
            setPixel(RIGHT, Material::Acid);
        }
    }

    // Dissipate
    if (randInt(0, 24) == 0)
        setPixel(CUR_POS, 0);
}
