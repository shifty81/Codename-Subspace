#pragma once

#include "core/Math.h"
#include "rendering/ProceduralVisualVariantSystem.h"

namespace subspace {

struct ShipyardVisualTransform {
    Vector3 worldOrigin{};
    float renderedYawRadians = 0.0f;
    Vector3 axisScale{1.0f,1.0f,1.0f};
};

/// Single inverse-transform authority for direct-manipulation in Shipyard.
/// It mirrors the renderer's ship transform exactly: gameplay yaw + recipe
/// forward visual normalization, followed by the rendered X/Y/Z axis scales.
class ShipyardScreenSpaceTransformSystem {
public:
    static ShipyardVisualTransform Build(const ProceduralShipVisualRecipe& recipe,
                                         const Vector3& worldOrigin,
                                         float gameplayYawRadians,
                                         float shipScale,
                                         float presentationWidthScale,
                                         float presentationLengthScale);

    static Vector3 WorldPointToRecipeLocal(const Vector3& worldPoint,
                                           const ShipyardVisualTransform& transform);
    static Vector3 WorldDeltaToRecipeLocal(const Vector3& worldDelta,
                                           const ShipyardVisualTransform& transform);
    static Vector3 RecipeLocalToWorldPoint(const Vector3& recipeLocal,
                                           const ShipyardVisualTransform& transform);
};

} // namespace subspace
