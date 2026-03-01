#pragma once
#include <vector>
#include <SDL.h>
#include "math/Vec2.h"

// Very simple collision world: static AABB obstacles + world bounds.
// Uses axis-separated movement with revert-on-hit (matches your current behavior).
class CollisionWorld2D
{
public:
    void SetWorldSize(int w, int h) { worldW_ = w; worldH_ = h; }

    void ClearObstacles() { obstacles_.clear(); }
    void SetObstacles(const std::vector<SDL_Rect>& obs) { obstacles_ = obs; }
    const std::vector<SDL_Rect>& Obstacles() const { return obstacles_; }

    // Clamp a top-left position (of an object with size w/h) to stay inside the world.
    Vec2 ClampTopLeft(const Vec2& topLeft, int objW, int objH) const;

    // Axis-separated move with collision:
    // - prevPos: current top-left before moving
    // - desiredPos: where you'd like to go (already includes dt and speed)
    // - objW/objH: sprite size used for world clamp (matches your current code)
    // - feetRectFunc: callback that returns the FEET collider rect in world space for a given top-left
    // Returns: final top-left position after collisions.
    using FeetRectFunc = SDL_Rect(*)(const Vec2& topLeft, int objW, int objH);

    Vec2 MoveWithCollisions(
        const Vec2& prevPos,
        const Vec2& desiredPos,
        int objW, int objH,
        FeetRectFunc feetRectFunc
    ) const;

private:
    int worldW_ = 0;
    int worldH_ = 0;
    std::vector<SDL_Rect> obstacles_;
};