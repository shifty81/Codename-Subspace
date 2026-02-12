#include "combat/CombatSystem.h"

namespace subspace {

// ---------------------------------------------------------------------------
// ShieldComponent
// ---------------------------------------------------------------------------
float ShieldComponent::GetShieldPercentage() const {
    if (maxShieldHP <= 0.0f) return 0.0f;
    return (currentShieldHP / maxShieldHP) * 100.0f;
}

bool ShieldComponent::IsShieldDepleted() const {
    return currentShieldHP <= 0.0f;
}

float ShieldComponent::AbsorbDamage(float damage) {
    if (!isShieldActive || currentShieldHP <= 0.0f) return damage;

    if (damage <= currentShieldHP) {
        currentShieldHP -= damage;
        return 0.0f;
    }

    float overflow = damage - currentShieldHP;
    currentShieldHP = 0.0f;
    return overflow;
}

// ---------------------------------------------------------------------------
// CombatComponent
// ---------------------------------------------------------------------------
bool CombatComponent::HasEnergy(float amount) const {
    return currentEnergy >= amount;
}

bool CombatComponent::ConsumeEnergy(float amount) {
    if (currentEnergy < amount) return false;
    currentEnergy -= amount;
    return true;
}

void CombatComponent::RegenerateEnergy(float deltaTime) {
    currentEnergy = std::min(currentEnergy + energyRegenRate * deltaTime, energyCapacity);
}

void CombatComponent::RegenerateShields(float deltaTime) {
    shields.timeSinceLastHit += deltaTime;
    if (shields.timeSinceLastHit >= shields.shieldRechargeDelay) {
        shields.currentShieldHP = std::min(
            shields.currentShieldHP + shields.shieldRegenRate * deltaTime,
            shields.maxShieldHP);
    }
}

// ---------------------------------------------------------------------------
// CombatSystem
// ---------------------------------------------------------------------------
CombatSystem::CombatSystem() : SystemBase("CombatSystem") {}

void CombatSystem::Update(float deltaTime) {
    UpdateProjectiles(deltaTime);
    // TODO: iterate CombatComponents for regen once ECS integration is in place
}

void CombatSystem::SpawnProjectile(const Projectile& proj) {
    _activeProjectiles.push_back(proj);
}

DamageInfo CombatSystem::CalculateDamage(float baseDamage, DamageType type, float armorRating) const {
    float reduction = GetArmorReduction(armorRating, type);
    float finalDamage = std::max(baseDamage - reduction, 0.0f);

    DamageInfo info;
    info.damage = finalDamage;
    info.damageType = type;
    return info;
}

float CombatSystem::ApplyDamageToTarget(CombatComponent& target, const DamageInfo& info) {
    float effectiveDamage = info.damage * GetShieldEffectiveness(info.damageType);

    // Shields absorb first
    if (target.shields.isShieldActive && !target.shields.IsShieldDepleted()) {
        target.shields.timeSinceLastHit = 0.0f;
        float overflow = target.shields.AbsorbDamage(effectiveDamage);

        if (overflow <= 0.0f) {
            return info.damage; // all absorbed by shields
        }

        // Overflow goes through armor to hull
        float armorReduction = GetArmorReduction(target.armorRating, info.damageType);
        float hullDamage = std::max(overflow - armorReduction, 0.0f);
        return info.damage - (overflow - hullDamage);
    }

    // No shields – apply armor reduction directly
    float armorReduction = GetArmorReduction(target.armorRating, info.damageType);
    float hullDamage = std::max(info.damage - armorReduction, 0.0f);
    return hullDamage;
}

void CombatSystem::UpdateProjectiles(float deltaTime) {
    for (auto& proj : _activeProjectiles) {
        proj.position = proj.position + proj.velocity * deltaTime;
        proj.lifetime -= deltaTime;
    }

    _activeProjectiles.erase(
        std::remove_if(_activeProjectiles.begin(), _activeProjectiles.end(),
                       [](const Projectile& p) { return p.lifetime <= 0.0f; }),
        _activeProjectiles.end());
}

const std::vector<Projectile>& CombatSystem::GetActiveProjectiles() const {
    return _activeProjectiles;
}

void CombatSystem::ClearAllProjectiles() {
    _activeProjectiles.clear();
}

int CombatSystem::GetActiveProjectileCount() const {
    return static_cast<int>(_activeProjectiles.size());
}

float CombatSystem::GetArmorReduction(float armorRating, DamageType type) {
    switch (type) {
        case DamageType::Kinetic:   return armorRating * 0.50f;
        case DamageType::Energy:    return armorRating * 0.25f;
        case DamageType::Explosive: return armorRating * 0.75f;
        case DamageType::Thermal:   return armorRating * 0.10f;
        case DamageType::EMP:       return armorRating * 0.00f;
    }
    return 0.0f; // fallback
}

float CombatSystem::GetShieldEffectiveness(DamageType type) {
    switch (type) {
        case DamageType::Kinetic:   return 0.80f;
        case DamageType::Energy:    return 1.00f;
        case DamageType::Explosive: return 0.60f;
        case DamageType::Thermal:   return 0.90f;
        case DamageType::EMP:       return 1.20f;
    }
    return 1.0f; // fallback
}

} // namespace subspace
