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
        yOffsets[y] = y * WIDTH;

    // Set cursor size
    cursor.size = SCALE_FACTOR;

    gfx_Begin();
    gfx_SetDrawBuffer();

    gfx_SetPalette(global_palette, sizeof_global_palette, 0);

    mainMenu();

    // Enable timer for frametime tracking
    timer_Enable(1, TIMER_32K, TIMER_NOINT, TIMER_UP);
    timer_Set(1, 0);
    uint16_t lastTick = timer_GetSafe(1, TIMER_UP), currentTick;

    while (!kb_IsDown(kb_KeyMode))
    {
        kb_Scan();
        handleInput();

        if (activeCount && !isPaused)
        {
            update();
            ++frame;
        }
        render();

        currentTick = timer_GetSafe(1, TIMER_UP);
        frametime = (currentTick - lastTick) * 1000 / 32768;
        lastTick = currentTick;
    };

    timer_Disable(1);
    gfx_End();

    return 0;
}
