#pragma once

#include <cstdint>
#include <SDL.h>
#include "math/Vec2.h"

enum class EntityTag
{
    None,

    Player,
    Enemy,
    Bullet,
    Destructible,

    Pickup,

    Trigger,

    Decoration
};

enum class CollisionLayer2D : uint32_t
{
    None = 0,
    Player = 1 << 0,
    Enemy = 1 << 1,
    Bullet = 1 << 2,
    World = 1 << 3,
    Destructible = 1 << 4,
    Pickup = 1 << 5,
    Trigger = 1 << 6
};

inline CollisionLayer2D operator|(CollisionLayer2D a, CollisionLayer2D b)
{
    return static_cast<CollisionLayer2D>(
        static_cast<uint32_t>(a) | static_cast<uint32_t>(b)
        );
}

inline bool HasLayer(CollisionLayer2D mask, CollisionLayer2D layer)
{
    return (static_cast<uint32_t>(mask) & static_cast<uint32_t>(layer)) != 0;
}

struct Entity2D
{
    uint32_t id = 0;

    Vec2 position{};
    Vec2 velocity{};

    SDL_Rect bounds{};

    bool active = true;

    EntityTag tag = EntityTag::None;

    CollisionLayer2D layer = CollisionLayer2D::None;
    CollisionLayer2D collisionMask = CollisionLayer2D::None;
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