#pragma once
#include "studio/StudioGizmoMath.h"
#include <cmath>

namespace subspace {
// Camera-aligned physical axes have a nearly zero projected line segment.
// A deterministic screen-space affordance keeps them visible and draggable;
// the selected StudioAxis remains the PHYSICAL axis in the command layer.
struct StudioGizmoProjectionPolicy {
    static constexpr float kMinimumProjectedPixels = 1.8f;
    static bool UsesScreenSpaceFallback(StudioPoint delta) noexcept {
        return !std::isfinite(delta.x) || !std::isfinite(delta.y) ||
               StudioGizmoMath::Length(delta) < kMinimumProjectedPixels;
    }
    static StudioPoint Direction(StudioAxis axis, StudioPoint delta) noexcept {
        if (!UsesScreenSpaceFallback(delta)) return StudioGizmoMath::Unit(delta);
        // Deliberately separated 2D directions: coincident screen-space axis
        // tips would make closest-tip picking ambiguous at a face-on camera.
        switch (axis) {
            case StudioAxis::X: return {1.0f, 0.0f};
            case StudioAxis::Y: return {-0.70710678f, -0.70710678f};
            case StudioAxis::Z: return {0.0f, 1.0f};
            default: return {0.0f, 0.0f};
        }
    }
};
} // namespace subspace
