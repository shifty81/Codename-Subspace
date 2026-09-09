#pragma once

#include "travel/InterstellarRailTravel.h"

#include <cstdint>
#include <string>
#include <vector>

namespace subspace {

enum class RailRouteEncounterType {
    ResourceDrift,
    SalvageCloud,
    PirateShadow,
    SolarStorm,
    DerelictSignal,
    QuietTransit
};

struct RailRouteEncounter {
    RailRouteEncounterType type = RailRouteEncounterType::QuietTransit;
    float progress01 = 0.0f;
    float rewardValue = 0.0f;
    float riskValue = 0.0f;
    std::string description;
};

const char* RailRouteEncounterTypeName(RailRouteEncounterType type);
std::vector<RailRouteEncounter> GenerateRailRouteEncounters(const RailTravelRouteOption& route, std::uint32_t seed);
std::string RailRouteEncounterSummary(const std::vector<RailRouteEncounter>& encounters);

} // namespace subspace
