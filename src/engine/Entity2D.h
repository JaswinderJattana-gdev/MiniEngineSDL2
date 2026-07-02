#pragma once

#include <cstdint>
#include <SDL.h>
#include "math/Vec2.h"

struct Entity2D
{
    uint32_t id = 0;

    Vec2 position{};
    Vec2 velocity{};

    SDL_Rect bounds{};

    bool active = true;
};