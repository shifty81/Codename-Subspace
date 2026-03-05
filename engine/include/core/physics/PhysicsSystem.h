#pragma once

#include "core/ecs/SystemBase.h"
#include "core/ecs/EntityManager.h"
#include "core/physics/PhysicsComponent.h"
#include "core/physics/SpatialHash.h"

namespace subspace {

/// System that handles Newtonian physics simulation (port of C# PhysicsSystem).
/// Uses a SpatialHash for broad-phase collision detection to scale beyond
/// O(n²) brute-force pair checking.
class PhysicsSystem : public SystemBase {
public:
    explicit PhysicsSystem(EntityManager& entityManager);

    void Update(float deltaTime) override;

    /// Interpolate physics state for smooth rendering.
    void InterpolatePhysics(float alpha);

    /// Read-only access to the spatial hash (useful for AI perception).
    const SpatialHash& GetSpatialHash() const { return _spatialHash; }

private:
    void DetectCollisions(std::vector<PhysicsComponent*>& components);
    void HandleCollision(PhysicsComponent& obj1, PhysicsComponent& obj2,
                         float distance, float minDistance);
    void RebuildSpatialHash(std::vector<PhysicsComponent*>& components);

    EntityManager& _entityManager;
    SpatialHash _spatialHash;
    static constexpr float kMaxVelocity = 1000.0f;
    static constexpr float kDefaultCellSize = 50.0f;
    static constexpr float kSeparationMargin = 0.01f;
};

} // namespace subspace
