#include "../include/render.h"

constexpr bool isImageBackground() { return materialShades[0][1]; }
constexpr bool isBlackBackground() { return !materialShades[0][0]; }

void render()
{
    // If empty, draw background
    if (!pixelData.activeCount)
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

    if (isImageBackground() && timing.frame == 1)
    {
        memset(pixelData.dirtyFlags, 1, TOTAL_PIXELS);
        memset(pixelData.dirtyRows, 1, HEIGHT);
    }

    // If empty, skip the pixel rendering pass
    if (!pixelData.activeCount)
        goto NONPIXEL_RENDERING;

    // Draw Pixels
#pragma unroll
    for (uint8_t y = 0; y < HEIGHT; ++y)
    {
        // Skip static rows
        if (!pixelData.dirtyRows[y])
            continue;

        uint8_t scaledY = y * SCALE_FACTOR + GUI_HEIGHT;
        uint8_t yScaleOffset = y == HEIGHT - 1 ? gcd(GFX_LCD_HEIGHT, SCALE_FACTOR) : 0;
        yScaleOffset -= yScaleOffset / 2; // Avoid overflowing issues
        uint24_t rowIdx = IDX(0, y);

        for (uint8_t x = 0; x < WIDTH; ++x)
        {
            // Do not draw if pixel is not flagged as dirty
            if (!pixelData.dirtyFlags[rowIdx + x])
                continue;

            uint8_t mat = getPixel(x, y);

            if (mat) // Fill non-empty pixels
            {
                uint8_t runStart = x, runEnd = x;

                // Find the end of the contiguous run with the same mat and a dirty flag
                for (uint8_t i = x + 1; i < WIDTH && pixelData.pixels[rowIdx + i] == mat &&
                                        pixelData.dirtyFlags[rowIdx + i];
                     ++i)
                {
                    runEnd = i;
                }

                uint8_t runLength = runEnd - runStart + 1;
                uint24_t runWidth = runLength * SCALE_FACTOR;

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
            else if (isImageBackground()) // Replace empty pixels with image background
            {
                uint8_t xScaleOffset =
                    (x == WIDTH - 1 && SCALE_FACTOR != gcd(GFX_LCD_WIDTH, SCALE_FACTOR))
                        ? gcd(GFX_LCD_WIDTH, SCALE_FACTOR)
                        : 0;
                uint8_t color = pixelData.bgColorCells[rowIdx + x];
                gfx_SetColor(color);
                gfx_FillRectangle_NoClip(x * SCALE_FACTOR, scaledY, SCALE_FACTOR + xScaleOffset,
                                         SCALE_FACTOR + yScaleOffset);
            }
            else // Clear empty pixels
            {
                uint8_t runStart = x, runEnd = x;

                // Find the end of the contiguous run that is inactive and has a dirty flag
                for (uint8_t i = x + 1;
                     i < WIDTH && !pixelData.pixels[rowIdx + i] && pixelData.dirtyFlags[rowIdx + i];
                     ++i)
                {
                    runEnd = i;
                }

                uint8_t runLength = runEnd - runStart + 1;
                uint24_t runWidth = runLength * SCALE_FACTOR;

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

NONPIXEL_RENDERING:
    if (avatar.spawned)
        drawAvatar();

    drawCursor();

    drawGUI();

    // --------------

    gfx_SwapDraw();
}

void drawAvatar()
{
    gfx_rletsprite_t *sprite;
    if (avatar.orientation == Avatar::Orientation::Right)
        if (!avatar.switchSprite)
            sprite = avatar.sprite0;
        else
            sprite = avatar.sprite1;
    else if (!avatar.switchSprite)
        sprite = avatar.flippedSprite0;
    else
        sprite = avatar.flippedSprite1;
    gfx_RLETSprite(sprite, avatar.pos.x, avatar.pos.y);
}

void drawCursor()
{
    gfx_SetColor(cursor.color);
    // A brush size of 2 otherwise causes the cursor to be off-center
    uint24_t cx = cursor.pos.x * cursor.size + cursor.size / (2 * gameState.brushSize) -
                  (gameState.brushSize == 2);
    uint24_t cy = cursor.pos.y * cursor.size + GUI_HEIGHT +
                  cursor.size / (2 * gameState.brushSize) - (gameState.brushSize == 2);
    gfx_HorizLine(cx - cursor.size * gameState.brushSize, cy,
                  cursor.size * 2 * gameState.brushSize);
    gfx_VertLine(cx, cy - cursor.size * gameState.brushSize, cursor.size * 2 * gameState.brushSize);
}

void drawGUI()
{
    constexpr uint8_t GUI_BG_COLOR = 57, GUI_FG_COLOR = 255, GUI_ACCENT_COLOR = 0;
    gfx_SetColor(GUI_BG_COLOR);
    gfx_FillRectangle_NoClip(0, 0, GFX_LCD_WIDTH, GUI_HEIGHT);

    gfx_SetTextFGColor(GUI_FG_COLOR);
    gfx_SetTextBGColor(GUI_BG_COLOR);
    gfx_SetTextTransparentColor(0);
    gfx_SetTextXY(10, 3);
    gfx_SetTextScale(1, 1);

    if (gameState.isDrawing)
        gfx_PrintString("DRAW");
    else if (gameState.isErasing)
        gfx_PrintString("ERASE");
    else
        gfx_PrintString("IDLE");

    if (gameState.isPaused)
    {
        gfx_SetColor(GUI_FG_COLOR);
        gfx_FillRectangle_NoClip(53, 3, 3, 7);
        gfx_FillRectangle_NoClip(58, 3, 3, 7);
    }

    gfx_SetColor(GUI_ACCENT_COLOR);
    if (gameState.circleBrush)
    {
        gfx_Circle_NoClip(75, 6, 4);
        gfx_SetColor(materialShades[palette[cursor.paletteIndex]][0]);
        gfx_FillCircle_NoClip(75, 6, 3);
    }
    else
    {
        gfx_Rectangle_NoClip(72, 2, 8, 8);
        gfx_SetColor(materialShades[palette[cursor.paletteIndex]][0]);
        gfx_FillRectangle_NoClip(73, 3, 6, 6);
    }
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
        case Material::Steam:
            gfx_PrintString("STEAM");
            break;
    }

    gfx_PrintStringXY("SZ:", 135, 3);
    gfx_SetTextXY(161, 3);
    gfx_PrintUInt(gameState.brushSize, 1);

    gfx_PrintStringXY("FT:", 190, 3);
    gfx_SetTextXY(218, 3);
    gfx_PrintUInt(timing.frametime, 2);

    gfx_PrintStringXY("PX:", 250, 3);
    gfx_SetTextXY(281, 3);
    gfx_PrintUInt(pixelData.activeCount, 1);
}

void mainMenu()
{
    gfx_FillScreen(56);

    gfx_ScaledTransparentSprite_NoClip(title, GFX_LCD_WIDTH / 8, GFX_LCD_HEIGHT / 3, 4, 4);

    gfx_SetTextFGColor(255);
    gfx_SetTextBGColor(56);
    gfx_SetTextTransparentColor(0);
    gfx_SetTextScale(1, 1);
    gfx_PrintStringXY("Press Any Key", GFX_LCD_WIDTH / 5, 170);

    gfx_SwapDraw();

    while (!kb_AnyKey())
    {
    }
}

void precomputeBgColors()
{
#pragma unroll
    for (uint8_t y = 0; y < HEIGHT; ++y)
    {
        // Sample at pixel center
        uint24_t screenY = y * SCALE_FACTOR + SCALE_FACTOR / 2 + GUI_HEIGHT;
        // Add rounding for better accuracy
        uint8_t bgY = (screenY * background_1_height + GFX_LCD_HEIGHT / 2) / GFX_LCD_HEIGHT;
        if (bgY >= background_1_height)
            bgY = background_1_height - 1;

        uint24_t rowIdx = IDX(0, y);
#pragma unroll
        for (uint8_t x = 0; x < WIDTH; ++x)
        {
            uint24_t screenX = x * SCALE_FACTOR + SCALE_FACTOR / 2;
            uint8_t bgX = (screenX * background_1_width + GFX_LCD_WIDTH / 2) / GFX_LCD_WIDTH;
            if (bgX >= background_1_width)
                bgX = background_1_width - 1;

            uint8_t color = background_1_data[bgY * background_1_width + bgX];
            pixelData.bgColorCells[rowIdx + x] = color;
        }
    }
}
