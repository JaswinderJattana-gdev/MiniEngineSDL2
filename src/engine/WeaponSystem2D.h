#pragma once
#include <vector>

#include "math/Vec2.h"
#include "BulletSystem2D.h"

enum class WeaponFireMode
{
    AllMuzzles,
    AlternatingMuzzles
};

struct WeaponDefinition2D
{
    double fireInterval = 0.12;
    double bulletSpeed = 900.0;
    double bulletLife = 1.2;

    int bulletW = 8;
    int bulletH = 8;

    WeaponFireMode fireMode = WeaponFireMode::AlternatingMuzzles;
};

class WeaponInstance2D
{
public:
    void SetDefinition(const WeaponDefinition2D& def)
    {
        def_ = def;
    }

    void Reset()
    {
        cooldown_ = 0.0;
        nextMuzzle_ = 0;
    }

    void Update(double dtSeconds)
    {
        cooldown_ -= dtSeconds;
        if (cooldown_ < 0.0)
            cooldown_ = 0.0;
    }

    bool CanFire() const
    {
        return cooldown_ <= 0.0;
    }

    bool TryFire(
        BulletSystem2D& bullets,
        const std::vector<Vec2>& muzzleWorldPositions,
        const Vec2& fireDirNormalized
    );

private:
    WeaponDefinition2D def_{};
    double cooldown_ = 0.0;
    int nextMuzzle_ = 0;
};