#pragma once

#include "core/Math.h"
#include "rendering/ProceduralVisualVariantSystem.h"
#include "ship_editor/ShipyardTransformSystem.h"

namespace subspace {

/// Canonical coordinate-space conversion for mouse/keyboard authoring.
///
/// Editor/View movement is screen/camera oriented and must never change merely
/// because the authored ship has a different runtime heading. The renderer
/// applies both the runtime root yaw and recipe forward-visual yaw, so input
/// conversion must invert that complete root transform before mutating the
/// ship-local recipe. Ship and Local remain explicit opt-in transform spaces.
class EditorTransformSpaceSystem {
public:
    static float DegreesToRadians(float degrees);
    static float RenderedRootYawRadians(float runtimeShipYawRadians,
                                        float recipeForwardVisualYawDegrees);

    static Vector3 WorldToAssemblyDelta(const Vector3& worldDelta,
                                        float renderedRootYawRadians);
    static Vector3 AssemblyToModuleLocalDelta(const Vector3& assemblyDelta,
                                              const VisualModulePlacement& placement);

    static Vector3 ResolveTranslation(ShipyardTransformSpace space,
                                      const Vector3& viewWorldDelta,
                                      const Vector3& shipSpaceDelta,
                                      float renderedRootYawRadians,
                                      const VisualModulePlacement* selectedModule = nullptr);
};

} // namespace subspace
