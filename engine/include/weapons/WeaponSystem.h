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
};

} // namespace subspace
