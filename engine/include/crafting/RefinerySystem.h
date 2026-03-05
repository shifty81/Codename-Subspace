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

/// Processing tier of a refinery, affecting efficiency and available recipes.
enum class RefineryTier {
    Basic,
    Advanced,
    Industrial,
    Military,
    Experimental
};

/// State of a refining job.
enum class RefiningState {
    Idle,
    Loading,
    Processing,
    Completed,
    Failed
};

/// A recipe that describes how to convert one material into another.
struct RefineryRecipe {
    int recipeId = 0;
    std::string inputMaterial;
    int inputAmount = 0;
    std::string outputMaterial;
    int outputAmount = 0;
    float processingTime = 0.0f;  // seconds to process one batch
    RefineryTier requiredTier = RefineryTier::Basic;
    float yieldMultiplier = 1.0f;

    /// Get the display name for a refinery tier.
    static std::string GetTierName(RefineryTier tier);

    /// Get the display name for a refining state.
    static std::string GetStateName(RefiningState state);

    /// Get the built-in set of default refinery recipes.
    static std::vector<RefineryRecipe> GetDefaultRecipes();
};

/// A single refining job being processed by a refinery.
struct RefiningJob {
    int jobId = 0;
    RefineryRecipe recipe;
    RefiningState state = RefiningState::Idle;
    float progress = 0.0f;       // 0 to 1
    float elapsedTime = 0.0f;
    int batchSize = 1;
    float efficiencyBonus = 0.0f;
};

/// ECS component that gives an entity refinery capabilities.
class RefineryComponent : public IComponent {
public:
    explicit RefineryComponent(RefineryTier tier = RefineryTier::Basic,
                               int maxJobs = 3);

    RefineryTier GetTier() const;
    void SetTier(RefineryTier tier);

    int GetMaxJobs() const;
    int GetActiveJobCount() const;
    int GetCompletedJobCount() const;

    /// Start a new refining job. Returns false if the refinery tier is too
    /// low for the recipe or the job queue is full.
    bool StartJob(const RefineryRecipe& recipe, int batchSize = 1);

    /// Cancel a job by its ID. Returns false if not found.
    bool CancelJob(int jobId);

    /// Collect the output of a completed job. Returns the output material
    /// name and total amount. Returns {"", 0} if the job is not completed.
    std::pair<std::string, int> CollectJob(int jobId);

    const RefiningJob* GetJob(int jobId) const;
    const std::vector<RefiningJob>& GetAllJobs() const;

    /// Tier-based efficiency multiplier (Basic 1.0 … Experimental 1.4).
    float GetEfficiencyMultiplier() const;

    /// Tier-based processing speed multiplier (Basic 1.0 … Experimental 1.6).
    float GetProcessingSpeedMultiplier() const;

    /// Serialize for save-game persistence.
    ComponentData Serialize() const;

    /// Restore from previously serialized data.
    void Deserialize(const ComponentData& data);

private:
    RefineryTier _tier = RefineryTier::Basic;
    int _maxJobs = 3;
    std::vector<RefiningJob> _jobs;
    int _nextJobId = 1;

    friend class RefinerySystem;
};

/// System that advances refining jobs each frame.
class RefinerySystem : public SystemBase {
public:
    RefinerySystem();
    explicit RefinerySystem(EntityManager& entityManager);

    void Update(float deltaTime) override;

    void SetEntityManager(EntityManager* em);

private:
    EntityManager* _entityManager = nullptr;
};

} // namespace subspace
