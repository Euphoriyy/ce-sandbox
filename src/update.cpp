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
                    if (color == SAND)
                        updateSand(x, y);
                    else if (color == WATER)
                        updateWater(x, y);
                    else if (color == ACID)
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
        setPixel(x, y + 1, SAND);
    }
    else if (!getPixel(x - 1, y + 1) && x > 0)
    {
        setPixel(x, y, 0);
        setPixel(x - 1, y + 1, SAND);
    }
    else if (!getPixel(x + 1, y + 1) && x + 1 < WIDTH)
    {
        setPixel(x, y, 0);
        setPixel(x + 1, y + 1, SAND);
    }
    else if (getPixel(x, y + 1) == WATER)
    {
        setPixel(x, y, WATER);
        setPixel(x, y + 1, SAND);
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
        setPixel(x, y + 1, WATER);
    }
    else if (x > 0 && !getPixel(x - 1, y + 1))
    {
        setPixel(x, y, 0);
        setPixel(x - 1, y + 1, WATER);
    }
    else if (x + 1 < WIDTH && !getPixel(x + 1, y + 1))
    {
        setPixel(x, y, 0);
        setPixel(x + 1, y + 1, WATER);
    }
    else if (x > 0 && !getPixel(x - 1, y))
    {
        setPixel(x, y, 0);
        setPixel(x - 1, y, WATER);
    }
    else if (x + 1 < WIDTH && !getPixel(x + 1, y))
    {
        setPixel(x, y, 0);
        setPixel(x + 1, y, WATER);
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
        setPixel(x, y + 1, ACID);
    }
    else if (x > 0 && !getPixel(x - 1, y + 1))
    {
        setPixel(x, y, 0);
        setPixel(x - 1, y + 1, ACID);
    }
    else if (x + 1 < WIDTH && !getPixel(x + 1, y + 1))
    {
        setPixel(x, y, 0);
        setPixel(x + 1, y + 1, ACID);
    }
    else if (x > 0 && !getPixel(x - 1, y))
    {
        setPixel(x, y, 0);
        setPixel(x - 1, y, ACID);
    }
    else if (x + 1 < WIDTH && !getPixel(x + 1, y))
    {
        setPixel(x, y, 0);
        setPixel(x + 1, y, ACID);
    }

    // Corrode nearby materials
    if (randInt(0, 9) == 0)
    {
        if (getPixel(x, y - 1) && getPixel(x, y - 1) != ACID)
        {
            setPixel(x, y, 0);
            setPixel(x, y - 1, ACID);
        }
        else if (getPixel(x, y + 1) && getPixel(x, y + 1) != ACID)
        {
            setPixel(x, y, 0);
            setPixel(x, y + 1, ACID);
        }
        else if (getPixel(x - 1, y + 1) && getPixel(x - 1, y + 1) != ACID && x > 0)
        {
            setPixel(x, y, 0);
            setPixel(x - 1, y + 1, ACID);
        }
        else if (getPixel(x + 1, y + 1) && getPixel(x + 1, y + 1) != ACID && x + 1 < WIDTH)
        {
            setPixel(x, y, 0);
            setPixel(x + 1, y + 1, ACID);
        }
        else if (getPixel(x - 1, y) && getPixel(x - 1, y) != ACID)
        {
            setPixel(x, y, 0);
            setPixel(x - 1, y, ACID);
        }
        else if (getPixel(x + 1, y) && getPixel(x + 1, y) != ACID)
        {
            setPixel(x, y, 0);
            setPixel(x + 1, y, ACID);
        }
    }

    // Dissipate
    if (randInt(0, 24) == 0)
        setPixel(x, y, 0);
}
