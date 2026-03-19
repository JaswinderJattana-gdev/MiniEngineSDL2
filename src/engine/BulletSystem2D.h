#pragma once
#include <vector>
#include <SDL.h>

#include "math/Vec2.h"
#include "Renderer.h"
#include "Camera2D.h"

class BulletSystem2D
{
public:
    struct Bullet
    {
        Vec2 pos;
        Vec2 vel;
        double life = 0.0;
        int w = 8;
        int h = 8;
    };

    void Clear()
    {
        bullets_.clear();
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

    void Spawn(const Vec2& worldPos, const Vec2& dirNormalized, double speed, double lifeSeconds, int w = 8, int h = 8)
    {
        Bullet b;
        b.pos = worldPos;
        b.vel = dirNormalized * speed;
        b.life = lifeSeconds;
        b.w = w;
        b.h = h;
        bullets_.push_back(b);
    }

    void Update(double dtSeconds);
    void Render(Renderer& renderer, const Camera2D& cam) const;

    const std::vector<Bullet>& Bullets() const { return bullets_; }

private:
    bool IsOutOfWorld(const Bullet& b) const;
    bool HitsObstacle(const Bullet& b) const;
    SDL_Rect BulletRect(const Bullet& b) const;

private:
    std::vector<Bullet> bullets_;
    int worldW_ = 0;
    int worldH_ = 0;
    const std::vector<SDL_Rect>* obstacles_ = nullptr;
};