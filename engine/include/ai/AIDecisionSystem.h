#pragma once

#include "core/ecs/IComponent.h"
#include "core/ecs/SystemBase.h"

#include <array>
#include <vector>

namespace subspace {

enum class AIState {
    Idle,
    Patrol,
    Mining,
    Salvaging,
    Trading,
    Combat,
    Fleeing,
    Evasion,
    ReturningToBase,
    Repairing,
    Scanning,
    Exploring
};

enum class AIPersonality {
    Balanced,
    Aggressive,
    Defensive,
    Miner,
    Trader,
    Salvager,
    Explorer,
    Coward
};

enum class CombatTactic {
    Aggressive,
    Kiting,
    Strafing,
    Broadsiding,
    Defensive
};

enum class TargetPriority {
    None = 0,
    Low = 1,
    Medium = 2,
    High = 3,
    Critical = 4
};

struct PerceivedEntity {
    EntityId entityId = 0;
    float posX = 0;
    float posY = 0;
    float posZ = 0;
    float distance = 0;
    bool isHostile = false;
    bool isFriendly = false;
    float shieldPercentage = 1.0f;
    float hullPercentage = 1.0f;
};

struct ThreatInfo {
    EntityId entityId = 0;
    float posX = 0;
    float posY = 0;
    float posZ = 0;
    float distance = 0;
    TargetPriority priority = TargetPriority::None;
    float threatLevel = 0.0f;
    bool isAttacking = false;
};

struct AIPerception {
    std::vector<PerceivedEntity> nearbyEntities;
    std::vector<EntityId> nearbyAsteroids;
    std::vector<EntityId> nearbyStations;
    std::vector<ThreatInfo> threats;

    void Clear();
    bool HasThreats() const;
    const ThreatInfo* GetHighestThreat() const;
};

struct AIComponent : public IComponent {
    AIState currentState = AIState::Idle;
    AIState previousState = AIState::Idle;
    AIPersonality personality = AIPersonality::Balanced;
    EntityId currentTarget = InvalidEntityId;
    CombatTactic combatTactic = CombatTactic::Defensive;
    float fleeThreshold = 0.25f;
    float minCombatDistance = 10.0f;
    float maxCombatDistance = 100.0f;
    std::vector<std::array<float, 3>> patrolWaypoints;
    int currentWaypointIndex = 0;
    EntityId homeBase = InvalidEntityId;
    bool canMine = false;
    bool canSalvage = false;
    bool canTrade = false;
    bool isEnabled = true;
    AIPerception perception;
};

class AIDecisionSystem : public SystemBase {
public:
    AIDecisionSystem();

    void Update(float deltaTime) override;

    AIState EvaluateState(const AIComponent& ai) const;
    bool ShouldFlee(const AIComponent& ai, float currentHullPercent) const;
    bool ShouldEnterCombat(const AIComponent& ai) const;
    bool ShouldReturnToBase(const AIComponent& ai, float cargoPercent) const;
    AIState EvaluateGatheringState(const AIComponent& ai) const;
    EntityId SelectTarget(const AIComponent& ai) const;
    float CalculateActionPriority(AIState state, const AIComponent& ai) const;
};

} // namespace subspace
