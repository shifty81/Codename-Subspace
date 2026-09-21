#pragma once
#include "studio/StudioGizmoMath.h"
#include <algorithm>
#include <cmath>

namespace subspace {
// Projection-aware input math. No renderer, UI state or alternate transform
// authority: takes the handle snapshot captured on mouse-down.
struct StudioGizmoDragPolicy {
    static constexpr float kMinResolvablePixelsPerUnit=2.0f;
    static float MoveUnits(const StudioAxisHandle& handle,StudioPoint fromPress,bool fine=false) noexcept {
        if(!handle.valid||!std::isfinite(fromPress.x)||!std::isfinite(fromPress.y))return 0.0f;
        const StudioPoint physical=handle.physicalPixelsPerUnit;
        const float pixels=StudioGizmoMath::Length(physical);
        float units=0.0f;
        if(handle.projectedAxisUsable&&std::isfinite(pixels)&&pixels>=kMinResolvablePixelsPerUnit){
            // Screen-space least squares of the actual authored axis.
            // The pointer offset at mouse-down is subtracted before calling.
            units=StudioGizmoMath::Dot(fromPress,physical)/(pixels*pixels);
        }else{
            // View-aligned depth cannot be tracked under perspective. Keep a
            // stable manual depth gesture, not an unbounded inverse singularity.
            const StudioPoint shown=StudioGizmoMath::Unit(
                StudioGizmoMath::Delta(handle.tip,handle.center));
            const float density=std::clamp(handle.fallbackPixelsPerUnit,8.0f,160.0f);
            units=StudioGizmoMath::Dot(fromPress,shown)/density;
        }
        if(!std::isfinite(units))return 0.0f;
        return units*(fine?0.1f:1.0f);
    }
};
} // namespace subspace
