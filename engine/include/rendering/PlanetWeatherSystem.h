#pragma once

#include "procedural/GalaxyGenerator.h"

#include <cstdint>
#include <vector>

namespace subspace {

enum class PlanetStormKind { ConvectiveCell, Cyclone, DustStorm, AshFront, IceStorm, GiantVortex };

struct PlanetStormCell {
    std::uint32_t id = 0;
    PlanetStormKind kind = PlanetStormKind::ConvectiveCell;
    float longitude = 0.0f;
    float latitude = 0.0f;
    float radius = 0.12f;
    float intensity = 0.0f;
    float angularVelocity = 0.0f;
    float lifetimeSeconds = 120.0f;
    float ageSeconds = 0.0f;
};

struct PlanetWeatherState {
    std::uint32_t seed = 1;
    double simulationSeconds = 0.0;
    float volatility = 0.0f;
    std::vector<PlanetStormCell> storms;
};

class PlanetWeatherSystem {
public:
    static PlanetWeatherState Initialize(const PlanetData& planet,std::uint32_t seed);
    static void Advance(PlanetWeatherState& state,const PlanetData& planet,double deltaSeconds);
    static float CloudFieldOffset(const PlanetWeatherState& state,float latitude,int layer);
};

} // namespace subspace
