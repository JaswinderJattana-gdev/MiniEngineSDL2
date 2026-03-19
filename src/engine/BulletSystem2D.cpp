#include "BulletSystem2D.h"
#include <algorithm>
#include <cmath>

void BulletSystem2D::Update(double dtSeconds)
{
    for (auto& b : bullets_)
    {
        b.life -= dtSeconds;
        if (b.life > 0.0)
        {
            b.pos += b.vel * dtSeconds;
        }
    }

    bullets_.erase(
        std::remove_if(bullets_.begin(), bullets_.end(),
            [this](const Bullet& b)
            {
                if (b.life <= 0.0)
                    return true;

                if (IsOutOfWorld(b))
                    return true;

                if (HitsObstacle(b))
                    return true;

                return false;
            }),
        bullets_.end()
    );
}

void BulletSystem2D::Render(Renderer& renderer, const Camera2D& cam) const
{
    renderer.SetDrawColor(255, 220, 80, 255);

    for (const auto& b : bullets_)
    {
        const int sx = cam.WorldToScreenX(b.pos.x);
        const int sy = cam.WorldToScreenY(b.pos.y);
        renderer.FillRect(sx, sy, b.w, b.h);
    }
}

bool BulletSystem2D::IsOutOfWorld(const Bullet& b) const
{
    if (b.pos.x + b.w < 0.0) return true;
    if (b.pos.y + b.h < 0.0) return true;
    if (b.pos.x > static_cast<double>(worldW_)) return true;
    if (b.pos.y > static_cast<double>(worldH_)) return true;
    return false;
}

bool BulletSystem2D::HitsObstacle(const Bullet& b) const
{
    if (!obstacles_)
        return false;

    SDL_Rect br = BulletRect(b);

    for (const auto& o : *obstacles_)
    {
        if (SDL_HasIntersection(&br, &o))
            return true;
    }

    return false;
}

SDL_Rect BulletSystem2D::BulletRect(const Bullet& b) const
{
    return SDL_Rect{
        static_cast<int>(std::round(b.pos.x)),
        static_cast<int>(std::round(b.pos.y)),
        b.w,
        b.h
    };
}