#include <sys/rtc.h>
#include <sys/timers.h>

#include "../include/game.h"
#include "../include/input.h"
#include "../include/pixels.h"
#include "../include/render.h"
#include "../include/update.h"

int main(void)
{
    // Seed random with time
    srandom(rtc_Time());

    // Precalculate y offsets
    for (uint8_t y = 0; y < HEIGHT; ++y)
        pixelData.yOffsets[y] = y * WIDTH;

    // Precalculate division by the scale factor
    for (uint24_t i = 0; i < GFX_LCD_WIDTH; ++i)
        pixelData.divByScaleFactor[i] = i / SCALE_FACTOR;

    // Set cursor size
    cursor.size = SCALE_FACTOR;

    gfx_Begin();
    gfx_SetDrawBuffer();

    gfx_SetPalette(global_palette, sizeof_global_palette, 0);

    mainMenu();

    // Enable timer for frametime tracking
    timer_Enable(1, TIMER_32K, TIMER_NOINT, TIMER_UP);
    timer_Set(1, 0);
    uint24_t lastTick = timer_GetSafe(1, TIMER_UP), currentTick;

    precomputeBgColors();
    initAvatarSprites();

    while (!kb_IsDown(kb_KeyMode))
    {
        kb_Scan();
        handleInput();

        if (pixelData.activeCount && !gameState.isPaused)
        {
            update();
            ++timing.frame;
        }
        render();

        // Clear dirty arrays at the end of each frame
        memset(pixelData.dirtyFlags, 0, TOTAL_PIXELS);
        memset(pixelData.dirtyRows, 0, HEIGHT);

        currentTick = timer_GetSafe(1, TIMER_UP);
        timing.frametime = (currentTick - lastTick) * 1000 / 32768;
        lastTick = currentTick;
    }

    timer_Disable(1);
    gfx_End();

    return 0;
}
