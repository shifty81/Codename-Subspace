#pragma once

#include "core/ecs/Entity.h"
#include "core/ecs/IComponent.h"
#include "core/ecs/SystemBase.h"
#include "core/ecs/EntityManager.h"
#include "core/persistence/SaveGameManager.h"

#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>
#include <utility>

namespace subspace {

/// Tier of a salvage module, affecting efficiency and yield.
enum class SalvageTier {
    Basic,
    Advanced,
    Industrial,
    Military,
    Experimental
};

/// Current state of a salvage operation.
enum class SalvageState {
    Idle,
    Approaching,
    Salvaging,
    Completed,
    Failed
};

/// Describes a wreck or debris field being salvaged.
struct SalvageTarget {
    uint64_t targetId = 0;
    std::string wreckName;
    std::string primaryMaterial;
    int totalYield = 0;
    int remainingYield = 0;
    float integrity = 1.0f;  ///< 0 to 1, determines bonus yield
    SalvageState state = SalvageState::Idle;
    float progress = 0.0f;   ///< 0 to 1 for current extraction cycle
    float distance = 0.0f;

    /// Get the display name for a salvage tier.
    static std::string GetTierName(SalvageTier tier);

    /// Get the display name for a salvage state.
    static std::string GetStateName(SalvageState state);

    /// Get the default salvage targets (wreck types).
    static std::vector<SalvageTarget> GetDefaultWreckTypes();
};

/// ECS component that gives an entity salvage capabilities.
class SalvageComponent : public IComponent {
public:
    explicit SalvageComponent(SalvageTier tier = SalvageTier::Basic,
                              float range = 500.0f);

    SalvageTier GetTier() const;
    void SetTier(SalvageTier tier);

    float GetRange() const;
    void SetRange(float range);

    int GetMaxTargets() const;
    int GetActiveTargetCount() const;

    /// Start salvaging a wreck. Returns false if at target limit or out of range.
    bool StartSalvage(const SalvageTarget& target);

    /// Cancel salvage on a specific target. Returns false if not found.
    bool CancelSalvage(uint64_t targetId);

    /// Collect materials from a completed salvage. Returns {material, amount}.
    std::pair<std::string, int> CollectSalvage(uint64_t targetId);

    /// Get a specific target being salvaged.
    const SalvageTarget* GetTarget(uint64_t targetId) const;

    /// Get all salvage targets (active and completed).
    const std::vector<SalvageTarget>& GetAllTargets() const;

    /// Get number of completed salvage operations.
    int GetCompletedCount() const;

    /// Total materials collected across all completed salvages.
    int GetTotalMaterialsCollected() const;

    /// Tier-based efficiency multiplier (Basic 1.0 … Experimental 1.5).
    float GetEfficiencyMultiplier() const;

    /// Tier-based salvage speed multiplier (Basic 1.0 … Experimental 1.8).
    float GetSpeedMultiplier() const;

    /// Serialize for save-game persistence.
    ComponentData Serialize() const;

    /// Restore from previously serialized data.
    void Deserialize(const ComponentData& data);

private:
    SalvageTier _tier = SalvageTier::Basic;
    float _range = 500.0f;
    int _maxTargets = 2;
    std::vector<SalvageTarget> _targets;
    int _totalCollected = 0;

    friend class SalvageSystem;
};

/// System that advances salvage operations each frame.
class SalvageSystem : public SystemBase {
public:
    SalvageSystem();
    explicit SalvageSystem(EntityManager& entityManager);

    void Update(float deltaTime) override;

    void SetEntityManager(EntityManager* em);

private:
    EntityManager* _entityManager = nullptr;
};

} // namespace subspace
