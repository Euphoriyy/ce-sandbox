#include "../include/game.h"

Cursor cursor;
Timing timing;
KeyState keyState;
GameState gameState;
Avatar avatar;

void initAvatarSprites() {
    gfx_TempSprite(flippedAvatar0, avatarWidth, avatarHeight);
    gfx_TempSprite(flippedAvatar1, avatarWidth, avatarHeight);
    gfx_FlipSpriteY(avatar0, flippedAvatar0);
    gfx_FlipSpriteY(avatar1, flippedAvatar1);
    
    avatar.sprite0 = gfx_ConvertMallocRLETSprite(avatar0);
    avatar.sprite1 = gfx_ConvertMallocRLETSprite(avatar1);
    avatar.flippedSprite0 = gfx_ConvertMallocRLETSprite(flippedAvatar0);
    avatar.flippedSprite1 = gfx_ConvertMallocRLETSprite(flippedAvatar1);
}

void resetAvatar() {
    avatar.pos = avatar.defaultPos;
    avatar.switchSprite = false;
}

void clearCursor()
{
    if (!pixelData.activeCount)
        return;

    uint8_t offsetBrushSize = gameState.brushSize + 1;
    for (int8_t dx = -offsetBrushSize; dx < offsetBrushSize; ++dx)
    {
        makeDirty(cursor.pos.x + dx, cursor.pos.y);
    }
    for (int8_t dy = -offsetBrushSize; dy < offsetBrushSize; ++dy)
    {
        makeDirty(cursor.pos.x, cursor.pos.y + dy);
    }
}

void clearAvatar() {
    if (!pixelData.activeCount)
        return;

    // Compute the scaled bounds of the sprite
    uint8_t left   = avatar.pos.x / SCALE_FACTOR;
    uint8_t top    = (avatar.pos.y - avatarHeight - SCALE_FACTOR + 1) / SCALE_FACTOR;
    uint8_t right  = (avatar.pos.x + avatarWidth + SCALE_FACTOR - 1) / SCALE_FACTOR;
    uint8_t bottom = (avatar.pos.y + avatarHeight + SCALE_FACTOR - 1) / SCALE_FACTOR;

    for (uint8_t y = top; y < bottom; ++y) {
        if (IN_BOUNDS(0, y)) {
            pixelData.dirtyRows[y] = true;

            for (uint8_t x = left; x < right; ++x) {
                if (IN_BOUNDS(x, y)) {
                    pixelData.dirtyFlags[IDX(x, y)] = true;
                }
            }
        }
    }
}
