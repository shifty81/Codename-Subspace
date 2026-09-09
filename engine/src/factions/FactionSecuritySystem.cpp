#include "factions/FactionSecuritySystem.h"

#include <algorithm>

namespace subspace {

SecurityAssessment FactionSecuritySystem::Assess(double securityRating, double localHeat, bool wormhole) const {
    SecurityAssessment a;
    a.rating = std::clamp(securityRating, 0.0, 1.0);
    localHeat = std::clamp(localHeat, 0.0, 1.0);
    if (wormhole) a.band = SecurityBand::Wormhole;
    else if (a.rating >= 0.85) a.band = SecurityBand::Core;
    else if (a.rating >= 0.65) a.band = SecurityBand::High;
    else if (a.rating >= 0.45) a.band = SecurityBand::Medium;
    else if (a.rating >= 0.25) a.band = SecurityBand::Low;
    else a.band = SecurityBand::Frontier;

    const double danger = wormhole ? 1.0 : (1.0 - a.rating);
    a.patrolStrength = wormhole ? 0.08 : std::clamp(a.rating * (1.0 - 0.35 * localHeat), 0.05, 1.0);
    a.pirateSpawnWeight = std::clamp(danger * 0.85 + localHeat * 0.5, 0.0, 1.5);
    a.rewardMultiplier = 1.0 + danger * 1.75 + localHeat * 0.35;
    a.permitsUnknownFactionSpawns = wormhole || a.band == SecurityBand::Frontier || a.band == SecurityBand::Low;
    return a;
}

void FactionSecuritySystem::SetStanding(const std::string& factionId, double standing) {
    if (!factionId.empty()) standings_[factionId] = std::clamp(standing, -1.0, 1.0);
}

double FactionSecuritySystem::GetStanding(const std::string& factionId) const {
    auto it = standings_.find(factionId); return it == standings_.end() ? 0.0 : it->second;
}

bool FactionSecuritySystem::IsHostile(const std::string& factionId) const { return GetStanding(factionId) <= -0.5; }
bool FactionSecuritySystem::IsFriendly(const std::string& factionId) const { return GetStanding(factionId) >= 0.5; }

} // namespace subspace
