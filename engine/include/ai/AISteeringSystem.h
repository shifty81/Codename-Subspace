#pragma once

#include "core/ecs/SystemBase.h"
#include "core/ecs/EntityManager.h"
#include "core/Math.h"
#include "ai/AIDecisionSystem.h"

namespace subspace {

/// Steering output that can be applied as a force to a physics component.
struct SteeringOutput {
    Vector3 linear;   // Linear force to apply
    float angular = 0.0f;  // Angular thrust (unused for now, placeholder)
};

/// System that converts AI decisions into steering forces applied to
/// PhysicsComponents. Works alongside AIDecisionSystem: decisions choose
/// the state, steering executes the movement.
class AISteeringSystem : public SystemBase {
public:
    explicit AISteeringSystem(EntityManager& entityManager);

    void Update(float deltaTime) override;

    // ---- Individual steering behaviors (stateless, testable) ----

    /// Seek: steer toward a target position at max speed.
    static SteeringOutput Seek(const Vector3& position, const Vector3& target,
                               float maxForce);

    /// Flee: steer away from a threat position.
    static SteeringOutput Flee(const Vector3& position, const Vector3& threat,
                               float maxForce);

    /// Arrive: seek with deceleration near the target.
    /// slowRadius is the distance at which deceleration begins.
    static SteeringOutput Arrive(const Vector3& position, const Vector3& target,
                                 float maxForce, float slowRadius);

    /// Pursue: predict target's future position and seek it.
    static SteeringOutput Pursue(const Vector3& position,
                                 const Vector3& targetPos,
                                 const Vector3& targetVel,
                                 float maxForce,
                                 float maxPredictionTime = 2.0f);

    /// Evade: predict target's future position and flee from it.
    static SteeringOutput Evade(const Vector3& position,
                                const Vector3& threatPos,
                                const Vector3& threatVel,
                                float maxForce,
                                float maxPredictionTime = 2.0f);

    /// Patrol: move through waypoints in sequence.
    /// Returns the steering output and updates waypointIndex when
    /// the entity is within arrivalThreshold of the current waypoint.
    static SteeringOutput Patrol(const Vector3& position,
                                 const std::vector<std::array<float, 3>>& waypoints,
                                 int& waypointIndex,
                                 float maxForce,
                                 float arrivalThreshold = 5.0f);

    /// Wander: produce a gentle pseudo-random steering force.
    /// wanderAngle is updated each call to produce smooth wandering.
    static SteeringOutput Wander(const Vector3& velocity,
                                 float& wanderAngle,
                                 float maxForce,
                                 float wanderRadius = 10.0f,
                                 float wanderJitter = 0.5f);

private:
    EntityManager& _entityManager;
};

} // namespace subspace
