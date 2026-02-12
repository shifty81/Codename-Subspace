#pragma once

#include "core/ecs/Entity.h"
#include "core/ecs/IComponent.h"
#include "core/ecs/SystemBase.h"
#include "core/Math.h"

#include <algorithm>
#include <vector>

namespace subspace {

/// Types of damage that can be dealt in combat.
enum class DamageType { Kinetic, Energy, Explosive, Thermal, EMP };

/// Describes a single damage event between entities.
struct DamageInfo {
    EntityId sourceEntityId = InvalidEntityId;
    EntityId targetEntityId = InvalidEntityId;
    float damage = 0.0f;
    DamageType damageType = DamageType::Kinetic;
    Vector3 hitPosition;
    float damageRadius = 0.0f;   // for area damage
    bool isPiercing = false;
};

/// Shield state attached to a combat entity.
struct ShieldComponent {
    float maxShieldHP = 100.0f;
    float currentShieldHP = 100.0f;
    float shieldRegenRate = 10.0f;    // per second
    float shieldRechargeDelay = 5.0f; // seconds after hit before regen starts
    float timeSinceLastHit = 100.0f;  // starts high so regen works immediately
    bool isShieldActive = true;

    /// Returns current shield health as a percentage of max.
    float GetShieldPercentage() const;

    /// Returns true when shields are fully depleted.
    bool IsShieldDepleted() const;

    /// Absorb as much damage as possible. Returns overflow damage.
    float AbsorbDamage(float damage);
};

/// A live projectile in the world.
struct Projectile {
    EntityId ownerId = InvalidEntityId;
    Vector3 position;
    Vector3 velocity;
    float damage = 0.0f;
    DamageType damageType = DamageType::Kinetic;
    float lifetime = 0.0f;       // remaining seconds
    float damageRadius = 0.0f;   // for explosive rounds
};

/// Component that gives an entity combat capabilities.
struct CombatComponent : public IComponent {
    ShieldComponent shields;
    float armorRating = 0.0f;      // reduces incoming damage
    bool autoTargetEnabled = false;
    EntityId currentTargetId = InvalidEntityId;
    float energyCapacity = 100.0f;
    float currentEnergy = 100.0f;
    float energyRegenRate = 20.0f;

    /// Returns true if the entity has at least the given amount of energy.
    bool HasEnergy(float amount) const;

    /// Consume energy. Returns false if insufficient.
    bool ConsumeEnergy(float amount);

    /// Regenerate energy over time, clamped to capacity.
    void RegenerateEnergy(float deltaTime);

    /// Regenerate shields if the recharge delay has elapsed.
    void RegenerateShields(float deltaTime);
};

/// Manages projectiles and damage resolution.
class CombatSystem : public SystemBase {
public:
    CombatSystem();

    /// Update projectiles and regeneration each frame.
    void Update(float deltaTime) override;

    /// Add a projectile to the simulation.
    void SpawnProjectile(const Projectile& proj);

    /// Build a DamageInfo with armor-reduced damage.
    DamageInfo CalculateDamage(float baseDamage, DamageType type, float armorRating) const;

    /// Apply damage to a target: shields first, then overflow to hull.
    /// Returns actual damage dealt.
    float ApplyDamageToTarget(CombatComponent& target, const DamageInfo& info);

    /// Move projectiles and remove expired ones.
    void UpdateProjectiles(float deltaTime);

    const std::vector<Projectile>& GetActiveProjectiles() const;
    void ClearAllProjectiles();
    int GetActiveProjectileCount() const;

    /// Armor reduction factor based on armor rating and damage type.
    static float GetArmorReduction(float armorRating, DamageType type);

    /// Shield effectiveness multiplier for a given damage type.
    static float GetShieldEffectiveness(DamageType type);

private:
    std::vector<Projectile> _activeProjectiles;
};

} // namespace subspace
