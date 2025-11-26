namespace AvorionLike.Core.Voxel;

/// <summary>
/// Geometric shape of a block for rendering
/// Determines how the block mesh is generated
/// </summary>
public enum BlockShape
{
    /// <summary>Standard cube/rectangular block</summary>
    Cube,
    
    /// <summary>Wedge shape - diagonal slope from one edge to opposite edge</summary>
    Wedge,
    
    /// <summary>Corner wedge - triangular corner piece</summary>
    Corner,
    
    /// <summary>Inner corner - inverted corner piece</summary>
    InnerCorner,
    
    /// <summary>Tetrahedron - pyramid shape</summary>
    Tetrahedron,
    
    /// <summary>Half block - sliced in half</summary>
    HalfBlock
}

/// <summary>
/// Orientation of a shaped block (which direction it faces)
/// Uses 24 possible orientations (6 faces × 4 rotations each)
/// Simplified to 6 main directions for wedges
/// </summary>
public enum BlockOrientation
{
    /// <summary>Slope faces positive X (+X is the high side)</summary>
    PosX,
    /// <summary>Slope faces negative X (-X is the high side)</summary>
    NegX,
    /// <summary>Slope faces positive Y (+Y is the high side)</summary>
    PosY,
    /// <summary>Slope faces negative Y (-Y is the high side)</summary>
    NegY,
    /// <summary>Slope faces positive Z (+Z is the high side)</summary>
    PosZ,
    /// <summary>Slope faces negative Z (-Z is the high side)</summary>
    NegZ
}

/// <summary>
/// Types of voxel blocks that can be placed
/// Following Avorion-style block categories
/// </summary>
public enum BlockType
{
    // Structural
    Hull,
    Armor,
    
    // Functional
    Engine,           // Linear thrust (placed facing backward)
    Thruster,         // Omnidirectional movement (strafing, braking)
    GyroArray,        // Rotation (pitch, yaw, roll) - better when external
    Generator,        // Power generation
    ShieldGenerator,  // Shield generation (integrity field)
    
    // Weapons
    TurretMount,      // Mount point for turrets
    
    // Systems
    HyperdriveCore,   // For jumping between sectors
    Cargo,            // Storage
    CrewQuarters,     // For crew (placed near generator)
    PodDocking,       // Docking port for player pod
    
    // Avorion-style upgrade/computer systems
    Computer,         // Increases upgrade slots based on volume
    Battery,          // Energy storage
    IntegrityField    // Structural integrity field generator
}

/// <summary>
/// Material properties for different tiers
/// Enhanced with vibrant colors and optimized for shiny rendering
/// </summary>
public class MaterialProperties
{
    public string Name { get; set; } = "";
    public float DurabilityMultiplier { get; set; } = 1.0f;
    public float MassMultiplier { get; set; } = 1.0f;
    public float EnergyEfficiency { get; set; } = 1.0f;
    public float ShieldMultiplier { get; set; } = 1.0f;
    public int TechLevel { get; set; } = 1; // Distance from galaxy core requirement
    public uint Color { get; set; } = 0x808080;
    
    public static readonly Dictionary<string, MaterialProperties> Materials = new()
    {
        ["Iron"] = new MaterialProperties
        {
            Name = "Iron",
            DurabilityMultiplier = 1.0f,
            MassMultiplier = 1.0f,
            EnergyEfficiency = 0.8f,
            ShieldMultiplier = 0.5f,
            TechLevel = 1,
            Color = 0xB8B8C0 // Polished steel grey (brighter)
        },
        ["Titanium"] = new MaterialProperties
        {
            Name = "Titanium",
            DurabilityMultiplier = 1.5f,
            MassMultiplier = 0.9f,
            EnergyEfficiency = 1.0f,
            ShieldMultiplier = 0.8f,
            TechLevel = 2,
            Color = 0xD0DEF2 // Brilliant silver-blue
        },
        ["Naonite"] = new MaterialProperties
        {
            Name = "Naonite",
            DurabilityMultiplier = 2.0f,
            MassMultiplier = 0.8f,
            EnergyEfficiency = 1.2f,
            ShieldMultiplier = 1.2f,
            TechLevel = 3,
            Color = 0x26EB59 // Vivid emerald green
        },
        ["Trinium"] = new MaterialProperties
        {
            Name = "Trinium",
            DurabilityMultiplier = 2.5f,
            MassMultiplier = 0.6f,
            EnergyEfficiency = 1.5f,
            ShieldMultiplier = 1.5f,
            TechLevel = 4,
            Color = 0x40A6FF // Brilliant sapphire blue
        },
        ["Xanion"] = new MaterialProperties
        {
            Name = "Xanion",
            DurabilityMultiplier = 3.0f,
            MassMultiplier = 0.5f,
            EnergyEfficiency = 1.8f,
            ShieldMultiplier = 2.0f,
            TechLevel = 5,
            Color = 0xFFD126 // Brilliant gold
        },
        ["Ogonite"] = new MaterialProperties
        {
            Name = "Ogonite",
            DurabilityMultiplier = 4.0f,
            MassMultiplier = 0.4f,
            EnergyEfficiency = 2.2f,
            ShieldMultiplier = 2.5f,
            TechLevel = 6,
            Color = 0xFF6626 // Fiery orange-red
        },
        ["Avorion"] = new MaterialProperties
        {
            Name = "Avorion",
            DurabilityMultiplier = 5.0f,
            MassMultiplier = 0.3f,
            EnergyEfficiency = 3.0f,
            ShieldMultiplier = 3.5f,
            TechLevel = 7,
            Color = 0xD933FF // Royal purple (vibrant)
        }
    };
    
    public static MaterialProperties GetMaterial(string name)
    {
        return Materials.GetValueOrDefault(name, Materials["Iron"]);
    }
}
