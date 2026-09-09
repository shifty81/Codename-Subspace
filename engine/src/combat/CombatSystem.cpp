#include "combat/CombatSystem.h"
#include <algorithm>

namespace subspace {

// ---------------------------------------------------------------------------
// Armor reduction multipliers per damage type
// ---------------------------------------------------------------------------
static constexpr float kArmorReduction_Kinetic   = 0.50f;
static constexpr float kArmorReduction_Energy    = 0.25f;
static constexpr float kArmorReduction_Explosive = 0.75f;
static constexpr float kArmorReduction_Thermal   = 0.10f;
static constexpr float kArmorReduction_EMP       = 0.00f;

// ---------------------------------------------------------------------------
// Shield effectiveness multipliers per damage type
// ---------------------------------------------------------------------------
static constexpr float kShieldEffect_Kinetic   = 0.80f;
static constexpr float kShieldEffect_Energy    = 1.00f;
static constexpr float kShieldEffect_Explosive = 0.60f;
static constexpr float kShieldEffect_Thermal   = 0.90f;
static constexpr float kShieldEffect_EMP       = 1.20f;

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
    if (shields.timeSinceLastHit >= shields.shieldRechargeDelay && shields.isShieldActive) {
        shields.currentShieldHP = std::min(
            shields.currentShieldHP + shields.shieldRegenRate * deltaTime,
            shields.maxShieldHP);
    }
}

namespace {
float SafeFraction(float current, float maximum) {
    if (maximum <= 0.0f) return 0.0f;
    return std::clamp(current / maximum, 0.0f, 1.0f);
}
}

void CombatComponent::ConfigureDurability(float shieldHP, float armorHP, float hullHP,
                                          float energyHP, bool preserveDamageFraction) {
    const float shieldFraction = preserveDamageFraction ? ShieldFraction() : 1.0f;
    const float armorFraction = preserveDamageFraction ? ArmorFraction() : 1.0f;
    const float hullFraction = preserveDamageFraction ? HullFraction() : 1.0f;
    const float powerFraction = preserveDamageFraction ? PowerFraction() : 1.0f;

    shields.maxShieldHP = std::max(0.0f, shieldHP);
    shields.currentShieldHP = shields.maxShieldHP * shieldFraction;
    shields.isShieldActive = shields.maxShieldHP > 0.0f;
    maxArmorHP = std::max(0.0f, armorHP);
    currentArmorHP = maxArmorHP * armorFraction;
    maxHullHP = std::max(1.0f, hullHP);
    currentHullHP = maxHullHP * hullFraction;
    energyCapacity = std::max(1.0f, energyHP);
    currentEnergy = energyCapacity * powerFraction;
}

float CombatComponent::ShieldFraction() const { return SafeFraction(shields.currentShieldHP, shields.maxShieldHP); }
float CombatComponent::ArmorFraction() const { return SafeFraction(currentArmorHP, maxArmorHP); }
float CombatComponent::HullFraction() const { return SafeFraction(currentHullHP, maxHullHP); }
float CombatComponent::PowerFraction() const { return SafeFraction(currentEnergy, energyCapacity); }

// ---------------------------------------------------------------------------
// CombatSystem
// ---------------------------------------------------------------------------
CombatSystem::CombatSystem() : SystemBase("CombatSystem") {}

CombatSystem::CombatSystem(EntityManager& entityManager)
    : SystemBase("CombatSystem")
    , _entityManager(&entityManager)
{
}

void CombatSystem::Update(float deltaTime) {
    UpdateProjectiles(deltaTime);

    if (_entityManager) {
        auto combatComponents = _entityManager->GetAllComponents<CombatComponent>();
        for (auto* comp : combatComponents) {
            comp->RegenerateEnergy(deltaTime);
            comp->RegenerateShields(deltaTime);
        }
    }
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
    if (info.damage <= 0.0f || target.IsDestroyed()) return 0.0f;

    float removed = 0.0f;
    float remaining = std::max(0.0f, info.damage);

    // Shield effectiveness changes how much of this damage family the shield
    // must absorb, but overflow is converted back to ordinary durability damage
    // so armor/hull do not inherit the shield multiplier a second time.
    if (target.shields.isShieldActive && !target.shields.IsShieldDepleted()) {
        target.shields.timeSinceLastHit = 0.0f;
        const float shieldMultiplier = std::max(0.01f, GetShieldEffectiveness(info.damageType));
        const float shieldDemand = remaining * shieldMultiplier;
        const float shieldBefore = target.shields.currentShieldHP;
        const float overflowDemand = target.shields.AbsorbDamage(shieldDemand);
        const float absorbedDemand = shieldBefore - target.shields.currentShieldHP;
        const float absorbedBase = absorbedDemand / shieldMultiplier;
        removed += absorbedDemand;
        remaining = std::max(0.0f, overflowDemand / shieldMultiplier);
        if (remaining <= 0.0f) return removed > 0.0f ? removed : absorbedBase;
    }

    // Armor rating is a per-hit resistance; surviving damage consumes actual
    // armor integrity before any hull integrity can be removed.
    const float mitigation = GetArmorReduction(target.armorRating, info.damageType);
    remaining = std::max(0.0f, remaining - mitigation);
    if (remaining <= 0.0f) return removed;

    if (target.currentArmorHP > 0.0f) {
        const float armorDamage = std::min(target.currentArmorHP, remaining);
        target.currentArmorHP -= armorDamage;
        remaining -= armorDamage;
        removed += armorDamage;
    }

    if (remaining > 0.0f && target.currentHullHP > 0.0f) {
        const float hullDamage = std::min(target.currentHullHP, remaining);
        target.currentHullHP -= hullDamage;
        removed += hullDamage;
    }
    return removed;
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
        case DamageType::Kinetic:   return armorRating * kArmorReduction_Kinetic;
        case DamageType::Energy:    return armorRating * kArmorReduction_Energy;
        case DamageType::Explosive: return armorRating * kArmorReduction_Explosive;
        case DamageType::Thermal:   return armorRating * kArmorReduction_Thermal;
        case DamageType::EMP:       return armorRating * kArmorReduction_EMP;
    }
    return 0.0f; // fallback
}

float CombatSystem::GetShieldEffectiveness(DamageType type) {
    switch (type) {
        case DamageType::Kinetic:   return kShieldEffect_Kinetic;
        case DamageType::Energy:    return kShieldEffect_Energy;
        case DamageType::Explosive: return kShieldEffect_Explosive;
        case DamageType::Thermal:   return kShieldEffect_Thermal;
        case DamageType::EMP:       return kShieldEffect_EMP;
    }
    return 1.0f; // fallback
}

} // namespace subspace
