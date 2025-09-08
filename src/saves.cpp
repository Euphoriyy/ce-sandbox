#include "../include/saves.h"

#include "../include/pixels.h"
#include "gfx/global_palette.h"

#include <fileioc.h>
#include <graphx.h>
#include <string.h>
#include <ti/ui.h>

static uint8_t appVarHandle = 0;

static void beforeGC()
{
    if (appVarHandle)
    {
        ti_Close(appVarHandle);
        appVarHandle = 0;
    }

    // Exit graphics mode
    gfx_End();
    os_DrawStatusBar();
}

static void afterGC()
{
    // Reset graphics mode
    gfx_Begin();
    gfx_SetPalette(global_palette, sizeof_global_palette, 0);

    // Refresh pixels
    memset(pixelData.dirtyFlags, 1, TOTAL_PIXELS);
    memset(pixelData.dirtyRows, 1, HEIGHT);
}

// Macros for Handling Errors Writing/Reading

#define TI_WRITE_CHECK(ptr, size, count, handle)                                                   \
    do                                                                                             \
    {                                                                                              \
        size_t __written = ti_Write((ptr), (size), (count), (handle));                             \
        if (__written != (count))                                                                  \
        {                                                                                          \
            ti_Close(handle);                                                                      \
            return false;                                                                          \
        }                                                                                          \
    } while (0)

#define TI_READ_CHECK(ptr, size, count, handle)                                                    \
    do                                                                                             \
    {                                                                                              \
        size_t __read = ti_Read((ptr), (size), (count), (handle));                                 \
        if (__read != (count))                                                                     \
        {                                                                                          \
            ti_Close(handle);                                                                      \
            return false;                                                                          \
        }                                                                                          \
    } while (0)

bool writeSave(const char *saveName)
{
    // Set routines for before and after garbage collection
    ti_SetGCBehavior(beforeGC, afterGC);

    appVarHandle = ti_Open(saveName, "w");
    if (!appVarHandle)
        return false;

    // Write Game Data
    TI_WRITE_CHECK(&gameState, sizeof(gameState), 1, appVarHandle);
    TI_WRITE_CHECK(&timing.frame, sizeof(uint24_t), 1, appVarHandle);
    // Do not write the cursor's sprite pointers
    TI_WRITE_CHECK(&cursor, sizeof(cursor) - sizeof(cursor.sprites), 1, appVarHandle);
    // Do not write the avatar's sprite pointers
    TI_WRITE_CHECK(&avatar, sizeof(avatar) - sizeof(avatar.sprites), 1, appVarHandle);

    // Write Pixel Data
    TI_WRITE_CHECK(pixelData.pixels, 1, TOTAL_PIXELS, appVarHandle);
    TI_WRITE_CHECK(&pixelData.activeCount, sizeof(uint24_t), 1, appVarHandle);
    TI_WRITE_CHECK(pixelData.activeRows, 1, HEIGHT, appVarHandle);
    TI_WRITE_CHECK(pixelData.lastUpdate, sizeof(uint24_t), TOTAL_PIXELS, appVarHandle);
    TI_WRITE_CHECK(pixelData.lastUpdateByRow, sizeof(uint24_t), HEIGHT, appVarHandle);

    ti_SetArchiveStatus(true, appVarHandle);
    // Close handle if not already closed before garbage collection
    if (appVarHandle)
    {
        ti_Close(appVarHandle);
        appVarHandle = 0;
    }

    return true;
}

bool loadSave(const char *saveName)
{
    appVarHandle = ti_Open(saveName, "r");
    if (!appVarHandle)
        return false;

    // Load Game Data
    TI_READ_CHECK(&gameState, sizeof(gameState), 1, appVarHandle);
    TI_READ_CHECK(&timing.frame, sizeof(uint24_t), 1, appVarHandle);
    TI_READ_CHECK(&cursor, sizeof(cursor) - sizeof(cursor.sprites), 1, appVarHandle);
    TI_READ_CHECK(&avatar, sizeof(avatar) - sizeof(avatar.sprites), 1, appVarHandle);

    // Load Pixel Data
    TI_READ_CHECK(pixelData.pixels, 1, TOTAL_PIXELS, appVarHandle);
    TI_READ_CHECK(&pixelData.activeCount, sizeof(uint24_t), 1, appVarHandle);
    TI_READ_CHECK(pixelData.activeRows, 1, HEIGHT, appVarHandle);
    TI_READ_CHECK(pixelData.lastUpdate, sizeof(uint24_t), TOTAL_PIXELS, appVarHandle);
    TI_READ_CHECK(pixelData.lastUpdateByRow, sizeof(uint24_t), HEIGHT, appVarHandle);

    ti_Close(appVarHandle);
    appVarHandle = 0;

    // Reset Rendering Data and Tool Mode
    memset(pixelData.dirtyFlags, 1, TOTAL_PIXELS);
    memset(pixelData.dirtyRows, 1, HEIGHT);
    gameState.isDrawing = gameState.isErasing = false;

    return true;
}
