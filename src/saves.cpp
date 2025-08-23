#include "../include/saves.h"

#include "../include/pixels.h"

#include <fileioc.h>
#include <string.h>

void writeSave(const char *saveName)
{
    uint8_t appVarHandle = ti_Open(saveName, "w");
    if (!appVarHandle)
        return;

    // Write Game Data
    ti_Write(&gameState, 1, sizeof(gameState), appVarHandle);
    ti_Write(&timing.frame, sizeof(uint24_t), 1, appVarHandle);
    // Do not write the cursor's sprite pointers
    ti_Write(&cursor, 1, sizeof(cursor) - sizeof(cursor.sprites), appVarHandle);
    // Do not write the avatar's sprite pointers
    ti_Write(&avatar, 1, sizeof(avatar) - sizeof(avatar.sprites), appVarHandle);

    // Write Pixel Data
    ti_Write(pixelData.pixels, 1, TOTAL_PIXELS, appVarHandle);
    ti_Write(&pixelData.activeCount, sizeof(uint24_t), 1, appVarHandle);
    ti_Write(pixelData.activeRows, 1, HEIGHT, appVarHandle);
    ti_Write(pixelData.lastUpdate, sizeof(uint24_t), TOTAL_PIXELS, appVarHandle);
    ti_Write(pixelData.lastUpdateByRow, sizeof(uint24_t), HEIGHT, appVarHandle);

    ti_SetArchiveStatus(true, appVarHandle);
    ti_Close(appVarHandle);
}

void loadSave(const char *saveName)
{
    uint8_t appVarHandle = ti_Open(saveName, "r");
    if (!appVarHandle)
        return;

    // Load Game Data
    ti_Read(&gameState, 1, sizeof(gameState), appVarHandle);
    ti_Read(&timing.frame, sizeof(uint24_t), 1, appVarHandle);
    ti_Read(&cursor, 1, sizeof(cursor) - sizeof(cursor.sprites), appVarHandle);
    ti_Read(&avatar, 1, sizeof(avatar) - sizeof(avatar.sprites), appVarHandle);

    // Load Pixel Data
    ti_Read(pixelData.pixels, 1, TOTAL_PIXELS, appVarHandle);
    ti_Read(&pixelData.activeCount, sizeof(uint24_t), 1, appVarHandle);
    ti_Read(pixelData.activeRows, 1, HEIGHT, appVarHandle);
    ti_Read(pixelData.lastUpdate, sizeof(uint24_t), TOTAL_PIXELS, appVarHandle);
    ti_Read(pixelData.lastUpdateByRow, sizeof(uint24_t), HEIGHT, appVarHandle);

    ti_Close(appVarHandle);

    // Reset Rendering Data and Tool Mode
    memset(pixelData.dirtyFlags, 1, TOTAL_PIXELS);
    memset(pixelData.dirtyRows, 1, HEIGHT);
    gameState.isDrawing = gameState.isErasing = false;
}
