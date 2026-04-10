#include "rendering/FrustumCuller.h"

#include <cmath>

namespace subspace {

// ---------------------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------------------

FrustumPlane FrustumCuller::NormalizePlane(float a, float b, float c, float d)
{
    float len = std::sqrt(a * a + b * b + c * c);
    if (len < 1e-8f) {
        // Degenerate plane (zero-length normal) — return a pass-through plane so
        // that a defective VP matrix does not accidentally cull everything.
        // The large positive d means every point satisfies (n·p + d >= 0).
        static constexpr float kPassThroughOffset = 1e9f;
        return {{0.0f, 1.0f, 0.0f}, kPassThroughOffset};
    }
    float inv = 1.0f / len;
    return {{a * inv, b * inv, c * inv}, d * inv};
}

// ---------------------------------------------------------------------------
// ExtractPlanes — Gribb-Hartmann method
//
// For a column-vector matrix M (i.e. TransformPoint applies M * v), the
// homogeneous clip coordinates for world point p = [x, y, z, 1]^T are:
//
//   clip.x = m[0][0]*x + m[0][1]*y + m[0][2]*z + m[0][3]   (row 0 · p)
//   clip.y = m[1][0]*x + m[1][1]*y + m[1][2]*z + m[1][3]   (row 1 · p)
//   clip.z = m[2][0]*x + m[2][1]*y + m[2][2]*z + m[2][3]   (row 2 · p)
//   clip.w = m[3][0]*x + m[3][1]*y + m[3][2]*z + m[3][3]   (row 3 · p)
//
// OpenGL visibility test: the point is inside when
//   -clip.w <= clip.x <= clip.w  (left/right)
//   -clip.w <= clip.y <= clip.w  (bottom/top)
//   -clip.w <= clip.z <= clip.w  (near/far)
//
// Rewriting as >= 0 inequalities using row arithmetic:
//   Left:   clip.w + clip.x >= 0   →  (row3 + row0) · p >= 0
//   Right:  clip.w - clip.x >= 0   →  (row3 - row0) · p >= 0
//   Bottom: clip.w + clip.y >= 0   →  (row3 + row1) · p >= 0
//   Top:    clip.w - clip.y >= 0   →  (row3 - row1) · p >= 0
//   Near:   clip.w + clip.z >= 0   →  (row3 + row2) · p >= 0
//   Far:    clip.w - clip.z >= 0   →  (row3 - row2) · p >= 0
// ---------------------------------------------------------------------------

void FrustumCuller::ExtractPlanes(const Matrix4& vp)
{
    const auto& m = vp.m;

    _planes[kLeft]   = NormalizePlane(m[3][0] + m[0][0],
                                      m[3][1] + m[0][1],
                                      m[3][2] + m[0][2],
                                      m[3][3] + m[0][3]);

    _planes[kRight]  = NormalizePlane(m[3][0] - m[0][0],
                                      m[3][1] - m[0][1],
                                      m[3][2] - m[0][2],
                                      m[3][3] - m[0][3]);

    _planes[kBottom] = NormalizePlane(m[3][0] + m[1][0],
                                      m[3][1] + m[1][1],
                                      m[3][2] + m[1][2],
                                      m[3][3] + m[1][3]);

    _planes[kTop]    = NormalizePlane(m[3][0] - m[1][0],
                                      m[3][1] - m[1][1],
                                      m[3][2] - m[1][2],
                                      m[3][3] - m[1][3]);

    _planes[kNear]   = NormalizePlane(m[3][0] + m[2][0],
                                      m[3][1] + m[2][1],
                                      m[3][2] + m[2][2],
                                      m[3][3] + m[2][3]);

    _planes[kFar]    = NormalizePlane(m[3][0] - m[2][0],
                                      m[3][1] - m[2][1],
                                      m[3][2] - m[2][2],
                                      m[3][3] - m[2][3]);

    _valid = true;
}

// ---------------------------------------------------------------------------
// TestSphere
//
// A sphere (center, radius) is outside the frustum when its signed distance
// to any normalised plane is less than -radius (sphere entirely on the
// negative / outside half-space).  We return false early in that case.
// ---------------------------------------------------------------------------

bool FrustumCuller::TestSphere(const Vector3& center, float radius) const
{
    for (const auto& plane : _planes) {
        float dist = plane.normal.x * center.x
                   + plane.normal.y * center.y
                   + plane.normal.z * center.z
                   + plane.d;
        if (dist < -radius) {
            return false;   // Sphere entirely outside this plane
        }
    }
    return true;    // Inside or intersecting all planes
}

// ---------------------------------------------------------------------------
// TestAABB
//
// For each frustum plane, pick the AABB vertex most in the direction of the
// plane normal (the "p-vertex").  If the p-vertex is outside any plane, the
// entire AABB is outside.
// ---------------------------------------------------------------------------

bool FrustumCuller::TestAABB(const Vector3& mins, const Vector3& maxs) const
{
    for (const auto& plane : _planes) {
        // p-vertex: most extreme in the direction of the plane normal
        float px = (plane.normal.x >= 0.0f) ? maxs.x : mins.x;
        float py = (plane.normal.y >= 0.0f) ? maxs.y : mins.y;
        float pz = (plane.normal.z >= 0.0f) ? maxs.z : mins.z;

        float dist = plane.normal.x * px
                   + plane.normal.y * py
                   + plane.normal.z * pz
                   + plane.d;

        if (dist < 0.0f) {
            return false;   // Most extreme vertex is outside — AABB fully culled
        }
    }
    return true;
}

} // namespace subspace
