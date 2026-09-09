#pragma once

#include "core/Math.h"
#include <cstdint>
#include <string>
#include <vector>

namespace subspace {

enum class StationArchetype { TradeHub, IndustrialRefinery, MiningDepot, Shipyard, Military, Research, TetherTerminal, FrontierOutpost, AsteroidStation, CorporateHQ };
enum class StationOrbitClass { LowPlanet, HighPlanet, Moon, Synchronous, LagrangeLike, Belt, Stellar, DeepSpace, AsteroidEmbedded };

struct GeneratedStationProfile {
    std::uint64_t id=0;
    std::string name;
    StationArchetype archetype=StationArchetype::TradeHub;
    StationOrbitClass orbitClass=StationOrbitClass::HighPlanet;
    bool dockable=true;
    bool asteroidEmbedded=false;
    int serviceCount=4;
    int defenseTier=0;
    std::vector<std::string> services;
};

class StationEcologySystem {
public:
    std::vector<GeneratedStationProfile> BuildStartingSystem(std::uint64_t seed, int populationTier, bool hasAsteroidBelt) const;
    GeneratedStationProfile BuildAsteroidStation(std::uint64_t seed, int industryTier) const;
    static const char* ArchetypeName(StationArchetype type);
};

} // namespace subspace
