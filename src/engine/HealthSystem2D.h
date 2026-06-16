#pragma once

struct HealthComponent2D
{
    int hp = 1;
    int maxHp = 1;

    bool IsAlive() const
    {
        return hp > 0;
    }

    bool IsDead() const
    {
        return hp <= 0;
    }

    void SetMax(int value)
    {
        maxHp = value;
        hp = value;
    }

    void ApplyDamage(int amount)
    {
        if (amount <= 0)
            return;

        hp -= amount;
        if (hp < 0)
            hp = 0;
    }

    void Heal(int amount)
    {
        if (amount <= 0)
            return;

        hp += amount;
        if (hp > maxHp)
            hp = maxHp;
    }
};