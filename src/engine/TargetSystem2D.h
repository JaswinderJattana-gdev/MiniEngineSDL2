#pragma once
#include <vector>
#include <SDL.h>

#include "Renderer.h"
#include "Camera2D.h"

class TargetSystem2D
{
public:
    struct Target
    {
        SDL_Rect rect{};
        bool alive = true;
    };

    void Clear()
    {
        targets_.clear();
    }

    void AddTarget(const SDL_Rect& rect)
    {
        targets_.push_back(Target{ rect, true });
    }

    void Render(Renderer& renderer, const Camera2D& cam) const
    {
        renderer.SetDrawColor(200, 70, 70, 255);

        for (const auto& t : targets_)
        {
            if (!t.alive)
                continue;

            SDL_Rect r = cam.WorldToScreenRect(t.rect);
            renderer.FillRect(r.x, r.y, r.w, r.h);
        }
    }

    // Returns true if bullet rect hit any living target.
    // If so, the first hit target is marked dead.
    bool HitAndRemoveFirst(const SDL_Rect& bulletRect)
    {
        for (auto& t : targets_)
        {
            if (!t.alive)
                continue;

            if (SDL_HasIntersection(&bulletRect, &t.rect))
            {
                t.alive = false;
                return true;
            }
        }
        return false;
    }

    void RemoveDead()
    {
        targets_.erase(
            std::remove_if(targets_.begin(), targets_.end(),
                [](const Target& t) { return !t.alive; }),
            targets_.end()
        );
    }

    const std::vector<Target>& Targets() const { return targets_; }
    std::vector<Target>& Targets() { return targets_; }

private:
    std::vector<Target> targets_;
};