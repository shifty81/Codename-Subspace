#pragma once

#include "celestial/SectorResourceModel.h"

#include <cstdint>
#include <string>
#include <vector>

namespace subspace {

struct MiningToolProfile {
    std::string id = "basic-mining-laser";
    float baseDamage = 34.0f;
    float efficiency = 1.0f;
    int tier = 1;
    bool canSalvage = false;
};

struct MiningTargetProfile {
    std::string id;
    std::string resourceTag = "ore";
    float radius = 32.0f;
    float remainingIntegrity = 100.0f;
    int richness = 1;
    bool derelict = false;
};

struct CargoYieldItem {
    std::string commodity = "ore";
    int units = 1;
    int creditValue = 10;
};

struct MiningYieldResult {
    bool fractured = false;
    float damageApplied = 0.0f;
    std::vector<CargoYieldItem> cargo;
    std::string message;
};

MiningYieldResult GenerateMiningYield(const AsteroidFieldProfile& field,
                                      const MiningTargetProfile& target,
                                      const MiningToolProfile& tool,
                                      std::uint32_t seed);
std::string MiningYieldSummary(const MiningYieldResult& result);
int EstimateCargoCreditValue(const std::vector<CargoYieldItem>& cargo);

} // namespace subspace
