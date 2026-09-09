#pragma once

#include "core/ecs/IComponent.h"
#include "core/ecs/SystemBase.h"

#include <algorithm>
#include <cmath>

namespace subspace {

/// Types of power systems on a ship.
enum class PowerSystemType {
    Weapons,
    Shields,
    Engines,
    Systems
};

/// Component for managing ship power generation and consumption.
/// Powers weapons, shields, engines, and other systems.
struct PowerComponent : public IComponent {
    // Power generation
    float maxPowerGeneration = 0.0f;
    float currentPowerGeneration = 0.0f;

    // Power consumption
    float totalPowerConsumption = 0.0f;
    float weaponsPowerConsumption = 0.0f;
    float shieldsPowerConsumption = 0.0f;
    float enginesPowerConsumption = 0.0f;
    float systemsPowerConsumption = 0.0f;

    // Power storage (capacitors)
    float maxStoredPower = 100.0f;
    float currentStoredPower = 100.0f;

    // Power efficiency
    float efficiency = 1.0f; // 1.0 = 100% efficient

    // System states
    bool weaponsEnabled = true;
    bool shieldsEnabled = true;
    bool enginesEnabled = true;
    bool systemsEnabled = true;

    // Priority system (1 = highest, 4 = lowest)
    int weaponsPriority = 2;
    int shieldsPriority = 1;
    int enginesPriority = 3;
    int systemsPriority = 4;

    /// Get available power after consumption.
    float GetAvailablePower() const;

    /// Check if there's enough power for a specific amount.
    bool HasEnoughPower(float requiredPower) const;

    /// Get power deficit (positive when consumption exceeds generation).
    float GetPowerDeficit() const;

    /// Check if ship is in low power state.
    bool IsLowPower() const;

    /// Update total power consumption from all enabled systems.
    void UpdateTotalConsumption();

    /// Toggle a system on/off.
    void ToggleSystem(PowerSystemType systemType);
};

/// System for managing ship power generation, distribution, and consumption.
/// Handles power priorities and automatic system shutdown when power is insufficient.
class PowerSystem : public SystemBase {
public:
    PowerSystem();

    void Update(float deltaTime) override;

    /// Calculate power generation from generator count.
    void CalculatePowerGeneration(PowerComponent& power, int generatorCount);

    /// Calculate power consumption from block/turret counts.
    void CalculatePowerConsumption(PowerComponent& power,
                                   int engineCount, int thrusterCount,
                                   int gyroCount, int shieldGenCount,
                                   int turretCount);

    /// Distribute power based on priorities when there's insufficient power.
    /// Returns the number of systems that were disabled.
    int DistributePower(PowerComponent& power);

    /// Charge power storage when excess power is available.
    void ChargePowerStorage(PowerComponent& power, float deltaTime);

    // Power consumption rates (per unit)
    static constexpr float kEnginePowerConsumption = 5.0f;
    static constexpr float kThrusterPowerConsumption = 3.0f;
    static constexpr float kShieldBaseConsumption = 10.0f;
    static constexpr float kWeaponBaseConsumption = 8.0f;
    static constexpr float kGyroPowerConsumption = 2.0f;
    static constexpr float kSystemsBaseConsumption = 5.0f;

    // Power storage rates
    static constexpr float kStorageChargeRate = 10.0f;
    static constexpr float kStorageCapacityPerGenerator = 50.0f;
};

} // namespace subspace
