#include "editor/EditorTransformSpaceSystem.h"

#include <algorithm>
#include <cmath>

namespace subspace {

float EditorTransformSpaceSystem::DegreesToRadians(float degrees) {
    constexpr float kDegToRad = 3.14159265358979323846f / 180.0f;
    return degrees * kDegToRad;
}

float EditorTransformSpaceSystem::RenderedRootYawRadians(float runtimeShipYawRadians,
                                                          float recipeForwardVisualYawDegrees) {
    return runtimeShipYawRadians + DegreesToRadians(recipeForwardVisualYawDegrees);
}

Vector3 EditorTransformSpaceSystem::WorldToAssemblyDelta(const Vector3& worldDelta,
                                                          float renderedRootYawRadians) {
    const float c = std::cos(renderedRootYawRadians);
    const float s = std::sin(renderedRootYawRadians);
    return {worldDelta.x * c + worldDelta.y * s,
            -worldDelta.x * s + worldDelta.y * c,
            worldDelta.z};
}

Vector3 EditorTransformSpaceSystem::AssemblyToModuleLocalDelta(
    const Vector3& assemblyDelta,
    const VisualModulePlacement& p) {
    Vector3 v = assemblyDelta;

    // Inverse of renderer Rz(yaw) * Rx(pitch) * Ry(roll).
    const float yaw = DegreesToRadians(-p.yawDegrees);
    const float cy = std::cos(yaw), sy = std::sin(yaw);
    v = {v.x * cy - v.y * sy, v.x * sy + v.y * cy, v.z};

    const float pitch = DegreesToRadians(-p.pitchDegrees);
    const float cp = std::cos(pitch), sp = std::sin(pitch);
    v = {v.x, v.y * cp - v.z * sp, v.y * sp + v.z * cp};

    const float roll = DegreesToRadians(-p.rollDegrees);
    const float cr = std::cos(roll), sr = std::sin(roll);
    v = {v.x * cr + v.z * sr, v.y, -v.x * sr + v.z * cr};

    const float sx = std::max(0.05f, std::fabs(p.scaleX));
    const float syScale = std::max(0.05f, std::fabs(p.scaleY));
    const float sz = std::max(0.05f, std::fabs(p.scaleZ));
    v.x /= sx; v.y /= syScale; v.z /= sz;
    if (p.mirrorX) v.x = -v.x;
    if (p.mirrorY) v.y = -v.y;
    if (p.mirrorZ) v.z = -v.z;
    return v;
}

Vector3 EditorTransformSpaceSystem::ResolveTranslation(
    ShipyardTransformSpace space,
    const Vector3& viewWorldDelta,
    const Vector3& shipSpaceDelta,
    float renderedRootYawRadians,
    const VisualModulePlacement* selectedModule) {
    if (space == ShipyardTransformSpace::View) {
        Vector3 assembly = WorldToAssemblyDelta(viewWorldDelta, renderedRootYawRadians);
        return selectedModule ? AssemblyToModuleLocalDelta(assembly, *selectedModule) : assembly;
    }
    if (space == ShipyardTransformSpace::Local && selectedModule)
        return AssemblyToModuleLocalDelta(shipSpaceDelta, *selectedModule);
    return shipSpaceDelta;
}

} // namespace subspace
