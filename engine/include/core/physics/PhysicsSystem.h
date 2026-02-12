#pragma once

#include "core/ecs/SystemBase.h"
#include "core/ecs/EntityManager.h"
#include "core/physics/PhysicsComponent.h"

namespace subspace {

/// System that handles Newtonian physics simulation (port of C# PhysicsSystem).
class PhysicsSystem : public SystemBase {
public:
    explicit PhysicsSystem(EntityManager& entityManager);

    void Update(float deltaTime) override;

    /// Interpolate physics state for smooth rendering.
    void InterpolatePhysics(float alpha);

private:
    void DetectCollisions(std::vector<PhysicsComponent*>& components);
    void HandleCollision(PhysicsComponent& obj1, PhysicsComponent& obj2);

    EntityManager& _entityManager;
    static constexpr float kMaxVelocity = 1000.0f;
};

} // namespace subspace
