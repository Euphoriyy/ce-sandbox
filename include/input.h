#pragma once

#include <keypadc.h>
#include <string.h>

#include "draw.h"
#include "pixels.h"
#include "saves.h"
#include "update.h"

struct KeyState
{
    struct Keys
    {
        bool enter = false, del = false, yequ = false, graph = false, second = false, zoom = false,
             window = false, trace = false, graphVar = false, apps = false, sto = false, on = false,
             ln = false, decPnt = false, power = false;
    };
    Keys prev, cur;
};

void handleInput(KeyState &keyState);
