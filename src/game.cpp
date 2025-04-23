#include "../include/game.h"

Cursor cursor;
uint16_t frame = 0;
uint8_t frametime = 0;
KeyState keyState;
bool isDrawing = false, isErasing = false, isPaused = false, enableFloor = true;
uint8_t brushSize = 1;
