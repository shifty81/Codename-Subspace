#include "core/physics/PhysicsSystem.h"

#include <cmath>

namespace subspace {

PhysicsSystem::PhysicsSystem(EntityManager& entityManager)
    : SystemBase("PhysicsSystem")
    , _entityManager(entityManager)
{
}

void PhysicsSystem::Update(float deltaTime)
{
    auto components = _entityManager.GetAllComponents<PhysicsComponent>();

    for (auto* physics : components) {
        if (physics->isStatic) continue;

        // Store previous state for interpolation
        physics->previousPosition = physics->position;
        physics->previousRotation = physics->rotation;

        // Calculate acceleration from forces (F = ma, a = F/m)
        physics->acceleration = physics->appliedForce * (1.0f / physics->mass);

        // Calculate angular acceleration from torque
        physics->angularAcceleration = physics->appliedTorque * (1.0f / physics->momentOfInertia);

        // Update velocities
        physics->velocity = physics->velocity + physics->acceleration * deltaTime;
        physics->angularVelocity = physics->angularVelocity + physics->angularAcceleration * deltaTime;

        // Apply drag with exponential decay
        float dragFactor = std::exp(-physics->drag * deltaTime);
        float angularDragFactor = std::exp(-physics->angularDrag * deltaTime);
        physics->velocity = physics->velocity * dragFactor;
        physics->angularVelocity = physics->angularVelocity * angularDragFactor;

        // Clamp velocities
        if (physics->velocity.length() > kMaxVelocity) {
            physics->velocity = physics->velocity.normalized() * kMaxVelocity;
        }

        // Update positions
        physics->position = physics->position + physics->velocity * deltaTime;
        physics->rotation = physics->rotation + physics->angularVelocity * deltaTime;

        // Initialize interpolated values
        physics->interpolatedPosition = physics->position;
        physics->interpolatedRotation = physics->rotation;

        // Clear forces for next frame
        physics->ClearForces();
    }

    // Simple collision detection
    DetectCollisions(components);
}

void PhysicsSystem::InterpolatePhysics(float alpha)
{
    auto components = _entityManager.GetAllComponents<PhysicsComponent>();

    for (auto* physics : components) {
        if (physics->isStatic) continue;

        // Linear interpolation between previous and current state
        physics->interpolatedPosition = physics->previousPosition +
            (physics->position - physics->previousPosition) * alpha;
        physics->interpolatedRotation = physics->previousRotation +
            (physics->rotation - physics->previousRotation) * alpha;
    }
}

void PhysicsSystem::DetectCollisions(std::vector<PhysicsComponent*>& components)
{
    for (size_t i = 0; i < components.size(); ++i) {
        for (size_t j = i + 1; j < components.size(); ++j) {
            auto* comp1 = components[i];
            auto* comp2 = components[j];

            Vector3 diff = comp2->position - comp1->position;
            float distance = diff.length();
            float minDistance = comp1->collisionRadius + comp2->collisionRadius;

            if (distance < minDistance && distance > 0.0f) {
                HandleCollision(*comp1, *comp2);
            }
        }
    }
}

void PhysicsSystem::HandleCollision(PhysicsComponent& obj1, PhysicsComponent& obj2)
{
    if (obj1.isStatic && obj2.isStatic) return;

    Vector3 diff = obj2.position - obj1.position;
    float len = diff.length();
    if (len == 0.0f) return;
    Vector3 normal = diff * (1.0f / len);

    auto dot = [](const Vector3& a, const Vector3& b) {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    };

    if (!obj1.isStatic && !obj2.isStatic) {
        float v1 = dot(obj1.velocity, normal);
        float v2 = dot(obj2.velocity, normal);
        float m1 = obj1.mass;
        float m2 = obj2.mass;

        float newV1 = (v1 * (m1 - m2) + 2 * m2 * v2) / (m1 + m2);
        float newV2 = (v2 * (m2 - m1) + 2 * m1 * v1) / (m1 + m2);

        obj1.velocity = obj1.velocity + normal * (newV1 - v1);
        obj2.velocity = obj2.velocity + normal * (newV2 - v2);
    } else if (obj1.isStatic) {
        float v = dot(obj2.velocity, normal);
        obj2.velocity = obj2.velocity - normal * (2 * v);
    } else {
        float v = dot(obj1.velocity, normal);
        obj1.velocity = obj1.velocity - normal * (2 * v);
    }
}

} // namespace subspace
