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

class EntityIdGenerator2D
{
public:
    uint32_t Next()
    {
        return nextId_++;
    }

    void Reset(uint32_t start = 1)
    {
        nextId_ = start;
    }

private:
    uint32_t nextId_ = 1;
};