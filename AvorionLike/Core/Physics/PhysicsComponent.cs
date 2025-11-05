using System.Numerics;
using AvorionLike.Core.ECS;
using AvorionLike.Core.Persistence;

namespace AvorionLike.Core.Physics;

/// <summary>
/// Component for Newtonian physics properties
/// </summary>
public class PhysicsComponent : IComponent, ISerializable
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

    /// <summary>
    /// Serialize the component to a dictionary
    /// </summary>
    public Dictionary<string, object> Serialize()
    {
        return new Dictionary<string, object>
        {
            ["EntityId"] = EntityId.ToString(),
            ["Position"] = SerializationHelper.SerializeVector3(Position),
            ["Velocity"] = SerializationHelper.SerializeVector3(Velocity),
            ["Acceleration"] = SerializationHelper.SerializeVector3(Acceleration),
            ["Rotation"] = SerializationHelper.SerializeVector3(Rotation),
            ["AngularVelocity"] = SerializationHelper.SerializeVector3(AngularVelocity),
            ["AngularAcceleration"] = SerializationHelper.SerializeVector3(AngularAcceleration),
            ["Mass"] = Mass,
            ["MomentOfInertia"] = MomentOfInertia,
            ["Drag"] = Drag,
            ["AngularDrag"] = AngularDrag,
            ["MaxThrust"] = MaxThrust,
            ["MaxTorque"] = MaxTorque,
            ["CollisionRadius"] = CollisionRadius,
            ["IsStatic"] = IsStatic
        };
    }

    /// <summary>
    /// Deserialize the component from a dictionary
    /// </summary>
    public void Deserialize(Dictionary<string, object> data)
    {
        EntityId = Guid.Parse(SerializationHelper.GetValue(data, "EntityId", Guid.Empty.ToString()));
        Position = SerializationHelper.DeserializeVector3(data["Position"]);
        Velocity = SerializationHelper.DeserializeVector3(data["Velocity"]);
        Acceleration = SerializationHelper.DeserializeVector3(data["Acceleration"]);
        Rotation = SerializationHelper.DeserializeVector3(data["Rotation"]);
        AngularVelocity = SerializationHelper.DeserializeVector3(data["AngularVelocity"]);
        AngularAcceleration = SerializationHelper.DeserializeVector3(data["AngularAcceleration"]);
        Mass = SerializationHelper.GetValue(data, "Mass", 1000f);
        MomentOfInertia = SerializationHelper.GetValue(data, "MomentOfInertia", 1000f);
        Drag = SerializationHelper.GetValue(data, "Drag", 0.1f);
        AngularDrag = SerializationHelper.GetValue(data, "AngularDrag", 0.1f);
        MaxThrust = SerializationHelper.GetValue(data, "MaxThrust", 100f);
        MaxTorque = SerializationHelper.GetValue(data, "MaxTorque", 50f);
        CollisionRadius = SerializationHelper.GetValue(data, "CollisionRadius", 10f);
        IsStatic = SerializationHelper.GetValue(data, "IsStatic", false);
        
        // Reset applied forces (these should not be persisted)
        AppliedForce = Vector3.Zero;
        AppliedTorque = Vector3.Zero;
    }
}
