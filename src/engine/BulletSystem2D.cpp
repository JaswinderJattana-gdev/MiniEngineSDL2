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
            b.entity.position += b.entity.velocity * dtSeconds;
            b.entity.bounds.x = static_cast<int>(std::round(b.entity.position.x));
            b.entity.bounds.y = static_cast<int>(std::round(b.entity.position.y));
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
        SDL_Rect r = cam.WorldToScreenRect(b.entity.bounds);
        renderer.FillRect(r.x, r.y, r.w, r.h);
    }
}

bool BulletSystem2D::IsOutOfWorld(const Bullet& b) const
{
    const SDL_Rect& r = b.entity.bounds;

    if (r.x + r.w < 0) return true;
    if (r.y + r.h < 0) return true;
    if (r.x > worldW_) return true;
    if (r.y > worldH_) return true;

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
    return b.entity.bounds;
}