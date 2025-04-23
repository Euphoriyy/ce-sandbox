#include <graphx.h>
#include <keypadc.h>
#include <stdint.h>
#include <string.h>
#include <sys/rtc.h>
#include <sys/timers.h>
#include <sys/util.h>
#include <ti/getcsc.h>

#include "../include/math.h"

const uint8_t GUI_HEIGHT = 12;
const uint8_t SCALE_FACTOR = 5;
const uint8_t WIDTH = GFX_LCD_WIDTH / SCALE_FACTOR;
const uint8_t HEIGHT = (GFX_LCD_HEIGHT - GUI_HEIGHT) / SCALE_FACTOR;
const uint16_t TOTAL_PIXELS = WIDTH * HEIGHT;

#define STRINGIFY(var) #var

struct Keys
{
    bool enter = false, del = false, yequ = false, graph = false, second = false, zoom = false,
         window = false, trace = false;
};

struct KeyState
{
    Keys prev;
    Keys cur;
};

const uint8_t SAND = 245;
const uint8_t WATER = 28;
const uint8_t STONE = 148;
const uint8_t ACID = 109;

const uint8_t palette[] = {SAND, WATER, STONE, ACID};

const uint8_t paletteLen = sizeof(palette) / sizeof(uint8_t);

uint8_t brushSize = 1;

struct Cursor
{
    Vector2 pos;
    uint8_t color = 255;
    uint8_t size = SCALE_FACTOR;
    uint8_t paletteIndex = 0;
};

Cursor cursor;
uint8_t pixels[TOTAL_PIXELS] = {0};
bool activeFlags[TOTAL_PIXELS] = {0};
bool activeRows[HEIGHT] = {0};
uint16_t activeCount = 0;
uint16_t frame = 0;
uint8_t frametime = 0;
uint16_t lastUpdate[TOTAL_PIXELS];
KeyState keyState;
bool isDrawing = false, isErasing = false, isPaused = false, enableFloor = true;

static uint16_t yOffsets[HEIGHT];

// Macros
// #define IDX(x, y) ((y) * WIDTH + (x))
#define IDX(x, y) (yOffsets[y] + (x))
#define IN_BOUNDS(x, y) ((x) < WIDTH && (y) < HEIGHT)

// Pixel I/O

inline uint8_t getPixel(uint8_t x, uint8_t y) { return IN_BOUNDS(x, y) ? pixels[IDX(x, y)] : 0; }

inline void setPixel(uint8_t x, uint8_t y, uint8_t color)
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

// MAIN FUNCTIONS

void handleInput()
{
    if (kb_IsDown(kb_KeyClear))
    {
        memset(pixels, 0, sizeof(pixels));
        memset(activeFlags, 0, TOTAL_PIXELS);
        memset(activeRows, 0, HEIGHT);
        memset(lastUpdate, 0, sizeof(lastUpdate));
        activeCount = 0;
    }
    if (kb_IsDown(kb_KeyRight) && cursor.pos.x < WIDTH - 1)
        cursor.pos.x++;
    if (kb_IsDown(kb_KeyLeft) && cursor.pos.x > 0)
        cursor.pos.x--;
    if (kb_IsDown(kb_KeyDown) && cursor.pos.y < HEIGHT - 1)
        cursor.pos.y++;
    if (kb_IsDown(kb_KeyUp) && cursor.pos.y > 0)
        cursor.pos.y--;

    // Get current key state
    keyState.cur.enter = kb_IsDown(kb_KeyEnter);
    keyState.cur.del = kb_IsDown(kb_KeyDel);
    keyState.cur.yequ = kb_IsDown(kb_KeyYequ);
    keyState.cur.graph = kb_IsDown(kb_KeyGraph);
    keyState.cur.second = kb_IsDown(kb_Key2nd);
    keyState.cur.zoom = kb_IsDown(kb_KeyZoom);
    keyState.cur.window = kb_IsDown(kb_KeyWindow);
    keyState.cur.trace = kb_IsDown(kb_KeyTrace);

    // Toggle Drawing Mode
    if (keyState.cur.enter && !keyState.prev.enter)
    {
        isDrawing = !isDrawing;
        if (isDrawing)
            isErasing = false;
    }

    // Toggle Erasing Mode
    if (keyState.cur.del && !keyState.prev.del)
    {
        isErasing = !isErasing;
        if (isErasing)
            isDrawing = false;
    }

    // Toggle Pausing
    if (keyState.cur.second && !keyState.prev.second)
        isPaused = !isPaused;

    // Toggle Floor
    if (keyState.cur.zoom && !keyState.prev.zoom)
    {
        enableFloor = !enableFloor;
        // Update the last row after disabling the floor
        if (!enableFloor && activeRows[HEIGHT - 1])
        {
            for (uint8_t x = 0; x < WIDTH; ++x)
                lastUpdate[IDX(x, HEIGHT - 1)] = frame;
        }
    }

    // Increase Brush Size
    if (keyState.cur.window && !keyState.prev.window)
        if (brushSize > 1)
            brushSize--;

    if (keyState.cur.trace && !keyState.prev.trace)
        if (brushSize < 10)
            ++brushSize;

    // Switch palette
    if (keyState.cur.yequ && !keyState.prev.yequ)
    {
        if (cursor.paletteIndex > 0)
            cursor.paletteIndex--;
        else
            cursor.paletteIndex = paletteLen - 1;
    }
    else if (keyState.cur.graph && !keyState.prev.graph)
    {
        if (cursor.paletteIndex < paletteLen - 1)
            cursor.paletteIndex++;
        else
            cursor.paletteIndex = 0;
    }

    // Draw or erase based on current mode
    if (isDrawing && !getPixel(cursor.pos.x, cursor.pos.y))
        setPixel(cursor.pos.x, cursor.pos.y, palette[cursor.paletteIndex]);
    else if (isErasing && getPixel(cursor.pos.x, cursor.pos.y))
        setPixel(cursor.pos.x, cursor.pos.y, 0);

    // Set the previous key state to be the current key state
    keyState.prev = keyState.cur;
}

void render()
{
    gfx_ZeroScreen();
    // Draw Pixels
    uint8_t prevColor = 0;
    for (uint8_t y = 0; activeCount && y < HEIGHT; ++y)
    {
        // Skips inactive rows
        if (!activeRows[y])
            continue;

        uint8_t scaledY = y * SCALE_FACTOR + GUI_HEIGHT;
        uint8_t yScaleOffset = y == HEIGHT - 1 ? gcd(GFX_LCD_HEIGHT, SCALE_FACTOR) : 0;
        yScaleOffset -= yScaleOffset / 2; // Avoid overflowing issues

        for (uint8_t x = 0; x < WIDTH; ++x)
        {
            // Do not draw if pixel is not flagged as active
            if (!activeFlags[IDX(x, y)])
                continue;

            uint8_t color = getPixel(x, y);
            uint8_t xScaleOffset =
                x == WIDTH - 1 && SCALE_FACTOR != gcd(GFX_LCD_WIDTH, SCALE_FACTOR)
                    ? gcd(GFX_LCD_WIDTH, SCALE_FACTOR)
                    : 0;

            if (color)
            {
                if (color != prevColor)
                {
                    gfx_SetColor(color);
                    prevColor = color;
                }
                gfx_FillRectangle_NoClip(x * SCALE_FACTOR, scaledY, SCALE_FACTOR + xScaleOffset,
                                         SCALE_FACTOR + yScaleOffset);
            }
        }
    }
    // Draw Cursor
    gfx_SetColor(cursor.color);
    uint16_t cx = cursor.pos.x * cursor.size;
    uint16_t cy = cursor.pos.y * cursor.size + GUI_HEIGHT;
    gfx_HorizLine(cx - cursor.size, cy, cursor.size * 2);
    gfx_VertLine(cx, cy - cursor.size, cursor.size * 2);
    // Draw GUI
    gfx_SetColor(50);
    gfx_FillRectangle_NoClip(0, 0, GFX_LCD_WIDTH, GUI_HEIGHT);
    gfx_SetTextFGColor(255);
    gfx_SetTextBGColor(50);
    gfx_SetTextTransparentColor(0);
    gfx_SetTextXY(10, 3);
    gfx_SetTextScale(1, 1);

    if (isDrawing)
        gfx_PrintString("DRAWING:");
    else if (isErasing)
        gfx_PrintString("ERASING");
    else
        gfx_PrintString("IDLE");

    gfx_SetColor(0);
    gfx_FillRectangle_NoClip(72, 2, 8, 8);
    gfx_SetColor(palette[cursor.paletteIndex]);
    gfx_FillRectangle_NoClip(73, 3, 6, 6);
    gfx_SetTextXY(82, 3);
    switch (palette[cursor.paletteIndex])
    {
        case SAND:
            gfx_PrintString("SAND");
            break;
        case WATER:
            gfx_PrintString("WATER");
            break;
        case STONE:
            gfx_PrintString("STONE");
            break;
        case ACID:
            gfx_PrintString("ACID");
            break;
    }

    gfx_PrintStringXY("SZ:", 135, 3);
    gfx_SetTextXY(158, 3);
    gfx_PrintUInt(brushSize, 1);

    gfx_PrintStringXY("FT:", 180, 3);
    gfx_SetTextXY(205, 3);
    gfx_PrintUInt(frametime, 2);

    gfx_PrintStringXY("PARTS:", 230, 3);
    gfx_SetTextXY(281, 3);
    gfx_PrintUInt(activeCount, 1);
    // --------------
    gfx_SwapDraw();
}

inline void updateSand(uint8_t x, uint8_t y);
inline void updateWater(uint8_t x, uint8_t y);
inline void updateAcid(uint8_t x, uint8_t y);

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

inline void updateSand(uint8_t x, uint8_t y)
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

inline void updateWater(uint8_t x, uint8_t y)
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

inline void updateAcid(uint8_t x, uint8_t y)
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

void mainMenu()
{
    gfx_FillScreen(50);
    gfx_SetTextFGColor(255);
    gfx_SetTextBGColor(50);
    gfx_SetTextTransparentColor(0);
    gfx_SetTextScale(3, 3);
    gfx_PrintStringXY("Sandbox", GFX_LCD_WIDTH / 4, GFX_LCD_HEIGHT / 3);
    gfx_SwapDraw();

    while (!os_GetCSC())
    {
    }
}

int main(void)
{
    // Seed random with time
    srandom(rtc_Time());

    // Precalculate y offsets
    for (uint8_t y = 0; y < HEIGHT; y++)
        yOffsets[y] = y * WIDTH;

    gfx_Begin();
    gfx_SetDrawBuffer();

    mainMenu();

    // Enable timer for frametime tracking
    timer_Enable(1, TIMER_32K, TIMER_NOINT, TIMER_UP);
    timer_Set(1, 0);
    uint16_t lastTick = timer_GetSafe(1, TIMER_UP), currentTick;

    while (!kb_IsDown(kb_KeyMode))
    {
        kb_Scan();
        handleInput();
        render();
        if (activeCount && !isPaused)
        {
            update();
            ++frame;
        }

        currentTick = timer_GetSafe(1, TIMER_UP);
        frametime = (currentTick - lastTick) * 1000 / 32768;
        lastTick = currentTick;
    };

    timer_Disable(1);
    gfx_End();

    return 0;
}
