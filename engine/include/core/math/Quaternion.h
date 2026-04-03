#pragma once

#include "core/Math.h"
#include "core/math/Matrix4.h"

#include <cmath>

namespace subspace {

/// Unit quaternion representing a 3-D rotation.
/// Convention: q = w + xi + yj + zk.
struct Quaternion {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float w = 1.0f;

    constexpr Quaternion() = default;
    constexpr Quaternion(float x, float y, float z, float w)
        : x(x), y(y), z(z), w(w) {}

    static constexpr Quaternion Identity() { return {0.0f, 0.0f, 0.0f, 1.0f}; }

    // -----------------------------------------------------------------------
    // Factory constructors
    // -----------------------------------------------------------------------

    /// Axis-angle (axis must be unit length, angle in radians).
    static Quaternion FromAxisAngle(const Vector3& axis, float radians) {
        float half = radians * 0.5f;
        float s = std::sin(half);
        return {axis.x * s, axis.y * s, axis.z * s, std::cos(half)};
    }

    /// Euler angles (roll=X, pitch=Y, yaw=Z) in radians applied in ZYX order.
    static Quaternion FromEuler(float rollX, float pitchY, float yawZ) {
        float cr = std::cos(rollX  * 0.5f), sr = std::sin(rollX  * 0.5f);
        float cp = std::cos(pitchY * 0.5f), sp = std::sin(pitchY * 0.5f);
        float cy = std::cos(yawZ   * 0.5f), sy = std::sin(yawZ   * 0.5f);
        return {
            sr*cp*cy - cr*sp*sy,
            cr*sp*cy + sr*cp*sy,
            cr*cp*sy - sr*sp*cy,
            cr*cp*cy + sr*sp*sy
        };
    }

    // -----------------------------------------------------------------------
    // Operators
    // -----------------------------------------------------------------------

    Quaternion operator*(const Quaternion& o) const {
        return {
             w*o.x + x*o.w + y*o.z - z*o.y,
             w*o.y - x*o.z + y*o.w + z*o.x,
             w*o.z + x*o.y - y*o.x + z*o.w,
             w*o.w - x*o.x - y*o.y - z*o.z
        };
    }

    constexpr bool operator==(const Quaternion& o) const {
        return x == o.x && y == o.y && z == o.z && w == o.w;
    }

    // -----------------------------------------------------------------------
    // Operations
    // -----------------------------------------------------------------------

    float LengthSquared() const { return x*x + y*y + z*z + w*w; }
    float Length()        const { return std::sqrt(LengthSquared()); }

    Quaternion Normalized() const {
        float len = Length();
        return len > 1e-6f ? Quaternion{x/len, y/len, z/len, w/len}
                           : Identity();
    }

    Quaternion Conjugate() const { return {-x, -y, -z, w}; }

    /// Rotate a vector by this quaternion.
    Vector3 Rotate(const Vector3& v) const {
        // q * (0,v) * q^-1  — optimised formula
        float tx = 2.0f*(y*v.z - z*v.y);
        float ty = 2.0f*(z*v.x - x*v.z);
        float tz = 2.0f*(x*v.y - y*v.x);
        return {
            v.x + w*tx + y*tz - z*ty,
            v.y + w*ty + z*tx - x*tz,
            v.z + w*tz + x*ty - y*tx
        };
    }

    /// Spherical linear interpolation (t in [0,1]).
    static Quaternion Slerp(const Quaternion& a, const Quaternion& b, float t) {
        float dot = a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w;
        Quaternion bb = b;
        if (dot < 0.0f) {
            dot = -dot;
            bb = {-b.x, -b.y, -b.z, -b.w};
        }
        if (dot > 0.9995f) {
            // Linear fallback for nearly identical quaternions
            Quaternion r{ a.x + t*(bb.x - a.x),
                          a.y + t*(bb.y - a.y),
                          a.z + t*(bb.z - a.z),
                          a.w + t*(bb.w - a.w) };
            return r.Normalized();
        }
        float theta0 = std::acos(dot);
        float theta  = theta0 * t;
        float sinT0  = std::sin(theta0);
        float sinT   = std::sin(theta);
        float s0 = std::cos(theta) - dot * sinT / sinT0;
        float s1 = sinT / sinT0;
        return {
            s0*a.x + s1*bb.x,
            s0*a.y + s1*bb.y,
            s0*a.z + s1*bb.z,
            s0*a.w + s1*bb.w
        };
    }

    /// Convert to rotation Matrix4.
    Matrix4 ToMatrix4() const {
        float xx = x*x, yy = y*y, zz = z*z;
        float xy = x*y, xz = x*z, yz = y*z;
        float wx = w*x, wy = w*y, wz = w*z;
        Matrix4 m = Matrix4::Identity();
        m.m[0][0] = 1 - 2*(yy + zz);
        m.m[0][1] =     2*(xy - wz);
        m.m[0][2] =     2*(xz + wy);
        m.m[1][0] =     2*(xy + wz);
        m.m[1][1] = 1 - 2*(xx + zz);
        m.m[1][2] =     2*(yz - wx);
        m.m[2][0] =     2*(xz - wy);
        m.m[2][1] =     2*(yz + wx);
        m.m[2][2] = 1 - 2*(xx + yy);
        return m;
    }
};

} // namespace subspace
