using System.Numerics;
using AvorionLike.Core.ECS;

namespace AvorionLike.Core.Physics;

/// <summary>
/// Component for Newtonian physics properties
/// </summary>
public class PhysicsComponent : IComponent
{
    public Guid EntityId { get; set; }
    
    // Linear motion
    public Vector3 Position { get; set; }
    public Vector3 Velocity { get; set; }
    public Vector3 Acceleration { get; set; }
    
    // Rotational motion
    public Vector3 Rotation { get; set; }
    public Vector3 AngularVelocity { get; set; }
    public Vector3 AngularAcceleration { get; set; }
    
    // Physical properties
    public float Mass { get; set; } = 1000f;
    public float MomentOfInertia { get; set; } = 1000f; // For rotational motion
    public float Drag { get; set; } = 0.1f;
    public float AngularDrag { get; set; } = 0.1f;
    
    // Thrust capabilities (from ship design)
    public float MaxThrust { get; set; } = 100f;
    public float MaxTorque { get; set; } = 50f;
    
    // Forces
    public Vector3 AppliedForce { get; set; }
    public Vector3 AppliedTorque { get; set; }
    
    // Collision
    public float CollisionRadius { get; set; } = 10f;
    public bool IsStatic { get; set; } = false;

    /// <summary>
    /// Apply a force to the object
    /// </summary>
    public void AddForce(Vector3 force)
    {
        AppliedForce += force;
    }

    /// <summary>
    /// Apply torque to the object
    /// </summary>
    public void AddTorque(Vector3 torque)
    {
        AppliedTorque += torque;
    }
    
    /// <summary>
    /// Apply thrust in a direction (limited by max thrust)
    /// </summary>
    public void ApplyThrust(Vector3 direction, float magnitude)
    {
        float actualMagnitude = Math.Min(magnitude, MaxThrust);
        AddForce(Vector3.Normalize(direction) * actualMagnitude);
    }
    
    /// <summary>
    /// Apply rotational thrust (limited by max torque)
    /// </summary>
    public void ApplyRotationalThrust(Vector3 axis, float magnitude)
    {
        float actualMagnitude = Math.Min(magnitude, MaxTorque);
        // Normalize axis to ensure correct torque scaling
        var normalizedAxis = axis.Length() > 0 ? Vector3.Normalize(axis) : Vector3.Zero;
        AddTorque(normalizedAxis * actualMagnitude);
    }

    /// <summary>
    /// Clear all applied forces
    /// </summary>
    public void ClearForces()
    {
        AppliedForce = Vector3.Zero;
        AppliedTorque = Vector3.Zero;
    }
}
