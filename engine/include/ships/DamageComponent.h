#pragma once

#include "combat/CombatSystem.h"
#include "core/Math.h"
#include "core/ecs/IComponent.h"
#include "core/persistence/SaveGameManager.h"

#include <string>
#include <vector>

namespace subspace {

/// A single recorded damage event.
struct DamageRecord {
    float timestamp = 0.0f;
    float damageAmount = 0.0f;
    DamageType damageType = DamageType::Kinetic;
    Vector3Int hitPosition;
};

/// ECS component that tracks per-entity damage state and history.
struct DamageComponent : public IComponent {
    /// Rolling history of the last kMaxHistorySize hits.
    std::vector<DamageRecord> damageHistory;
    static constexpr size_t kMaxHistorySize = 50;

    /// Global damage modifier applied to all incoming damage.
    float damageMultiplier = 1.0f;

    /// Automatic repair rate (HP per second, 0 = no auto-repair).
    float repairRate = 0.0f;

    /// When true the entity cannot take damage.
    bool isInvulnerable = false;

    /// Structural state flags.
    bool hasStructuralDamage = false;
    int disconnectedFragments = 0;

    /// Append a damage record, evicting the oldest entry if at capacity.
    void AddDamageRecord(const DamageRecord& record);

    /// Sum of all damage in the history.
    float GetTotalDamageReceived() const;

    /// Sum of damage recorded within the last `withinSeconds` before `currentTime`.
    float GetRecentDamage(float withinSeconds, float currentTime) const;

    /// Serialize for save-game persistence.
    ComponentData Serialize() const;

    /// Restore from a previously serialized ComponentData.
    void Deserialize(const ComponentData& data);
};

} // namespace subspace
