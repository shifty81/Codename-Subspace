#include "weapons/WeaponSystem.h"
#include "ships/BlockPlacement.h"

namespace subspace {

// ---------------------------------------------------------------------------
// AmmoPool
// ---------------------------------------------------------------------------
bool AmmoPool::CanFire() const {
    return !isReloading && currentAmmo > 0;
}

bool AmmoPool::ConsumeAmmo() {
    if (!CanFire()) return false;
    --currentAmmo;
    return true;
}

void AmmoPool::StartReload() {
    isReloading = true;
    currentReloadTimer = reloadTime;
}

bool AmmoPool::UpdateReload(float deltaTime) {
    if (!isReloading) return false;
    currentReloadTimer -= deltaTime;
    if (currentReloadTimer <= 0.0f) {
        currentAmmo = maxAmmo;
        isReloading = false;
        currentReloadTimer = 0.0f;
        return true;
    }
    return false;
}

void AmmoPool::Refill() {
    currentAmmo = maxAmmo;
    isReloading = false;
    currentReloadTimer = 0.0f;
}

float AmmoPool::GetAmmoPercentage() const {
    if (maxAmmo <= 0) return 0.0f;
    return (currentAmmo / (float)maxAmmo) * 100.0f;
}

// ---------------------------------------------------------------------------
// WeaponStats
// ---------------------------------------------------------------------------
float WeaponStats::EffectiveDPS() const {
    if (cooldown <= 0.0f) return 0.0f;
    return (damage * accuracy * (arcDegrees / 360.0f)) / cooldown;
}

// ---------------------------------------------------------------------------
// WeaponSystem
// ---------------------------------------------------------------------------
WeaponStats WeaponSystem::GetWeaponStats(WeaponType type) {
    switch (type) {
        case WeaponType::BroadsideCannon:
            return { 120.0f, 4.0f, 120.0f, 0.75f, 20.0f };
        case WeaponType::SpinalRailgun:
            return { 800.0f, 12.0f, 5.0f, 0.95f, 50.0f };
        case WeaponType::InwardFlak:
            return { 240.0f, 3.0f, 180.0f, 0.6f, 15.0f };
        case WeaponType::BurstLancer:
            return { 900.0f, 15.0f, 15.0f, 0.85f, 35.0f };
        case WeaponType::BeamArray:
            return { 35.0f, 1.0f, 60.0f, 1.0f, 40.0f };
    }
    return {}; // fallback
}

std::vector<std::pair<WeaponType, WeaponStats>> WeaponSystem::GetAllWeaponStats() {
    return {
        { WeaponType::BroadsideCannon, GetWeaponStats(WeaponType::BroadsideCannon) },
        { WeaponType::SpinalRailgun,   GetWeaponStats(WeaponType::SpinalRailgun)   },
        { WeaponType::InwardFlak,      GetWeaponStats(WeaponType::InwardFlak)      },
        { WeaponType::BurstLancer,     GetWeaponStats(WeaponType::BurstLancer)     },
        { WeaponType::BeamArray,       GetWeaponStats(WeaponType::BeamArray)       },
    };
}

bool WeaponSystem::IsValidHardpoint(const Ship& ship, const Block& block) {
    auto adjacentCells = BlockPlacement::GetAdjacentCells(block);
    for (const auto& cell : adjacentCells) {
        if (ship.occupiedCells.find(cell) == ship.occupiedCells.end()) {
            return true; // At least one face is exposed (not adjacent to another block)
        }
    }
    return false;
}

AmmoPool WeaponSystem::GetDefaultAmmoPool(WeaponType type) {
    AmmoPool pool;
    switch (type) {
        case WeaponType::BroadsideCannon:
            pool.type = AmmoType::Standard;
            pool.maxAmmo = 30;
            pool.currentAmmo = 30;
            pool.reloadTime = 4.0f;
            break;
        case WeaponType::SpinalRailgun:
            pool.type = AmmoType::ArmorPiercing;
            pool.maxAmmo = 5;
            pool.currentAmmo = 5;
            pool.reloadTime = 8.0f;
            break;
        case WeaponType::InwardFlak:
            pool.type = AmmoType::Explosive;
            pool.maxAmmo = 60;
            pool.currentAmmo = 60;
            pool.reloadTime = 3.0f;
            break;
        case WeaponType::BurstLancer:
            pool.type = AmmoType::Incendiary;
            pool.maxAmmo = 8;
            pool.currentAmmo = 8;
            pool.reloadTime = 6.0f;
            break;
        case WeaponType::BeamArray:
            pool.type = AmmoType::EMP;
            pool.maxAmmo = 200;
            pool.currentAmmo = 200;
            pool.reloadTime = 2.0f;
            break;
    }
    return pool;
}

float WeaponSystem::GetAmmoDamageMultiplier(AmmoType ammoType) {
    switch (ammoType) {
        case AmmoType::Standard:      return 1.0f;
        case AmmoType::ArmorPiercing: return 1.3f;
        case AmmoType::Explosive:     return 1.5f;
        case AmmoType::EMP:           return 0.5f;
        case AmmoType::Incendiary:    return 1.2f;
    }
    return 1.0f; // fallback
}

} // namespace subspace
