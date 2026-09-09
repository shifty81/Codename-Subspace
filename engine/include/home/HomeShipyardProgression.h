#pragma once

#include "home/HomeFactoryNetwork.h"

#include <map>
#include <string>
#include <vector>

namespace subspace {

enum class ShipyardUpgradeType {
    RepairBay,
    ModulePrinter,
    BlueprintArchive,
    DrydockExpansion,
    DroneAutomation,
    ResearchLab,
    SolarPowerLink,
    SubspaceLaunchArray
};

struct ShipyardUpgradeDefinition {
    ShipyardUpgradeType type = ShipyardUpgradeType::RepairBay;
    std::string id;
    std::string displayName;
    int maxTier = 5;
    std::vector<HomeInventoryStack> baseCost;
};

struct ShipyardProgressionState {
    int shipyardLevel = 1;
    int researchData = 0;
    int blueprintProgress = 0;
    std::map<std::string, int> upgradeTiers;
    std::vector<std::string> unlockedBlueprints;
};

std::vector<ShipyardUpgradeDefinition> CreateDefaultShipyardUpgrades();
ShipyardProgressionState CreateStarterShipyardProgression();
bool CanAffordShipyardUpgrade(const HomeFactoryNetworkState& inventory,
                               const ShipyardProgressionState& state,
                               const ShipyardUpgradeDefinition& upgrade);
bool ApplyShipyardUpgrade(HomeFactoryNetworkState& inventory,
                          ShipyardProgressionState& state,
                          const ShipyardUpgradeDefinition& upgrade);
int GetShipyardUpgradeTier(const ShipyardProgressionState& state, const std::string& upgradeId);
int EstimateShipyardBuildCapacity(const ShipyardProgressionState& state);
std::string ShipyardUpgradeTypeName(ShipyardUpgradeType type);
std::string ShipyardProgressionSummary(const ShipyardProgressionState& state);

} // namespace subspace
