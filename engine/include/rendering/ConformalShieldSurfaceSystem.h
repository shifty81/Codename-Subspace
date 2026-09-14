#pragma once

#include <cstddef>
#include <string>

namespace subspace {

enum class ShieldSurfaceLod { NearConformal, MidDecimated, FarSilhouette, StrategicOnly };
struct ShieldSurfacePolicy {
    // Existing renderer-safe close shell. The new cohesive-envelope policy below
    // can request a larger player-habitable clearance once a baked unified hull
    // replaces per-module source triangles.
    float hullGapMeters=.3048f;
    float minimumCohesiveGapMeters=.75f;
    float walkableExteriorClearanceMeters=2.40f;
    float bridgeMicroDetailBelowMeters=1.25f;
    unsigned smoothingPasses=3;
    float calmWaveAmplitudeMeters=.006f;
    float impactWaveAmplitudeMeters=.065f;
    float rippleLifetimeSeconds=2.35f;
    std::size_t nearTriangleBudget=60000;
    std::size_t midTriangleBudget=12000;
    std::size_t farTriangleBudget=1800;
    std::size_t maxNearRipples=4;
    std::size_t maxMidRipples=2;
    std::size_t maxFarRipples=1;
};
struct ShieldSurfaceBudgetContext {
    float distanceMeters=0.0f;
    std::size_t visibleShieldCount=1;
    std::size_t sourceTriangleCount=0;
    bool localPlayer=false;
    bool editorPreview=false;
};
struct ShieldSurfaceBudgetDecision {
    ShieldSurfaceLod lod=ShieldSurfaceLod::NearConformal;
    std::size_t triangleStride=1;
    std::size_t rippleBudget=4;
    bool animateCalmWater=true;
    bool receivePointImpacts=true;
};
struct ShieldEnvelopeDecision {
    float requestedGapMeters=.75f;
    float bridgeDetailBelowMeters=1.25f;
    unsigned smoothingPasses=3;
    bool preserveMajorSilhouette=true;
    bool suppressMicroDetail=true;
    bool requireSingleClosedComponent=true;
};

/// Performance + geometry policy for a one-piece hull-following shield. Near
/// ships retain major silhouette while micro-detail and tiny module seams are
/// bridged into one closed surface; large fleet scenes progressively decimate
/// the same cohesive envelope and keep only bounded shader ripple impulses.
class ConformalShieldSurfaceSystem {
public:
    static ShieldSurfacePolicy DefaultPolicy();
    static ShieldSurfaceBudgetDecision Select(const ShieldSurfaceBudgetContext& context,
                                              const ShieldSurfacePolicy& policy=DefaultPolicy());
    static ShieldEnvelopeDecision Envelope(bool exteriorWalkable,
                                           const ShieldSurfacePolicy& policy=DefaultPolicy());
    static const char* LodName(ShieldSurfaceLod lod);
};

} // namespace subspace
