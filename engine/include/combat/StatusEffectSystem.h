#pragma once

#include "core/ecs/Entity.h"
#include "core/ecs/IComponent.h"
#include "core/ecs/SystemBase.h"
#include "core/ecs/EntityManager.h"
#include "core/persistence/SaveGameManager.h"

#include <string>
#include <vector>

namespace subspace {

/// Types of status effects.
enum class StatusEffectType {
    EMPDisruption,    // Disables weapons/shields temporarily
    FireDOT,          // Damage over time (fire)
    RadiationDOT,     // Damage over time (radiation)
    ShieldDrain,      // Continuously drains shield
    EngineJam,        // Reduces speed
    SensorScramble    // Reduces targeting accuracy
};

/// A single active status effect instance.
struct StatusEffect {
    StatusEffectType type = StatusEffectType::EMPDisruption;
    float duration = 5.0f;       // total effect duration in seconds
    float remainingTime = 5.0f;  // time left before expiry
    float tickInterval = 1.0f;   // time between effect ticks
    float tickTimer = 0.0f;      // time since last tick
    float magnitude = 10.0f;     // strength of the effect (damage per tick, speed %, etc.)
    EntityId sourceId = InvalidEntityId; // who applied this

    /// Is this effect still active?
    bool IsActive() const;

    /// Get progress as percentage (0-100, 100 = just applied, 0 = about to expire).
    float GetRemainingPercent() const;

    /// Get the display name for this effect type.
    static std::string GetEffectName(StatusEffectType type);

    /// Get the default duration for an effect type.
    static float GetDefaultDuration(StatusEffectType type);

    /// Get the default magnitude for an effect type.
    static float GetDefaultMagnitude(StatusEffectType type);
};

/// ECS component that holds all active status effects on an entity.
struct StatusEffectComponent : public IComponent {
    std::vector<StatusEffect> activeEffects;
    static constexpr size_t kMaxEffects = 10;

    /// Is the entity immune to status effects?
    bool isImmune = false;

    /// Global resistance multiplier (0.0 = immune, 1.0 = normal, >1.0 = vulnerable).
    float resistanceMultiplier = 1.0f;

    /// Apply a new effect. Returns true if applied, false if immune or at capacity.
    bool ApplyEffect(const StatusEffect& effect);

    /// Remove all effects of a given type.
    void RemoveEffectsByType(StatusEffectType type);

    /// Remove all expired effects.
    void ClearExpired();

    /// Check if entity has an active effect of given type.
    bool HasEffect(StatusEffectType type) const;

    /// Get the strongest magnitude of a given effect type (or 0 if none).
    float GetEffectMagnitude(StatusEffectType type) const;

    /// Count of active effects.
    size_t GetActiveCount() const;

    /// Serialize for save-game persistence.
    ComponentData Serialize() const;

    /// Restore from previously serialized data.
    void Deserialize(const ComponentData& data);
};

/// System that ticks status effects each frame and removes expired ones.
class StatusEffectSystem : public SystemBase {
public:
    StatusEffectSystem();
    explicit StatusEffectSystem(EntityManager& entityManager);

    void Update(float deltaTime) override;

private:
    EntityManager* _entityManager = nullptr;
};

} // namespace subspace
