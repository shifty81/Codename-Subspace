#pragma once

#include "rendering/PlanetWeatherSystem.h"

namespace subspace {

struct GasGiantBandState {
    float latitude = 0.0f;
    float angularVelocity = 0.0f;
    float turbulence = 0.0f;
    float brightness = 1.0f;
};

struct GasGiantWeatherProfile {
    bool volatileAtmosphere = false;
    float lightningProbability = 0.0f;
    float giantVortexProbability = 0.0f;
    std::vector<GasGiantBandState> bands;
};

class GasGiantWeatherSystem {
public:
    static GasGiantWeatherProfile Build(const PlanetData& planet,std::uint32_t seed);
    static float BandLongitude(const GasGiantBandState& band,double seconds);
};

} // namespace subspace
