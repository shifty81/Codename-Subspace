#pragma once

#include "core/Math.h"
#include "core/math/Vector2.h"

#include <array>
#include <cmath>

namespace subspace {

/// Row-major 4×4 floating-point matrix for transforms, projection, and view.
///
/// Elements are stored as m[row][col].
struct Matrix4 {
    std::array<std::array<float, 4>, 4> m{};

    /// Construct the identity matrix.
    static Matrix4 Identity() {
        Matrix4 r{};
        r.m[0][0] = r.m[1][1] = r.m[2][2] = r.m[3][3] = 1.0f;
        return r;
    }

    /// Multiply two matrices.
    Matrix4 operator*(const Matrix4& o) const {
        Matrix4 result{};
        for (int r = 0; r < 4; ++r)
            for (int c = 0; c < 4; ++c)
                for (int k = 0; k < 4; ++k)
                    result.m[r][c] += m[r][k] * o.m[k][c];
        return result;
    }

    /// Transform a 3-D point (w=1) by this matrix; returns the xyz components.
    Vector3 TransformPoint(const Vector3& p) const {
        float x = m[0][0]*p.x + m[0][1]*p.y + m[0][2]*p.z + m[0][3];
        float y = m[1][0]*p.x + m[1][1]*p.y + m[1][2]*p.z + m[1][3];
        float z = m[2][0]*p.x + m[2][1]*p.y + m[2][2]*p.z + m[2][3];
        float w = m[3][0]*p.x + m[3][1]*p.y + m[3][2]*p.z + m[3][3];
        if (w != 0.0f && w != 1.0f) { x /= w; y /= w; z /= w; }
        return {x, y, z};
    }

    /// Transform a 3-D direction (w=0) — does not apply translation.
    Vector3 TransformDirection(const Vector3& d) const {
        return {
            m[0][0]*d.x + m[0][1]*d.y + m[0][2]*d.z,
            m[1][0]*d.x + m[1][1]*d.y + m[1][2]*d.z,
            m[2][0]*d.x + m[2][1]*d.y + m[2][2]*d.z
        };
    }

    /// Transpose.
    Matrix4 Transposed() const {
        Matrix4 t{};
        for (int r = 0; r < 4; ++r)
            for (int c = 0; c < 4; ++c)
                t.m[r][c] = m[c][r];
        return t;
    }

    // -----------------------------------------------------------------------
    // Factory constructors
    // -----------------------------------------------------------------------

    /// Translation matrix.
    static Matrix4 Translate(float tx, float ty, float tz) {
        Matrix4 r = Identity();
        r.m[0][3] = tx; r.m[1][3] = ty; r.m[2][3] = tz;
        return r;
    }

    /// Uniform scale matrix.
    static Matrix4 Scale(float s) {
        Matrix4 r = Identity();
        r.m[0][0] = r.m[1][1] = r.m[2][2] = s;
        return r;
    }

    /// Non-uniform scale.
    static Matrix4 Scale(float sx, float sy, float sz) {
        Matrix4 r = Identity();
        r.m[0][0] = sx; r.m[1][1] = sy; r.m[2][2] = sz;
        return r;
    }

    /// Rotation around the X axis (radians).
    static Matrix4 RotateX(float radians) {
        Matrix4 r = Identity();
        float c = std::cos(radians), s = std::sin(radians);
        r.m[1][1] =  c; r.m[1][2] = -s;
        r.m[2][1] =  s; r.m[2][2] =  c;
        return r;
    }

    /// Rotation around the Y axis (radians).
    static Matrix4 RotateY(float radians) {
        Matrix4 r = Identity();
        float c = std::cos(radians), s = std::sin(radians);
        r.m[0][0] =  c; r.m[0][2] =  s;
        r.m[2][0] = -s; r.m[2][2] =  c;
        return r;
    }

    /// Rotation around the Z axis (radians).
    static Matrix4 RotateZ(float radians) {
        Matrix4 r = Identity();
        float c = std::cos(radians), s = std::sin(radians);
        r.m[0][0] =  c; r.m[0][1] = -s;
        r.m[1][0] =  s; r.m[1][1] =  c;
        return r;
    }

    /// Look-at view matrix (right-handed, Y-up).
    static Matrix4 LookAt(const Vector3& eye,
                           const Vector3& center,
                           const Vector3& up) {
        auto sub = [](const Vector3& a, const Vector3& b) -> Vector3 {
            return {a.x - b.x, a.y - b.y, a.z - b.z};
        };
        auto normalise = [](const Vector3& v) -> Vector3 {
            float l = std::sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
            return l > 1e-6f ? Vector3{v.x/l, v.y/l, v.z/l} : Vector3{};
        };
        auto cross3 = [](const Vector3& a, const Vector3& b) -> Vector3 {
            return {a.y*b.z - a.z*b.y,
                    a.z*b.x - a.x*b.z,
                    a.x*b.y - a.y*b.x};
        };
        auto dot3 = [](const Vector3& a, const Vector3& b) -> float {
            return a.x*b.x + a.y*b.y + a.z*b.z;
        };

        Vector3 f = normalise(sub(center, eye));
        Vector3 s = normalise(cross3(f, up));
        Vector3 u = cross3(s, f);

        Matrix4 r = Identity();
        r.m[0][0] =  s.x; r.m[0][1] =  s.y; r.m[0][2] =  s.z;
        r.m[1][0] =  u.x; r.m[1][1] =  u.y; r.m[1][2] =  u.z;
        r.m[2][0] = -f.x; r.m[2][1] = -f.y; r.m[2][2] = -f.z;
        r.m[0][3] = -dot3(s, eye);
        r.m[1][3] = -dot3(u, eye);
        r.m[2][3] =  dot3(f, eye);
        return r;
    }

    /// Perspective projection matrix (right-handed, depth -1..1).
    /// @param fovYRad  Vertical field of view in radians.
    /// @param aspect   Width / height.
    /// @param nearZ    Near clip plane.
    /// @param farZ     Far clip plane.
    static Matrix4 Perspective(float fovYRad, float aspect,
                                float nearZ, float farZ) {
        float tanHalf = std::tan(fovYRad * 0.5f);
        Matrix4 r{};
        r.m[0][0] = 1.0f / (aspect * tanHalf);
        r.m[1][1] = 1.0f / tanHalf;
        r.m[2][2] = -(farZ + nearZ) / (farZ - nearZ);
        r.m[2][3] = -(2.0f * farZ * nearZ) / (farZ - nearZ);
        r.m[3][2] = -1.0f;
        return r;
    }

    /// Orthographic projection matrix.
    static Matrix4 Ortho(float left, float right,
                          float bottom, float top,
                          float nearZ, float farZ) {
        Matrix4 r = Identity();
        r.m[0][0] =  2.0f / (right - left);
        r.m[1][1] =  2.0f / (top - bottom);
        r.m[2][2] = -2.0f / (farZ - nearZ);
        r.m[0][3] = -(right + left) / (right - left);
        r.m[1][3] = -(top + bottom) / (top - bottom);
        r.m[2][3] = -(farZ + nearZ) / (farZ - nearZ);
        return r;
    }

    /// Return a const pointer to the first element (for OpenGL upload).
    const float* Data() const { return &m[0][0]; }
};

} // namespace subspace
