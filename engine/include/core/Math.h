#pragma once

#include <cmath>
#include <cstddef>
#include <functional>

namespace subspace {

struct Vector3Int {
    int x = 0;
    int y = 0;
    int z = 0;

    constexpr Vector3Int() = default;
    constexpr Vector3Int(int x, int y, int z) : x(x), y(y), z(z) {}

    static constexpr Vector3Int Zero() { return {0, 0, 0}; }
    static constexpr Vector3Int One()  { return {1, 1, 1}; }

    static Vector3Int FloorFromFloat(float fx, float fy, float fz);

    constexpr bool operator==(const Vector3Int& o) const { return x == o.x && y == o.y && z == o.z; }
    constexpr bool operator!=(const Vector3Int& o) const { return !(*this == o); }

    constexpr Vector3Int operator+(const Vector3Int& o) const { return {x + o.x, y + o.y, z + o.z}; }
    constexpr Vector3Int operator-(const Vector3Int& o) const { return {x - o.x, y - o.y, z - o.z}; }

    // Strict weak ordering for use in std::map
    constexpr bool operator<(const Vector3Int& o) const {
        if (x != o.x) return x < o.x;
        if (y != o.y) return y < o.y;
        return z < o.z;
    }
};

struct Vector3 {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    constexpr Vector3() = default;
    constexpr Vector3(float x, float y, float z) : x(x), y(y), z(z) {}

    constexpr Vector3 operator+(const Vector3& o) const { return {x + o.x, y + o.y, z + o.z}; }
    constexpr Vector3 operator-(const Vector3& o) const { return {x - o.x, y - o.y, z - o.z}; }
    constexpr Vector3 operator*(float s)          const { return {x * s, y * s, z * s}; }

    float length() const;
    Vector3 normalized() const;
};

} // namespace subspace

// Hash specialization for Vector3Int (std::unordered_map support)
template<>
struct std::hash<subspace::Vector3Int> {
    std::size_t operator()(const subspace::Vector3Int& v) const noexcept {
        std::size_t h = 0;
        h ^= std::hash<int>{}(v.x) + 0x9e3779b9 + (h << 6) + (h >> 2);
        h ^= std::hash<int>{}(v.y) + 0x9e3779b9 + (h << 6) + (h >> 2);
        h ^= std::hash<int>{}(v.z) + 0x9e3779b9 + (h << 6) + (h >> 2);
        return h;
    }
};
