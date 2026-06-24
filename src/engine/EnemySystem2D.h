#pragma once
#include <vector>
#include <algorithm>
#include <SDL.h>

#include "math/Vec2.h"
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

        Vec2 pos{};
        Vec2 vel{};
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
        e.pos.x = static_cast<double>(rect.x);
        e.pos.y = static_cast<double>(rect.y);
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

            // HP bar background
            renderer.SetDrawColor(40, 40, 40, 255);
            renderer.FillRect(r.x, r.y - 8, r.w, 4);

            // HP bar fill
            const double t =
                (e.health.maxHp > 0)
                ? static_cast<double>(e.health.hp) / static_cast<double>(e.health.maxHp)
                : 0.0;

            renderer.SetDrawColor(0, 220, 0, 255);
            renderer.FillRect(
                r.x,
                r.y - 8,
                static_cast<int>(r.w * t),
                4
            );
        }
    }

    void Update(double dtSeconds)
    {
        for (auto& e : enemies_)
        {
            if (e.health.IsDead())
                continue;

            e.pos += e.vel * dtSeconds;

            e.rect.x = static_cast<int>(std::round(e.pos.x));
            e.rect.y = static_cast<int>(std::round(e.pos.y));
        }
    }

    void SetVelocityTowardPoint(const Vec2& target, double speed)
    {
        for (auto& e : enemies_)
        {
            if (e.health.IsDead())
                continue;

            Vec2 center{
                e.pos.x + e.rect.w * 0.5,
                e.pos.y + e.rect.h * 0.5
            };

            Vec2 toTarget = target - center;

            if (toTarget.Length() <= 0.0001)
            {
                e.vel = Vec2{ 0.0, 0.0 };
                continue;
            }

            e.vel = toTarget.Normalized() * speed;
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

    bool AnyEnemyIntersects(const SDL_Rect& rect) const
    {
        for (const auto& e : enemies_)
        {
            if (e.health.IsDead())
                continue;

            if (SDL_HasIntersection(&rect, &e.rect))
                return true;
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

    bool CanPlaceEnemy(const SDL_Rect& rect, const std::vector<SDL_Rect>* obstacles = nullptr) const
    {
        if (obstacles)
        {
            for (const auto& o : *obstacles)
            {
                if (SDL_HasIntersection(&rect, &o))
                    return false;
            }
        }

        for (const auto& e : enemies_)
        {
            if (e.health.IsDead())
                continue;

            if (SDL_HasIntersection(&rect, &e.rect))
                return false;
        }

        return true;
    }

    bool TryAddEnemy(const SDL_Rect& rect, int hp = 1, const std::vector<SDL_Rect>* obstacles = nullptr)
    {
        if (!CanPlaceEnemy(rect, obstacles))
            return false;

        AddEnemy(rect, hp);
        return true;
    }

    int AliveCount() const
    {
        int count = 0;

        for (const auto& e : enemies_)
        {
            if (e.health.IsAlive())
                ++count;
        }

        return count;
    }

    bool AllDead() const
    {
        return AliveCount() == 0;
    }

    const std::vector<Enemy>& Enemies() const { return enemies_; }
    std::vector<Enemy>& Enemies() { return enemies_; }

private:
    std::vector<Enemy> enemies_;
};