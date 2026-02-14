#include "ai/AISteeringSystem.h"

#include "core/physics/PhysicsComponent.h"

#include <cmath>

namespace subspace {

AISteeringSystem::AISteeringSystem(EntityManager& entityManager)
    : SystemBase("AISteeringSystem")
    , _entityManager(entityManager)
{
}

void AISteeringSystem::Update(float deltaTime)
{
    (void)deltaTime;
    auto aiComponents = _entityManager.GetAllComponents<AIComponent>();

    for (auto* ai : aiComponents) {
        if (!ai->isEnabled) continue;

        auto* physics = _entityManager.GetComponent<PhysicsComponent>(ai->entityId);
        if (!physics) continue;

        SteeringOutput steering;

        switch (ai->currentState) {
        case AIState::Patrol:
            if (!ai->patrolWaypoints.empty()) {
                steering = Patrol(physics->position, ai->patrolWaypoints,
                                  ai->currentWaypointIndex, physics->maxThrust);
            }
            break;

        case AIState::Fleeing: {
            const ThreatInfo* threat = ai->perception.GetHighestThreat();
            if (threat) {
                Vector3 threatPos(threat->posX, threat->posY, threat->posZ);
                steering = Flee(physics->position, threatPos, physics->maxThrust);
            }
            break;
        }

        case AIState::Combat: {
            const ThreatInfo* threat = ai->perception.GetHighestThreat();
            if (threat) {
                Vector3 tPos(threat->posX, threat->posY, threat->posZ);
                // Arrive at combat distance instead of charging into the target
                steering = Arrive(physics->position, tPos,
                                  physics->maxThrust, ai->maxCombatDistance);
            }
            break;
        }

        case AIState::Mining:
        case AIState::Salvaging:
        case AIState::Trading:
        case AIState::ReturningToBase:
            // These states move toward a target entity; for now use Seek
            // toward the perceived entity position if available.
            if (!ai->perception.nearbyEntities.empty()) {
                const auto& pe = ai->perception.nearbyEntities.front();
                Vector3 targetPos(pe.posX, pe.posY, pe.posZ);
                steering = Arrive(physics->position, targetPos,
                                  physics->maxThrust, 20.0f);
            }
            break;

        case AIState::Exploring: {
            // Use Wander for exploration-style movement
            steering = Wander(physics->velocity, ai->wanderAngle, physics->maxThrust);
            break;
        }

        case AIState::Idle:
        case AIState::Repairing:
        case AIState::Evasion:
        case AIState::Scanning:
        default:
            break;
        }

        // Apply the resulting steering force
        physics->AddForce(steering.linear);
    }
}

// ---------- Static steering behaviors ----------

SteeringOutput AISteeringSystem::Seek(const Vector3& position, const Vector3& target,
                                      float maxForce)
{
    SteeringOutput out;
    Vector3 desired = target - position;
    float dist = desired.length();
    if (dist > 0.0f) {
        out.linear = desired.normalized() * maxForce;
    }
    return out;
}

SteeringOutput AISteeringSystem::Flee(const Vector3& position, const Vector3& threat,
                                      float maxForce)
{
    SteeringOutput out;
    Vector3 away = position - threat;
    float dist = away.length();
    if (dist > 0.0f) {
        out.linear = away.normalized() * maxForce;
    }
    return out;
}

SteeringOutput AISteeringSystem::Arrive(const Vector3& position, const Vector3& target,
                                        float maxForce, float slowRadius)
{
    SteeringOutput out;
    Vector3 desired = target - position;
    float dist = desired.length();
    if (dist < 0.001f) return out;

    float speed = maxForce;
    if (dist < slowRadius && slowRadius > 0.0f) {
        speed = maxForce * (dist / slowRadius);
    }

    out.linear = desired.normalized() * speed;
    return out;
}

SteeringOutput AISteeringSystem::Pursue(const Vector3& position,
                                        const Vector3& targetPos,
                                        const Vector3& targetVel,
                                        float maxForce,
                                        float maxPredictionTime)
{
    Vector3 toTarget = targetPos - position;
    float dist = toTarget.length();
    float predictionTime = dist > 0.0f
        ? std::min(dist / maxForce, maxPredictionTime)
        : 0.0f;

    Vector3 futurePos = targetPos + targetVel * predictionTime;
    return Seek(position, futurePos, maxForce);
}

SteeringOutput AISteeringSystem::Evade(const Vector3& position,
                                       const Vector3& threatPos,
                                       const Vector3& threatVel,
                                       float maxForce,
                                       float maxPredictionTime)
{
    Vector3 toThreat = threatPos - position;
    float dist = toThreat.length();
    float predictionTime = dist > 0.0f
        ? std::min(dist / maxForce, maxPredictionTime)
        : 0.0f;

    Vector3 futurePos = threatPos + threatVel * predictionTime;
    return Flee(position, futurePos, maxForce);
}

SteeringOutput AISteeringSystem::Patrol(const Vector3& position,
                                        const std::vector<std::array<float, 3>>& waypoints,
                                        int& waypointIndex,
                                        float maxForce,
                                        float arrivalThreshold)
{
    if (waypoints.empty()) return {};

    int idx = waypointIndex % static_cast<int>(waypoints.size());
    Vector3 wp(waypoints[idx][0], waypoints[idx][1], waypoints[idx][2]);

    Vector3 diff = wp - position;
    if (diff.length() < arrivalThreshold) {
        waypointIndex = (idx + 1) % static_cast<int>(waypoints.size());
        idx = waypointIndex;
        wp = Vector3(waypoints[idx][0], waypoints[idx][1], waypoints[idx][2]);
    }

    return Arrive(position, wp, maxForce, arrivalThreshold * 3.0f);
}

SteeringOutput AISteeringSystem::Wander(const Vector3& velocity,
                                        float& wanderAngle,
                                        float maxForce,
                                        float wanderRadius,
                                        float wanderJitter)
{
    SteeringOutput out;

    // Offset wanderAngle slightly
    wanderAngle += wanderJitter * 0.5f; // deterministic drift for testability

    float wx = wanderRadius * std::cos(wanderAngle);
    float wz = wanderRadius * std::sin(wanderAngle);

    // Project wander circle ahead of current velocity direction
    Vector3 forward = velocity.length() > 0.001f ? velocity.normalized() : Vector3(1, 0, 0);
    Vector3 ahead = forward * wanderRadius * 2.0f;

    out.linear = (ahead + Vector3(wx, 0, wz)).normalized() * maxForce;
    return out;
}

} // namespace subspace
