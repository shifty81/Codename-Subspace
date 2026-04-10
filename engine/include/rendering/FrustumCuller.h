#pragma once

#include "core/Math.h"
#include "core/math/Matrix4.h"

#include <array>

namespace subspace {

/// A single frustum plane.
/// A point P is on the inside (positive half-space) when:
///   normal.x*P.x + normal.y*P.y + normal.z*P.z + d >= 0
struct FrustumPlane {
    Vector3 normal;
    float   d = 0.0f;
};

/// View-frustum culler built from a combined View×Projection matrix.
///
/// Usage:
///   FrustumCuller culler;
///   culler.ExtractPlanes(projMatrix * viewMatrix);
///   if (culler.TestSphere(worldPos, radius)) { /* visible — submit for render */ }
///
/// The culler uses the Gribb-Hartmann method which extracts the six clip
/// planes directly from the VP matrix without requiring a matrix inverse.
/// Planes are normalised so that TestSphere distances are metrically correct.
class FrustumCuller {
public:
    FrustumCuller() = default;

    /// Extract the 6 clip planes from a View×Projection matrix.
    ///
    /// Assumes column-vector convention (point transformed as M * v) and
    /// OpenGL-style NDC: clip coords ∈ [-w, w] on all three axes.
    /// Planes are normalised so sphere-distance comparisons are exact.
    ///
    /// @param viewProj  Combined Projection * View matrix (or any MVP matrix
    ///                  whose clip-space you want to cull against).
    void ExtractPlanes(const Matrix4& viewProj);

    /// Returns true when the bounding sphere intersects or is fully inside
    /// the frustum.  Returns false only when the sphere lies entirely on the
    /// outside of at least one frustum plane.
    bool TestSphere(const Vector3& center, float radius) const;

    /// Returns true when the axis-aligned bounding box [mins, maxs] is at
    /// least partially inside the frustum.
    bool TestAABB(const Vector3& mins, const Vector3& maxs) const;

    /// Returns true once ExtractPlanes() has been called at least once.
    bool IsValid() const { return _valid; }

    // Named indices into the internal plane array.
    static constexpr int kLeft   = 0;
    static constexpr int kRight  = 1;
    static constexpr int kBottom = 2;
    static constexpr int kTop    = 3;
    static constexpr int kNear   = 4;
    static constexpr int kFar    = 5;

private:
    std::array<FrustumPlane, 6> _planes{};
    bool _valid = false;

    /// Build a normalised plane from the four raw homogeneous coefficients.
    static FrustumPlane NormalizePlane(float a, float b, float c, float d);
};

} // namespace subspace
