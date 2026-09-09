#pragma once

#include <string>
#include <unordered_map>

namespace subspace {

enum class SecurityBand { Core, High, Medium, Low, Frontier, Wormhole };

struct SecurityAssessment {
    SecurityBand band = SecurityBand::Medium;
    double rating = 0.5;
    double patrolStrength = 0.5;
    double pirateSpawnWeight = 0.5;
    double rewardMultiplier = 1.0;
    bool permitsUnknownFactionSpawns = false;
};

class FactionSecuritySystem {
public:
    SecurityAssessment Assess(double securityRating, double localHeat = 0.0, bool wormhole = false) const;
    void SetStanding(const std::string& factionId, double standing);
    double GetStanding(const std::string& factionId) const;
    bool IsHostile(const std::string& factionId) const;
    bool IsFriendly(const std::string& factionId) const;

private:
    std::unordered_map<std::string, double> standings_;
};

} // namespace subspace
