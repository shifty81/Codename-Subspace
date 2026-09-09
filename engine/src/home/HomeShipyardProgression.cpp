#include "home/HomeShipyardProgression.h"

#include <algorithm>
#include <sstream>

namespace subspace {

std::vector<ShipyardUpgradeDefinition> CreateDefaultShipyardUpgrades() {
    return {
        {ShipyardUpgradeType::RepairBay, "repair-bay", "Repair Bay", 5, {{"hull-plate", 4}, {"recovered-parts", 2}}},
        {ShipyardUpgradeType::ModulePrinter, "module-printer", "Module Printer", 5, {{"module-component", 2}, {"hull-plate", 3}}},
        {ShipyardUpgradeType::BlueprintArchive, "blueprint-archive", "Blueprint Archive", 5, {{"research-data", 2}, {"recovered-parts", 2}}},
        {ShipyardUpgradeType::DrydockExpansion, "drydock-expansion", "Drydock Expansion", 5, {{"hull-plate", 8}, {"ingot", 8}}},
        {ShipyardUpgradeType::DroneAutomation, "drone-automation", "Drone Automation", 5, {{"module-component", 3}, {"fuel", 2}}},
        {ShipyardUpgradeType::ResearchLab, "research-lab", "Research Lab", 5, {{"research-data", 4}, {"module-component", 2}}},
        {ShipyardUpgradeType::SolarPowerLink, "solar-power-link", "Solar Power Link", 5, {{"hull-plate", 6}, {"module-component", 2}}},
        {ShipyardUpgradeType::SubspaceLaunchArray, "subspace-launch-array", "Subspace Launch Array", 3, {{"module-component", 8}, {"fuel", 6}, {"research-data", 8}}}
    };
}

ShipyardProgressionState CreateStarterShipyardProgression() {
    ShipyardProgressionState state;
    state.shipyardLevel = 1;
    state.researchData = 0;
    state.blueprintProgress = 0;
    state.upgradeTiers["repair-bay"] = 1;
    state.upgradeTiers["blueprint-archive"] = 1;
    state.unlockedBlueprints = {"starter-scout", "starter-miner"};
    return state;
}

int GetShipyardUpgradeTier(const ShipyardProgressionState& state, const std::string& upgradeId) {
    const auto found = state.upgradeTiers.find(upgradeId);
    return found == state.upgradeTiers.end() ? 0 : found->second;
}

namespace {
std::vector<HomeInventoryStack> ScaledCost(const ShipyardProgressionState& state, const ShipyardUpgradeDefinition& upgrade) {
    const int nextTier = GetShipyardUpgradeTier(state, upgrade.id) + 1;
    std::vector<HomeInventoryStack> cost = upgrade.baseCost;
    for (auto& item : cost) {
        item.units = std::max(1, item.units * nextTier);
    }
    return cost;
}
}

bool CanAffordShipyardUpgrade(const HomeFactoryNetworkState& inventory,
                               const ShipyardProgressionState& state,
                               const ShipyardUpgradeDefinition& upgrade) {
    const int currentTier = GetShipyardUpgradeTier(state, upgrade.id);
    if (currentTier >= upgrade.maxTier) return false;
    for (const auto& item : ScaledCost(state, upgrade)) {
        const int available = item.commodity == "research-data" ? state.researchData : GetHomeInventoryUnits(inventory, item.commodity);
        if (available < item.units) return false;
    }
    return true;
}

bool ApplyShipyardUpgrade(HomeFactoryNetworkState& inventory,
                          ShipyardProgressionState& state,
                          const ShipyardUpgradeDefinition& upgrade) {
    if (!CanAffordShipyardUpgrade(inventory, state, upgrade)) return false;
    for (const auto& item : ScaledCost(state, upgrade)) {
        if (item.commodity == "research-data") state.researchData -= item.units;
        else AddHomeInventory(inventory, item.commodity, -item.units);
    }
    state.upgradeTiers[upgrade.id] = GetShipyardUpgradeTier(state, upgrade.id) + 1;
    state.shipyardLevel = std::max(state.shipyardLevel, 1 + EstimateShipyardBuildCapacity(state) / 2);
    return true;
}

int EstimateShipyardBuildCapacity(const ShipyardProgressionState& state) {
    return 1 + GetShipyardUpgradeTier(state, "drydock-expansion") + GetShipyardUpgradeTier(state, "module-printer") / 2;
}

std::string ShipyardUpgradeTypeName(ShipyardUpgradeType type) {
    switch (type) {
        case ShipyardUpgradeType::RepairBay: return "RepairBay";
        case ShipyardUpgradeType::ModulePrinter: return "ModulePrinter";
        case ShipyardUpgradeType::BlueprintArchive: return "BlueprintArchive";
        case ShipyardUpgradeType::DrydockExpansion: return "DrydockExpansion";
        case ShipyardUpgradeType::DroneAutomation: return "DroneAutomation";
        case ShipyardUpgradeType::ResearchLab: return "ResearchLab";
        case ShipyardUpgradeType::SolarPowerLink: return "SolarPowerLink";
        case ShipyardUpgradeType::SubspaceLaunchArray: return "SubspaceLaunchArray";
    }
    return "Unknown";
}

std::string ShipyardProgressionSummary(const ShipyardProgressionState& state) {
    std::ostringstream out;
    out << "Shipyard level=" << state.shipyardLevel
        << " capacity=" << EstimateShipyardBuildCapacity(state)
        << " research=" << state.researchData
        << " blueprints=" << state.unlockedBlueprints.size();
    return out.str();
}

} // namespace subspace
