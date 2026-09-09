#include "core/Math.h"

#include <cmath>

namespace subspace {

static constexpr float kMinVectorLength = 1e-8f;

Vector3Int Vector3Int::FloorFromFloat(float fx, float fy, float fz) {
    return {
        static_cast<int>(std::floor(fx)),
        static_cast<int>(std::floor(fy)),
        static_cast<int>(std::floor(fz))
    };
}

float Vector3::length() const {
    return std::sqrt(x * x + y * y + z * z);
}

Vector3 Vector3::normalized() const {
    float len = length();
    if (len < kMinVectorLength) {
        return {0.0f, 0.0f, 0.0f};
    }
    return {x / len, y / len, z / len};
}

} // namespace subspace
