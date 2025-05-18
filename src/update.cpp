#include "../include/update.h"

void update()
{
    for (uint8_t y = HEIGHT - 1, x; y != 0xFF; --y)
    {
        if (!activeRows[y])
            continue;
        bool flip = randInt(0, 1);
        for (x = (flip ? 0 : WIDTH - 1); flip ? x < WIDTH : x != 0xFF; x += (flip ? 1 : -1))
        {
            uint16_t idx = IDX(x, y);
            // Only update if the pixel is active and it was updated within the last 5 frames
            if (activeFlags[idx] && frame - lastUpdate[idx] < 5)
            {
                uint8_t color = getPixel(x, y);
                if (color)
                {
                    if (color == Material::Sand)
                        updateSand(x, y);
                    else if (color == Material::Water)
                        updateWater(x, y);
                    else if (color == Material::Acid)
                        updateAcid(x, y);
                }
            }
        }
    }
}

void updateSand(uint8_t x, uint8_t y)
{
    if (enableFloor && y == HEIGHT - 1)
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
    if (enableFloor && y == HEIGHT - 1)
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

void updateAcid(uint8_t x, uint8_t y)
{
    if (enableFloor && y == HEIGHT - 1)
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
