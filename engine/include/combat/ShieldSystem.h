#pragma once

#include "core/ecs/Entity.h"
#include "core/ecs/IComponent.h"
#include "core/ecs/SystemBase.h"
#include "core/ecs/EntityManager.h"
#include "core/persistence/SaveGameManager.h"

#include <string>

namespace subspace {

/// Types of shield technology.
enum class ShieldType { Standard, Hardened, Phase, Regenerative };

/// ECS component giving an entity an advanced modular shield.
struct ShieldModuleComponent : public IComponent {
    ShieldType shieldType = ShieldType::Standard;
    float maxShield = 100.0f;
    float currentShield = 100.0f;
    float regenRate = 5.0f;         // HP per second
    float regenDelay = 3.0f;        // seconds after last hit before regen starts
    float timeSinceLastHit = 0.0f;
    bool isActive = true;
    float overchargeAmount = 0.0f;  // bonus shield above max, decays over time
    float overchargeDecayRate = 10.0f; // units per second

    /// Apply damage to the shield. Returns the overflow damage that passes through.
    float AbsorbDamage(float damage);

    /// Add overcharge (bonus shield above max capacity).
    void ApplyOvercharge(float amount);

    /// Get total effective shield (current + overcharge).
    float GetEffectiveShield() const;

    /// Get shield percentage (0-100) based on currentShield / maxShield.
    float GetShieldPercentage() const;

    /// Is the shield fully depleted?
    bool IsDepleted() const;

    /// Fully restore shield to max.
    void RestoreShield();

    /// Get the damage absorption multiplier for this shield type.
    static float GetAbsorptionMultiplier(ShieldType type);

    /// Serialize for save-game persistence.
    ComponentData Serialize() const;

    /// Restore from previously serialized data.
    void Deserialize(const ComponentData& data);
};

/// System that updates shield regen and overcharge decay each frame.
class ShieldSystem : public SystemBase {
public:
    ShieldSystem();
    explicit ShieldSystem(EntityManager& entityManager);

    void Update(float deltaTime) override;

private:
    EntityManager* _entityManager = nullptr;
};

} // namespace subspace
