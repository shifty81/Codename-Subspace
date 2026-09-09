#pragma once

#include "core/Math.h"
#include "procedural/GalaxyGenerator.h"

#include <cstddef>
#include <vector>

namespace subspace {

struct RingPresentationBand {
    float innerRadiusMultiplier = 1.45f;
    float outerRadiusMultiplier = 1.70f;
    float opacity = 0.3f;
    float resourceDensity = 0.4f;
};

struct CelestialPresentationProfile {
    float radiusMultiplier = 4.8f;
    float atmosphereShellMultiplier = 1.055f;
    float cloudShellMultiplier = 1.028f;
    float ringInnerMultiplier = 1.48f;
    float ringOuterMultiplier = 4.0f;
    float ringTiltDegrees = 18.0f;
    float localDominanceRadiusMultiplier = 5.0f;
    float maximumScreenFraction = 3.60f;
    bool hasCloudLayer = false;
    bool supportsDenseRingField = false;
    float terminatorContrast = 0.72f;
    float surfaceDetail = 0.68f;
    float cloudOpacity = 0.10f;
    float atmosphereGlow = 0.16f;
    float ringShadowStrength = 0.42f;
    float nightLightStrength = 0.0f;
};

struct CelestialLocalContext {
    bool renderMajorDisc = false;
    bool dominant = false;
    float distanceWorld = 0.0f;
    float radiusWorld = 0.0f;
    float hazeStrength = 0.0f;
};

class CelestialEnvironmentSystem {
public:
    CelestialPresentationProfile ProfileFor(const PlanetData& planet) const;
    float WorldRadius(const PlanetData& planet) const;
    float MinimumOrbitalSeparation(const PlanetData& a, const PlanetData& b) const;
    float SafeLocalOrbitRadius(const PlanetData& planet, float additionalClearance = 0.0f) const;
    Vector3 ProjectOutsideLocalOrbit(const Vector3& candidate, const Vector3& planetWorld,
                                     const PlanetData& planet, float additionalClearance = 0.0f) const;
    std::vector<RingPresentationBand> RingBands(const PlanetData& planet) const;
    CelestialLocalContext EvaluateLocalContext(const Vector3& playerWorld,
                                                const Vector3& planetWorld,
                                                const PlanetData& planet,
                                                bool nearestMajor) const;
    std::size_t MaximumMajorDiscsInLocalFlight() const { return 1; }
};

} // namespace subspace
