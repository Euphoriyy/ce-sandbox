#include "../include/render.h"

constexpr bool isImageBackground() { return materialShades[0][1]; }
constexpr bool isBlackBackground() { return !materialShades[0][0]; }

void render()
{
    // If empty, draw background
    if (!activeCount)
    {
        if (isImageBackground())
            gfx_ScaledSprite_NoClip(background_1, 0, 0, 8, 8);
        else if (isBlackBackground())
            gfx_ZeroScreen();
        else
            gfx_FillScreen(materialShades[0][0]);
    }
    else // Otherwise, copy previous frame
        gfx_BlitScreen();

    // Draw Pixels
    for (uint8_t y = 0; activeCount && y < HEIGHT; ++y)
    {
        // Skip static rows
        if (!dirtyRows[y])
            continue;

        uint8_t scaledY = y * SCALE_FACTOR + GUI_HEIGHT;
        uint8_t yScaleOffset = y == HEIGHT - 1 ? gcd(GFX_LCD_HEIGHT, SCALE_FACTOR) : 0;
        yScaleOffset -= yScaleOffset / 2; // Avoid overflowing issues

        for (uint8_t x = 0; x < WIDTH; ++x)
        {
            // Do not draw if pixel is not flagged as dirty
            if (!dirtyFlags[IDX(x, y)])
                continue;

            uint8_t mat = getPixel(x, y);

            if (mat) // Fill non-empty pixels
            {
                uint8_t runStart = x, runEnd = x;

                // Find the end of the contiguous run with the same mat, an active flag, and a dirty
                // flag
                for (int i = x + 1; i < WIDTH && activeFlags[IDX(i, y)] && dirtyFlags[IDX(i, y)] &&
                                    getPixel(i, y) == mat;
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

                // Set color to the material shade
                bool yDivisible = (y * 0xAAAAAAABULL >> 33) * 3 == y;
                uint8_t shadeIndex = ((yDivisible ? 2 : 1) & -(y & 1));
                uint8_t shade = materialShades[mat][shadeIndex];
                gfx_SetColor(shade ? shade : materialShades[mat][0]);

                // Fill a single rectangle for the whole contiguous run
                gfx_FillRectangle_NoClip(runStart * SCALE_FACTOR, scaledY, runWidth + xScaleOffset,
                                         SCALE_FACTOR + yScaleOffset);

                x = runEnd; // Advance x to skip this run
            }
            else // Clear empty pixels
            {
                uint8_t runStart = x, runEnd = x;

                // Find the end of the contiguous run without an active flag and with a dirty flag
                for (int i = x + 1; i < WIDTH && !activeFlags[IDX(i, y)] && dirtyFlags[IDX(i, y)];
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

                // Set color to the material shade
                gfx_SetColor(materialShades[0][0]);

                // Fill a single rectangle for the whole contiguous run
                gfx_FillRectangle_NoClip(runStart * SCALE_FACTOR, scaledY, runWidth + xScaleOffset,
                                         SCALE_FACTOR + yScaleOffset);

                x = runEnd; // Advance x to skip this run
            }
        }
    }

    // Draw Cursor
    gfx_SetColor(cursor.color);
    // A brush size of 2 otherwise causes the cursor to be off-center
    uint16_t cx = cursor.pos.x * cursor.size + cursor.size / (2 * brushSize) - (brushSize == 2);
    uint16_t cy =
        cursor.pos.y * cursor.size + GUI_HEIGHT + cursor.size / (2 * brushSize) - (brushSize == 2);
    gfx_HorizLine(cx - cursor.size * brushSize, cy, cursor.size * 2 * brushSize);
    gfx_VertLine(cx, cy - cursor.size * brushSize, cursor.size * 2 * brushSize);

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
    gfx_SetColor(materialShades[palette[cursor.paletteIndex]][0]);
    gfx_FillRectangle_NoClip(73, 3, 6, 6);
    gfx_SetTextXY(82, 3);

    switch (palette[cursor.paletteIndex])
    {
        case Material::Sand:
            gfx_PrintString("SAND");
            break;
        case Material::Water:
            gfx_PrintString("WATER");
            break;
        case Material::Stone:
            gfx_PrintString("STONE");
            break;
        case Material::Dirt:
            gfx_PrintString("DIRT");
            break;
        case Material::Acid:
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
    gfx_FillScreen(16);

    gfx_ScaledTransparentSprite_NoClip(title, GFX_LCD_WIDTH / 8, GFX_LCD_HEIGHT / 3, 4, 4);

    gfx_SetTextFGColor(255);
    gfx_SetTextBGColor(16);
    gfx_SetTextTransparentColor(0);
    gfx_SetTextScale(1, 1);
    gfx_PrintStringXY("Press Any Key", GFX_LCD_WIDTH / 5, 170);

    gfx_SwapDraw();

    while (!os_GetCSC())
    {
    }
}
