#pragma once
#include <SDL.h>
#include <algorithm>
#include <cmath>

#include "math/Vec2.h"

// Simple 2D camera: position = WORLD top-left of the view.
class Camera2D
{
public:
    void SetViewSize(int w, int h)
    {
        viewW_ = w;
        viewH_ = h;
    }

    void SetWorldSize(int w, int h)
    {
        worldW_ = w;
        worldH_ = h;
    }

    // Top-left position in world space
    void SetPosition(const Vec2& topLeftWorld)
    {
        pos_ = topLeftWorld;
    }

    const Vec2& Position() const { return pos_; }

    void CenterOn(const Vec2& worldPoint)
    {
        pos_.x = worldPoint.x - viewW_ * 0.5;
        pos_.y = worldPoint.y - viewH_ * 0.5;
    }

    void ClampToWorld()
    {
        // If world smaller than view, clamp to 0
        const double maxX = std::max(0, worldW_ - viewW_);
        const double maxY = std::max(0, worldH_ - viewH_);

        pos_.x = std::clamp(pos_.x, 0.0, static_cast<double>(maxX));
        pos_.y = std::clamp(pos_.y, 0.0, static_cast<double>(maxY));
    }

    // World -> Screen helpers (int rounding matches your current behavior)
    int WorldToScreenX(double worldX) const { return static_cast<int>(std::round(worldX - pos_.x)); }
    int WorldToScreenY(double worldY) const { return static_cast<int>(std::round(worldY - pos_.y)); }

    SDL_Rect WorldToScreenRect(const SDL_Rect& rWorld) const
    {
        return SDL_Rect{
            WorldToScreenX(rWorld.x),
            WorldToScreenY(rWorld.y),
            rWorld.w,
            rWorld.h
        };
    }

    SDL_Point WorldToScreenPoint(const SDL_Point& pWorld) const
    {
        return SDL_Point{
            WorldToScreenX(pWorld.x),
            WorldToScreenY(pWorld.y)
        };
    }

    // Screen -> World (very useful later for editor picking)
    SDL_Point ScreenToWorldPoint(const SDL_Point& pScreen) const
    {
        return SDL_Point{
            static_cast<int>(std::round(pScreen.x + pos_.x)),
            static_cast<int>(std::round(pScreen.y + pos_.y))
        };
    }

private:
    Vec2 pos_{ 0.0, 0.0 }; // top-left in world space
    int viewW_ = 0;
    int viewH_ = 0;
    int worldW_ = 0;
    int worldH_ = 0;
};
