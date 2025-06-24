#include "../include/game.h"

Vector2_24 Avatar::defaultPos = {GFX_LCD_WIDTH / 2 - AVATAR_WIDTH / 2,
                                 GFX_LCD_HEIGHT - AVATAR_HEIGHT + 1};

Cursor cursor;
KeyState keyState;
GameState gameState;
Avatar avatar;

void initAvatarSprites()
{
    gfx_TempSprite(flippedAvatar0, AVATAR_WIDTH, AVATAR_HEIGHT);
    gfx_TempSprite(flippedAvatar1, AVATAR_WIDTH, AVATAR_HEIGHT);
    gfx_FlipSpriteY(avatar0, flippedAvatar0);
    gfx_FlipSpriteY(avatar1, flippedAvatar1);

    avatar.sprite0 = gfx_ConvertMallocRLETSprite(avatar0);
    avatar.sprite1 = gfx_ConvertMallocRLETSprite(avatar1);
    avatar.flippedSprite0 = gfx_ConvertMallocRLETSprite(flippedAvatar0);
    avatar.flippedSprite1 = gfx_ConvertMallocRLETSprite(flippedAvatar1);
}

void resetAvatar()
{
    avatar.pos = Avatar::defaultPos;
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

void clearAvatar()
{
    if (!pixelData.activeCount)
        return;

    // Compute the scaled bounds of the sprite
    uint8_t left = pixelData.divByScaleFactor[avatar.pos.x];
    uint8_t right = pixelData.divByScaleFactor[avatar.pos.x + AVATAR_WIDTH + SCALE_FACTOR - 1];
    uint8_t top = pixelData.divByScaleFactor[avatar.pos.y - AVATAR_HEIGHT + SCALE_FACTOR];
    uint8_t bottom = pixelData.divByScaleFactor[avatar.pos.y + AVATAR_HEIGHT + SCALE_FACTOR - 1];

    // Clamp the top bound to 0
    if (top >= HEIGHT || top == bottom)
        top = 0;

    for (uint8_t y = top; y < bottom; ++y)
    {
        uint24_t rowIdx = IDX(0, y);
        if (IN_BOUNDS(rowIdx))
        {
            pixelData.dirtyRows[y] = true;

            for (uint8_t x = left; x < right; ++x)
            {
                if (IN_BOUNDS(rowIdx + x))
                {
                    pixelData.dirtyFlags[rowIdx + x] = true;
                }
            }
        }
    }
}

bool avatarCanMoveHorizontally(int8_t offset)
{
    uint8_t scaledX =
        pixelData.divByScaleFactor[avatar.pos.x + HALF_OF_AVATAR_WIDTH + offset * SCALE_FACTOR];
    uint8_t topY = pixelData.divByScaleFactor[avatar.pos.y];
    uint8_t bottomY =
        pixelData.divByScaleFactor[avatar.pos.y + HALF_OF_AVATAR_HEIGHT - SCALE_FACTOR];

    if (bottomY >= HEIGHT - 1)
        --bottomY;

    // Check all side pixels that could collide with the avatar from top to bottom
    for (uint8_t scaledY = topY; scaledY <= bottomY; ++scaledY)
    {
        uint8_t mat = getPixel(scaledX, scaledY);

        if (mat && isSolid(mat))
            return false;
    }
    return true;
}

bool avatarIsGrounded()
{
    uint8_t scaledX = pixelData.divByScaleFactor[avatar.pos.x + HALF_OF_AVATAR_WIDTH];
    uint8_t bottomY = pixelData.divByScaleFactor[avatar.pos.y + HALF_OF_AVATAR_HEIGHT];
    uint8_t mat = getPixel(scaledX, bottomY + 1);
    return (mat && isSolid(mat)) || bottomY >= HEIGHT - 1;
}
