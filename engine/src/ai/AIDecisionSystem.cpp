#include "ai/AIDecisionSystem.h"

namespace subspace {

// --- AIPerception ---

void AIPerception::Clear() {
    nearbyEntities.clear();
    nearbyAsteroids.clear();
    nearbyStations.clear();
    threats.clear();
}

bool AIPerception::HasThreats() const {
    return !threats.empty();
}

const ThreatInfo* AIPerception::GetHighestThreat() const {
    if (threats.empty()) {
        return nullptr;
    }

    const ThreatInfo* highest = &threats[0];
    for (size_t i = 1; i < threats.size(); ++i) {
        const auto& t = threats[i];
        if (static_cast<int>(t.priority) > static_cast<int>(highest->priority)) {
            highest = &t;
        } else if (t.priority == highest->priority && t.threatLevel > highest->threatLevel) {
            highest = &t;
        }
    }
    return highest;
}

// --- AIDecisionSystem ---

AIDecisionSystem::AIDecisionSystem()
    : SystemBase("AIDecisionSystem") {}

void AIDecisionSystem::Update(float /*deltaTime*/) {
    // In a full ECS integration this would iterate over all AIComponents.
    // Per-entity update logic is driven by EvaluateState and helper methods.
}

AIState AIDecisionSystem::EvaluateState(const AIComponent& ai) const {
    if (!ai.isEnabled) {
        return ai.currentState;
    }

    // Highest priority: flee when hull is critical.
    // Hull percentage is not stored in AIComponent, so the caller should
    // invoke ShouldFlee() separately with the actual hull value and
    // transition to Fleeing before calling EvaluateState().
    if (ai.currentState == AIState::Fleeing) {
        return AIState::Fleeing;
    }

    // Combat if hostile threats detected
    if (ShouldEnterCombat(ai)) {
        return AIState::Combat;
    }

    // Return to base when cargo is nearly full
    if (ShouldReturnToBase(ai, 0.8f)) {
        return AIState::ReturningToBase;
    }

    // Gathering activities (mining / salvaging)
    AIState gatherState = EvaluateGatheringState(ai);
    if (gatherState != AIState::Idle) {
        return gatherState;
    }

    // Patrol if waypoints are defined
    if (!ai.patrolWaypoints.empty()) {
        return AIState::Patrol;
    }

    return AIState::Idle;
}

bool AIDecisionSystem::ShouldFlee(const AIComponent& ai, float currentHullPercent) const {
    return currentHullPercent < ai.fleeThreshold;
}

bool AIDecisionSystem::ShouldEnterCombat(const AIComponent& ai) const {
    if (!ai.perception.HasThreats()) {
        return false;
    }

    if (ai.personality == AIPersonality::Coward) {
        return false;
    }

    if (ai.personality == AIPersonality::Aggressive) {
        return true;
    }

    // Other personalities require at least a Medium priority threat
    const ThreatInfo* highest = ai.perception.GetHighestThreat();
    return highest != nullptr &&
           static_cast<int>(highest->priority) >= static_cast<int>(TargetPriority::Medium);
}

bool AIDecisionSystem::ShouldReturnToBase(const AIComponent& ai, float cargoPercent) const {
    return ai.homeBase != InvalidEntityId && cargoPercent > 0.8f;
}

AIState AIDecisionSystem::EvaluateGatheringState(const AIComponent& ai) const {
    if (ai.personality == AIPersonality::Miner && ai.canMine &&
        !ai.perception.nearbyAsteroids.empty()) {
        return AIState::Mining;
    }

    if (ai.personality == AIPersonality::Salvager && ai.canSalvage) {
        return AIState::Salvaging;
    }

    // Non-specialist: prefer mining if asteroids are available, else salvaging
    if (ai.canMine && !ai.perception.nearbyAsteroids.empty()) {
        return AIState::Mining;
    }

    if (ai.canSalvage) {
        return AIState::Salvaging;
    }

    return AIState::Idle;
}

EntityId AIDecisionSystem::SelectTarget(const AIComponent& ai) const {
    const ThreatInfo* highest = ai.perception.GetHighestThreat();
    if (highest != nullptr) {
        return highest->entityId;
    }
    return InvalidEntityId;
}

float AIDecisionSystem::CalculateActionPriority(AIState state, const AIComponent& ai) const {
    switch (state) {
        case AIState::Combat:
            if (ai.personality == AIPersonality::Aggressive) return 0.9f;
            if (ai.personality == AIPersonality::Coward)     return 0.3f;
            return 0.6f;

        case AIState::Fleeing:
            if (ai.personality == AIPersonality::Coward)     return 0.95f;
            if (ai.personality == AIPersonality::Aggressive) return 0.3f;
            return 0.7f;

        case AIState::Mining:
            if (ai.personality == AIPersonality::Miner)      return 0.8f;
            return 0.5f;

        case AIState::Trading:
            if (ai.personality == AIPersonality::Trader)     return 0.8f;
            return 0.4f;

        case AIState::Salvaging:
            if (ai.personality == AIPersonality::Salvager)   return 0.8f;
            return 0.5f;

        case AIState::Exploring:
            if (ai.personality == AIPersonality::Explorer)   return 0.8f;
            return 0.4f;

        case AIState::Patrol:
            return 0.3f;

        case AIState::ReturningToBase:
            return 0.6f;

        case AIState::Repairing:
            return 0.7f;

        case AIState::Scanning:
            return 0.4f;

        case AIState::Evasion:
            return 0.8f;

        case AIState::Idle:
        default:
            return 0.1f;
    }
}

} // namespace subspace
