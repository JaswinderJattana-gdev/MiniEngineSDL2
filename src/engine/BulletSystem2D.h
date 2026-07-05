#pragma once
#include <vector>
#include <SDL.h>
#include <cmath>

#include "math/Vec2.h"
#include "Renderer.h"
#include "Camera2D.h"
#include "Entity2D.h"

class BulletSystem2D
{
public:
    struct Bullet
    {
        Entity2D entity{};
        double life = 0.0;
        int damage = 1;
    };

    void Clear()
    {
        bullets_.clear();
        ids_.Reset();
    }

    void SetWorldSize(int w, int h)
    {
        worldW_ = w;
        worldH_ = h;
    }

    void SetObstacles(const std::vector<SDL_Rect>* obstacles)
    {
        obstacles_ = obstacles;
    }

    void Spawn(const Vec2& worldPos, const Vec2& dirNormalized, double speed, double lifeSeconds, int w = 8, int h = 8, int damage = 1)
    {
        Bullet b;
        b.entity.position = worldPos;
        b.entity.id = ids_.Next();
        b.entity.velocity = dirNormalized * speed;
        b.entity.bounds = SDL_Rect{
            static_cast<int>(std::round(worldPos.x)),
            static_cast<int>(std::round(worldPos.y)),
            w,
            h
        };
        b.entity.active = true;
        b.life = lifeSeconds;
        b.damage = damage;
        bullets_.push_back(b);
    }

    void Update(double dtSeconds);
    void Render(Renderer& renderer, const Camera2D& cam) const;

    const std::vector<Bullet>& Bullets() const { return bullets_; }
    std::vector<Bullet>& Bullets() { return bullets_; }

    SDL_Rect BulletRect(const Bullet& b) const;

private:
    bool IsOutOfWorld(const Bullet& b) const;
    bool HitsObstacle(const Bullet& b) const;

private:
    std::vector<Bullet> bullets_;
    EntityIdGenerator2D ids_;
    int worldW_ = 0;
    int worldH_ = 0;
    const std::vector<SDL_Rect>* obstacles_ = nullptr;
};