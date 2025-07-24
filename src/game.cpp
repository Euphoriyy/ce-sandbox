#include "../include/game.h"

Vector2_24 Avatar::defaultPos = {GFX_LCD_WIDTH / 2 - AVATAR_WIDTH / 2,
                                 GFX_LCD_HEIGHT - AVATAR_HEIGHT + 1};

Cursor cursor;
KeyState keyState;
GameState gameState;
Avatar avatar;

void initCursorSprites()
{
    gfx_TempSprite(largeBrush, brush_cursor_width << 1, brush_cursor_height << 1);
    gfx_TempSprite(largeEraser, eraser_cursor_width << 1, eraser_cursor_height << 1);
    gfx_TempSprite(largeHand, hand_cursor_width << 1, hand_cursor_height << 1);
    gfx_ScaleSprite(brush_cursor, largeBrush);
    gfx_ScaleSprite(eraser_cursor, largeEraser);
    gfx_ScaleSprite(hand_cursor, largeHand);

    cursor.sprites[0][0] = gfx_ConvertMallocRLETSprite(brush_cursor);
    cursor.sprites[1][0] = gfx_ConvertMallocRLETSprite(eraser_cursor);
    cursor.sprites[2][0] = gfx_ConvertMallocRLETSprite(hand_cursor);
    cursor.sprites[0][1] = gfx_ConvertMallocRLETSprite(largeBrush);
    cursor.sprites[1][1] = gfx_ConvertMallocRLETSprite(largeEraser);
    cursor.sprites[2][1] = gfx_ConvertMallocRLETSprite(largeHand);
}

void initAvatarSprites()
{
    gfx_TempSprite(flippedAvatar0, AVATAR_WIDTH, AVATAR_HEIGHT);
    gfx_TempSprite(flippedAvatar1, AVATAR_WIDTH, AVATAR_HEIGHT);
    gfx_TempSprite(flippedAvatar2, AVATAR_WIDTH, AVATAR_HEIGHT);
    gfx_FlipSpriteY(avatar0, flippedAvatar0);
    gfx_FlipSpriteY(avatar1, flippedAvatar1);
    gfx_FlipSpriteY(avatar2, flippedAvatar2);

    avatar.sprites[0][0] = gfx_ConvertMallocRLETSprite(avatar0);
    avatar.sprites[1][0] = gfx_ConvertMallocRLETSprite(avatar1);
    avatar.sprites[2][0] = gfx_ConvertMallocRLETSprite(avatar2);
    avatar.sprites[0][1] = gfx_ConvertMallocRLETSprite(flippedAvatar0);
    avatar.sprites[1][1] = gfx_ConvertMallocRLETSprite(flippedAvatar1);
    avatar.sprites[2][1] = gfx_ConvertMallocRLETSprite(flippedAvatar2);
}

void resetAvatar()
{
    avatar.pos = Avatar::defaultPos;
    avatar.spriteState = Avatar::Sprite::Standing;
}

void clearCursor()
{
    if (!pixelData.activeCount)
        return;

    if (!cursor.spriteCursor)
    {
        uint8_t offsetBrushSize = gameState.brushSize + 1;
        for (int8_t dx = -offsetBrushSize; dx < offsetBrushSize; ++dx)
        {
            makeDirty(cursor.pos.x + dx, cursor.pos.y);
        }
        for (int8_t dy = -offsetBrushSize; dy < offsetBrushSize; ++dy)
        {
            makeDirty(cursor.pos.x, cursor.pos.y + dy);
        }
        return;
    }

    uint8_t horizontalScaledSize, verticalScaledSize;

    // Scale to the size of the hand cursor (which is the maximum sprite size)
    if (cursor.largeSprite)
    {
        horizontalScaledSize = pixelData.divByScaleFactor[hand_cursor_width << 1];
        verticalScaledSize = pixelData.divByScaleFactor[hand_cursor_height << 1];
    }
    else
    {
        horizontalScaledSize = verticalScaledSize = pixelData.divByScaleFactor[hand_cursor_width];
    }

    for (int8_t dy = -verticalScaledSize; dy <= 1; ++dy)
    {
        for (int8_t dx = -1; dx < horizontalScaledSize + 1; ++dx)
        {
            makeDirty(cursor.pos.x + dx, cursor.pos.y + dy);
        }
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

void initSelectedPoints() { memset(cursor.selectedPoints, 0xFF, sizeof(cursor.selectedPoints)); }

void clearPointOutline(Vector2 pos)
{
    if (!pixelData.activeCount)
        return;

    uint24_t idx = IDX(pos.x, pos.y);
    uint24_t upIdx = idx - WIDTH;
    uint24_t downIdx = idx + WIDTH;

    // Clear Pixel Up
    if (IN_BOUNDS(upIdx))
    {
        pixelData.dirtyFlags[upIdx] = true;
        pixelData.dirtyRows[pos.y - 1] = true;
    }

    // Clear Pixel Up Left
    if (IN_BOUNDS(upIdx - 1))
    {
        pixelData.dirtyFlags[upIdx - 1] = true;
    }
    // Clear Pixel Up Right
    if (IN_BOUNDS(upIdx + 1))
    {
        pixelData.dirtyFlags[upIdx + 1] = true;
    }

    // Clear Pixel Left
    if (IN_BOUNDS(idx - 1))
    {
        pixelData.dirtyFlags[idx - 1] = true;
        pixelData.dirtyRows[pos.y] = true;
    }
    // Clear Pixel Right
    if (IN_BOUNDS(idx + 1))
    {
        pixelData.dirtyFlags[idx + 1] = true;
        pixelData.dirtyRows[pos.y] = true;
    }

    // Clear Pixel Down
    if (IN_BOUNDS(downIdx))
    {
        pixelData.dirtyFlags[downIdx] = true;
        pixelData.dirtyRows[pos.y + 1] = true;
    }
    else // Clear Pixel at Position
    {
        pixelData.dirtyFlags[idx] = true;
        pixelData.dirtyRows[pos.y] = true;
    }

    // Clear Pixel Down Left
    if (IN_BOUNDS(downIdx - 1))
    {
        pixelData.dirtyFlags[downIdx - 1] = true;
    }
    // Clear Pixel Down Right
    if (IN_BOUNDS(downIdx + 1))
    {
        pixelData.dirtyFlags[downIdx + 1] = true;
    }
}
