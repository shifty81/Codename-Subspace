#include "power/PowerSystem.h"

#include <utility>
#include <vector>

namespace subspace {

// ---------------------------------------------------------------------------
// PowerComponent
// ---------------------------------------------------------------------------

float PowerComponent::GetAvailablePower() const {
    float generated = currentPowerGeneration * efficiency;
    return std::max(0.0f, generated - totalPowerConsumption);
}

bool PowerComponent::HasEnoughPower(float requiredPower) const {
    return GetAvailablePower() >= requiredPower;
}

float PowerComponent::GetPowerDeficit() const {
    return std::max(0.0f, totalPowerConsumption - (currentPowerGeneration * efficiency));
}

bool PowerComponent::IsLowPower() const {
    return GetPowerDeficit() > 0.0f;
}

void PowerComponent::UpdateTotalConsumption() {
    totalPowerConsumption = 0.0f;
    if (weaponsEnabled) totalPowerConsumption += weaponsPowerConsumption;
    if (shieldsEnabled) totalPowerConsumption += shieldsPowerConsumption;
    if (enginesEnabled) totalPowerConsumption += enginesPowerConsumption;
    if (systemsEnabled) totalPowerConsumption += systemsPowerConsumption;
}

void PowerComponent::ToggleSystem(PowerSystemType systemType) {
    switch (systemType) {
        case PowerSystemType::Weapons: weaponsEnabled = !weaponsEnabled; break;
        case PowerSystemType::Shields: shieldsEnabled = !shieldsEnabled; break;
        case PowerSystemType::Engines: enginesEnabled = !enginesEnabled; break;
        case PowerSystemType::Systems: systemsEnabled = !systemsEnabled; break;
    }
    UpdateTotalConsumption();
}

// ---------------------------------------------------------------------------
// PowerSystem
// ---------------------------------------------------------------------------

PowerSystem::PowerSystem() : SystemBase("PowerSystem") {}

void PowerSystem::Update(float /*deltaTime*/) {
    // Standalone: callers drive the per-entity loop externally.
}

void PowerSystem::CalculatePowerGeneration(PowerComponent& power, int generatorCount) {
    // Each generator contributes to max and current equally.
    // The actual per-generator wattage comes from the block stats in the
    // voxel layer, but we mirror the C# pattern: set max = current = voxel power.
    // Here we just set storage capacity from generator count.
    power.maxStoredPower = generatorCount * kStorageCapacityPerGenerator;
}

void PowerSystem::CalculatePowerConsumption(PowerComponent& power,
                                            int engineCount, int thrusterCount,
                                            int gyroCount, int shieldGenCount,
                                            int turretCount) {
    power.enginesPowerConsumption =
        engineCount * kEnginePowerConsumption +
        thrusterCount * kThrusterPowerConsumption +
        gyroCount * kGyroPowerConsumption;

    power.shieldsPowerConsumption = shieldGenCount * kShieldBaseConsumption;
    power.weaponsPowerConsumption = turretCount * kWeaponBaseConsumption;
    power.systemsPowerConsumption = kSystemsBaseConsumption;

    power.UpdateTotalConsumption();
}

int PowerSystem::DistributePower(PowerComponent& power) {
    if (power.GetPowerDeficit() <= 0.0f) return 0;

    // If stored power can cover the deficit, use it.
    if (power.currentStoredPower > 0.0f) {
        float needed = std::min(power.GetPowerDeficit(), power.currentStoredPower);
        power.currentStoredPower -= needed;
        return 0;
    }

    // Sort systems by priority (highest number = lowest importance, disabled first).
    struct Entry {
        PowerSystemType type;
        int priority;
    };
    std::vector<Entry> entries = {
        {PowerSystemType::Weapons, power.weaponsPriority},
        {PowerSystemType::Shields, power.shieldsPriority},
        {PowerSystemType::Engines, power.enginesPriority},
        {PowerSystemType::Systems, power.systemsPriority}
    };

    // Descending by priority number → lowest-importance first.
    std::sort(entries.begin(), entries.end(),
              [](const Entry& a, const Entry& b) { return a.priority > b.priority; });

    int disabled = 0;
    for (auto& e : entries) {
        if (power.GetPowerDeficit() <= 0.0f) break;

        bool enabled = false;
        switch (e.type) {
            case PowerSystemType::Weapons: enabled = power.weaponsEnabled; break;
            case PowerSystemType::Shields: enabled = power.shieldsEnabled; break;
            case PowerSystemType::Engines: enabled = power.enginesEnabled; break;
            case PowerSystemType::Systems: enabled = power.systemsEnabled; break;
        }

        if (enabled) {
            power.ToggleSystem(e.type);
            ++disabled;
        }
    }
    return disabled;
}

void PowerSystem::ChargePowerStorage(PowerComponent& power, float deltaTime) {
    if (power.currentStoredPower >= power.maxStoredPower) return;

    float excess = power.GetAvailablePower();
    if (excess <= 0.0f) return;

    float charge = std::min(kStorageChargeRate * deltaTime,
                            std::min(excess, power.maxStoredPower - power.currentStoredPower));
    power.currentStoredPower += charge;
}

} // namespace subspace
