#include "mining/MiningSystem.h"

namespace subspace {

MiningSystem::MiningSystem() : SystemBase("MiningSystem") {}

void MiningSystem::Update(float /*deltaTime*/) {
    // Standalone: callers drive per-entity mining/salvaging via ProcessMining/ProcessSalvaging.
}

void MiningSystem::AddAsteroid(const Asteroid& asteroid) {
    _asteroids[asteroid.id] = asteroid;
}

void MiningSystem::AddWreckage(const Wreckage& wreckage) {
    _wreckage[wreckage.id] = wreckage;
}

bool MiningSystem::StartMining(MiningComponent& miner, EntityId asteroidId,
                               const MiningPosition& minerPos) {
    auto it = _asteroids.find(asteroidId);
    if (it == _asteroids.end()) return false;

    float distance = minerPos.DistanceTo(it->second.position);
    if (distance > miner.miningRange) return false;

    miner.isMining = true;
    miner.targetAsteroidId = asteroidId;
    return true;
}

void MiningSystem::StopMining(MiningComponent& miner) {
    miner.isMining = false;
    miner.targetAsteroidId = InvalidEntityId;
}

bool MiningSystem::StartSalvaging(SalvagingComponent& salvager, EntityId wreckageId,
                                  const MiningPosition& salvagerPos) {
    auto it = _wreckage.find(wreckageId);
    if (it == _wreckage.end()) return false;

    float distance = salvagerPos.DistanceTo(it->second.position);
    if (distance > salvager.salvageRange) return false;

    salvager.isSalvaging = true;
    salvager.targetWreckageId = wreckageId;
    return true;
}

void MiningSystem::StopSalvaging(SalvagingComponent& salvager) {
    salvager.isSalvaging = false;
    salvager.targetWreckageId = InvalidEntityId;
}

float MiningSystem::ProcessMining(MiningComponent& miner, Inventory& inventory,
                                  float deltaTime) {
    if (!miner.isMining || miner.targetAsteroidId == InvalidEntityId) return 0.0f;

    auto it = _asteroids.find(miner.targetAsteroidId);
    if (it == _asteroids.end()) {
        StopMining(miner);
        return 0.0f;
    }

    Asteroid& asteroid = it->second;
    float extracted = std::min(miner.miningPower * deltaTime, asteroid.remainingResources);
    asteroid.remainingResources -= extracted;

    inventory.AddResource(asteroid.resourceType, static_cast<int>(extracted));

    // Remove asteroid if depleted
    if (asteroid.remainingResources <= 0.0f) {
        _asteroids.erase(it);
        StopMining(miner);
    }

    return extracted;
}

int MiningSystem::ProcessSalvaging(SalvagingComponent& salvager, Inventory& inventory,
                                   float deltaTime) {
    if (!salvager.isSalvaging || salvager.targetWreckageId == InvalidEntityId) return 0;

    auto it = _wreckage.find(salvager.targetWreckageId);
    if (it == _wreckage.end()) {
        StopSalvaging(salvager);
        return 0;
    }

    Wreckage& wreck = it->second;
    int salvageAmount = static_cast<int>(salvager.salvagePower * deltaTime);
    int totalSalvaged = 0;

    for (auto& [resType, remaining] : wreck.resources) {
        if (remaining <= 0) continue;
        int toSalvage = std::min(salvageAmount, remaining);
        if (inventory.AddResource(resType, toSalvage)) {
            remaining -= toSalvage;
            totalSalvaged += toSalvage;
        }
    }

    // Remove wreckage if fully salvaged
    if (wreck.IsFullySalvaged()) {
        _wreckage.erase(it);
        StopSalvaging(salvager);
    }

    return totalSalvaged;
}

const std::unordered_map<EntityId, Asteroid>& MiningSystem::GetAsteroids() const {
    return _asteroids;
}

const std::unordered_map<EntityId, Wreckage>& MiningSystem::GetWreckage() const {
    return _wreckage;
}

size_t MiningSystem::GetAsteroidCount() const { return _asteroids.size(); }
size_t MiningSystem::GetWreckageCount() const { return _wreckage.size(); }

} // namespace subspace
