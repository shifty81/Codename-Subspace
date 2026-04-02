#pragma once

#include <cmath>

namespace subspace {

/// 2-D floating-point vector used by UI, input axes, and 2-D physics.
struct Vector2 {
    float x = 0.0f;
    float y = 0.0f;

    constexpr Vector2() = default;
    constexpr Vector2(float x, float y) : x(x), y(y) {}

    static constexpr Vector2 Zero()  { return {0.0f, 0.0f}; }
    static constexpr Vector2 One()   { return {1.0f, 1.0f}; }
    static constexpr Vector2 Right() { return {1.0f, 0.0f}; }
    static constexpr Vector2 Up()    { return {0.0f, 1.0f}; }

    constexpr Vector2 operator+(const Vector2& o) const { return {x + o.x, y + o.y}; }
    constexpr Vector2 operator-(const Vector2& o) const { return {x - o.x, y - o.y}; }
    constexpr Vector2 operator*(float s)          const { return {x * s,   y * s};   }
    constexpr Vector2 operator/(float s)          const { return {x / s,   y / s};   }
    constexpr Vector2& operator+=(const Vector2& o) { x += o.x; y += o.y; return *this; }
    constexpr Vector2& operator-=(const Vector2& o) { x -= o.x; y -= o.y; return *this; }
    constexpr Vector2& operator*=(float s)           { x *= s;   y *= s;   return *this; }
    constexpr bool operator==(const Vector2& o) const { return x == o.x && y == o.y; }
    constexpr bool operator!=(const Vector2& o) const { return !(*this == o); }

    float Length()        const { return std::sqrt(x * x + y * y); }
    float LengthSquared() const { return x * x + y * y; }

    Vector2 Normalized() const {
        float len = Length();
        return len > 1e-6f ? Vector2{x / len, y / len} : Vector2{};
    }

    static constexpr float Dot(const Vector2& a, const Vector2& b) {
        return a.x * b.x + a.y * b.y;
    }

    /// Signed perpendicular (cross) product (scalar z component of 3-D cross).
    static constexpr float Cross(const Vector2& a, const Vector2& b) {
        return a.x * b.y - a.y * b.x;
    }

    static float Distance(const Vector2& a, const Vector2& b) {
        return (a - b).Length();
    }
};

} // namespace subspace
