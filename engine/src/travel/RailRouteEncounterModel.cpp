#include "travel/RailRouteEncounterModel.h"

#include <algorithm>
#include <sstream>

namespace subspace {

static std::uint32_t Next(std::uint32_t& seed) { seed = seed * 1664525u + 1013904223u; return seed; }
static float Rand01(std::uint32_t& seed) { return static_cast<float>((Next(seed) >> 8) & 0x00FFFFFFu) / static_cast<float>(0x01000000u); }

const char* RailRouteEncounterTypeName(RailRouteEncounterType type) {
    switch (type) {
        case RailRouteEncounterType::ResourceDrift: return "ResourceDrift";
        case RailRouteEncounterType::SalvageCloud: return "SalvageCloud";
        case RailRouteEncounterType::PirateShadow: return "PirateShadow";
        case RailRouteEncounterType::SolarStorm: return "SolarStorm";
        case RailRouteEncounterType::DerelictSignal: return "DerelictSignal";
        case RailRouteEncounterType::QuietTransit: return "QuietTransit";
        default: return "Unknown";
    }
}

std::vector<RailRouteEncounter> GenerateRailRouteEncounters(const RailTravelRouteOption& route, std::uint32_t seed) {
    std::vector<RailRouteEncounter> out;
    const int count = std::max(1, static_cast<int>(route.distanceUnits / 4.0f));
    for (int i = 0; i < count; ++i) {
        const float roll = Rand01(seed);
        RailRouteEncounter e;
        e.progress01 = std::min(0.92f, 0.12f + (static_cast<float>(i) / static_cast<float>(count)) + Rand01(seed) * 0.08f);
        if (roll < 0.24f) { e.type = RailRouteEncounterType::ResourceDrift; e.rewardValue = route.cargoOpportunity * (0.15f + Rand01(seed) * 0.35f); e.description = "Collectable resource drift on the rail lane"; }
        else if (roll < 0.46f) { e.type = RailRouteEncounterType::SalvageCloud; e.rewardValue = route.salvageOpportunity * (0.2f + Rand01(seed) * 0.45f); e.description = "Loose salvage cloud near old traffic corridor"; }
        else if (roll < 0.62f) { e.type = RailRouteEncounterType::DerelictSignal; e.rewardValue = route.salvageOpportunity * 0.55f; e.riskValue = route.hazardPressure * 0.2f; e.description = "Derelict signal detected off-lane"; }
        else if (roll < 0.78f) { e.type = RailRouteEncounterType::SolarStorm; e.riskValue = route.hazardPressure * (0.3f + Rand01(seed) * 0.5f); e.description = "Solar storm front crosses the lane"; }
        else if (roll < 0.92f) { e.type = RailRouteEncounterType::PirateShadow; e.riskValue = route.hazardPressure * (0.4f + Rand01(seed) * 0.6f); e.description = "Unidentified pirate shadow trailing route"; }
        else { e.type = RailRouteEncounterType::QuietTransit; e.description = "Quiet transit window"; }
        out.push_back(e);
    }
    return out;
}

std::string RailRouteEncounterSummary(const std::vector<RailRouteEncounter>& encounters) {
    float reward = 0.0f;
    float risk = 0.0f;
    for (const auto& e : encounters) { reward += e.rewardValue; risk += e.riskValue; }
    std::ostringstream ss;
    ss << "encounters=" << encounters.size() << " reward=" << reward << " risk=" << risk;
    return ss.str();
}

} // namespace subspace
