#pragma once

#include <cstdint>
#include <string>

namespace subspace {

enum class DamageKind {
    Kinetic,
    Energy,
    Mining,
    Salvage,
    Thermal,
    Explosive
};

struct WeaponProfile {
    std::string id = "pulse-laser";
    DamageKind damageKind = DamageKind::Energy;
    float damage = 12.0f;
    float shieldPierce = 0.0f;
    float armorPierce = 0.0f;
    float cooldownSeconds = 0.8f;
};

struct ShieldProfile {
    float shield = 0.0f;
    float maxShield = 0.0f;
    float armor = 0.0f;
    float hull = 100.0f;
};

struct CombatHitResult {
    float shieldDamage = 0.0f;
    float armorDamage = 0.0f;
    float hullDamage = 0.0f;
    bool destroyed = false;
    std::string message;
};

CombatHitResult ResolveWeaponHit(const WeaponProfile& weapon, ShieldProfile& target, std::uint32_t seed = 0);
std::string DamageKindName(DamageKind kind);

} // namespace subspace
