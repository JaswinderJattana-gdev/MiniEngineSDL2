#include "WeaponSystem2D.h"

bool WeaponInstance2D::TryFire(
    BulletSystem2D& bullets,
    const std::vector<Vec2>& muzzleWorldPositions,
    const Vec2& fireDirNormalized
)
{
    if (!CanFire())
        return false;

    if (muzzleWorldPositions.empty())
        return false;

    if (fireDirNormalized.x == 0.0 && fireDirNormalized.y == 0.0)
        return false;

    cooldown_ = def_.fireInterval;

    if (def_.fireMode == WeaponFireMode::AllMuzzles)
    {
        for (const Vec2& muzzle : muzzleWorldPositions)
        {
            bullets.Spawn(
                muzzle,
                fireDirNormalized,
                def_.bulletSpeed,
                def_.bulletLife,
                def_.bulletW,
                def_.bulletH
            );
        }
    }
    else
    {
        if (nextMuzzle_ < 0 || nextMuzzle_ >= static_cast<int>(muzzleWorldPositions.size()))
            nextMuzzle_ = 0;

        bullets.Spawn(
            muzzleWorldPositions[nextMuzzle_],
            fireDirNormalized,
            def_.bulletSpeed,
            def_.bulletLife,
            def_.bulletW,
            def_.bulletH
        );

        nextMuzzle_ = (nextMuzzle_ + 1) % static_cast<int>(muzzleWorldPositions.size());
    }

    return true;
}