#pragma once

#include "editor/ConstructionTransformBasisSystem.h"

#include <cmath>

namespace subspace {

enum class ConstructionScalePivotMode : int {
    Center = 0,
    OppositeFace,
    SelectedSocket
};

// Scaling in the current renderer is local-axis scaling.  When an axis handle
// is dragged, OppositeFace keeps the face opposite that positive handle fixed
// by translating the object's center by half the local dimension delta.
class ConstructionScalePivotSystem {
public:
    static Vector3 OppositeFaceShift(const Vector3& beforeSize,
                                     const Vector3& afterSize,
                                     const ConstructionTransformBasis& localBasis) noexcept {
        const Vector3 halfDelta{(afterSize.x-beforeSize.x)*.5f,
                                (afterSize.y-beforeSize.y)*.5f,
                                (afterSize.z-beforeSize.z)*.5f};
        return localBasis.x*halfDelta.x+localBasis.y*halfDelta.y+localBasis.z*halfDelta.z;
    }

    static Vector3 ModuleSize(const VisualModuleSource& source,
                              const VisualModulePlacement& p) noexcept {
        return {2.0f*std::fabs(source.halfWidth*p.scaleX),
                2.0f*std::fabs(source.halfLength*p.scaleY),
                2.0f*std::fabs(source.halfHeight*p.scaleZ)};
    }

    static Vector3 AnchoredModulePosition(const VisualModuleSource& source,
                                          const VisualModulePlacement& before,
                                          const VisualModulePlacement& after) noexcept {
        const auto basis=ConstructionTransformBasisSystem::AssemblyLocal(before,true);
        const auto shift=OppositeFaceShift(ModuleSize(source,before),ModuleSize(source,after),basis);
        return {before.x+shift.x,before.y+shift.y,before.z+shift.z};
    }
};

} // namespace subspace
