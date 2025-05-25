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
            uint16_t idx = IDX(x, y);
            // Only update if the pixel is active
            if (pixelData.activeFlags[idx])
            {
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
    }
}

void updateSand(uint8_t x, uint8_t y)
{
    if (gameState.enableFloor && y == HEIGHT - 1)
        return;
    // If down is empty
    if (!getPixel(x, y + 1))
    {
        setPixel(x, y, 0);
        setPixel(x, y + 1, Material::Sand);
    }
    else if (!getPixel(x - 1, y + 1) && x > 0)
    {
        setPixel(x, y, 0);
        setPixel(x - 1, y + 1, Material::Sand);
    }
    else if (!getPixel(x + 1, y + 1) && x + 1 < WIDTH)
    {
        setPixel(x, y, 0);
        setPixel(x + 1, y + 1, Material::Sand);
    }
    else if (getPixel(x, y + 1) == Material::Water)
    {
        setPixel(x, y, Material::Water);
        setPixel(x, y + 1, Material::Sand);
    }
}

void updateWater(uint8_t x, uint8_t y)
{
    if (gameState.enableFloor && y == HEIGHT - 1)
        return;
    // If down is empty
    if (!getPixel(x, y + 1))
    {
        setPixel(x, y, 0);
        setPixel(x, y + 1, Material::Water);
    }
    else if (x > 0 && !getPixel(x - 1, y + 1))
    {
        setPixel(x, y, 0);
        setPixel(x - 1, y + 1, Material::Water);
    }
    else if (x + 1 < WIDTH && !getPixel(x + 1, y + 1))
    {
        setPixel(x, y, 0);
        setPixel(x + 1, y + 1, Material::Water);
    }
    else if (x > 0 && !getPixel(x - 1, y))
    {
        setPixel(x, y, 0);
        setPixel(x - 1, y, Material::Water);
    }
    else if (x + 1 < WIDTH && !getPixel(x + 1, y))
    {
        setPixel(x, y, 0);
        setPixel(x + 1, y, Material::Water);
    }
}

void updateDirt(uint8_t x, uint8_t y)
{
    if (gameState.enableFloor && y == HEIGHT - 1)
        return;
    // If down is empty
    if (!getPixel(x, y + 1))
    {
        setPixel(x, y, 0);
        setPixel(x, y + 1, Material::Dirt);
    }
    else if (!getPixel(x - 1, y + 1) && x > 0)
    {
        setPixel(x, y, 0);
        setPixel(x - 1, y + 1, Material::Dirt);
    }
    else if (!getPixel(x + 1, y + 1) && x + 1 < WIDTH)
    {
        setPixel(x, y, 0);
        setPixel(x + 1, y + 1, Material::Dirt);
    }
    else if (getPixel(x, y + 1) == Material::Water)
    {
        setPixel(x, y, Material::Water);
        setPixel(x, y + 1, Material::Dirt);
    }
}

void updateAcid(uint8_t x, uint8_t y)
{
    if (gameState.enableFloor && y == HEIGHT - 1)
        return;
    // If down is empty
    if (!getPixel(x, y + 1))
    {
        setPixel(x, y, 0);
        setPixel(x, y + 1, Material::Acid);
    }
    else if (x > 0 && !getPixel(x - 1, y + 1))
    {
        setPixel(x, y, 0);
        setPixel(x - 1, y + 1, Material::Acid);
    }
    else if (x + 1 < WIDTH && !getPixel(x + 1, y + 1))
    {
        setPixel(x, y, 0);
        setPixel(x + 1, y + 1, Material::Acid);
    }
    else if (x > 0 && !getPixel(x - 1, y))
    {
        setPixel(x, y, 0);
        setPixel(x - 1, y, Material::Acid);
    }
    else if (x + 1 < WIDTH && !getPixel(x + 1, y))
    {
        setPixel(x, y, 0);
        setPixel(x + 1, y, Material::Acid);
    }

    // Corrode nearby materials
    if (randInt(0, 9) == 0)
    {
        if (getPixel(x, y - 1) && getPixel(x, y - 1) != Material::Acid)
        {
            setPixel(x, y, 0);
            setPixel(x, y - 1, Material::Acid);
        }
        else if (getPixel(x, y + 1) && getPixel(x, y + 1) != Material::Acid)
        {
            setPixel(x, y, 0);
            setPixel(x, y + 1, Material::Acid);
        }
        else if (getPixel(x - 1, y + 1) && getPixel(x - 1, y + 1) != Material::Acid && x > 0)
        {
            setPixel(x, y, 0);
            setPixel(x - 1, y + 1, Material::Acid);
        }
        else if (getPixel(x + 1, y + 1) && getPixel(x + 1, y + 1) != Material::Acid &&
                 x + 1 < WIDTH)
        {
            setPixel(x, y, 0);
            setPixel(x + 1, y + 1, Material::Acid);
        }
        else if (getPixel(x - 1, y) && getPixel(x - 1, y) != Material::Acid)
        {
            setPixel(x, y, 0);
            setPixel(x - 1, y, Material::Acid);
        }
        else if (getPixel(x + 1, y) && getPixel(x + 1, y) != Material::Acid)
        {
            setPixel(x, y, 0);
            setPixel(x + 1, y, Material::Acid);
        }
    }

    // Dissipate
    if (randInt(0, 24) == 0)
        setPixel(x, y, 0);
}
