#include "../include/game.h"

Cursor cursor;
uint16_t frame = 0;
uint8_t frametime = 0;
KeyState keyState;
bool isDrawing = false, isErasing = false, isPaused = false, enableFloor = true;
uint8_t brushSize = 1;

void clearCursor()
{
    for (int8_t dx = -brushSize; dx < brushSize + 1; ++dx)
    {
        makeDirty(cursor.pos.x + dx, cursor.pos.y);
    }
    for (int8_t dy = -brushSize; dy < brushSize + 1; ++dy)
    {
        makeDirty(cursor.pos.x, cursor.pos.y + dy);
    }
}
