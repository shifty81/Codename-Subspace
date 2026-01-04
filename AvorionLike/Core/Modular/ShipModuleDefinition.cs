using System.Numerics;

namespace AvorionLike.Core.Modular;

/// <summary>
/// Defines a type of ship module (like a prefab or template)
/// Inspired by modular spaceship assets like Star Sparrow
/// </summary>
public class ShipModuleDefinition
{
    public string Id { get; set; } = "";
    public string Name { get; set; } = "";
    public string Description { get; set; } = "";
    
    /// <summary>
    /// Category of this module
    /// </summary>
    public ModuleCategory Category { get; set; } = ModuleCategory.Hull;
    
    /// <summary>
    /// Sub-category for more specific classification
    /// </summary>
    public string SubCategory { get; set; } = "";
    
    /// <summary>
    /// Path to the 3D model file (OBJ, FBX, GLTF)
    /// </summary>
    public string ModelPath { get; set; } = "";
    
    /// <summary>
    /// Path to the texture file (optional)
    /// </summary>
    public string TexturePath { get; set; } = "";
    
    /// <summary>
    /// Bounding box size of the module
    /// </summary>
    public Vector3 Size { get; set; } = Vector3.One;
    
    /// <summary>
    /// Base mass of the module
    /// </summary>
    public float BaseMass { get; set; } = 10f;
    
    /// <summary>
    /// Base health of the module
    /// </summary>
    public float BaseHealth { get; set; } = 100f;
    
    /// <summary>
    /// Base cost to build this module
    /// </summary>
    public int BaseCost { get; set; } = 100;
    
    /// <summary>
    /// Attachment points where other modules can connect
    /// Key = attachment point name, Value = relative position
    /// </summary>
    public Dictionary<string, AttachmentPoint> AttachmentPoints { get; set; } = new();
    
    /// <summary>
    /// Base functional stats for this module type
    /// </summary>
    public ModuleFunctionalStats BaseStats { get; set; } = new();
    
    /// <summary>
    /// Minimum tech level required to use this module
    /// </summary>
    public int TechLevel { get; set; } = 1;
    
    /// <summary>
    /// Tags for filtering and searching
    /// </summary>
    public List<string> Tags { get; set; } = new();
    
    /// <summary>
    /// Calculate actual stats based on material
    /// </summary>
    public ModuleFunctionalStats GetStatsForMaterial(string materialType)
    {
        var material = MaterialProperties.GetMaterial(materialType);
        var stats = new ModuleFunctionalStats
        {
            ThrustPower = BaseStats.ThrustPower * material.EnergyEfficiency,
            MaxSpeed = BaseStats.MaxSpeed,
            PowerGeneration = BaseStats.PowerGeneration * material.EnergyEfficiency,
            PowerConsumption = BaseStats.PowerConsumption,
            PowerStorage = BaseStats.PowerStorage * material.EnergyEfficiency,
            ShieldCapacity = BaseStats.ShieldCapacity * material.ShieldMultiplier,
            ShieldRechargeRate = BaseStats.ShieldRechargeRate * material.ShieldMultiplier,
            WeaponDamage = BaseStats.WeaponDamage,
            WeaponRange = BaseStats.WeaponRange,
            WeaponMountPoints = BaseStats.WeaponMountPoints,
            CargoCapacity = BaseStats.CargoCapacity,
            CrewCapacity = BaseStats.CrewCapacity,
            CrewRequired = BaseStats.CrewRequired,
            HasHyperdrive = BaseStats.HasHyperdrive,
            HyperdriveRange = BaseStats.HyperdriveRange * material.EnergyEfficiency,
            MiningPower = BaseStats.MiningPower,
            SensorRange = BaseStats.SensorRange
        };
        return stats;
    }
    
    /// <summary>
    /// Calculate actual health based on material
    /// </summary>
    public float GetHealthForMaterial(string materialType)
    {
        var material = MaterialProperties.GetMaterial(materialType);
        return BaseHealth * material.DurabilityMultiplier;
    }
    
    /// <summary>
    /// Calculate actual mass based on material
    /// </summary>
    public float GetMassForMaterial(string materialType)
    {
        var material = MaterialProperties.GetMaterial(materialType);
        return BaseMass * material.MassMultiplier;
    }
}

/// <summary>
/// Attachment point on a module where other modules can connect
/// </summary>
public class AttachmentPoint
{
    /// <summary>
    /// Name of this attachment point
    /// </summary>
    public string Name { get; set; } = "";
    
    /// <summary>
    /// Position relative to module center
    /// </summary>
    public Vector3 Position { get; set; }
    
    /// <summary>
    /// Direction this attachment point faces (normalized)
    /// </summary>
    public Vector3 Direction { get; set; } = Vector3.UnitZ;
    
    /// <summary>
    /// Types of modules that can attach here (empty = any)
    /// </summary>
    public List<ModuleCategory> AllowedCategories { get; set; } = new();
    
    /// <summary>
    /// Tags required for modules to attach here (empty = no requirements)
    /// </summary>
    public List<string> RequiredTags { get; set; } = new();
    
    /// <summary>
    /// Size of this attachment point (for compatibility checking)
    /// </summary>
    public AttachmentSize Size { get; set; } = AttachmentSize.Medium;
}

/// <summary>
/// Categories of ship modules
/// </summary>
public enum ModuleCategory
{
    // Core structural modules
    Hull,           // Main hull sections, cockpits, cores
    Wing,           // Wing sections
    Tail,           // Tail sections
    
    // Propulsion
    Engine,         // Main engines
    Thruster,       // Maneuvering thrusters
    
    // Weapons
    WeaponMount,    // Turret mounts, weapon hardpoints
    Weapon,         // Actual weapon modules
    
    // Utility
    PowerCore,      // Power generators
    Shield,         // Shield generators
    Cargo,          // Cargo bays
    CrewQuarters,   // Crew compartments
    Hyperdrive,     // FTL drives
    Sensor,         // Radar and sensors
    Mining,         // Mining equipment
    
    // Decorative
    Decorative,     // Non-functional decorative elements
    Antenna,        // Antennas and communication arrays
}

/// <summary>
/// Size categories for attachment points
/// </summary>
public enum AttachmentSize
{
    Small,
    Medium,
    Large,
    ExtraLarge
}

/// <summary>
/// Material properties for calculating module stats
/// </summary>
public static class MaterialProperties
{
    private static readonly Dictionary<string, MaterialData> _materials = new()
    {
        ["Iron"] = new MaterialData 
        { 
            DurabilityMultiplier = 1.0f, 
            MassMultiplier = 1.0f, 
            EnergyEfficiency = 0.8f, 
            ShieldMultiplier = 0.5f,
            Color = 0xB8B8C0
        },
        ["Titanium"] = new MaterialData 
        { 
            DurabilityMultiplier = 1.5f, 
            MassMultiplier = 0.9f, 
            EnergyEfficiency = 1.0f, 
            ShieldMultiplier = 0.8f,
            Color = 0xD0DEF2
        },
        ["Naonite"] = new MaterialData 
        { 
            DurabilityMultiplier = 2.0f, 
            MassMultiplier = 0.8f, 
            EnergyEfficiency = 1.2f, 
            ShieldMultiplier = 1.2f,
            Color = 0x26EB59
        },
        ["Trinium"] = new MaterialData 
        { 
            DurabilityMultiplier = 2.5f, 
            MassMultiplier = 0.6f, 
            EnergyEfficiency = 1.5f, 
            ShieldMultiplier = 1.5f,
            Color = 0x40A6FF
        },
        ["Xanion"] = new MaterialData 
        { 
            DurabilityMultiplier = 3.0f, 
            MassMultiplier = 0.5f, 
            EnergyEfficiency = 1.8f, 
            ShieldMultiplier = 2.0f,
            Color = 0xFFD126
        },
        ["Ogonite"] = new MaterialData 
        { 
            DurabilityMultiplier = 4.0f, 
            MassMultiplier = 0.4f, 
            EnergyEfficiency = 2.2f, 
            ShieldMultiplier = 2.5f,
            Color = 0xFF6626
        },
        ["Avorion"] = new MaterialData 
        { 
            DurabilityMultiplier = 5.0f, 
            MassMultiplier = 0.3f, 
            EnergyEfficiency = 3.0f, 
            ShieldMultiplier = 3.5f,
            Color = 0xD933FF
        }
    };
    
    public static MaterialData GetMaterial(string name)
    {
        return _materials.TryGetValue(name, out var material) 
            ? material 
            : _materials["Iron"];
    }
    
    public static IEnumerable<string> GetAllMaterialNames() => _materials.Keys;
}

/// <summary>
/// Material data for stat calculations
/// </summary>
public class MaterialData
{
    public float DurabilityMultiplier { get; set; } = 1.0f;
    public float MassMultiplier { get; set; } = 1.0f;
    public float EnergyEfficiency { get; set; } = 1.0f;
    public float ShieldMultiplier { get; set; } = 1.0f;
    public uint Color { get; set; } = 0xFFFFFF;
}
