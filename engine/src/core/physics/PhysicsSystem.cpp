#include "core/physics/PhysicsSystem.h"

#include <cmath>
#include <unordered_map>
#include <unordered_set>

namespace subspace {

PhysicsSystem::PhysicsSystem(EntityManager& entityManager)
    : SystemBase("PhysicsSystem")
    , _entityManager(entityManager)
    , _spatialHash(kDefaultCellSize)
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

void PhysicsSystem::RebuildSpatialHash(std::vector<PhysicsComponent*>& components)
{
    _spatialHash.Clear();
    for (auto* comp : components) {
        _spatialHash.Insert(comp->entityId, comp->position, comp->collisionRadius);
    }
}

void PhysicsSystem::DetectCollisions(std::vector<PhysicsComponent*>& components)
{
    // Rebuild spatial hash each frame with updated positions
    RebuildSpatialHash(components);

    // Build a lookup from entityId to component pointer
    std::unordered_map<EntityId, PhysicsComponent*> byId;
    byId.reserve(components.size());
    for (auto* comp : components) {
        byId[comp->entityId] = comp;
    }

    // Use spatial hash for broad-phase: only check nearby pairs
    // Track checked pairs to avoid duplicate collision handling
    std::unordered_set<uint64_t> checkedPairs;

    for (auto* comp : components) {
        float queryRadius = comp->collisionRadius * 2.0f;
        auto nearby = _spatialHash.QueryNearby(comp->position, queryRadius);

        for (EntityId otherId : nearby) {
            if (otherId == comp->entityId) continue;

            // Create a canonical pair key to avoid checking A-B and B-A
            EntityId lo = std::min(comp->entityId, otherId);
            EntityId hi = std::max(comp->entityId, otherId);
            uint64_t pairKey = (static_cast<uint64_t>(lo) << 32) | static_cast<uint64_t>(hi);

            if (checkedPairs.count(pairKey)) continue;
            checkedPairs.insert(pairKey);

            auto otherIt = byId.find(otherId);
            if (otherIt == byId.end()) continue;
            auto* other = otherIt->second;

            // Check collision layers
            if (!comp->ShouldCollideWith(*other)) continue;

            Vector3 diff = other->position - comp->position;
            float distance = diff.length();
            float minDistance = comp->collisionRadius + other->collisionRadius;

            if (distance < minDistance) {
                HandleCollision(*comp, *other, distance, minDistance);
            }
        }
    }
}

void PhysicsSystem::HandleCollision(PhysicsComponent& obj1, PhysicsComponent& obj2,
                                    float distance, float minDistance)
{
    // Trigger volumes don't generate physics response
    if (obj1.isTrigger || obj2.isTrigger) return;

    if (obj1.isStatic && obj2.isStatic) return;

    Vector3 diff = obj2.position - obj1.position;
    float len = diff.length();

    // Handle the degenerate case where two objects are exactly coincident.
    // Returning without response would leave them permanently stuck; instead
    // pick a separation axis derived from the entity IDs so different pairs
    // get different axes and don't all pile up in one direction.
    Vector3 normal;
    if (len < 1e-6f) {
        // XOR the two IDs to produce a deterministic but varied direction.
        uint64_t seed = static_cast<uint64_t>(obj1.entityId) ^
                        (static_cast<uint64_t>(obj2.entityId) * 2654435761ULL);
        float dx = ((seed & 0xFF)       / 127.5f) - 1.0f;
        float dy = (((seed >> 8) & 0xFF) / 127.5f) - 1.0f;
        float dz = (((seed >> 16) & 0xFF)/ 127.5f) - 1.0f;
        float dlen = std::sqrt(dx*dx + dy*dy + dz*dz);
        if (dlen < 1e-6f) dlen = 1.0f; // Absolute fallback
        normal = {dx / dlen, dy / dlen, dz / dlen};
    } else {
        normal = diff * (1.0f / len);
    }

    // --- Positional separation to prevent objects from getting stuck ---
    float overlap = minDistance - distance;
    if (overlap > 0.0f) {
        float separation = overlap + kSeparationMargin;

        if (!obj1.isStatic && !obj2.isStatic) {
            // Distribute separation inversely proportional to mass
            float totalMass = obj1.mass + obj2.mass;
            float ratio1 = obj2.mass / totalMass;
            float ratio2 = obj1.mass / totalMass;
            obj1.position = obj1.position - normal * (separation * ratio1);
            obj2.position = obj2.position + normal * (separation * ratio2);
        } else if (obj1.isStatic) {
            obj2.position = obj2.position + normal * separation;
        } else {
            obj1.position = obj1.position - normal * separation;
        }
    }

    // --- Velocity response with restitution ---
    // Use the minimum restitution of the two objects
    float restitution = std::min(obj1.restitution, obj2.restitution);

    auto dot = [](const Vector3& a, const Vector3& b) {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    };

    if (!obj1.isStatic && !obj2.isStatic) {
        float v1 = dot(obj1.velocity, normal);
        float v2 = dot(obj2.velocity, normal);
        float relNormalVel = v1 - v2;   // positive = converging

        if (relNormalVel <= 0.0f) {
            // Objects are already separating (or have identical velocity along
            // the normal).  If the overlap is negligible, no further action is
            // needed.  However, when objects are significantly embedded — which
            // happens when thrust continuously pushes ships into each other —
            // the positional correction alone is insufficient because the next
            // force application can re-create the overlap.  Inject a minimum
            // separating impulse so the ships always escape each other.
            if (overlap <= kBaumgarteSlop) return;

            float m1 = obj1.mass;
            float m2 = obj2.mass;
            float j = kMinSeparationSpeed / (1.0f / m1 + 1.0f / m2);
            obj1.velocity = obj1.velocity - normal * (j / m1);
            obj2.velocity = obj2.velocity + normal * (j / m2);
            return;
        }

        float m1 = obj1.mass;
        float m2 = obj2.mass;

        float newV1 = (v1 * (m1 - m2) + 2 * m2 * v2) / (m1 + m2);
        float newV2 = (v2 * (m2 - m1) + 2 * m1 * v1) / (m1 + m2);

        // Apply restitution to scale the impulse
        float dv1 = restitution * (newV1 - v1);
        float dv2 = restitution * (newV2 - v2);

        obj1.velocity = obj1.velocity + normal * dv1;
        obj2.velocity = obj2.velocity + normal * dv2;
    } else if (obj1.isStatic) {
        float v = dot(obj2.velocity, normal);
        if (v >= 0.0f) return;  // Moving away already
        obj2.velocity = obj2.velocity - normal * ((1.0f + restitution) * v);
    } else {
        float v = dot(obj1.velocity, normal);
        if (v <= 0.0f) return;  // Moving away already
        obj1.velocity = obj1.velocity - normal * ((1.0f + restitution) * v);
    }
}

} // namespace subspace
