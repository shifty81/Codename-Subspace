#pragma once

#include "core/ecs/Entity.h"
#include "core/ecs/IComponent.h"
#include "core/ecs/SystemBase.h"
#include "core/resources/Inventory.h"

#include <algorithm>
#include <cmath>
#include <string>
#include <unordered_map>
#include <vector>

namespace subspace {

/// Component for mining capabilities.
struct MiningComponent : public IComponent {
    float miningPower = 10.0f;   // Resources per second
    float miningRange = 50.0f;
    bool isMining = false;
    EntityId targetAsteroidId = InvalidEntityId;
};

/// Component for salvaging capabilities.
struct SalvagingComponent : public IComponent {
    float salvagePower = 8.0f;   // Resources per second
    float salvageRange = 50.0f;
    bool isSalvaging = false;
    EntityId targetWreckageId = InvalidEntityId;
};

/// Simple 3-float position for mining objects.
struct MiningPosition {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    float DistanceTo(const MiningPosition& other) const {
        float dx = x - other.x;
        float dy = y - other.y;
        float dz = z - other.z;
        return std::sqrt(dx * dx + dy * dy + dz * dz);
    }
};

/// Represents a mineable asteroid.
struct Asteroid {
    EntityId id = 0;
    MiningPosition position;
    float size = 10.0f;
    ResourceType resourceType = ResourceType::Iron;
    float remainingResources = 100.0f;

    Asteroid() = default;
    Asteroid(EntityId id_, MiningPosition pos, float sz, ResourceType res)
        : id(id_), position(pos), size(sz), resourceType(res),
          remainingResources(sz * 10.0f) {}
};

/// Represents salvageable wreckage.
struct Wreckage {
    EntityId id = 0;
    MiningPosition position;
    std::unordered_map<ResourceType, int> resources;

    bool IsFullySalvaged() const {
        for (auto& kv : resources) {
            if (kv.second > 0) return false;
        }
        return true;
    }
};

/// System for mining asteroids and salvaging wreckage.
class MiningSystem : public SystemBase {
public:
    MiningSystem();

    void Update(float deltaTime) override;

    /// Add an asteroid to the sector.
    void AddAsteroid(const Asteroid& asteroid);

    /// Add wreckage to the sector.
    void AddWreckage(const Wreckage& wreckage);

    /// Start mining an asteroid. Returns false if out of range or invalid.
    bool StartMining(MiningComponent& miner, EntityId asteroidId,
                     const MiningPosition& minerPos);

    /// Stop mining.
    void StopMining(MiningComponent& miner);

    /// Start salvaging wreckage. Returns false if out of range or invalid.
    bool StartSalvaging(SalvagingComponent& salvager, EntityId wreckageId,
                        const MiningPosition& salvagerPos);

    /// Stop salvaging.
    void StopSalvaging(SalvagingComponent& salvager);

    /// Process one mining tick for a single miner. Returns resources extracted.
    float ProcessMining(MiningComponent& miner, Inventory& inventory,
                        float deltaTime);

    /// Process one salvaging tick for a single salvager. Returns total extracted.
    int ProcessSalvaging(SalvagingComponent& salvager, Inventory& inventory,
                         float deltaTime);

    /// Get all asteroids.
    const std::unordered_map<EntityId, Asteroid>& GetAsteroids() const;

    /// Get all wreckage.
    const std::unordered_map<EntityId, Wreckage>& GetWreckage() const;

    /// Get asteroid count.
    size_t GetAsteroidCount() const;

    /// Get wreckage count.
    size_t GetWreckageCount() const;

private:
    std::unordered_map<EntityId, Asteroid> _asteroids;
    std::unordered_map<EntityId, Wreckage> _wreckage;
};

} // namespace subspace
