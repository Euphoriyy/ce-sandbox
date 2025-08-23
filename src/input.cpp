#include "../include/input.h"

#include "../include/draw.h"
#include "../include/pixels.h"
#include "../include/saves.h"
#include "../include/update.h"
#include "gfx/pinching_cursor.h"

#include <keypadc.h>
#include <string.h>

void handleInput(KeyState &keyState)
{
    if (kb_IsDown(kb_KeyClear))
    {
        memset(pixelData.pixels, 0, TOTAL_PIXELS);
        memset(pixelData.colors, 0, TOTAL_PIXELS);
        memset(pixelData.props, 0, TOTAL_PIXELS);
        memset(pixelData.activeRows, 0, HEIGHT);
        memset(pixelData.dirtyFlags, 0, TOTAL_PIXELS);
        memset(pixelData.dirtyRows, 0, HEIGHT);
        memset(pixelData.lastUpdate, 0, TOTAL_PIXELS * sizeof(uint24_t));
        memset(pixelData.lastUpdateByRow, 0, HEIGHT * sizeof(uint24_t));
        pixelData.activeCount = 0;
        timing.frame = 0;
        avatar.frameAtLastJump = 0;
        resetAvatar();
    }

    if (kb_IsDown(kb_KeyRight) && cursor.pos.x < WIDTH - 1)
    {
        if (!cursor.movedThisFrame)
        {
            clearCursor();
            cursor.prevPos = cursor.pos;
            cursor.movedThisFrame = true;
        }
        ++cursor.pos.x;
    }
    if (kb_IsDown(kb_KeyLeft) && cursor.pos.x > 0)
    {
        if (!cursor.movedThisFrame)
        {
            clearCursor();
            cursor.prevPos = cursor.pos;
            cursor.movedThisFrame = true;
        }
        --cursor.pos.x;
    }
    if (kb_IsDown(kb_KeyDown) && cursor.pos.y < HEIGHT - 1)
    {
        if (!cursor.movedThisFrame)
        {
            clearCursor();
            cursor.prevPos = cursor.pos;
            cursor.movedThisFrame = true;
        }
        ++cursor.pos.y;
    }
    if (kb_IsDown(kb_KeyUp) && cursor.pos.y > 0)
    {
        if (!cursor.movedThisFrame)
        {
            clearCursor();
            cursor.prevPos = cursor.pos;
            cursor.movedThisFrame = true;
        }
        --cursor.pos.y;
    }

    bool avatarMoved = false;

    // Move Avatar Right
    if (kb_IsDown(kb_KeyTan) && avatar.spawned)
    {
        uint8_t prevOrientation = avatar.orientation;
        uint24_t nx = avatar.pos.x;

        // Switch orientation to the right
        avatar.orientation = Avatar::Orientation::Right;

        // Check if avatar can move to the right
        if (avatar.pos.x < GFX_LCD_WIDTH - AVATAR_WIDTH && avatarCanMoveHorizontally(1) &&
            avatar.spriteState != Avatar::Sprite::Hanging)
        {
            if (avatar.pos.x & 3)
            {
                if (avatar.spriteState == Avatar::Sprite::Standing)
                {
                    avatar.spriteState = Avatar::Sprite::Walking;
                }
                else if (avatar.spriteState == Avatar::Sprite::Walking)
                {
                    avatar.spriteState = Avatar::Sprite::Standing;
                }
            }
            nx += avatar.speed;
            avatarMoved = true;
        }

        // Clear avatar and update its x position
        if (avatar.orientation != prevOrientation || avatarMoved)
        {
            clearAvatar();
            avatar.pos.x = nx;
        }
    }
    // Move Avatar Left
    if (kb_IsDown(kb_KeySin) && avatar.spawned)
    {
        uint8_t prevOrientation = avatar.orientation;
        uint24_t nx = avatar.pos.x;

        // Switch orientation to the left
        avatar.orientation = Avatar::Orientation::Left;

        // Check if avatar can move to the left
        if (avatar.pos.x > 0 && avatarCanMoveHorizontally(-1) &&
            avatar.spriteState != Avatar::Sprite::Hanging)
        {
            if (avatar.pos.x & 3)
            {
                if (avatar.spriteState == Avatar::Sprite::Standing)
                {
                    avatar.spriteState = Avatar::Sprite::Walking;
                }
                else if (avatar.spriteState == Avatar::Sprite::Walking)
                {
                    avatar.spriteState = Avatar::Sprite::Standing;
                }
            }
            nx -= avatar.speed;
            avatarMoved = true;
        }

        // Clear avatar and update its x position
        if (avatar.orientation != prevOrientation || avatarMoved)
        {
            clearAvatar();
            avatar.pos.x = nx;
        }
    }

    // Jump Avatar
    if (kb_IsDown(kb_KeyPrgm) && avatar.spawned &&
        avatar.pos.y > pixelData.divByScaleFactor[AVATAR_HEIGHT] && avatarIsGrounded() &&
        pixelData.activeCount && timing.frame - avatar.frameAtLastJump >= 10)
    {
        constexpr uint8_t jumpHeight = SCALE_FACTOR * 4;
        if (avatar.pos.y >= GUI_HEIGHT + jumpHeight)
        {
            clearAvatar();
            avatar.pos.y -= jumpHeight;
            avatar.frameAtLastJump = timing.frame;
            avatar.spriteState = Avatar::Sprite::Jumping;
            avatarMoved = true;
        }
    }

    // Reset Avatar Sprite When Still
    if ((!avatarMoved && avatar.spriteState == Avatar::Sprite::Walking) ||
        (avatarIsGrounded() && avatar.spriteState == Avatar::Sprite::Jumping))
    {
        clearAvatar();
        avatar.spriteState = Avatar::Sprite::Standing;
    }

    // Pinching
    if (!gameState.isDrawing && !gameState.isErasing && kb_IsDown(kb_KeyAlpha))
    {
        // Set cursor to pinching
        if (!cursor.pinching)
        {
            clearCursor();
            cursor.pinching = true;
        }

        // Pinching Avatar
        if (pixelData.activeCount && cursorIntersectingAvatar() && !IN_BOUNDS(cursor.pinchedPixel))
        {
            avatar.spriteState = Avatar::Sprite::Hanging;

            // Clamp cursor position within screen bounds
            if (cursor.pos.x > WIDTH - pixelData.divByScaleFactor[pinching_cursor_width])
            {
                cursor.pos.x = WIDTH - pixelData.divByScaleFactor[pinching_cursor_width];
            }
            if (cursor.pos.y < pixelData.divByScaleFactor[pinching_cursor_height])
            {
                cursor.pos.y = pixelData.divByScaleFactor[pinching_cursor_height];
            }
            else if (cursor.pos.y > HEIGHT - pixelData.divByScaleFactor[pinching_cursor_height] - 1)
            {
                cursor.pos.y = HEIGHT - pixelData.divByScaleFactor[pinching_cursor_height] - 1;
            }

            // Keep cursor above the avatar
            if (cursor.pos.y > pixelData.divByScaleFactor[avatar.pos.y])
            {
                cursor.pos.y = pixelData.divByScaleFactor[avatar.pos.y - 5];
            }

            Vector2_24 newPos;

            // Calculate the avatar's new position
            newPos.x = cursor.pos.x * SCALE_FACTOR;
            newPos.y = cursor.pos.y * SCALE_FACTOR + SCALE_FACTOR;

            if (avatar.pos.x != newPos.x || avatar.pos.y != newPos.y)
            {
                clearAvatar();
            }

            // Set new position coordinates if within bounds
            if (newPos.x >= 0 && newPos.x < GFX_LCD_WIDTH - AVATAR_WIDTH)
            {
                const uint8_t ORIENTATION_OFFSET = 5, DISTANCE_TO_SIDE = 25;
                if (avatar.orientation == Avatar::Orientation::Left &&
                    newPos.x - ORIENTATION_OFFSET >= 0 &&
                    newPos.x - ORIENTATION_OFFSET < GFX_LCD_WIDTH - DISTANCE_TO_SIDE)
                {
                    newPos.x -= ORIENTATION_OFFSET;
                }

                avatar.pos.x = newPos.x;
            }
            if (newPos.y >= 0 && newPos.y < GFX_LCD_HEIGHT - AVATAR_HEIGHT)
            {
                avatar.pos.y = newPos.y;
            }
        }
        else if (getPixel(cursor.prevPos.x, cursor.prevPos.y)) // Pinching Pixels
        {
            switchMat(cursor.pos.x, cursor.pos.y, cursor.prevPos.x, cursor.prevPos.y,
                      getPixel(cursor.pos.x, cursor.pos.y),
                      getPixel(cursor.prevPos.x, cursor.prevPos.y));
            // Marks the pinched pixel to ensure it does not update
            cursor.pinchedPixel = IDX(cursor.pos.x, cursor.pos.y);
        }
    }
    else if (cursor.pinching) // Set cursor to not pinching & release
    {
        clearCursor();
        cursor.pinching = false;

        if (avatar.spriteState == Avatar::Sprite::Hanging)
            avatar.spriteState = Avatar::Sprite::Standing;

        if (cursor.pinchedPixel < TOTAL_PIXELS)
        {
            pixelData.lastUpdate[cursor.pinchedPixel] = timing.frame;
            pixelData.lastUpdateByRow[cursor.pinchedPixel / WIDTH] = timing.frame;
            cursor.pinchedPixel = TOTAL_PIXELS;
        }
    }

    // Get Current Key State
    keyState.cur.enter = kb_IsDown(kb_KeyEnter);
    keyState.cur.del = kb_IsDown(kb_KeyDel);
    keyState.cur.yequ = kb_IsDown(kb_KeyYequ);
    keyState.cur.graph = kb_IsDown(kb_KeyGraph);
    keyState.cur.second = kb_IsDown(kb_Key2nd);
    keyState.cur.zoom = kb_IsDown(kb_KeyZoom);
    keyState.cur.window = kb_IsDown(kb_KeyWindow);
    keyState.cur.trace = kb_IsDown(kb_KeyTrace);
    keyState.cur.graphVar = kb_IsDown(kb_KeyGraphVar);
    keyState.cur.apps = kb_IsDown(kb_KeyApps);
    keyState.cur.sto = kb_IsDown(kb_KeySto);
    keyState.cur.on = kb_On;
    keyState.cur.ln = kb_IsDown(kb_KeyLn);
    keyState.cur.decPnt = kb_IsDown(kb_KeyDecPnt);
    keyState.cur.power = kb_IsDown(kb_KeyPower);

    // Toggle Drawing Mode
    if (keyState.cur.enter && !keyState.prev.enter)
    {
        if (cursor.spriteCursor)
            clearCursor();

        gameState.isDrawing = !gameState.isDrawing;
        if (gameState.isDrawing)
            gameState.isErasing = false;
    }

    // Toggle Erasing Mode
    if (keyState.cur.del && !keyState.prev.del)
    {
        if (cursor.spriteCursor)
            clearCursor();

        gameState.isErasing = !gameState.isErasing;
        if (gameState.isErasing)
            gameState.isDrawing = false;
    }

    // Toggle Circular Brush
    if (keyState.cur.graphVar && !keyState.prev.graphVar)
        gameState.circleBrush = !gameState.circleBrush;

    // Toggle Pausing
    if (keyState.cur.second && !keyState.prev.second)
        gameState.isPaused = !gameState.isPaused;

    // Toggle Floor
    if (keyState.cur.zoom && !keyState.prev.zoom)
    {
        gameState.enableFloor = !gameState.enableFloor;
        // Update the last row after disabling the floor
        if (!gameState.enableFloor && pixelData.activeRows[HEIGHT - 1])
        {
            pixelData.lastUpdateByRow[HEIGHT - 1] = timing.frame;
            uint24_t rowIdx = IDX(0, HEIGHT - 1);
#pragma unroll
            for (uint8_t x = 0; x < WIDTH; ++x)
            {
                pixelData.lastUpdate[rowIdx + x] = timing.frame;
            }
        }
    }

    // Increase Brush Size
    if (keyState.cur.trace && !keyState.prev.trace)
    {
        if (gameState.brushSize < 10)
        {
            clearCursor();
            ++gameState.brushSize;
        }
    }

    // Decrease Brush Size
    if (keyState.cur.window && !keyState.prev.window)
    {
        if (gameState.brushSize > 1)
        {
            clearCursor();
            --gameState.brushSize;
        }
    }

    // Switch Palette
    if (keyState.cur.yequ && !keyState.prev.yequ)
    {
        if (cursor.paletteIndex > 0)
            --cursor.paletteIndex;
        else
            cursor.paletteIndex = PALETTE_LEN - 1;
    }
    else if (keyState.cur.graph && !keyState.prev.graph)
    {
        if (cursor.paletteIndex < PALETTE_LEN - 1)
            ++cursor.paletteIndex;
        else
            cursor.paletteIndex = 0;
    }

    // Toggle Avatar Spawning
    if (keyState.cur.apps && !keyState.prev.apps)
    {
        avatar.spawned = !avatar.spawned;
        if (!avatar.spawned)
        {
            clearAvatar();
            resetAvatar();
        }
    }

    // Save Game
    if (keyState.cur.sto && !keyState.prev.sto)
    {
        writeSave("SANDSAVE");
    }

    // Load Game
    if (keyState.cur.on && !keyState.prev.on)
    {
        loadSave("SANDSAVE");
    }

    // Select Points
    if (keyState.cur.ln && !keyState.prev.ln && !gameState.isDrawing && !gameState.isErasing)
    {
        if (!pointIsSelected(0)) // Set the first point to the cursor's position if unselected
        {
            cursor.selectedPoints[0] = cursor.pos;
        }
        else if (!pointIsSelected(1)) // Set the second point to the cursor's position if unselected
        {
            cursor.selectedPoints[1] = cursor.pos;
        }
        else // Clear the selected points
        {
            clearPointOutline(cursor.selectedPoints[0]);
            clearPointOutline(cursor.selectedPoints[1]);
            cursor.selectedPoints[0] = cursor.selectedPoints[1] = {0xFF, 0xFF};
        }
    }

    // Cycle Cursor Styles
    if (keyState.cur.decPnt && !keyState.prev.decPnt)
    {
        clearCursor();

        if (!cursor.spriteCursor)
            cursor.spriteCursor = true;
        else if (!cursor.largeSprite)
            cursor.largeSprite = true;
        else
            cursor.spriteCursor = cursor.largeSprite = false;
    }

    // Pick Material
    if (keyState.cur.power && !keyState.prev.power)
    {
        uint8_t mat = pixelData.pixels[IDX(cursor.pos.x, cursor.pos.y)];
        if (mat)
        {
            for (uint8_t i = 0; i < PALETTE_LEN; ++i)
            {
                if (palette[i] == mat)
                    cursor.paletteIndex = i;
            }
        }
    }

    // Draw or Erase Based on Current Mode
    if (gameState.isDrawing)
    {
        if (pointIsSelected(0) && pointIsSelected(1))
        {
            // Draw a line
            drawLine(cursor.selectedPoints[0], cursor.selectedPoints[1], gameState.brushSize,
                     palette[cursor.paletteIndex]);
            gameState.isDrawing = false;
        }
        else if (gameState.brushSize == 1)
        {
            if (!getPixel(cursor.pos.x, cursor.pos.y))
                setPixel(cursor.pos.x, cursor.pos.y, palette[cursor.paletteIndex]);
        }
        else if (gameState.brushSize > 1)
        {
            if (gameState.circleBrush)
                drawCircle(cursor.pos, gameState.brushSize, palette[cursor.paletteIndex]);
            else
                drawSquare(cursor.pos, gameState.brushSize, palette[cursor.paletteIndex]);
        }
    }
    else if (gameState.isErasing)
    {
        if (pointIsSelected(0) && pointIsSelected(1))
        {
            // Erase a line
            drawLine(cursor.selectedPoints[0], cursor.selectedPoints[1], gameState.brushSize,
                     Material::Empty);
            gameState.isErasing = false;
        }
        else if (gameState.brushSize == 1)
        {
            if (getPixel(cursor.pos.x, cursor.pos.y))
                setPixel(cursor.pos.x, cursor.pos.y, Material::Empty);
        }
        else if (gameState.brushSize > 1)
        {
            if (gameState.circleBrush)
                drawCircle(cursor.pos, gameState.brushSize, Material::Empty);
            else
                drawSquare(cursor.pos, gameState.brushSize, Material::Empty);
        }
    }

    // Reset status of whether the cursor was moved
    cursor.movedThisFrame = false;
    // Set previous key state to current key state
    keyState.prev = keyState.cur;
}
