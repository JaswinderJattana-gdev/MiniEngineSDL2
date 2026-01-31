#pragma once
#include <cmath>

struct Vec2
{
    double x = 0.0;
    double y = 0.0;

    Vec2() = default;
    Vec2(double x_, double y_) : x(x_), y(y_) {}

    Vec2 operator+(const Vec2& r) const { return { x + r.x, y + r.y }; }
    Vec2 operator-(const Vec2& r) const { return { x - r.x, y - r.y }; }
    Vec2 operator*(double s) const { return { x * s, y * s }; }

    Vec2& operator+=(const Vec2& r) { x += r.x; y += r.y; return *this; }
    Vec2& operator*=(double s) { x *= s; y *= s; return *this; }

    double Length() const { return std::sqrt(x * x + y * y); }

    Vec2 Normalized() const
    {
        const double len = Length();
        if (len <= 0.0) return { 0.0, 0.0 };
        return { x / len, y / len };
    }
};
