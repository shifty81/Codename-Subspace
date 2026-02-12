#pragma once

#include <algorithm>
#include <cstdint>
#include <string>

namespace subspace {

/// RGBA color with float components in [0, 1].
struct Color {
    float r = 1.0f;
    float g = 1.0f;
    float b = 1.0f;
    float a = 1.0f;

    constexpr Color() = default;
    constexpr Color(float r, float g, float b, float a = 1.0f)
        : r(r), g(g), b(b), a(a) {}

    constexpr bool operator==(const Color& o) const {
        return r == o.r && g == o.g && b == o.b && a == o.a;
    }
    constexpr bool operator!=(const Color& o) const { return !(*this == o); }

    /// Pack into 32-bit RGBA.
    uint32_t ToRGBA32() const {
        auto clamp = [](float v) -> uint8_t {
            return static_cast<uint8_t>(std::max(0.0f, std::min(1.0f, v)) * 255.0f);
        };
        return (static_cast<uint32_t>(clamp(r)) << 24) |
               (static_cast<uint32_t>(clamp(g)) << 16) |
               (static_cast<uint32_t>(clamp(b)) << 8)  |
               (static_cast<uint32_t>(clamp(a)));
    }

    // Common colors
    static constexpr Color White()       { return {1.0f, 1.0f, 1.0f, 1.0f}; }
    static constexpr Color Black()       { return {0.0f, 0.0f, 0.0f, 1.0f}; }
    static constexpr Color Red()         { return {1.0f, 0.0f, 0.0f, 1.0f}; }
    static constexpr Color Green()       { return {0.0f, 1.0f, 0.0f, 1.0f}; }
    static constexpr Color Blue()        { return {0.0f, 0.0f, 1.0f, 1.0f}; }
    static constexpr Color Yellow()      { return {1.0f, 1.0f, 0.0f, 1.0f}; }
    static constexpr Color Cyan()        { return {0.0f, 1.0f, 1.0f, 1.0f}; }
    static constexpr Color Transparent() { return {0.0f, 0.0f, 0.0f, 0.0f}; }
    static constexpr Color Gray()        { return {0.5f, 0.5f, 0.5f, 1.0f}; }
    static constexpr Color DarkGray()    { return {0.2f, 0.2f, 0.2f, 0.8f}; }

    /// Linearly interpolate between two colors.
    static Color Lerp(const Color& a, const Color& b, float t) {
        t = std::max(0.0f, std::min(1.0f, t));
        return {a.r + (b.r - a.r) * t,
                a.g + (b.g - a.g) * t,
                a.b + (b.b - a.b) * t,
                a.a + (b.a - a.a) * t};
    }
};

/// 2D vector for UI positions and sizes.
struct Vec2 {
    float x = 0.0f;
    float y = 0.0f;

    constexpr Vec2() = default;
    constexpr Vec2(float x, float y) : x(x), y(y) {}

    constexpr Vec2 operator+(const Vec2& o) const { return {x + o.x, y + o.y}; }
    constexpr Vec2 operator-(const Vec2& o) const { return {x - o.x, y - o.y}; }
    constexpr Vec2 operator*(float s)       const { return {x * s, y * s}; }
    constexpr bool operator==(const Vec2& o) const { return x == o.x && y == o.y; }
    constexpr bool operator!=(const Vec2& o) const { return !(*this == o); }
};

/// Axis-aligned rectangle (position is top-left corner).
struct Rect {
    float x = 0.0f;
    float y = 0.0f;
    float width  = 0.0f;
    float height = 0.0f;

    constexpr Rect() = default;
    constexpr Rect(float x, float y, float w, float h)
        : x(x), y(y), width(w), height(h) {}

    constexpr float Left()   const { return x; }
    constexpr float Top()    const { return y; }
    constexpr float Right()  const { return x + width; }
    constexpr float Bottom() const { return y + height; }

    constexpr Vec2 Position() const { return {x, y}; }
    constexpr Vec2 Size()     const { return {width, height}; }
    constexpr Vec2 Center()   const { return {x + width * 0.5f, y + height * 0.5f}; }

    /// Test whether a point lies inside the rectangle.
    constexpr bool Contains(float px, float py) const {
        return px >= x && px <= x + width && py >= y && py <= y + height;
    }

    constexpr bool Contains(const Vec2& p) const { return Contains(p.x, p.y); }
};

/// Anchor point for positioning UI elements relative to their parent.
enum class UIAnchor {
    TopLeft,
    TopCenter,
    TopRight,
    CenterLeft,
    Center,
    CenterRight,
    BottomLeft,
    BottomCenter,
    BottomRight
};

/// Types of draw commands emitted by the UI renderer.
enum class DrawCommandType {
    FilledRect,
    OutlineRect,
    Line,
    Text,
    Circle,
    FilledCircle
};

/// A single draw command in the render queue.
struct DrawCommand {
    DrawCommandType type = DrawCommandType::FilledRect;
    Rect rect;
    Color color;
    std::string text;      // For DrawCommandType::Text
    Vec2 p1, p2;           // For Line: start/end points; Circle: center + (radius, 0)
    float lineWidth = 1.0f;
    int fontSize = 14;
};

} // namespace subspace
