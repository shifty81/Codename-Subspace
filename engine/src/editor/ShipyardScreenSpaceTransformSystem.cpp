#include "editor/ShipyardScreenSpaceTransformSystem.h"

#include <algorithm>
#include <cmath>

namespace subspace {
namespace {
constexpr float kPi = 3.14159265358979323846f;
float SafeAxis(float v){return std::max(1.0e-5f,std::fabs(v));}
}

ShipyardVisualTransform ShipyardScreenSpaceTransformSystem::Build(
    const ProceduralShipVisualRecipe& recipe,
    const Vector3& worldOrigin,
    float gameplayYawRadians,
    float shipScale,
    float presentationWidthScale,
    float presentationLengthScale)
{
    ShipyardVisualTransform t;
    t.worldOrigin=worldOrigin;
    t.renderedYawRadians=gameplayYawRadians+recipe.forwardVisualYawDegrees*kPi/180.0f;
    t.axisScale={SafeAxis(shipScale*presentationWidthScale*recipe.widthScale),
                 SafeAxis(shipScale*presentationLengthScale*recipe.lengthScale),
                 SafeAxis(shipScale)};
    return t;
}

Vector3 ShipyardScreenSpaceTransformSystem::WorldDeltaToRecipeLocal(
    const Vector3& worldDelta,const ShipyardVisualTransform& t)
{
    const float c=std::cos(t.renderedYawRadians),s=std::sin(t.renderedYawRadians);
    const Vector3 rotated{worldDelta.x*c+worldDelta.y*s,
                          -worldDelta.x*s+worldDelta.y*c,
                          worldDelta.z};
    return {rotated.x/t.axisScale.x,rotated.y/t.axisScale.y,rotated.z/t.axisScale.z};
}

Vector3 ShipyardScreenSpaceTransformSystem::WorldPointToRecipeLocal(
    const Vector3& worldPoint,const ShipyardVisualTransform& t)
{
    return WorldDeltaToRecipeLocal(worldPoint-t.worldOrigin,t);
}

Vector3 ShipyardScreenSpaceTransformSystem::RecipeLocalToWorldPoint(
    const Vector3& p,const ShipyardVisualTransform& t)
{
    const Vector3 scaled{p.x*t.axisScale.x,p.y*t.axisScale.y,p.z*t.axisScale.z};
    const float c=std::cos(t.renderedYawRadians),s=std::sin(t.renderedYawRadians);
    return {t.worldOrigin.x+scaled.x*c-scaled.y*s,
            t.worldOrigin.y+scaled.x*s+scaled.y*c,
            t.worldOrigin.z+scaled.z};
}

} // namespace subspace
