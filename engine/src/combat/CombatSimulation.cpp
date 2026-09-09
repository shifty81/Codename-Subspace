#include "combat/CombatSimulation.h"

#include <algorithm>
#include <sstream>

namespace subspace {

CombatHitResult ResolveWeaponHit(const WeaponProfile& weapon, ShieldProfile& target, std::uint32_t)
{
    CombatHitResult result;
    float remaining = std::max(0.0f, weapon.damage);
    const float shieldBypass = std::min(0.95f, std::max(0.0f, weapon.shieldPierce));
    const float armorBypass = std::min(0.95f, std::max(0.0f, weapon.armorPierce));

    if (target.shield > 0.0f) {
        const float toShield = remaining * (1.0f - shieldBypass);
        result.shieldDamage = std::min(target.shield, toShield);
        target.shield -= result.shieldDamage;
        remaining -= result.shieldDamage;
    }
    if (target.armor > 0.0f && remaining > 0.0f) {
        const float toArmor = remaining * (1.0f - armorBypass);
        result.armorDamage = std::min(target.armor, toArmor);
        target.armor -= result.armorDamage;
        remaining -= result.armorDamage;
    }
    if (remaining > 0.0f) {
        result.hullDamage = std::min(target.hull, remaining);
        target.hull -= result.hullDamage;
    }

    result.destroyed = target.hull <= 0.0f;
    std::ostringstream stream;
    stream << DamageKindName(weapon.damageKind) << " hit shield=" << result.shieldDamage
           << " armor=" << result.armorDamage << " hull=" << result.hullDamage;
    if (result.destroyed) {
        stream << " destroyed";
    }
    result.message = stream.str();
    return result;
}

std::string DamageKindName(DamageKind kind)
{
    switch (kind) {
        case DamageKind::Kinetic: return "Kinetic";
        case DamageKind::Energy: return "Energy";
        case DamageKind::Mining: return "Mining";
        case DamageKind::Salvage: return "Salvage";
        case DamageKind::Thermal: return "Thermal";
        case DamageKind::Explosive: return "Explosive";
        default: return "Unknown";
    }
}

} // namespace subspace
