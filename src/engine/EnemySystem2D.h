#pragma once
#include <vector>
#include <algorithm>
#include <SDL.h>

#include "HealthSystem2D.h"
#include "Renderer.h"
#include "Camera2D.h"

class EnemySystem2D
{
public:
    struct Enemy
    {
        SDL_Rect rect{};
        HealthComponent2D health{};
    };

    void Clear()
    {
        enemies_.clear();
    }

    void AddEnemy(const SDL_Rect& rect, int hp = 1)
    {
        Enemy e;
        e.rect = rect;
        e.health.SetMax(hp);
        enemies_.push_back(e);
    }

    void Render(Renderer& renderer, const Camera2D& cam) const
    {
        renderer.SetDrawColor(120, 60, 200, 255);

        for (const auto& e : enemies_)
        {
            if (e.health.IsDead())
                continue;

            SDL_Rect r = cam.WorldToScreenRect(e.rect);
            renderer.FillRect(r.x, r.y, r.w, r.h);
        }
    }

    bool HitAndDamageFirst(const SDL_Rect& bulletRect, int damage)
    {
        for (auto& e : enemies_)
        {
            if (e.health.IsDead())
                continue;

            if (SDL_HasIntersection(&bulletRect, &e.rect))
            {
                e.health.ApplyDamage(damage);
                return true;
            }
        }

        return false;
    }

    void RemoveDead()
    {
        enemies_.erase(
            std::remove_if(enemies_.begin(), enemies_.end(),
                [](const Enemy& e) { return e.health.IsDead(); }),
            enemies_.end()
        );
    }

    const std::vector<Enemy>& Enemies() const { return enemies_; }
    std::vector<Enemy>& Enemies() { return enemies_; }

private:
    std::vector<Enemy> enemies_;
};