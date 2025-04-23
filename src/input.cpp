#include "../include/input.h"

void handleInput()
{
    if (kb_IsDown(kb_KeyClear))
    {
        memset(pixels, 0, TOTAL_PIXELS);
        memset(activeFlags, 0, TOTAL_PIXELS);
        memset(activeRows, 0, HEIGHT);
        memset(lastUpdate, 0, TOTAL_PIXELS * sizeof(uint16_t));
        activeCount = 0;
    }
    if (kb_IsDown(kb_KeyRight) && cursor.pos.x < WIDTH - 1)
        ++cursor.pos.x;
    if (kb_IsDown(kb_KeyLeft) && cursor.pos.x > 0)
        --cursor.pos.x;
    if (kb_IsDown(kb_KeyDown) && cursor.pos.y < HEIGHT - 1)
        ++cursor.pos.y;
    if (kb_IsDown(kb_KeyUp) && cursor.pos.y > 0)
        --cursor.pos.y;

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
            --brushSize;

    if (keyState.cur.trace && !keyState.prev.trace)
        if (brushSize < 10)
            ++brushSize;

    // Switch palette
    if (keyState.cur.yequ && !keyState.prev.yequ)
    {
        if (cursor.paletteIndex > 0)
            --cursor.paletteIndex;
        else
            cursor.paletteIndex = paletteLen - 1;
    }
    else if (keyState.cur.graph && !keyState.prev.graph)
    {
        if (cursor.paletteIndex < paletteLen - 1)
            ++cursor.paletteIndex;
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
