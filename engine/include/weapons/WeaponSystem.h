#pragma once

#include "ships/Block.h"
#include "ships/Ship.h"

#include <utility>
#include <vector>

namespace subspace {

enum class WeaponType { BroadsideCannon, SpinalRailgun, InwardFlak, BurstLancer, BeamArray };
enum class HardpointSize { Small, Medium, Large };

struct WeaponStats {
    float damage;
    float cooldown;
    float arcDegrees;
    float accuracy;
    float powerDraw;

    float EffectiveDPS() const;
};

/// Types of ammunition used by different weapons.
enum class AmmoType { Standard, ArmorPiercing, Explosive, EMP, Incendiary };

/// Ammunition pool: tracks current/max ammo and reload state.
struct AmmoPool {
    AmmoType type = AmmoType::Standard;
    int maxAmmo = 100;
    int currentAmmo = 100;
    float reloadTime = 3.0f;        // seconds for a full reload
    float currentReloadTimer = 0.0f; // remaining reload time (0 = ready)
    bool isReloading = false;

    /// Returns true if there's at least one round available and not reloading.
    bool CanFire() const;

    /// Consume one round. Returns false if empty or reloading.
    bool ConsumeAmmo();

    /// Begin a reload cycle.
    void StartReload();

    /// Advance the reload timer by deltaTime. Returns true when reload completes.
    bool UpdateReload(float deltaTime);

    /// Immediately fill ammo to max and cancel reload.
    void Refill();

    /// Returns ammo as percentage of max (0-100).
    float GetAmmoPercentage() const;
};

struct WeaponMountBlock {
    Block block;
    HardpointSize size;
    float rotationArc;
    WeaponType weaponType;
};

struct Turret {
    WeaponMountBlock* mount = nullptr;
    float aimYaw = 0.0f;
    float aimPitch = 0.0f;
    float cooldownRemaining = 0.0f;
};

class WeaponSystem {
public:
    // Check if a block is a valid hardpoint (has exposed face)
    static bool IsValidHardpoint(const Ship& ship, const Block& block);

    // Get weapon stats for an archetype
    static WeaponStats GetWeaponStats(WeaponType type);

    // Get all weapon archetypes
    static std::vector<std::pair<WeaponType, WeaponStats>> GetAllWeaponStats();

    /// Get the default AmmoPool for a given weapon type.
    static AmmoPool GetDefaultAmmoPool(WeaponType type);

    /// Get the damage multiplier for a given ammo type.
    static float GetAmmoDamageMultiplier(AmmoType ammoType);
};

} // namespace subspace
