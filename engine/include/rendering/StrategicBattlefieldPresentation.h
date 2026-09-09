#pragma once

#include "core/Math.h"

#include <string>
#include <vector>

namespace subspace {

/// Visual-only depth lanes for the strategic battlefield. Gameplay remains on
/// the X/Y plane; the lane only affects projection, scale, parallax and sort.
enum class BattlefieldVisualLayer {
    DeepBackground = -2,
    Background = -1,
    Gameplay = 0,
    Foreground = 1,
    NearForeground = 2
};

enum class ThrusterSocketKind { Main, Reverse, Maneuvering };

struct ThrusterVisualSocket {
    std::string id;
    ThrusterSocketKind kind = ThrusterSocketKind::Main;
    Vector3 localPosition;      // +Y = ship nose, +X = starboard, +Z = visual height
    Vector3 localDirection;     // normalized plume direction in ship-local space
    float plumeLength = 1.0f;
    float plumeWidth = 0.25f;
    float intensity = 1.0f;
};

/// Pass157/158 authority for the starter industrial hull presentation.
/// This is intentionally renderer-facing rather than physics-facing.
struct IndustrialShipVisualProfile {
    std::string profileId = "industrial_starter_v1";
    float hullReliefDepth = 0.18f;
    float machineryReliefDepth = 0.28f;
    float canopyReliefDepth = 0.34f;
    float cargoReliefDepth = 0.22f;
    float panelBreakup = 0.65f;
    float exposedMachinery = 0.42f;
    float hazardMarkingDensity = 0.12f;
    bool useAsymmetricUtilityDetails = true;
    bool keepPrimarySilhouetteReadable = true;
    std::vector<ThrusterVisualSocket> thrusters;

    /// Industrial starter profile with corrected reverse-thrust sockets:
    /// moved inward and lower, adjacent to the forward yellow maneuvering pods.
    static IndustrialShipVisualProfile StarterIndustrial();
};

struct Faux3DBattlefieldConfig {
    bool gameplayStrictly2D = true;
    float gameplayPlaneZ = 0.0f;
    float visualDepthStep = 0.35f;
    float parallaxPerLayer = 0.035f;
    float scalePerLayer = 0.08f;
    float maxVisualDepth = 0.90f;
    bool allowForegroundCelestials = true;
};

struct BattlefieldVisualObject {
    std::string id;
    Vector3 gameplayPosition; // z is ignored for gameplay projection
    float baseScale = 1.0f;
    BattlefieldVisualLayer layer = BattlefieldVisualLayer::Gameplay;
    bool celestial = false;
    bool landable = false;
};

struct BattlefieldProjection {
    Vector3 renderPosition;
    float renderScale = 1.0f;
    float depthSortKey = 0.0f;
};

struct InteriorCutawayConfig {
    bool autoOpenWhenSelected = true;
    float autoOpenZoom = 1.35f;
    float exteriorShellAlpha = 0.24f;
    float roofAlpha = 0.10f;
    float interiorLightingBoost = 1.20f;
    float cutawayDepthBias = 0.45f;
};

struct InteriorCutawayState {
    bool visible = false;
    bool explicitOverride = false;
    float exteriorShellAlpha = 1.0f;
    float roofAlpha = 1.0f;
    float interiorLightingBoost = 1.0f;
    float cutawayDepthBias = 0.0f;
};

class StrategicBattlefieldPresentation {
public:
    static BattlefieldProjection Project(const BattlefieldVisualObject& object,
                                          const Faux3DBattlefieldConfig& config,
                                          const Vector3& cameraGameplayPosition = {});

    static InteriorCutawayState EvaluateCutaway(bool shipSelected,
                                                float cameraZoom,
                                                bool explicitToggle,
                                                const InteriorCutawayConfig& config = {});
};

} // namespace subspace
