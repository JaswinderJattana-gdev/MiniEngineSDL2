#pragma once
#include <vector>
#include <SDL.h>

#include "Renderer.h"
#include "Camera2D.h"
#include "HealthSystem2D.h"

class TargetSystem2D
{
public:
    struct Target
    {
        SDL_Rect rect{};
        HealthComponent2D health{};
    };

    void Clear()
    {
        targets_.clear();
    }

    void AddTarget(const SDL_Rect& rect, int hp = 1)
    {
        Target t;
        t.rect = rect;
        t.health.SetMax(hp);
        targets_.push_back(t);
    }

    void Render(Renderer& renderer, const Camera2D& cam) const
    {
        renderer.SetDrawColor(200, 70, 70, 255);

        for (const auto& t : targets_)
        {
            if (t.health.IsDead())
                continue;

            SDL_Rect r = cam.WorldToScreenRect(t.rect);
            renderer.FillRect(r.x, r.y, r.w, r.h);
        }
    }

    // Returns true if bullet rect hit any living target.
    // If so, the first hit target is marked dead.
    bool HitAndDamageFirst(const SDL_Rect& bulletRect, int damage)
    {
        for (auto& t : targets_)
        {
            if (t.health.IsDead())
                continue;

            if (SDL_HasIntersection(&bulletRect, &t.rect))
            {
                t.health.ApplyDamage(damage);
                return true;
            }
        }

        return false;
    }

    void RemoveDead()
    {
        targets_.erase(
            std::remove_if(targets_.begin(), targets_.end(),
                [](const Target& t) { return t.health.IsDead(); }),
            targets_.end()
        );
    }

    const std::vector<Target>& Targets() const { return targets_; }
    std::vector<Target>& Targets() { return targets_; }

private:
    std::vector<Target> targets_;
};