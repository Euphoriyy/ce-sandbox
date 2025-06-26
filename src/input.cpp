#include "../include/input.h"

void handleInput()
{
    if (kb_IsDown(kb_KeyClear))
    {
        memset(pixelData.pixels, 0, TOTAL_PIXELS);
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
        clearCursor();
        ++cursor.pos.x;
    }
    if (kb_IsDown(kb_KeyLeft) && cursor.pos.x > 0)
    {
        clearCursor();
        --cursor.pos.x;
    }
    if (kb_IsDown(kb_KeyDown) && cursor.pos.y < HEIGHT - 1)
    {
        clearCursor();
        ++cursor.pos.y;
    }
    if (kb_IsDown(kb_KeyUp) && cursor.pos.y > 0)
    {
        clearCursor();
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
        if (avatar.pos.x < GFX_LCD_WIDTH - AVATAR_WIDTH && avatarCanMoveHorizontally(1))
        {
            if (avatar.pos.x & 3)
                avatar.switchSprite = !avatar.switchSprite;
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
        if (avatar.pos.x > 0 && avatarCanMoveHorizontally(-1))
        {
            if (avatar.pos.x & 3)
                avatar.switchSprite = !avatar.switchSprite;
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
            avatarMoved = true;
        }
    }

    // Reset Avatar Sprite When Still
    if (!avatarMoved && avatar.switchSprite)
    {
        clearAvatar();
        avatar.switchSprite = false;
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

    // Toggle Drawing Mode
    if (keyState.cur.enter && !keyState.prev.enter)
    {
        gameState.isDrawing = !gameState.isDrawing;
        if (gameState.isDrawing)
            gameState.isErasing = false;
    }

    // Toggle Erasing Mode
    if (keyState.cur.del && !keyState.prev.del)
    {
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
            cursor.paletteIndex = paletteLen - 1;
    }
    else if (keyState.cur.graph && !keyState.prev.graph)
    {
        if (cursor.paletteIndex < paletteLen - 1)
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

    // Draw or Erase Based on Current Mode
    if (gameState.isDrawing)
    {
        if (gameState.brushSize == 1)
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
        if (gameState.brushSize == 1)
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

    // Set Previous Key State to Current Key State
    keyState.prev = keyState.cur;
}
