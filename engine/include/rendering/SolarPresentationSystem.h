#pragma once

#include "core/Math.h"
#include "procedural/GalaxyGenerator.h"

namespace subspace {

struct SolarLightProfile {
    Vector3 direction{0.0f,0.0f,1.0f};
    float diffuseIntensity = 1.0f;
    float ambientIntensity = 0.025f;
    float normalizedDistance = 1.0f;
};

struct StarVisualProfile {
    float worldRadius = 120.0f;
    float photosphereVariation = 0.72f;
    float coronaInnerMultiplier = 1.14f;
    float coronaOuterMultiplier = 1.52f;
    float prominenceStrength = 0.35f;
};

/// Pass357 solar authority. The generated star is normally the only dominant
/// natural light in a system; distance and luminosity therefore materially
/// affect the visible light level instead of using nearly constant fill.
class SolarPresentationSystem {
public:
    static SolarLightProfile EvaluateLight(const StarData& star,
                                           const Vector3& starWorld,
                                           const Vector3& receiverWorld);
    static StarVisualProfile VisualFor(const StarData& star);
};

} // namespace subspace
