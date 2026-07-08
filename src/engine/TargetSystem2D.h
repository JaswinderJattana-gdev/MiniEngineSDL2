#pragma once
#include <vector>
#include <SDL.h>
#include <cmath>

#include "Renderer.h"
#include "Camera2D.h"
#include "HealthSystem2D.h"
#include "Entity2D.h"

class TargetSystem2D
{
public:
    struct Target
    {
        Entity2D entity{};
        HealthComponent2D health{};
    };

    void Clear()
    {
        targets_.clear();
        ids_.Reset();
    }

    void AddTarget(const SDL_Rect& rect, int hp = 1)
    {
        Target t;
        t.entity.position = Vec2{ static_cast<double>(rect.x), static_cast<double>(rect.y) };
        t.entity.id = ids_.Next();
        t.entity.velocity = Vec2{ 0.0, 0.0 };
        t.entity.bounds = rect;
        t.entity.active = true;
        t.health.SetMax(hp);
        t.entity.tag = EntityTag::Destructible;
        t.entity.layer = CollisionLayer2D::Destructible;
        t.entity.collisionMask = CollisionLayer2D::Bullet | CollisionLayer2D::World;
        targets_.push_back(t);
    }

    void Render(Renderer& renderer, const Camera2D& cam) const
    {
        renderer.SetDrawColor(200, 70, 70, 255);

        for (const auto& t : targets_)
        {
            if (t.health.IsDead())
                continue;

            SDL_Rect r = cam.WorldToScreenRect(t.entity.bounds);
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

            if (SDL_HasIntersection(&bulletRect, &t.entity.bounds))
            {
                t.health.ApplyDamage(damage);
                return true;
            }
        }

        return false;
    }

    bool HitAndDamageFirst(const Entity2D& sourceEntity, int damage)
    {
        for (auto& t : targets_)
        {
            if (t.health.IsDead())
                continue;

            if (!CanInteract(sourceEntity, t.entity))
                continue;

            if (SDL_HasIntersection(&sourceEntity.bounds, &t.entity.bounds))
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
    EntityIdGenerator2D ids_;
};