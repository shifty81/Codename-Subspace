#pragma once

#include "combat/AdvancedCombatSystem.h"
#include "combat/ElectronicWarfareSystem.h"
#include "combat/ShipFailureSystem.h"
#include "fleet/DroneOperationsSystem.h"
#include "rendering/EnvironmentPresentationSystem.h"
#include "ships/EngineeringRepairSystem.h"
#include "ships/EnvironmentalHazardSystem.h"
#include "ships/FittingSystem.h"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace subspace {

struct InstalledEquipmentRuntime {
    std::string instanceId;
    std::string equipmentId;
    std::string subsystemId;
    std::string weaponRuntimeId;
    std::string ammoCommodity;
    bool weapon = false;
    bool droneBay = false;
};

struct ProductionCombatShip {
    std::string shipId;
    FittingCapacity capacity;
    std::vector<std::string> fittedEquipment;
    std::unordered_map<std::string, InstalledEquipmentRuntime> installed;
    double availablePower = 100.0;
    double generatedHeat = 0.0;
    double baseSensorRange = 20000.0;
};

struct SensorIntegrationReport {
    double range = 0.0;
    double sensorMultiplier = 1.0;
    double missileMultiplier = 1.0;
    double propulsionMultiplier = 1.0;
    double droneMultiplier = 1.0;
};

/// Pass266-270 integration authority. It binds the existing fitting catalog,
/// combat runtime, physical subsystem failures, engineering repairs, drones,
/// EWAR and environment penalties into one installed-equipment path.
class CombatShipIntegration {
public:
    bool Install(ProductionCombatShip& ship, const std::string& equipmentId,
                 const std::string& instanceId, const FittingSystem& fitting,
                 AdvancedCombatSystem& combat, ShipFailureSystem& failures,
                 DroneOperationsSystem& drones) const;

    CombatFireResult Fire(ProductionCombatShip& ship, const std::string& instanceId,
                          AdvancedCombatSystem& combat,
                          const ShipFailureSystem& failures) const;

    bool ApplyModuleDamage(const ProductionCombatShip& ship, const std::string& instanceId,
                           double damage, ShipFailureSystem& failures) const;

    int ReloadFromInventory(const ProductionCombatShip& ship, const std::string& instanceId,
                            int requestedRounds,
                            std::unordered_map<std::string, std::uint64_t>& inventory,
                            AdvancedCombatSystem& combat) const;

    bool BeginRepair(const ProductionCombatShip& ship, const std::string& instanceId,
                     const ShipFailureSystem& failures, EngineeringRepairSystem& repairs,
                     double integrityPerSecond = 5.0) const;

    double AdvanceRepair(const ProductionCombatShip& ship, const std::string& instanceId,
                         double seconds, EngineeringRepairSystem& repairs,
                         ShipFailureSystem& failures) const;

    SensorIntegrationReport EvaluateSensors(const ProductionCombatShip& ship,
                                             const ElectronicWarfareSystem& ewar,
                                             const EnvironmentalExposureReport& environment,
                                             const ShipFailureSystem& failures) const;

private:
    static ShipSubsystemType SubsystemFor(EquipmentCategory category);
    static bool IsWeapon(EquipmentCategory category);
    static bool IsDroneBay(EquipmentCategory category);
    static DroneRole DroneRoleFor(EquipmentCategory category);
    static CombatWeaponSpec WeaponSpecFor(const EquipmentDefinitionNative& equipment,
                                          const std::string& runtimeId);
    static std::string AmmoCommodityFor(const EquipmentDefinitionNative& equipment);
};

} // namespace subspace
