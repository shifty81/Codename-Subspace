#pragma once

#include "rendering/ProceduralVisualVariantSystem.h"

#include <array>
#include <cmath>

namespace subspace {
// Presentation-only values read directly from the authoritative placement.
// The dimensions are *nominal local bounds* from the source catalog. They
// are not a rotated world AABB or a mesh-exact measurement of carved geometry.
struct StudioTransformReadout {
    std::array<float,3> position{};          // assembly X/Y/Z, world units
    std::array<float,3> rotationDegrees{};   // physical X=pitch, Y=roll, Z=yaw
    std::array<float,3> scalePercent{};      // actual per-axis multiplier * 100
    std::array<float,3> nominalLocalMeters{};// W/L/H after recipe size multipliers
    bool nominalDimensionsAvailable=false;

    static StudioTransformReadout Build(const VisualModulePlacement& part,
                                        const ProceduralShipVisualRecipe& recipe,
                                        const VisualModuleSource* source) noexcept {
        StudioTransformReadout out;
        out.position={{part.x,part.y,part.z}};
        out.rotationDegrees={{part.pitchDegrees,part.rollDegrees,part.yawDegrees}};
        out.scalePercent={{part.scaleX*100.0f,part.scaleY*100.0f,part.scaleZ*100.0f}};
        if(source && std::isfinite(source->halfWidth) && std::isfinite(source->halfLength) &&
           std::isfinite(source->halfHeight) && source->halfWidth>0 &&
           source->halfLength>0 && source->halfHeight>0 &&
           std::isfinite(recipe.widthScale) && std::isfinite(recipe.lengthScale) &&
           std::isfinite(part.scaleX) && std::isfinite(part.scaleY) && std::isfinite(part.scaleZ)) {
            out.nominalLocalMeters={{2.0f*source->halfWidth*std::fabs(part.scaleX*recipe.widthScale),
                                    2.0f*source->halfLength*std::fabs(part.scaleY*recipe.lengthScale),
                                    2.0f*source->halfHeight*std::fabs(part.scaleZ)}};
            out.nominalDimensionsAvailable=std::isfinite(out.nominalLocalMeters[0]) &&
                std::isfinite(out.nominalLocalMeters[1]) &&
                std::isfinite(out.nominalLocalMeters[2]);
        }
        return out;
    }
};
} // namespace subspace
