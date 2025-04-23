#include "../include/pixels.h"

const uint8_t GUI_HEIGHT = 12;
const uint8_t SCALE_FACTOR = 5;
const uint8_t WIDTH = GFX_LCD_WIDTH / SCALE_FACTOR;
const uint8_t HEIGHT = (GFX_LCD_HEIGHT - GUI_HEIGHT) / SCALE_FACTOR;
const uint16_t TOTAL_PIXELS = WIDTH * HEIGHT;

uint8_t pixels[TOTAL_PIXELS] = {0};
bool activeFlags[TOTAL_PIXELS] = {0};
bool activeRows[HEIGHT] = {0};
uint16_t activeCount = 0;
uint16_t lastUpdate[TOTAL_PIXELS];
uint16_t yOffsets[HEIGHT];

uint8_t getPixel(uint8_t x, uint8_t y) { return IN_BOUNDS(x, y) ? pixels[IDX(x, y)] : 0; }

void setPixel(uint8_t x, uint8_t y, uint8_t color)
{
    if (IN_BOUNDS(x, y))
    {
        if (color != getPixel(x, y))
        {
            // If the color is being set to something else, adjust the active count
            if (color != 0)
                ++activeCount; // Increment active count if the new color is non-zero
            else
                --activeCount; // Decrement active count if it is zero
        }

        pixels[IDX(x, y)] = color;
        if (color != 0)
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
        for (int8_t dy = -1; dy <= 1; dy++)
        {
            for (int8_t dx = -1; dx <= 1; dx++)
            {
                uint8_t nx = x + dx, ny = y + dy;
                if (IN_BOUNDS(nx, ny))
                    lastUpdate[IDX(nx, ny)] = frame;
            }
        }
    }
}
