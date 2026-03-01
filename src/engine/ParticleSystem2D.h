#pragma once
#include <vector>
#include <algorithm>

#include "math/Vec2.h"
#include "Renderer.h"
#include "Camera2D.h"

class ParticleSystem2D
{
public:
    struct Particle
    {
        Vec2 pos;
        Vec2 vel;
        double life = 0.0;
        double maxLife = 0.0;
        int size = 2; // pixels
    };

    void Clear()
    {
        particles_.clear();
        spawnAcc_ = 0.0;
        seq_ = 0;
    }

    // Call every frame
    void Update(double dtSeconds)
    {
        // Update particles
        for (auto& p : particles_)
        {
            p.life -= dtSeconds;
            if (p.life > 0.0)
            {
                p.pos += p.vel * dtSeconds;
                p.vel *= 0.85; // damp (kept same as your DemoScene)
            }
        }

        // Remove dead
        particles_.erase(
            std::remove_if(particles_.begin(), particles_.end(),
                [](const Particle& p) { return p.life <= 0.0; }),
            particles_.end()
        );
    }

    // Render in world space (camera converts to screen)
    void RenderSquares(Renderer& renderer, const Camera2D& cam) const
    {
        renderer.SetDrawColor(120, 120, 120, 255);
        for (const auto& p : particles_)
        {
            const int px = cam.WorldToScreenX(p.pos.x);
            const int py = cam.WorldToScreenY(p.pos.y);
            renderer.FillRect(px, py, p.size, p.size);
        }
    }

    // Dust helper: exactly reproduces your current spawn behavior.
    // feetWorld: feet collider rect in WORLD space
    // dtSeconds: frame dt
    // spawnRatePerSec: e.g. 30.0
    void SpawnDustFromFeet(const SDL_Rect& feetWorld, double dtSeconds, double spawnRatePerSec = 30.0)
    {
        // Accumulate time and spawn at fixed rate
        spawnAcc_ += dtSeconds;
        const double spawnInterval = 1.0 / spawnRatePerSec;

        while (spawnAcc_ >= spawnInterval)
        {
            spawnAcc_ -= spawnInterval;

            const double fx = feetWorld.x + feetWorld.w * 0.5;
            const double fy = feetWorld.y + feetWorld.h - 30; // same offset as before

            Particle p;
            p.pos = Vec2{ fx, fy };

            // deterministic "random-ish" spread: -1.5, 0, +1.5 repeating
            const int m = (seq_++ % 3);
            const double dir = (m == 0) ? -1.5 : (m == 1) ? 0.0 : 1.5;

            p.vel = Vec2{ 30.0 * dir, 20.0 };
            p.maxLife = p.life = 0.25;
            p.size = 3;

            particles_.push_back(p);
        }
    }

    // If you stop moving, call this to reset accumulator (matches your previous behavior)
    void ResetSpawnAccumulator()
    {
        spawnAcc_ = 0.0;
    }

private:
    std::vector<Particle> particles_;
    double spawnAcc_ = 0.0;
    int seq_ = 0; // deterministic spread counter
};
