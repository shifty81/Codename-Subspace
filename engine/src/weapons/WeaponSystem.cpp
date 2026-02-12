#include "weapons/WeaponSystem.h"
#include "ships/BlockPlacement.h"

namespace subspace {

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

} // namespace subspace
