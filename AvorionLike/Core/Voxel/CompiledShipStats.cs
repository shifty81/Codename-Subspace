using System.Numerics;

namespace AvorionLike.Core.Voxel;

/// <summary>
/// Immutable snapshot of compiled ship stats produced by ShipStatsCompiler.
/// At runtime, gameplay systems should read from this struct instead of
/// iterating over blocks. Blocks only matter for damage resolution and
/// visual destruction.
/// </summary>
public readonly struct CompiledShipStats
{
    // Structural
    public float Mass { get; init; }
    public float TotalHitPoints { get; init; }
    public float CurrentHitPoints { get; init; }
    public Vector3 CenterOfMass { get; init; }
    public float MomentOfInertia { get; init; }
    public float StructuralIntegrity { get; init; }

    // Power system
    public float PowerGeneration { get; init; }
    public float PowerConsumption { get; init; }
    public float AvailablePower => PowerGeneration - PowerConsumption;
    public bool HasSufficientPower => PowerGeneration >= PowerConsumption;
    /// <summary>
    /// Performance factor (1.0 = full power, less = brownout).
    /// </summary>
    public float PowerFactor => PowerConsumption > 0
        ? Math.Min(1.0f, PowerGeneration / PowerConsumption)
        : 1.0f;

    // Propulsion
    public float Thrust { get; init; }
    public float Torque { get; init; }
    /// <summary>Thrust adjusted for brownout.</summary>
    public float EffectiveThrust => Thrust * PowerFactor;
    /// <summary>Torque adjusted for brownout.</summary>
    public float EffectiveTorque => Torque * PowerFactor;
    public float Acceleration => Mass > 0 ? EffectiveThrust / Mass : 0f;
    public float MaxSpeed => Acceleration * 10f;
    public float MaxRotationSpeed => MomentOfInertia > 0 ? EffectiveTorque / MomentOfInertia : 0f;

    // Defense
    public float ShieldCapacity { get; init; }
    public float ArmorPoints { get; init; }

    // Utility
    public float CargoCapacity { get; init; }
    public int CrewCapacity { get; init; }
    public int WeaponMounts { get; init; }

    // Special systems
    public bool HasHyperdrive { get; init; }
    public bool HasPodDocking { get; init; }

    // Block counts
    public int TotalBlocks { get; init; }

    /// <summary>
    /// Whether this is a valid (non-default) set of stats.
    /// </summary>
    public bool IsValid => TotalBlocks > 0;
}
