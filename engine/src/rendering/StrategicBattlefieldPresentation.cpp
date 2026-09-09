#include "rendering/StrategicBattlefieldPresentation.h"

#include <algorithm>
#include <cmath>

namespace subspace {

IndustrialShipVisualProfile IndustrialShipVisualProfile::StarterIndustrial() {
    IndustrialShipVisualProfile profile;

    // Main drive sockets remain wide and aft for a heavy industrial silhouette.
    profile.thrusters.push_back({
        "main_port", ThrusterSocketKind::Main,
        {-0.58f, -0.82f, -0.02f}, {0.0f, -1.0f, 0.0f}, 1.35f, 0.34f, 1.0f
    });
    profile.thrusters.push_back({
        "main_starboard", ThrusterSocketKind::Main,
        {0.58f, -0.82f, -0.02f}, {0.0f, -1.0f, 0.0f}, 1.35f, 0.34f, 1.0f
    });

    // Pass158 correction: the forward retro/reverse sockets are intentionally
    // inboard and lower than the old outer-edge placement. They now sit close
    // to the yellow forward maneuvering housings near the cockpit instead of
    // reading like detached wing-tip effects.
    profile.thrusters.push_back({
        "reverse_port", ThrusterSocketKind::Reverse,
        {-0.43f, 0.56f, -0.16f}, {0.0f, 1.0f, 0.0f}, 0.78f, 0.22f, 0.82f
    });
    profile.thrusters.push_back({
        "reverse_starboard", ThrusterSocketKind::Reverse,
        {0.43f, 0.56f, -0.16f}, {0.0f, 1.0f, 0.0f}, 0.78f, 0.22f, 0.82f
    });

    // Small lateral maneuvering sockets sell mass/inertia without changing
    // the authoritative 2D movement model.
    profile.thrusters.push_back({
        "maneuver_port", ThrusterSocketKind::Maneuvering,
        {-0.72f, 0.18f, -0.08f}, {-1.0f, 0.0f, 0.0f}, 0.44f, 0.13f, 0.60f
    });
    profile.thrusters.push_back({
        "maneuver_starboard", ThrusterSocketKind::Maneuvering,
        {0.72f, 0.18f, -0.08f}, {1.0f, 0.0f, 0.0f}, 0.44f, 0.13f, 0.60f
    });

    return profile;
}

BattlefieldProjection StrategicBattlefieldPresentation::Project(
    const BattlefieldVisualObject& object,
    const Faux3DBattlefieldConfig& config,
    const Vector3& cameraGameplayPosition)
{
    BattlefieldProjection out;

    int rawLayer = static_cast<int>(object.layer);
    float layer = static_cast<float>(rawLayer);
    float z = std::clamp(layer * config.visualDepthStep,
                         -config.maxVisualDepth,
                         config.maxVisualDepth);

    // Gameplay Z is never imported into the strategic projection. Parallax is
    // a visual offset relative to camera position only.
    float parallax = layer * config.parallaxPerLayer;
    out.renderPosition.x = object.gameplayPosition.x - cameraGameplayPosition.x * parallax;
    out.renderPosition.y = object.gameplayPosition.y - cameraGameplayPosition.y * parallax;
    out.renderPosition.z = config.gameplayPlaneZ + z;

    out.renderScale = std::max(0.05f, object.baseScale * (1.0f + layer * config.scalePerLayer));
    out.depthSortKey = z;
    return out;
}

InteriorCutawayState StrategicBattlefieldPresentation::EvaluateCutaway(
    bool shipSelected,
    float cameraZoom,
    bool explicitToggle,
    const InteriorCutawayConfig& config)
{
    InteriorCutawayState out;
    out.explicitOverride = explicitToggle;
    out.visible = explicitToggle ||
                  (config.autoOpenWhenSelected && shipSelected && cameraZoom >= config.autoOpenZoom);

    if (out.visible) {
        out.exteriorShellAlpha = config.exteriorShellAlpha;
        out.roofAlpha = config.roofAlpha;
        out.interiorLightingBoost = config.interiorLightingBoost;
        out.cutawayDepthBias = config.cutawayDepthBias;
    }
    return out;
}

} // namespace subspace
