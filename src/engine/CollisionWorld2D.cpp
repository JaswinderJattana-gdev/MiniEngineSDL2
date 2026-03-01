#include "CollisionWorld2D.h"
#include <algorithm>

Vec2 CollisionWorld2D::ClampTopLeft(const Vec2& topLeft, int objW, int objH) const
{
    Vec2 out = topLeft;
    out.x = std::clamp(out.x, 0.0, static_cast<double>(worldW_ - objW));
    out.y = std::clamp(out.y, 0.0, static_cast<double>(worldH_ - objH));
    return out;
}

Vec2 CollisionWorld2D::MoveWithCollisions(
    const Vec2& prevPos,
    const Vec2& desiredPos,
    int objW, int objH,
    FeetRectFunc feetRectFunc
) const
{
    Vec2 pos = prevPos;

    // --- X axis ---
    pos.x = desiredPos.x;
    pos = ClampTopLeft(pos, objW, objH);

    SDL_Rect feet = feetRectFunc(pos, objW, objH);
    for (const auto& o : obstacles_)
    {
        if (SDL_HasIntersection(&feet, &o))
        {
            pos.x = prevPos.x; // revert X
            break;
        }
    }

    // --- Y axis ---
    pos.y = desiredPos.y;
    pos = ClampTopLeft(pos, objW, objH);

    feet = feetRectFunc(pos, objW, objH);
    for (const auto& o : obstacles_)
    {
        if (SDL_HasIntersection(&feet, &o))
        {
            pos.y = prevPos.y; // revert Y
            break;
        }
    }

    return pos;
}