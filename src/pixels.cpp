#include "../include/pixels.h"

const uint8_t GUI_HEIGHT = 12;
const uint8_t SCALE_FACTOR = 5;
const uint8_t WIDTH = GFX_LCD_WIDTH / SCALE_FACTOR;
const uint8_t HEIGHT = (GFX_LCD_HEIGHT - GUI_HEIGHT) / SCALE_FACTOR;
const uint16_t TOTAL_PIXELS = WIDTH * HEIGHT;

uint8_t pixels[TOTAL_PIXELS] = {0};
bool activeFlags[TOTAL_PIXELS] = {0};
bool activeRows[HEIGHT] = {0};
bool dirtyFlags[TOTAL_PIXELS] = {0};
bool dirtyRows[HEIGHT] = {0};
uint16_t activeCount = 0;
uint16_t lastUpdate[TOTAL_PIXELS];
uint16_t yOffsets[HEIGHT];
uint8_t bgColorCells[TOTAL_PIXELS];

uint8_t getPixel(uint8_t x, uint8_t y) { return IN_BOUNDS(x, y) ? pixels[IDX(x, y)] : 0; }

void setPixel(uint8_t x, uint8_t y, uint8_t mat)
{
    if (IN_BOUNDS(x, y))
    {
        uint8_t prevMat = getPixel(x, y);
        if (mat != prevMat)
        {
            // If the mat is being set to something else, adjust the active count
            if (prevMat == 0 && mat != 0)
                ++activeCount; // Increment active count if the new mat is non-zero
            else if (prevMat != 0 && mat == 0)
                --activeCount; // Decrement active count if it is zero
            makeDirty(x, y);
            pixels[IDX(x, y)] = mat;
        }

        if (mat != 0)
        {
            activeFlags[IDX(x, y)] = true;
            activeRows[y] = true;
        }
        else
        {
            activeFlags[IDX(x, y)] = false;
            bool rowActive = false;
            for (int j = 0; j < WIDTH; ++j)
            {
                if (activeFlags[IDX(j, y)])
                    rowActive = true;
            }
            activeRows[y] = rowActive;
        }
        // When a pixel is updated, change the updated status of itself and its adjacent pixels
        for (int8_t dy = -1; dy <= 1; ++dy)
        {
            for (int8_t dx = -1; dx <= 1; ++dx)
            {
                uint8_t nx = x + dx, ny = y + dy;
                if (IN_BOUNDS(nx, ny))
                    lastUpdate[IDX(nx, ny)] = frame;
            }
        }
    }
}

void makeDirty(uint8_t x, uint8_t y)
{
    if (IN_BOUNDS(x, y))
    {
        dirtyFlags[IDX(x, y)] = true;
        dirtyRows[y] = true;
    }
}
