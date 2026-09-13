#pragma once

#include <cstddef>
#include <string>

namespace subspace {

enum class ShieldSurfaceLod { NearConformal, MidDecimated, FarSilhouette, StrategicOnly };
struct ShieldSurfacePolicy {
    float hullGapMeters=.3048f;
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

/// Performance contract for the hull-conformal one-layer shield. Near/local
/// ships use the actual baked hull surface; large fleet scenes progressively
/// decimate to one cohesive silhouette shell while retaining bounded ripples.
class ConformalShieldSurfaceSystem {
public:
    static ShieldSurfacePolicy DefaultPolicy();
    static ShieldSurfaceBudgetDecision Select(const ShieldSurfaceBudgetContext& context,
                                              const ShieldSurfacePolicy& policy=DefaultPolicy());
    static const char* LodName(ShieldSurfaceLod lod);
};

} // namespace subspace
