#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace subspace {

enum class ShipPartCategory {
    Hull,
    Cockpit,
    Reactor,
    MainThruster,
    ManeuverThruster,
    MiningTool,
    Weapon,
    Shield,
    Cargo,
    Scanner,
    Utility
};

struct ShipPartDefinition {
    std::string id;
    std::string displayName;
    ShipPartCategory category = ShipPartCategory::Utility;
    int tier = 1;
    int installCostCredits = 0;
    int mass = 0;
    int powerDraw = 0;
    float thrustMultiplier = 1.0f;
    float turnMultiplier = 1.0f;
    float fuelBurnMultiplier = 1.0f;
    float cargoMultiplier = 1.0f;
    float miningMultiplier = 1.0f;
    float shieldBonus = 0.0f;
    bool hotSwappableAtHome = true;
    bool expeditionLocked = true;
};

struct ShipLoadoutSlot {
    ShipPartCategory category = ShipPartCategory::Utility;
    std::string partId;
};

struct ShipLoadout {
    std::string id = "starter-skiff-loadout";
    std::string displayName = "Starter Skiff";
    std::vector<ShipLoadoutSlot> slots;
};

struct ShipPartStats {
    float thrust = 430.0f;
    float reverseThrust = 280.0f;
    float strafeThrust = 320.0f;
    float turnSpeed = 2.7f;
    float maxSpeed = 620.0f;
    float fuelCapacity = 100.0f;
    float fuelBurnPerSecond = 1.0f;
    float miningEfficiency = 1.0f;
    float shieldBonus = 0.0f;
    float shieldCapacity = 0.0f;
    float scannerRange = 1.0f;
    int cargoCapacity = 24;
    int mass = 0;
    int powerDraw = 0;
};

struct ShipPartInstallResult {
    bool success = false;
    std::string message;
    int costCredits = 0;
};

const char* ShipPartCategoryName(ShipPartCategory category);

std::vector<ShipPartDefinition> CreateStarterShipPartCatalog();
ShipLoadout CreateStarterShipLoadout();
ShipPartStats CalculateShipPartStats(const ShipLoadout& loadout, const std::vector<ShipPartDefinition>& catalog);
const ShipPartDefinition* FindShipPart(const std::vector<ShipPartDefinition>& catalog, const std::string& partId);
std::vector<ShipPartDefinition> FilterShipPartsByCategory(const std::vector<ShipPartDefinition>& catalog, ShipPartCategory category);
ShipPartInstallResult InstallShipPart(ShipLoadout& loadout,
                                      const ShipPartDefinition& part,
                                      int availableCredits,
                                      bool atHome,
                                      bool expeditionActive);
std::string ShipLoadoutSummary(const ShipLoadout& loadout, const std::vector<ShipPartDefinition>& catalog);

} // namespace subspace
