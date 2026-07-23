#pragma once
#include <vector>
#include <algorithm>
#include <SDL.h>
#include <cmath>
#include <cstdint>
#include <cstddef>

#include "math/Vec2.h"
#include "HealthSystem2D.h"
#include "Renderer.h"
#include "Camera2D.h"
#include "Entity2D.h"
#include "NavigationGrid2D.h"

class EnemySystem2D
{
public:
    struct Enemy
    {
        Entity2D entity{};
        HealthComponent2D health{};

        std::vector<Vec2> path{};
        std::size_t pathIndex = 0;

        double pathRecalculateTimer = 0.0;
    };

    void Clear()
    {
        enemies_.clear();
        ids_.Reset();
    }

    void AddEnemy(const SDL_Rect& rect, int hp = 1)
    {
        Enemy e;
        e.entity.position = Vec2{ static_cast<double>(rect.x), static_cast<double>(rect.y) };
        e.entity.id = ids_.Next();
        e.entity.velocity = Vec2{ 0.0, 0.0 };
        e.entity.bounds = rect;
        e.entity.active = true;
        e.health.SetMax(hp);
        e.entity.tag = EntityTag::Enemy;
        e.entity.layer = CollisionLayer2D::Enemy;
        e.entity.collisionMask = CollisionLayer2D::Player | CollisionLayer2D::Bullet | CollisionLayer2D::World;
        enemies_.push_back(e);
    }

    void SetCollisionWorld(
        int worldW,
        int worldH,
        const std::vector<SDL_Rect>* obstacles
    )
    {
        worldW_ = worldW;
        worldH_ = worldH;
        obstacles_ = obstacles;
    }

    void Render(Renderer& renderer, const Camera2D& cam) const
    {
        for (const auto& e : enemies_)
        {
            if (e.health.IsDead())
                continue;

            SDL_Rect r = cam.WorldToScreenRect(e.entity.bounds);

            // Enemy body
            renderer.SetDrawColor(120, 60, 200, 255);
            renderer.FillRect(r.x, r.y, r.w, r.h);

            // HP bar background
            renderer.SetDrawColor(40, 40, 40, 255);
            renderer.FillRect(r.x, r.y - 8, r.w, 4);

            // HP bar fill
            const double t =
                (e.health.maxHp > 0)
                ? static_cast<double>(e.health.hp) /
                static_cast<double>(e.health.maxHp)
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
            if (e.health.IsDead() || !e.entity.active)
                continue;

            const Vec2 previous = e.entity.position;
            const Vec2 desired =
                e.entity.position + e.entity.velocity * dtSeconds;

            const double maxX = std::max(
                0.0,
                static_cast<double>(worldW_ - e.entity.bounds.w)
            );

            const double maxY = std::max(
                0.0,
                static_cast<double>(worldH_ - e.entity.bounds.h)
            );

            // Try X-axis movement first.
            e.entity.position.x = std::clamp(
                desired.x,
                0.0,
                maxX
            );

            e.entity.bounds.x =
                static_cast<int>(std::round(e.entity.position.x));

            if (IsMovementBlocked(
                e.entity.bounds,
                e.entity.id
            ))
            {
                e.entity.position.x = previous.x;
                e.entity.bounds.x =
                    static_cast<int>(std::round(previous.x));
            }

            // Try Y-axis movement second.
            e.entity.position.y = std::clamp(
                desired.y,
                0.0,
                maxY
            );

            e.entity.bounds.y =
                static_cast<int>(std::round(e.entity.position.y));

            if (IsMovementBlocked(
                e.entity.bounds,
                e.entity.id
            ))
            {
                e.entity.position.y = previous.y;
                e.entity.bounds.y =
                    static_cast<int>(std::round(previous.y));
            }
        }
    }

    void SetVelocityTowardPoint(const Vec2& target, double speed)
    {
        for (auto& e : enemies_)
        {
            if (e.health.IsDead())
                continue;

            Vec2 center{
                e.entity.position.x + e.entity.bounds.w * 0.5,
                e.entity.position.y + e.entity.bounds.h * 0.5
            };

            Vec2 toTarget = target - center;

            if (toTarget.Length() <= 0.0001)
            {
                e.entity.velocity = Vec2{ 0.0, 0.0 };
                continue;
            }

            e.entity.velocity = toTarget.Normalized() * speed;
        }
    }

    void SetVelocityAlongPath(
        const NavigationGrid2D& navigationGrid,
        const Vec2& target,
        double speed,
        double dtSeconds
    )
    {
        constexpr double pathRecalculateInterval = 0.5;
        constexpr double waypointReachDistance = 10.0;

        for (auto& e : enemies_)
        {
            if (e.health.IsDead() || !e.entity.active)
                continue;

            e.pathRecalculateTimer -= dtSeconds;

            Vec2 center{
                e.entity.position.x +
                    e.entity.bounds.w * 0.5,

                e.entity.position.y +
                    e.entity.bounds.h * 0.5
            };

            // Recalculate periodically instead of running A* every frame.
            if (e.pathRecalculateTimer <= 0.0)
            {
                e.path = navigationGrid.FindPath(
                    center,
                    target
                );

                // The first point is normally the enemy's current cell.
                e.pathIndex =
                    e.path.size() > 1
                    ? 1
                    : 0;

                e.pathRecalculateTimer =
                    pathRecalculateInterval;
            }

            if (e.path.empty() ||
                e.pathIndex >= e.path.size())
            {
                e.entity.velocity = Vec2{ 0.0, 0.0 };
                continue;
            }

            Vec2 waypoint = e.path[e.pathIndex];
            Vec2 toWaypoint = waypoint - center;

            // Move on to the next point once the current point is reached.
            while (
                toWaypoint.Length() <= waypointReachDistance &&
                e.pathIndex + 1 < e.path.size()
                )
            {
                ++e.pathIndex;

                waypoint = e.path[e.pathIndex];
                toWaypoint = waypoint - center;
            }

            // Final waypoint reached.
            if (
                toWaypoint.Length() <= waypointReachDistance &&
                e.pathIndex + 1 >= e.path.size()
                )
            {
                e.entity.velocity = Vec2{ 0.0, 0.0 };
                continue;
            }

            if (toWaypoint.Length() <= 0.0001)
            {
                e.entity.velocity = Vec2{ 0.0, 0.0 };
                continue;
            }

            e.entity.velocity =
                toWaypoint.Normalized() * speed;
        }
    }

    bool HitAndDamageFirst(const SDL_Rect& bulletRect, int damage)
    {
        for (auto& e : enemies_)
        {
            if (e.health.IsDead())
                continue;

            if (SDL_HasIntersection(&bulletRect, &e.entity.bounds))
            {
                e.health.ApplyDamage(damage);
                return true;
            }
        }

        return false;
    }

    bool HitAndDamageFirst(const Entity2D& sourceEntity, int damage)
    {
        for (auto& e : enemies_)
        {
            if (e.health.IsDead())
                continue;

            if (!CanInteract(sourceEntity, e.entity))
                continue;

            if (SDL_HasIntersection(&sourceEntity.bounds, &e.entity.bounds))
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

            if (SDL_HasIntersection(&rect, &e.entity.bounds))
                return true;
        }

        return false;
    }

    bool AnyEnemyIntersects(const Entity2D& sourceEntity) const
    {
        for (const auto& e : enemies_)
        {
            if (e.health.IsDead())
                continue;

            if (!CanInteract(sourceEntity, e.entity))
                continue;

            if (SDL_HasIntersection(&sourceEntity.bounds, &e.entity.bounds))
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

            if (SDL_HasIntersection(&rect, &e.entity.bounds))
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
    EntityIdGenerator2D ids_;
    int worldW_ = 0;
    int worldH_ = 0;
    const std::vector<SDL_Rect>* obstacles_ = nullptr;

    bool IntersectsObstacle(const SDL_Rect& rect) const
    {
        if (!obstacles_)
            return false;

        for (const auto& obstacle : *obstacles_)
        {
            if (SDL_HasIntersection(&rect, &obstacle))
                return true;
        }

        return false;
    }

    bool IntersectsOtherEnemy(
        const SDL_Rect& rect,
        uint32_t ignoredEntityId
    ) const
    {
        for (const auto& other : enemies_)
        {
            if (other.entity.id == ignoredEntityId)
                continue;

            if (!other.entity.active || other.health.IsDead())
                continue;

            if (SDL_HasIntersection(&rect, &other.entity.bounds))
                return true;
        }

        return false;
    }

    bool IsMovementBlocked(
        const SDL_Rect& rect,
        uint32_t ignoredEntityId
    ) const
    {
        return IntersectsObstacle(rect) ||
            IntersectsOtherEnemy(rect, ignoredEntityId);
    }
};