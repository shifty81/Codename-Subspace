#pragma once

#include "core/ecs/IComponent.h"
#include "core/ecs/SystemBase.h"
#include "core/ecs/EntityManager.h"
#include "core/resources/Inventory.h"
#include "core/persistence/SaveGameManager.h"

#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace subspace {

// ---------------------------------------------------------------------------
// NPCAgentType
// ---------------------------------------------------------------------------

enum class NPCAgentType { Miner, Trader, Hauler, Producer };

/// Return human-readable name for an agent type.
const char* NPCAgentTypeName(NPCAgentType type);

// ---------------------------------------------------------------------------
// NPCEconomicAgentComponent
// ---------------------------------------------------------------------------

/// ECS component for NPC market participants (miners, traders, haulers,
/// producers).  Drives the background economy simulation.
class NPCEconomicAgentComponent : public IComponent {
public:
    NPCAgentType agentType    = NPCAgentType::Trader;
    int          credits      = 10000;
    int          maxCargo     = 1000;
    float        efficiency   = 1.0f;     ///< 0.5 – 2.0
    float        actionIntervalSec = 60.0f;
    float        timeUntilNextAction = 0.0f;

    /// Current cargo (resource name → quantity).
    std::unordered_map<std::string, int> cargo;

    std::optional<uint64_t> homeStationId;
    std::optional<uint64_t> targetStationId;
    std::string targetResource;   ///< For miners

    float destX = 0, destY = 0, destZ = 0;
    bool  hasDestination = false;

    int CurrentCargoTotal() const;
    bool CanAddCargo(int amount) const;
    void AddCargo(const std::string& resource, int amount);
    bool RemoveCargo(const std::string& resource, int amount);

    ComponentData Serialize()   const;
    void Deserialize(const ComponentData& data);
};

// ---------------------------------------------------------------------------
// NPCEconomicAgentSystem
// ---------------------------------------------------------------------------

/// Ticks all NPCEconomicAgentComponent entities, simulating
/// background trading, mining, and production.
class NPCEconomicAgentSystem : public SystemBase {
public:
    static constexpr int MaxNPCAgents = 200;

    NPCEconomicAgentSystem();
    explicit NPCEconomicAgentSystem(EntityManager& em);

    void Update(float deltaTime) override;
    void SetEntityManager(EntityManager* em);

    /// Spawn up to MaxNPCAgents background traders/miners.
    void PopulateAgents(int targetCount = MaxNPCAgents);

private:
    EntityManager* _em = nullptr;

    void TickMiner  (NPCEconomicAgentComponent& agent, float dt);
    void TickTrader (NPCEconomicAgentComponent& agent, float dt);
    void TickHauler (NPCEconomicAgentComponent& agent, float dt);
    void TickProducer(NPCEconomicAgentComponent& agent, float dt);
};

// ---------------------------------------------------------------------------
// ManufacturingRecipe
// ---------------------------------------------------------------------------

struct ManufacturingRecipe {
    int    recipeId   = 0;
    std::string inputResource;
    int         inputAmount  = 1;
    std::string outputResource;
    int         outputAmount = 1;
    float       productionTimeSec = 10.0f;
    int         requiredFacilityLevel = 1;

    static std::vector<ManufacturingRecipe> GetDefaultRecipes();
};

// ---------------------------------------------------------------------------
// ManufacturingJob
// ---------------------------------------------------------------------------

enum class ManufacturingState { Idle, Loading, Processing, Completed, Failed };

struct ManufacturingJob {
    int                recipeId   = 0;
    ManufacturingRecipe recipe;
    ManufacturingState  state     = ManufacturingState::Idle;
    float               progress  = 0.0f;   ///< 0 – 1
    float               elapsed   = 0.0f;
    int                 batchSize = 1;
};

// ---------------------------------------------------------------------------
// ManufacturingComponent
// ---------------------------------------------------------------------------

class ManufacturingComponent : public IComponent {
public:
    int  facilityLevel = 1;
    int  maxJobs       = 3;
    float speedMultiplier = 1.0f;

    std::vector<ManufacturingJob> jobs;

    bool StartJob(const ManufacturingRecipe& recipe, int batchSize = 1);
    bool CancelJob(int recipeId);
    /// Returns output {resource, amount}, or {"",0} if not complete.
    std::pair<std::string, int> CollectJob(int recipeId);

    int ActiveJobCount()    const;
    int CompletedJobCount() const;

    ComponentData Serialize()   const;
    void Deserialize(const ComponentData& data);

private:
    int _nextJobId = 1;
};

// ---------------------------------------------------------------------------
// ManufacturingSystem
// ---------------------------------------------------------------------------

class ManufacturingSystem : public SystemBase {
public:
    ManufacturingSystem();
    explicit ManufacturingSystem(EntityManager& em);

    void Update(float deltaTime) override;
    void SetEntityManager(EntityManager* em);

private:
    EntityManager* _em = nullptr;

    void AdvanceJob(ManufacturingJob& job, float dt,
                    float speedMultiplier) const;
};

} // namespace subspace
