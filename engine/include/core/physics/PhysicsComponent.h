#pragma once

#include "core/ecs/IComponent.h"
#include "core/Math.h"
#include "core/physics/CollisionLayers.h"

namespace subspace {

/// Component for Newtonian physics properties (port of C# PhysicsComponent).
struct PhysicsComponent : IComponent {
    // Linear motion
    Vector3 position;
    Vector3 velocity;
    Vector3 acceleration;

    // Rotational motion
    Vector3 rotation;
    Vector3 angularVelocity;
    Vector3 angularAcceleration;

    // Interpolation for smooth rendering
    Vector3 previousPosition;
    Vector3 previousRotation;
    Vector3 interpolatedPosition;
    Vector3 interpolatedRotation;

    // Physical properties
    float mass            = 1000.0f;
    float momentOfInertia = 1000.0f;
    float drag            = 0.1f;
    float angularDrag     = 0.1f;

    // Thrust capabilities
    float maxThrust = 100.0f;
    float maxTorque = 50.0f;

    // Accumulated forces (cleared each frame)
    Vector3 appliedForce;
    Vector3 appliedTorque;

    // Collision
    float collisionRadius = 10.0f;
    bool  isStatic        = false;

    // Collision layers
    CollisionCategory collisionLayer = CollisionCategory::All;  // Which layer(s) this object belongs to
    CollisionCategory collisionMask  = CollisionCategory::All;  // Which layer(s) this object collides with
    bool isTrigger = false;  // If true, generates events but no physics response

    /// Apply a force to the object.
    void AddForce(const Vector3& force);

    /// Apply torque to the object.
    void AddTorque(const Vector3& torque);

    /// Apply thrust in a direction (limited by maxThrust).
    void ApplyThrust(const Vector3& direction, float magnitude);

    /// Apply rotational thrust (limited by maxTorque).
    void ApplyRotationalThrust(const Vector3& axis, float magnitude);

    /// Clear all applied forces.
    void ClearForces();

    /// Apply a collision preset.
    void SetCollisionPreset(CollisionPresets::Preset preset);

    /// Check whether this component should collide with another.
    bool ShouldCollideWith(const PhysicsComponent& other) const;
};

} // namespace subspace
