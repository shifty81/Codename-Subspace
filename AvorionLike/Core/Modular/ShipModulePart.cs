using System.Numerics;

namespace AvorionLike.Core.Modular;

/// <summary>
/// Represents a physical part/module of a modular ship
/// This replaces voxel-based ship construction with pre-defined modular parts
/// </summary>
public class ShipModulePart
{
    public Guid Id { get; set; } = Guid.NewGuid();
    
    /// <summary>
    /// Reference to the module definition (what type of module this is)
    /// </summary>
    public string ModuleDefinitionId { get; set; } = "";
    
    /// <summary>
    /// Position of this module in ship space
    /// </summary>
    public Vector3 Position { get; set; }
    
    /// <summary>
    /// Rotation of this module (euler angles)
    /// </summary>
    public Vector3 Rotation { get; set; }
    
    /// <summary>
    /// Scale of this module (default 1,1,1)
    /// </summary>
    public Vector3 Scale { get; set; } = Vector3.One;
    
    /// <summary>
    /// Material type for this module (affects stats and appearance)
    /// </summary>
    public string MaterialType { get; set; } = "Iron";
    
    /// <summary>
    /// Current health of this module
    /// </summary>
    public float Health { get; set; } = 100f;
    
    /// <summary>
    /// Maximum health of this module
    /// </summary>
    public float MaxHealth { get; set; } = 100f;
    
    /// <summary>
    /// Mass of this module
    /// </summary>
    public float Mass { get; set; } = 10f;
    
    /// <summary>
    /// Is this module destroyed (used for damage visualization)
    /// </summary>
    public bool IsDestroyed => Health <= 0f;
    
    /// <summary>
    /// Damage level (0.0 = pristine, 1.0 = destroyed) for voxel damage visualization
    /// </summary>
    public float DamageLevel => 1.0f - (Health / MaxHealth);
    
    /// <summary>
    /// IDs of modules this module is attached to
    /// </summary>
    public List<Guid> AttachedToModules { get; set; } = new();
    
    /// <summary>
    /// IDs of modules attached to this module
    /// </summary>
    public List<Guid> AttachedModules { get; set; } = new();
    
    /// <summary>
    /// Which attachment point on this module is used
    /// </summary>
    public string AttachmentPointUsed { get; set; } = "";
    
    /// <summary>
    /// Custom color tint for this module (RGB as uint)
    /// </summary>
    public uint ColorTint { get; set; } = 0xFFFFFF;
    
    /// <summary>
    /// Functional properties for different module types
    /// </summary>
    public ModuleFunctionalStats FunctionalStats { get; set; } = new();
    
    public ShipModulePart(string moduleDefinitionId, Vector3 position, string materialType = "Iron")
    {
        ModuleDefinitionId = moduleDefinitionId;
        Position = position;
        MaterialType = materialType;
    }
    
    /// <summary>
    /// Take damage to this module
    /// </summary>
    public void TakeDamage(float damage)
    {
        Health -= damage;
        if (Health < 0) Health = 0;
    }
    
    /// <summary>
    /// Repair this module
    /// </summary>
    public void Repair(float amount)
    {
        Health += amount;
        if (Health > MaxHealth) Health = MaxHealth;
    }
}

/// <summary>
/// Functional statistics for a ship module
/// </summary>
public class ModuleFunctionalStats
{
    // Engine/Thrust properties
    public float ThrustPower { get; set; } = 0f;
    public float MaxSpeed { get; set; } = 0f;
    
    // Power properties
    public float PowerGeneration { get; set; } = 0f;
    public float PowerConsumption { get; set; } = 0f;
    public float PowerStorage { get; set; } = 0f;
    
    // Shield properties
    public float ShieldCapacity { get; set; } = 0f;
    public float ShieldRechargeRate { get; set; } = 0f;
    
    // Weapon properties
    public float WeaponDamage { get; set; } = 0f;
    public float WeaponRange { get; set; } = 0f;
    public int WeaponMountPoints { get; set; } = 0;
    
    // Cargo/Storage
    public float CargoCapacity { get; set; } = 0f;
    
    // Crew
    public int CrewCapacity { get; set; } = 0;
    public int CrewRequired { get; set; } = 0;
    
    // Hyperdrive
    public bool HasHyperdrive { get; set; } = false;
    public float HyperdriveRange { get; set; } = 0f;
    
    // Mining
    public float MiningPower { get; set; } = 0f;
    
    // Radar/Sensors
    public float SensorRange { get; set; } = 0f;
}
