#include "core/physics/PhysicsComponent.h"

#include <algorithm>
#include <cmath>

namespace subspace {

void PhysicsComponent::AddForce(const Vector3& force)
{
    appliedForce = appliedForce + force;
}

void PhysicsComponent::AddTorque(const Vector3& torque)
{
    appliedTorque = appliedTorque + torque;
}

void PhysicsComponent::ApplyThrust(const Vector3& direction, float magnitude)
{
    float actualMagnitude = std::min(magnitude, maxThrust);
    Vector3 dir = direction.normalized();
    AddForce(dir * actualMagnitude);
}

void PhysicsComponent::ApplyRotationalThrust(const Vector3& axis, float magnitude)
{
    float actualMagnitude = std::min(magnitude, maxTorque);
    float len = axis.length();
    Vector3 normalizedAxis = len > 0.0f ? axis.normalized() : Vector3();
    AddTorque(normalizedAxis * actualMagnitude);
}

void PhysicsComponent::ClearForces()
{
    appliedForce  = Vector3();
    appliedTorque = Vector3();
}

} // namespace subspace
