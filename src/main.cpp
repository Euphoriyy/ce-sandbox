#include <stdint.h>
#ifndef __INT24_TYPE__
typedef int32_t int24_t;
typedef uint32_t uint24_t;
#endif
#include <graphx.h>
#include <keypadc.h>
#include <sys/rtc.h>
#include <sys/util.h>
#include <string.h>

const uint8_t GUI_HEIGHT = 12;
const uint8_t SCALE_FACTOR = 6;
const uint8_t WIDTH = GFX_LCD_WIDTH / SCALE_FACTOR;
const uint8_t HEIGHT = (GFX_LCD_HEIGHT - GUI_HEIGHT) / SCALE_FACTOR;
const uint16_t TOTAL_PIXELS = WIDTH * HEIGHT;

#define STRINGIFY(var) #var

uint8_t gcd(uint16_t a, uint16_t b) {
    return !b ? a : gcd(b, a % b);
}

struct Vector2 {
    uint8_t x = 0, y = 0;
};

struct Keys {
    bool enter = false, del = false, yequ = false, graph = false, second = false, window = false;
};

struct KeyState {
    Keys prev;
    Keys cur;
};

const uint8_t SAND = 245;
const uint8_t WATER = 28;
const uint8_t STONE = 148;
const uint8_t ACID = 109;
uint16_t pixelCount = 0;

const uint8_t palette[] = { SAND, WATER, STONE, ACID };

const uint8_t paletteLen = sizeof(palette) / sizeof(uint8_t);

struct Cursor {
    Vector2 pos;
    uint8_t color = 255;
    uint8_t size = SCALE_FACTOR;
    uint8_t paletteIndex = 0;
};

Cursor cursor;
uint8_t pixels[TOTAL_PIXELS] = {0};
bool active[TOTAL_PIXELS] = {0};
KeyState keyState;
bool isDrawing = false, isErasing = false, isPaused = false, enableFloor = true;

static uint16_t yOffsets[HEIGHT];

// Macros
// #define IDX(x, y) ((y) * WIDTH + (x))
#define IDX(x, y) (yOffsets[y] + (x))
#define IN_BOUNDS(x, y) ((x) < WIDTH && (y) < HEIGHT)

// Pixel I/O
// inline void setPixel(uint8_t x, uint8_t y, uint8_t color) {
//     if (IN_BOUNDS(x, y)) pixels[IDX(x, y)] = color;
// }

// inline uint8_t getPixel(uint8_t x, uint8_t y) {
//     return IN_BOUNDS(x, y) ? pixels[IDX(x, y)] : 0;
// }

inline void setPixel(uint8_t x, uint8_t y, uint8_t color) {
    if (IN_BOUNDS(x, y)) {
        pixels[IDX(x, y)] = color;
        if (color != 0) {
            active[IDX(x, y)] = true;
        }
        else {
            active[IDX(x, y)] = false;
        }
    }
}

inline uint8_t getPixel(uint8_t x, uint8_t y) {
    return IN_BOUNDS(x, y) ? pixels[IDX(x, y)] : 0;
}

void handleInput() {
    if (kb_IsDown(kb_KeyClear)) {
        memset(pixels, 0, sizeof(pixels));
        memset(active, 0, TOTAL_PIXELS);
        pixelCount = 0;
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
    keyState.cur.window = kb_IsDown(kb_KeyWindow);

    // Toggle Drawing Mode
    if (keyState.cur.enter && !keyState.prev.enter) {
        isDrawing = !isDrawing;
        if (isDrawing)
            isErasing = false;
    }
    
    // Toggle Erasing Mode
    if (keyState.cur.del && !keyState.prev.del) {
        isErasing = !isErasing;
        if (isErasing)
            isDrawing = false;
    }

    // Toggle Pausing
    if (keyState.cur.second && !keyState.prev.second)
        isPaused = !isPaused;

    // Toggle Floor
    if (keyState.cur.window && !keyState.prev.window)
        enableFloor = !enableFloor;

    // Switch palette
    if (keyState.cur.yequ && !keyState.prev.yequ) {
        if (cursor.paletteIndex > 0)
            cursor.paletteIndex--;
        else
            cursor.paletteIndex = paletteLen - 1;
    }
    else if (keyState.cur.graph && !keyState.prev.graph) {
        if (cursor.paletteIndex < paletteLen - 1)
            cursor.paletteIndex++;
        else
            cursor.paletteIndex = 0;
    }
 
    // Draw or erase based on current mode
    if (isDrawing && !getPixel(cursor.pos.x, cursor.pos.y)) {
        setPixel(cursor.pos.x, cursor.pos.y, palette[cursor.paletteIndex]);
        ++pixelCount;
    }
    else if (isErasing && getPixel(cursor.pos.x, cursor.pos.y)) {
        setPixel(cursor.pos.x, cursor.pos.y, 0);
        --pixelCount;
    }

    // Set the previous key state to be the current key state
    keyState.prev = keyState.cur;
}

void render() {
    gfx_FillScreen(0);
    // Draw Pixels
    uint8_t prevColor = 0;
    for (uint8_t y = 0; pixelCount && y < HEIGHT; ++y) {
        uint8_t scaledY = y * SCALE_FACTOR + GUI_HEIGHT;
        uint8_t yScaleOffset = y == HEIGHT - 1 ? gcd(GFX_LCD_HEIGHT, SCALE_FACTOR): 0;
        yScaleOffset -= yScaleOffset / 2; // Avoid overflowing issues
        for (uint8_t x = 0; x < WIDTH; ++x) {
            if (active[IDX(x, y)]) {
                uint8_t color = getPixel(x, y);
                uint8_t xScaleOffset = x == WIDTH - 1 && SCALE_FACTOR != gcd(GFX_LCD_WIDTH, SCALE_FACTOR) ? gcd(GFX_LCD_WIDTH, SCALE_FACTOR): 0;
                if (color && color != prevColor) {
                    gfx_SetColor(color);
                    prevColor = color;
                }
                if (color) {
                    gfx_FillRectangle_NoClip(x * SCALE_FACTOR, scaledY, SCALE_FACTOR + xScaleOffset, SCALE_FACTOR + yScaleOffset);
                }
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
    
    if (isDrawing) {
        gfx_PrintString("DRAWING:");
    }
    else if (isErasing) {
        gfx_PrintString("ERASING");
    }
    else {
        gfx_PrintString("IDLE");
    }

    gfx_SetColor(0);
    gfx_FillRectangle_NoClip(72, 2, 8, 8);
    gfx_SetColor(palette[cursor.paletteIndex]);
    gfx_FillRectangle_NoClip(73, 3, 6, 6);
    gfx_SetTextXY(82, 3);
    switch (palette[cursor.paletteIndex]) {
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

    gfx_PrintStringXY("PARTS:", 220, 3);
    gfx_SetTextXY(280, 3);
    gfx_PrintUInt(pixelCount, 4);
    // --------------
    gfx_SwapDraw();
}

void updateSand(uint8_t x, uint8_t y);
void updateWater(uint8_t x, uint8_t y);
void updateAcid(uint8_t x, uint8_t y);

void update() {
    for (uint8_t y = HEIGHT - 1, x; y != 0xFF; --y) {
        bool flip = randInt(0, 1);
        if (flip) {
            for (x = 0; x < WIDTH; ++x) {
                if (active[IDX(x, y)]) {
                    uint8_t color = getPixel(x, y);
                    if (color) {
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
        else {
            for (x = WIDTH - 1; x != 0xFF; --x) {
                if (active[IDX(x, y)]) {
                    uint8_t color = getPixel(x, y);
                    if (color) {
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
}

void updateSand(uint8_t x, uint8_t y) {
    if (enableFloor && y == HEIGHT - 1)
        return;
    // If down is empty
    if (!getPixel(x, y + 1)) {
        setPixel(x, y, 0);
        setPixel(x, y + 1, SAND);
    }
    else if (!getPixel(x - 1, y + 1) && x > 0) {
        setPixel(x, y, 0);
        setPixel(x - 1, y + 1, SAND);
    }
    else if (!getPixel(x + 1, y + 1) && x + 1 < WIDTH) {
        setPixel(x, y, 0);
        setPixel(x + 1, y + 1, SAND);
    }
    else if (getPixel(x, y + 1) == WATER) {
        setPixel(x, y, WATER);
        setPixel(x, y + 1, SAND);
    }
}

// void updateWater(uint8_t x, uint8_t y) {
//     if (y + 1 >= HEIGHT)
//         return;
//     // If down is empty
//     if (!getPixel(x, y + 1)) {
//         setPixel(x, y, 0);
//         setPixel(x, y + 1, WATER);
//     }
//     else if (!getPixel(x - 1, y + 1) && x > 0) {
//         setPixel(x, y, 0);
//         setPixel(x - 1, y + 1, WATER);
//     }
//     else if (!getPixel(x + 1, y + 1) && x + 1 < WIDTH) {
//         setPixel(x, y, 0);
//         setPixel(x + 1, y + 1, WATER);
//     }
//     else if (!getPixel(x - 1, y) && x > 0) {
//         setPixel(x, y, 0);
//         setPixel(x - 1, y, WATER);
//     }
//     else if (!getPixel(x + 1, y) && x + 1 < WIDTH) {
//         setPixel(x, y, 0);
//         setPixel(x + 1, y, WATER);
//     }
// }

void updateWater(uint8_t x, uint8_t y) {
    if (enableFloor && y == HEIGHT - 1)
        return;
    // If down is empty
    if (!getPixel(x, y + 1)) {
        setPixel(x, y, 0);
        setPixel(x, y + 1, WATER);
    }
    else if (x > 0 && !getPixel(x - 1, y + 1)) {
        setPixel(x, y, 0);
        setPixel(x - 1, y + 1, WATER);
    }
    else if (x + 1 < WIDTH && !getPixel(x + 1, y + 1)) {
        setPixel(x, y, 0);
        setPixel(x + 1, y + 1, WATER);
    }
    else if (x > 0 && !getPixel(x - 1, y)) {
        setPixel(x, y, 0);
        setPixel(x - 1, y, WATER);
    }
    else if (x + 1 < WIDTH && !getPixel(x + 1, y)) {
        setPixel(x, y, 0);
        setPixel(x + 1, y, WATER);
    }
}

void updateAcid(uint8_t x, uint8_t y) {
    if (enableFloor && y == HEIGHT - 1)
        return;
    // If down is empty
    if (!getPixel(x, y + 1)) {
        setPixel(x, y, 0);
        setPixel(x, y + 1, ACID);
    }
    else if (x > 0 && !getPixel(x - 1, y + 1)) {
        setPixel(x, y, 0);
        setPixel(x - 1, y + 1, ACID);
    }
    else if (x + 1 < WIDTH && !getPixel(x + 1, y + 1)) {
        setPixel(x, y, 0);
        setPixel(x + 1, y + 1, ACID);
    }
    else if (x > 0 && !getPixel(x - 1, y)) {
        setPixel(x, y, 0);
        setPixel(x - 1, y, ACID);
    }
    else if (x + 1 < WIDTH && !getPixel(x + 1, y)) {
        setPixel(x, y, 0);
        setPixel(x + 1, y, ACID);
    }

    // Corrode nearby materials
    if (randInt(0, 9) == 0) {
        if (getPixel(x, y - 1) && getPixel(x, y - 1) != ACID) {
            setPixel(x, y, 0);
            setPixel(x, y - 1, ACID);
        }
        else if (getPixel(x, y + 1) && getPixel(x, y + 1) != ACID) {
            setPixel(x, y, 0);
            setPixel(x, y + 1, ACID);
        }
        else if (getPixel(x - 1, y + 1) && getPixel(x - 1, y + 1) != ACID && x > 0) {
            setPixel(x, y, 0);
            setPixel(x - 1, y + 1, ACID);
        }
        else if (getPixel(x + 1, y + 1) && getPixel(x + 1, y + 1) != ACID && x + 1 < WIDTH) {
            setPixel(x, y, 0);
            setPixel(x + 1, y + 1, ACID);
        }
    }
    
    // Dissipate
    if (randInt(0, 49) == 0) {
        setPixel(x, y, 0);
    }
}


int main(void)
{
    srandom(rtc_Time());
    gfx_Begin();
    gfx_SetDrawBuffer();
    for (uint8_t y = 0; y < HEIGHT; y++) {
        yOffsets[y] = y * WIDTH;
}

    while (!kb_IsDown(kb_KeyMode)) {
        kb_Scan();
        handleInput();
        render();
        if (pixelCount && !isPaused)
            update();
    };
    gfx_End();
    
    return 0;
}
