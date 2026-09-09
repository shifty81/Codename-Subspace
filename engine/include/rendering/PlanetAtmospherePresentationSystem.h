#pragma once

#include <algorithm>

namespace subspace {

struct PlanetAtmospherePresentationProfile {
    float cloudRadiusMultiplier = 1.006f;
    float atmosphereRadiusMultiplier = 1.018f;
    float centerOpacity = 0.015f;
    float limbOpacity = 0.42f;
    float limbPower = 3.2f;
};

class PlanetAtmospherePresentationSystem {
public:
    static PlanetAtmospherePresentationProfile Default();
    static float Alpha(float viewNormalDot,const PlanetAtmospherePresentationProfile& profile,float sunlight);
};

} // namespace subspace
