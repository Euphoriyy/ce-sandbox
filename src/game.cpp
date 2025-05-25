#include "../include/game.h"

Cursor cursor;
Timing timing;
KeyState keyState;
bool isDrawing = false, isErasing = false, isPaused = false, enableFloor = true;
uint8_t brushSize = 1;

void clearCursor()
{
    uint8_t offsetBrushSize = brushSize + 1;
    for (int8_t dx = -offsetBrushSize; dx < offsetBrushSize; ++dx)
    {
        makeDirty(cursor.pos.x + dx, cursor.pos.y);
    }
    for (int8_t dy = -offsetBrushSize; dy < offsetBrushSize; ++dy)
    {
        makeDirty(cursor.pos.x, cursor.pos.y + dy);
    }
}
