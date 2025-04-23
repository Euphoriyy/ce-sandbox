#include "../include/render.h"

void render()
{
    gfx_ZeroScreen();

    // Draw Pixels
    for (uint8_t y = 0; activeCount && y < HEIGHT; ++y)
    {
        // Skip inactive rows
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

            if (color)
            {
                gfx_SetColor(color);

                uint8_t runStart = x, runEnd = x;

                // Find the end of the contiguous run with the same color and an active flag
                for (int i = x + 1; i < WIDTH && activeFlags[IDX(i, y)] && getPixel(i, y) == color;
                     ++i)
                {
                    runEnd = i;
                }

                uint8_t runLength = runEnd - runStart + 1;
                uint16_t runWidth = runLength * SCALE_FACTOR;

                uint8_t xScaleOffset =
                    (runEnd == WIDTH - 1 && SCALE_FACTOR != gcd(GFX_LCD_WIDTH, SCALE_FACTOR))
                        ? gcd(GFX_LCD_WIDTH, SCALE_FACTOR)
                        : 0;

                // Fill a single rectangle for the whole contiguous run
                gfx_FillRectangle_NoClip(runStart * SCALE_FACTOR, scaledY, runWidth + xScaleOffset,
                                         SCALE_FACTOR + yScaleOffset);

                x = runEnd; // Advance x to skip this run
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
