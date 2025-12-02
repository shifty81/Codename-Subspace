using System.Numerics;
using AvorionLike.Core.Voxel;

namespace AvorionLike.Core.Procedural;

/// <summary>
/// Types of ship modules that can be combined to build ships
/// Based on No Man's Sky style modular generation approach
/// </summary>
public enum ModuleType
{
    // Core modules (required)
    Cockpit,         // Command center - various styles (bubble, angular, sleek, etc.)
    CoreHull,        // Main body section
    
    // Propulsion modules
    EngineNacelle,   // Engine pods/nacelles
    ThrusterArray,   // Maneuvering thrusters
    MainEngine,      // Large primary engine
    
    // Structure modules
    MidSection,      // Middle body segments
    Connector,       // Connecting pieces between sections
    Wing,            // Wing attachments (swept, delta, stubby, etc.)
    Fin,             // Stabilizer fins
    Pylon,           // Support structures
    
    // Functional modules
    WeaponMount,     // Hardpoints for weapons
    CargoContainer,  // Cargo storage pods
    SensorArray,     // Sensor dishes and arrays
    ShieldEmitter,   // Shield generator modules
    PowerCore,       // Power generation modules
    
    // Decorative modules
    Antenna,         // Communication antennas
    Greeble,         // Surface detail panels
    LightArray,      // Running lights and beacons
    Decal            // Surface markings
}

/// <summary>
/// Visual style categories for modules
/// </summary>
public enum ModuleStyle
{
    Military,     // Angular, armored, aggressive
    Industrial,   // Blocky, utilitarian, exposed
    Sleek,        // Streamlined, curved, elegant
    Organic,      // Flowing, biological-inspired
    Pirate,       // Cobbled, asymmetric, worn
    Ancient,      // Mysterious, geometric, alien
    Civilian      // Simple, clean, practical
}

/// <summary>
/// Attachment point for connecting modules together
/// </summary>
public class AttachmentPoint
{
    /// <summary>
    /// Position relative to module center
    /// </summary>
    public Vector3 Position { get; set; }
    
    /// <summary>
    /// Direction the attachment point faces (outward normal)
    /// </summary>
    public Vector3 Normal { get; set; }
    
    /// <summary>
    /// Types of modules that can attach here
    /// </summary>
    public List<ModuleType> CompatibleTypes { get; set; } = new();
    
    /// <summary>
    /// Size category (must match for connection)
    /// </summary>
    public int SizeClass { get; set; } = 1;
    
    /// <summary>
    /// Whether this attachment point is occupied
    /// </summary>
    public bool IsOccupied { get; set; } = false;
    
    /// <summary>
    /// Unique identifier for this attachment point
    /// </summary>
    public string Id { get; set; } = "";
}

/// <summary>
/// A pre-defined ship module that can be combined with other modules
/// to create complete ships in a NMS-style approach
/// </summary>
public class ShipModule
{
    /// <summary>
    /// Unique identifier for this module variant
    /// </summary>
    public string Id { get; set; } = "";
    
    /// <summary>
    /// Human-readable name
    /// </summary>
    public string Name { get; set; } = "";
    
    /// <summary>
    /// Type of module
    /// </summary>
    public ModuleType Type { get; set; }
    
    /// <summary>
    /// Visual style of this module variant
    /// </summary>
    public ModuleStyle Style { get; set; }
    
    /// <summary>
    /// Blocks that make up this module
    /// </summary>
    public List<VoxelBlock> Blocks { get; set; } = new();
    
    /// <summary>
    /// Attachment points for connecting to other modules
    /// </summary>
    public List<AttachmentPoint> AttachmentPoints { get; set; } = new();
    
    /// <summary>
    /// Bounding box size
    /// </summary>
    public Vector3 Size { get; set; }
    
    /// <summary>
    /// Module center offset (for positioning)
    /// </summary>
    public Vector3 CenterOffset { get; set; } = Vector3.Zero;
    
    /// <summary>
    /// Scale multiplier (1.0 = normal)
    /// </summary>
    public float Scale { get; set; } = 1.0f;
    
    /// <summary>
    /// Whether this module can be mirrored
    /// </summary>
    public bool CanMirror { get; set; } = true;
    
    /// <summary>
    /// Weight of module (affects ship stats)
    /// </summary>
    public float Mass { get; set; } = 100f;
    
    /// <summary>
    /// Power generation if any
    /// </summary>
    public float PowerGeneration { get; set; } = 0f;
    
    /// <summary>
    /// Power consumption if any
    /// </summary>
    public float PowerConsumption { get; set; } = 0f;
    
    /// <summary>
    /// Thrust provided if propulsion module
    /// </summary>
    public float Thrust { get; set; } = 0f;
    
    /// <summary>
    /// Cargo capacity if cargo module
    /// </summary>
    public float CargoCapacity { get; set; } = 0f;
    
    /// <summary>
    /// Rarity tier (affects spawn chance)
    /// </summary>
    public int RarityTier { get; set; } = 1;
    
    /// <summary>
    /// Create a transformed copy of this module at a new position and rotation
    /// </summary>
    public ShipModule CreateTransformedCopy(Vector3 position, Quaternion rotation, bool mirror = false)
    {
        var copy = new ShipModule
        {
            Id = Id,
            Name = Name,
            Type = Type,
            Style = Style,
            Size = Size,
            CenterOffset = CenterOffset,
            Scale = Scale,
            CanMirror = CanMirror,
            Mass = Mass,
            PowerGeneration = PowerGeneration,
            PowerConsumption = PowerConsumption,
            Thrust = Thrust,
            CargoCapacity = CargoCapacity,
            RarityTier = RarityTier
        };
        
        // Transform blocks
        foreach (var block in Blocks)
        {
            var newPos = Vector3.Transform(block.Position, rotation);
            if (mirror)
            {
                newPos.X = -newPos.X;
            }
            newPos += position;
            
            var newBlock = new VoxelBlock(
                newPos,
                block.Size,
                block.MaterialType,
                block.BlockType,
                block.Shape,
                block.Orientation
            );
            newBlock.ColorRGB = block.ColorRGB;
            copy.Blocks.Add(newBlock);
        }
        
        // Transform attachment points
        foreach (var ap in AttachmentPoints)
        {
            var newPos = Vector3.Transform(ap.Position, rotation);
            var newNormal = Vector3.Transform(ap.Normal, rotation);
            if (mirror)
            {
                newPos.X = -newPos.X;
                newNormal.X = -newNormal.X;
            }
            newPos += position;
            
            copy.AttachmentPoints.Add(new AttachmentPoint
            {
                Id = ap.Id,
                Position = newPos,
                Normal = newNormal,
                CompatibleTypes = new List<ModuleType>(ap.CompatibleTypes),
                SizeClass = ap.SizeClass,
                IsOccupied = false
            });
        }
        
        return copy;
    }
}

/// <summary>
/// Factory for creating pre-defined ship modules
/// Contains templates for various module types and styles
/// </summary>
public static class ModuleFactory
{
    private static readonly Random _random = new();
    
    /// <summary>
    /// Create a cockpit module of the specified style
    /// </summary>
    public static ShipModule CreateCockpit(ModuleStyle style, string material = "Titanium")
    {
        var module = new ShipModule
        {
            Id = $"cockpit_{style}_{Guid.NewGuid():N}",
            Name = $"{style} Cockpit",
            Type = ModuleType.Cockpit,
            Style = style,
            Mass = 50f,
            PowerConsumption = 5f
        };
        
        switch (style)
        {
            case ModuleStyle.Military:
                CreateMilitaryCockpit(module, material);
                break;
            case ModuleStyle.Sleek:
                CreateSleekCockpit(module, material);
                break;
            case ModuleStyle.Industrial:
                CreateIndustrialCockpit(module, material);
                break;
            case ModuleStyle.Pirate:
                CreatePirateCockpit(module, material);
                break;
            default:
                CreateCivilianCockpit(module, material);
                break;
        }
        
        return module;
    }
    
    /// <summary>
    /// Create a core hull module of the specified style
    /// </summary>
    public static ShipModule CreateCoreHull(ModuleStyle style, int sizeClass = 2, string material = "Iron")
    {
        var module = new ShipModule
        {
            Id = $"hull_{style}_{sizeClass}_{Guid.NewGuid():N}",
            Name = $"{style} Hull Section",
            Type = ModuleType.CoreHull,
            Style = style,
            Mass = 200f * sizeClass,
            PowerConsumption = 2f
        };
        
        switch (style)
        {
            case ModuleStyle.Military:
                CreateMilitaryHull(module, sizeClass, material);
                break;
            case ModuleStyle.Industrial:
                CreateIndustrialHull(module, sizeClass, material);
                break;
            case ModuleStyle.Sleek:
                CreateSleekHull(module, sizeClass, material);
                break;
            default:
                CreateCivilianHull(module, sizeClass, material);
                break;
        }
        
        return module;
    }
    
    /// <summary>
    /// Create an engine nacelle module
    /// </summary>
    public static ShipModule CreateEngineNacelle(ModuleStyle style, int sizeClass = 1, string material = "Iron")
    {
        var module = new ShipModule
        {
            Id = $"engine_{style}_{sizeClass}_{Guid.NewGuid():N}",
            Name = $"{style} Engine Nacelle",
            Type = ModuleType.EngineNacelle,
            Style = style,
            Mass = 100f * sizeClass,
            Thrust = 500f * sizeClass,
            PowerConsumption = 50f * sizeClass
        };
        
        CreateEngineNacelleBlocks(module, style, sizeClass, material);
        return module;
    }
    
    /// <summary>
    /// Create a wing module
    /// </summary>
    public static ShipModule CreateWing(ModuleStyle style, bool isRightSide = true, string material = "Iron")
    {
        var module = new ShipModule
        {
            Id = $"wing_{style}_{(isRightSide ? "R" : "L")}_{Guid.NewGuid():N}",
            Name = $"{style} Wing ({(isRightSide ? "Right" : "Left")})",
            Type = ModuleType.Wing,
            Style = style,
            CanMirror = true,
            Mass = 60f
        };
        
        CreateWingBlocks(module, style, isRightSide, material);
        return module;
    }
    
    /// <summary>
    /// Create a cargo container module
    /// </summary>
    public static ShipModule CreateCargoContainer(ModuleStyle style, int sizeClass = 1, string material = "Iron")
    {
        var module = new ShipModule
        {
            Id = $"cargo_{style}_{sizeClass}_{Guid.NewGuid():N}",
            Name = $"{style} Cargo Container",
            Type = ModuleType.CargoContainer,
            Style = style,
            Mass = 50f * sizeClass,
            CargoCapacity = 100f * sizeClass
        };
        
        CreateCargoBlocks(module, style, sizeClass, material);
        return module;
    }
    
    /// <summary>
    /// Create a weapon mount module
    /// </summary>
    public static ShipModule CreateWeaponMount(ModuleStyle style, string material = "Iron")
    {
        var module = new ShipModule
        {
            Id = $"weapon_{style}_{Guid.NewGuid():N}",
            Name = $"{style} Weapon Mount",
            Type = ModuleType.WeaponMount,
            Style = style,
            Mass = 30f,
            PowerConsumption = 20f
        };
        
        CreateWeaponMountBlocks(module, style, material);
        return module;
    }
    
    /// <summary>
    /// Create a mid-section connector module
    /// </summary>
    public static ShipModule CreateMidSection(ModuleStyle style, int length = 2, string material = "Iron")
    {
        var module = new ShipModule
        {
            Id = $"mid_{style}_{length}_{Guid.NewGuid():N}",
            Name = $"{style} Mid Section",
            Type = ModuleType.MidSection,
            Style = style,
            Mass = 100f * length
        };
        
        CreateMidSectionBlocks(module, style, length, material);
        return module;
    }
    
    /// <summary>
    /// Create a sensor array module
    /// </summary>
    public static ShipModule CreateSensorArray(ModuleStyle style, string material = "Titanium")
    {
        var module = new ShipModule
        {
            Id = $"sensor_{style}_{Guid.NewGuid():N}",
            Name = $"{style} Sensor Array",
            Type = ModuleType.SensorArray,
            Style = style,
            Mass = 20f,
            PowerConsumption = 15f
        };
        
        CreateSensorArrayBlocks(module, style, material);
        return module;
    }
    
    // ========== Private module creation methods ==========
    
    private static void CreateMilitaryCockpit(ShipModule module, string material)
    {
        float bs = 2f; // block size
        
        // Angular, wedge-shaped cockpit
        // Main cockpit body
        module.Blocks.Add(new VoxelBlock(new Vector3(0, 0, 0), new Vector3(bs * 2, bs * 1.5f, bs * 2), material, BlockType.Hull));
        module.Blocks.Add(new VoxelBlock(new Vector3(0, 0, bs * 2), new Vector3(bs * 1.5f, bs, bs * 2), material, BlockType.Hull));
        
        // Pointed nose with wedge
        module.Blocks.Add(new VoxelBlock(new Vector3(0, 0, bs * 4), new Vector3(bs, bs * 0.75f, bs), material, BlockType.Hull, BlockShape.Wedge, BlockOrientation.PosZ));
        
        // Canopy (glass)
        module.Blocks.Add(new VoxelBlock(new Vector3(0, bs * 0.75f, bs), new Vector3(bs * 1.5f, bs * 0.5f, bs * 2), "Crystal", BlockType.Hull));
        
        // Armor ridge
        module.Blocks.Add(new VoxelBlock(new Vector3(0, bs, -bs * 0.5f), new Vector3(bs * 2.5f, bs * 0.5f, bs), material, BlockType.Armor));
        
        module.Size = new Vector3(bs * 3, bs * 2, bs * 5);
        
        // Attachment point at rear
        module.AttachmentPoints.Add(new AttachmentPoint
        {
            Id = "rear",
            Position = new Vector3(0, 0, -bs),
            Normal = new Vector3(0, 0, -1),
            CompatibleTypes = new List<ModuleType> { ModuleType.CoreHull, ModuleType.MidSection },
            SizeClass = 2
        });
    }
    
    private static void CreateSleekCockpit(ShipModule module, string material)
    {
        float bs = 2f;
        
        // Streamlined, teardrop-shaped cockpit
        module.Blocks.Add(new VoxelBlock(new Vector3(0, 0, 0), new Vector3(bs * 1.5f, bs, bs * 2), material, BlockType.Hull));
        module.Blocks.Add(new VoxelBlock(new Vector3(0, 0, bs * 2), new Vector3(bs, bs * 0.75f, bs * 2), material, BlockType.Hull));
        module.Blocks.Add(new VoxelBlock(new Vector3(0, 0, bs * 4), new Vector3(bs * 0.5f, bs * 0.5f, bs), material, BlockType.Hull, BlockShape.Wedge, BlockOrientation.PosZ));
        
        // Bubble canopy
        module.Blocks.Add(new VoxelBlock(new Vector3(0, bs * 0.5f, bs), new Vector3(bs, bs * 0.75f, bs * 2), "Crystal", BlockType.Hull));
        
        module.Size = new Vector3(bs * 2, bs * 1.5f, bs * 5);
        
        module.AttachmentPoints.Add(new AttachmentPoint
        {
            Id = "rear",
            Position = new Vector3(0, 0, -bs),
            Normal = new Vector3(0, 0, -1),
            CompatibleTypes = new List<ModuleType> { ModuleType.CoreHull, ModuleType.MidSection },
            SizeClass = 1
        });
    }
    
    private static void CreateIndustrialCockpit(ShipModule module, string material)
    {
        float bs = 2f;
        
        // Boxy, utilitarian cockpit with exposed frame
        module.Blocks.Add(new VoxelBlock(new Vector3(0, 0, 0), new Vector3(bs * 2.5f, bs * 2, bs * 2), material, BlockType.Hull));
        module.Blocks.Add(new VoxelBlock(new Vector3(0, 0, bs * 2), new Vector3(bs * 2, bs * 1.5f, bs), material, BlockType.Hull));
        
        // Frame struts
        module.Blocks.Add(new VoxelBlock(new Vector3(bs, bs * 0.75f, bs * 1.5f), new Vector3(bs * 0.25f, bs * 0.25f, bs * 2), material, BlockType.Hull));
        module.Blocks.Add(new VoxelBlock(new Vector3(-bs, bs * 0.75f, bs * 1.5f), new Vector3(bs * 0.25f, bs * 0.25f, bs * 2), material, BlockType.Hull));
        
        // Window panels
        module.Blocks.Add(new VoxelBlock(new Vector3(0, bs, bs * 1.5f), new Vector3(bs * 1.5f, bs * 0.5f, bs), "Crystal", BlockType.Hull));
        
        module.Size = new Vector3(bs * 3, bs * 2.5f, bs * 3);
        
        module.AttachmentPoints.Add(new AttachmentPoint
        {
            Id = "rear",
            Position = new Vector3(0, 0, -bs),
            Normal = new Vector3(0, 0, -1),
            CompatibleTypes = new List<ModuleType> { ModuleType.CoreHull, ModuleType.MidSection },
            SizeClass = 2
        });
    }
    
    private static void CreatePirateCockpit(ShipModule module, string material)
    {
        float bs = 2f;
        
        // Asymmetric, cobbled together
        module.Blocks.Add(new VoxelBlock(new Vector3(-bs * 0.25f, 0, 0), new Vector3(bs * 2, bs * 1.5f, bs * 2), material, BlockType.Hull));
        module.Blocks.Add(new VoxelBlock(new Vector3(bs * 0.5f, bs * 0.25f, bs), new Vector3(bs, bs, bs * 1.5f), material, BlockType.Hull));
        
        // Patched armor
        module.Blocks.Add(new VoxelBlock(new Vector3(-bs, 0, bs * 0.5f), new Vector3(bs * 0.5f, bs, bs), "Iron", BlockType.Armor));
        
        // Cracked window
        module.Blocks.Add(new VoxelBlock(new Vector3(0, bs * 0.5f, bs * 1.5f), new Vector3(bs, bs * 0.5f, bs), "Crystal", BlockType.Hull));
        
        module.Size = new Vector3(bs * 2.5f, bs * 2, bs * 3);
        
        module.AttachmentPoints.Add(new AttachmentPoint
        {
            Id = "rear",
            Position = new Vector3(0, 0, -bs),
            Normal = new Vector3(0, 0, -1),
            CompatibleTypes = new List<ModuleType> { ModuleType.CoreHull, ModuleType.MidSection },
            SizeClass = 2
        });
    }
    
    private static void CreateCivilianCockpit(ShipModule module, string material)
    {
        float bs = 2f;
        
        // Simple, clean design
        module.Blocks.Add(new VoxelBlock(new Vector3(0, 0, 0), new Vector3(bs * 2, bs * 1.5f, bs * 2), material, BlockType.Hull));
        module.Blocks.Add(new VoxelBlock(new Vector3(0, 0, bs * 2), new Vector3(bs * 1.5f, bs, bs), material, BlockType.Hull));
        
        // Clear canopy
        module.Blocks.Add(new VoxelBlock(new Vector3(0, bs * 0.5f, bs), new Vector3(bs * 1.25f, bs * 0.75f, bs * 1.5f), "Crystal", BlockType.Hull));
        
        module.Size = new Vector3(bs * 2, bs * 2, bs * 3);
        
        module.AttachmentPoints.Add(new AttachmentPoint
        {
            Id = "rear",
            Position = new Vector3(0, 0, -bs),
            Normal = new Vector3(0, 0, -1),
            CompatibleTypes = new List<ModuleType> { ModuleType.CoreHull, ModuleType.MidSection },
            SizeClass = 1
        });
    }
    
    private static void CreateMilitaryHull(ShipModule module, int sizeClass, string material)
    {
        float bs = 2f;
        float sc = sizeClass;
        
        // Angular, armored hull
        float width = bs * 3 * sc;
        float height = bs * 2 * sc;
        float length = bs * 4 * sc;
        
        // Main body
        module.Blocks.Add(new VoxelBlock(new Vector3(0, 0, 0), new Vector3(width, height, length), material, BlockType.Hull));
        
        // Armor plating
        module.Blocks.Add(new VoxelBlock(new Vector3(0, height / 2 + bs * 0.25f, 0), new Vector3(width * 0.8f, bs * 0.5f, length * 0.8f), material, BlockType.Armor));
        module.Blocks.Add(new VoxelBlock(new Vector3(0, -height / 2 - bs * 0.25f, 0), new Vector3(width * 0.8f, bs * 0.5f, length * 0.8f), material, BlockType.Armor));
        
        // Side armor ridges
        module.Blocks.Add(new VoxelBlock(new Vector3(width / 2 + bs * 0.25f, 0, 0), new Vector3(bs * 0.5f, height * 0.6f, length * 0.6f), material, BlockType.Armor));
        module.Blocks.Add(new VoxelBlock(new Vector3(-width / 2 - bs * 0.25f, 0, 0), new Vector3(bs * 0.5f, height * 0.6f, length * 0.6f), material, BlockType.Armor));
        
        module.Size = new Vector3(width + bs, height + bs, length);
        
        // Attachment points
        module.AttachmentPoints.Add(new AttachmentPoint
        {
            Id = "front",
            Position = new Vector3(0, 0, length / 2 + bs * 0.5f),
            Normal = new Vector3(0, 0, 1),
            CompatibleTypes = new List<ModuleType> { ModuleType.Cockpit, ModuleType.MidSection },
            SizeClass = sizeClass
        });
        module.AttachmentPoints.Add(new AttachmentPoint
        {
            Id = "rear",
            Position = new Vector3(0, 0, -length / 2 - bs * 0.5f),
            Normal = new Vector3(0, 0, -1),
            CompatibleTypes = new List<ModuleType> { ModuleType.EngineNacelle, ModuleType.MainEngine, ModuleType.MidSection },
            SizeClass = sizeClass
        });
        module.AttachmentPoints.Add(new AttachmentPoint
        {
            Id = "left_wing",
            Position = new Vector3(-width / 2 - bs * 0.5f, 0, 0),
            Normal = new Vector3(-1, 0, 0),
            CompatibleTypes = new List<ModuleType> { ModuleType.Wing, ModuleType.EngineNacelle },
            SizeClass = sizeClass
        });
        module.AttachmentPoints.Add(new AttachmentPoint
        {
            Id = "right_wing",
            Position = new Vector3(width / 2 + bs * 0.5f, 0, 0),
            Normal = new Vector3(1, 0, 0),
            CompatibleTypes = new List<ModuleType> { ModuleType.Wing, ModuleType.EngineNacelle },
            SizeClass = sizeClass
        });
        module.AttachmentPoints.Add(new AttachmentPoint
        {
            Id = "top",
            Position = new Vector3(0, height / 2 + bs * 0.5f, 0),
            Normal = new Vector3(0, 1, 0),
            CompatibleTypes = new List<ModuleType> { ModuleType.WeaponMount, ModuleType.SensorArray, ModuleType.Antenna },
            SizeClass = 1
        });
    }
    
    private static void CreateIndustrialHull(ShipModule module, int sizeClass, string material)
    {
        float bs = 2f;
        float sc = sizeClass;
        
        // Boxy, exposed frame design
        float width = bs * 4 * sc;
        float height = bs * 3 * sc;
        float length = bs * 5 * sc;
        
        // Main body (hollowed frame effect)
        module.Blocks.Add(new VoxelBlock(new Vector3(0, 0, 0), new Vector3(width, height, length), material, BlockType.Hull));
        
        // External frame struts
        float strutSize = bs * 0.4f;
        module.Blocks.Add(new VoxelBlock(new Vector3(width / 2, height / 2, 0), new Vector3(strutSize, strutSize, length), material, BlockType.Hull));
        module.Blocks.Add(new VoxelBlock(new Vector3(-width / 2, height / 2, 0), new Vector3(strutSize, strutSize, length), material, BlockType.Hull));
        module.Blocks.Add(new VoxelBlock(new Vector3(width / 2, -height / 2, 0), new Vector3(strutSize, strutSize, length), material, BlockType.Hull));
        module.Blocks.Add(new VoxelBlock(new Vector3(-width / 2, -height / 2, 0), new Vector3(strutSize, strutSize, length), material, BlockType.Hull));
        
        module.Size = new Vector3(width + bs, height + bs, length);
        
        // Attachment points - more for modular cargo attachment
        module.AttachmentPoints.Add(new AttachmentPoint
        {
            Id = "front",
            Position = new Vector3(0, 0, length / 2 + bs * 0.5f),
            Normal = new Vector3(0, 0, 1),
            CompatibleTypes = new List<ModuleType> { ModuleType.Cockpit, ModuleType.MidSection },
            SizeClass = sizeClass
        });
        module.AttachmentPoints.Add(new AttachmentPoint
        {
            Id = "rear",
            Position = new Vector3(0, 0, -length / 2 - bs * 0.5f),
            Normal = new Vector3(0, 0, -1),
            CompatibleTypes = new List<ModuleType> { ModuleType.EngineNacelle, ModuleType.MainEngine },
            SizeClass = sizeClass
        });
        module.AttachmentPoints.Add(new AttachmentPoint
        {
            Id = "top_cargo",
            Position = new Vector3(0, height / 2 + bs, 0),
            Normal = new Vector3(0, 1, 0),
            CompatibleTypes = new List<ModuleType> { ModuleType.CargoContainer, ModuleType.SensorArray },
            SizeClass = sizeClass
        });
        module.AttachmentPoints.Add(new AttachmentPoint
        {
            Id = "bottom_cargo",
            Position = new Vector3(0, -height / 2 - bs, 0),
            Normal = new Vector3(0, -1, 0),
            CompatibleTypes = new List<ModuleType> { ModuleType.CargoContainer },
            SizeClass = sizeClass
        });
    }
    
    private static void CreateSleekHull(ShipModule module, int sizeClass, string material)
    {
        float bs = 2f;
        float sc = sizeClass;
        
        // Streamlined, elongated hull
        float width = bs * 2 * sc;
        float height = bs * 1.5f * sc;
        float length = bs * 6 * sc;
        
        // Main body with tapered ends
        module.Blocks.Add(new VoxelBlock(new Vector3(0, 0, 0), new Vector3(width, height, length), material, BlockType.Hull));
        
        // Tapered front
        module.Blocks.Add(new VoxelBlock(new Vector3(0, 0, length / 2 + bs), new Vector3(width * 0.7f, height * 0.7f, bs * 2), material, BlockType.Hull));
        
        // Tapered rear
        module.Blocks.Add(new VoxelBlock(new Vector3(0, 0, -length / 2 - bs), new Vector3(width * 0.8f, height * 0.8f, bs * 2), material, BlockType.Hull));
        
        module.Size = new Vector3(width, height, length + bs * 4);
        
        module.AttachmentPoints.Add(new AttachmentPoint
        {
            Id = "front",
            Position = new Vector3(0, 0, length / 2 + bs * 2),
            Normal = new Vector3(0, 0, 1),
            CompatibleTypes = new List<ModuleType> { ModuleType.Cockpit },
            SizeClass = sizeClass
        });
        module.AttachmentPoints.Add(new AttachmentPoint
        {
            Id = "rear",
            Position = new Vector3(0, 0, -length / 2 - bs * 2),
            Normal = new Vector3(0, 0, -1),
            CompatibleTypes = new List<ModuleType> { ModuleType.EngineNacelle, ModuleType.MainEngine },
            SizeClass = sizeClass
        });
    }
    
    private static void CreateCivilianHull(ShipModule module, int sizeClass, string material)
    {
        float bs = 2f;
        float sc = sizeClass;
        
        float width = bs * 3 * sc;
        float height = bs * 2 * sc;
        float length = bs * 4 * sc;
        
        // Simple rectangular hull
        module.Blocks.Add(new VoxelBlock(new Vector3(0, 0, 0), new Vector3(width, height, length), material, BlockType.Hull));
        
        module.Size = new Vector3(width, height, length);
        
        module.AttachmentPoints.Add(new AttachmentPoint
        {
            Id = "front",
            Position = new Vector3(0, 0, length / 2 + bs * 0.5f),
            Normal = new Vector3(0, 0, 1),
            CompatibleTypes = new List<ModuleType> { ModuleType.Cockpit, ModuleType.MidSection },
            SizeClass = sizeClass
        });
        module.AttachmentPoints.Add(new AttachmentPoint
        {
            Id = "rear",
            Position = new Vector3(0, 0, -length / 2 - bs * 0.5f),
            Normal = new Vector3(0, 0, -1),
            CompatibleTypes = new List<ModuleType> { ModuleType.EngineNacelle, ModuleType.MainEngine },
            SizeClass = sizeClass
        });
    }
    
    private static void CreateEngineNacelleBlocks(ShipModule module, ModuleStyle style, int sizeClass, string material)
    {
        float bs = 2f;
        float sc = sizeClass;
        
        float width = bs * 1.5f * sc;
        float height = bs * 1.5f * sc;
        float length = bs * 3 * sc;
        
        // Engine housing
        module.Blocks.Add(new VoxelBlock(new Vector3(0, 0, 0), new Vector3(width, height, length), material, BlockType.Hull));
        
        // Engine nozzle (at rear)
        module.Blocks.Add(new VoxelBlock(new Vector3(0, 0, -length / 2 - bs * 0.5f), new Vector3(width * 0.8f, height * 0.8f, bs), material, BlockType.Engine));
        
        // Engine glow
        module.Blocks.Add(new VoxelBlock(new Vector3(0, 0, -length / 2 - bs), new Vector3(width * 0.6f, height * 0.6f, bs * 0.5f), "Energy", BlockType.Hull));
        
        if (style == ModuleStyle.Military || style == ModuleStyle.Industrial)
        {
            // Add intake vents at front
            module.Blocks.Add(new VoxelBlock(new Vector3(0, 0, length / 2 + bs * 0.25f), new Vector3(width * 0.9f, height * 0.3f, bs * 0.5f), material, BlockType.Hull));
        }
        
        module.Size = new Vector3(width, height, length + bs * 2);
        
        // Attachment point at front
        module.AttachmentPoints.Add(new AttachmentPoint
        {
            Id = "front",
            Position = new Vector3(0, 0, length / 2 + bs * 0.5f),
            Normal = new Vector3(0, 0, 1),
            CompatibleTypes = new List<ModuleType> { ModuleType.CoreHull, ModuleType.MidSection },
            SizeClass = sizeClass
        });
    }
    
    private static void CreateWingBlocks(ShipModule module, ModuleStyle style, bool isRightSide, string material)
    {
        float bs = 2f;
        float xMult = isRightSide ? 1 : -1;
        
        switch (style)
        {
            case ModuleStyle.Military:
                // Swept-back delta wing
                module.Blocks.Add(new VoxelBlock(new Vector3(bs * 2 * xMult, 0, 0), new Vector3(bs * 3, bs * 0.5f, bs * 4), material, BlockType.Hull));
                module.Blocks.Add(new VoxelBlock(new Vector3(bs * 4 * xMult, 0, -bs), new Vector3(bs * 2, bs * 0.4f, bs * 3), material, BlockType.Hull));
                module.Blocks.Add(new VoxelBlock(new Vector3(bs * 5.5f * xMult, 0, -bs * 2), new Vector3(bs, bs * 0.3f, bs * 2), material, BlockType.Hull, BlockShape.Wedge, isRightSide ? BlockOrientation.PosX : BlockOrientation.NegX));
                module.Size = new Vector3(bs * 6, bs, bs * 5);
                break;
                
            case ModuleStyle.Sleek:
                // Thin, graceful wing
                module.Blocks.Add(new VoxelBlock(new Vector3(bs * 2 * xMult, 0, 0), new Vector3(bs * 4, bs * 0.3f, bs * 3), material, BlockType.Hull));
                module.Blocks.Add(new VoxelBlock(new Vector3(bs * 5 * xMult, 0, 0), new Vector3(bs * 2, bs * 0.2f, bs * 2), material, BlockType.Hull));
                module.Size = new Vector3(bs * 7, bs * 0.5f, bs * 3);
                break;
                
            case ModuleStyle.Industrial:
                // Stubby, functional wing
                module.Blocks.Add(new VoxelBlock(new Vector3(bs * 1.5f * xMult, 0, 0), new Vector3(bs * 2, bs * 0.75f, bs * 3), material, BlockType.Hull));
                // Support strut
                module.Blocks.Add(new VoxelBlock(new Vector3(bs * 0.75f * xMult, -bs * 0.25f, 0), new Vector3(bs * 0.5f, bs * 0.5f, bs * 2), material, BlockType.Hull));
                module.Size = new Vector3(bs * 3, bs, bs * 3);
                break;
                
            default:
                // Simple rectangular wing
                module.Blocks.Add(new VoxelBlock(new Vector3(bs * 2 * xMult, 0, 0), new Vector3(bs * 3, bs * 0.5f, bs * 2), material, BlockType.Hull));
                module.Size = new Vector3(bs * 4, bs * 0.5f, bs * 2);
                break;
        }
        
        // Attachment point
        module.AttachmentPoints.Add(new AttachmentPoint
        {
            Id = "base",
            Position = new Vector3(bs * 0.5f * xMult, 0, 0),
            Normal = new Vector3(-xMult, 0, 0),
            CompatibleTypes = new List<ModuleType> { ModuleType.CoreHull },
            SizeClass = 1
        });
    }
    
    private static void CreateCargoBlocks(ShipModule module, ModuleStyle style, int sizeClass, string material)
    {
        float bs = 2f;
        float sc = sizeClass;
        
        float width = bs * 2 * sc;
        float height = bs * 2 * sc;
        float length = bs * 3 * sc;
        
        // Cargo container
        module.Blocks.Add(new VoxelBlock(new Vector3(0, 0, 0), new Vector3(width, height, length), material, BlockType.Cargo));
        
        if (style == ModuleStyle.Industrial)
        {
            // Frame rails
            module.Blocks.Add(new VoxelBlock(new Vector3(width / 2, height / 2, 0), new Vector3(bs * 0.25f, bs * 0.25f, length), material, BlockType.Hull));
            module.Blocks.Add(new VoxelBlock(new Vector3(-width / 2, height / 2, 0), new Vector3(bs * 0.25f, bs * 0.25f, length), material, BlockType.Hull));
        }
        
        module.Size = new Vector3(width + bs * 0.5f, height + bs * 0.5f, length);
        
        // Attachment point
        module.AttachmentPoints.Add(new AttachmentPoint
        {
            Id = "base",
            Position = new Vector3(0, -height / 2 - bs * 0.25f, 0),
            Normal = new Vector3(0, -1, 0),
            CompatibleTypes = new List<ModuleType> { ModuleType.CoreHull },
            SizeClass = sizeClass
        });
    }
    
    private static void CreateWeaponMountBlocks(ShipModule module, ModuleStyle style, string material)
    {
        float bs = 2f;
        
        // Turret base
        module.Blocks.Add(new VoxelBlock(new Vector3(0, 0, 0), new Vector3(bs * 1.5f, bs, bs * 1.5f), material, BlockType.TurretMount));
        
        // Barrel
        module.Blocks.Add(new VoxelBlock(new Vector3(0, bs * 0.25f, bs), new Vector3(bs * 0.5f, bs * 0.5f, bs * 2), material, BlockType.Hull));
        
        module.Size = new Vector3(bs * 2, bs * 1.5f, bs * 3);
        
        module.AttachmentPoints.Add(new AttachmentPoint
        {
            Id = "base",
            Position = new Vector3(0, -bs * 0.5f, 0),
            Normal = new Vector3(0, -1, 0),
            CompatibleTypes = new List<ModuleType> { ModuleType.CoreHull, ModuleType.Wing },
            SizeClass = 1
        });
    }
    
    private static void CreateMidSectionBlocks(ShipModule module, ModuleStyle style, int length, string material)
    {
        float bs = 2f;
        float l = bs * 2 * length;
        
        float width = bs * 2;
        float height = bs * 1.5f;
        
        // Connecting section
        module.Blocks.Add(new VoxelBlock(new Vector3(0, 0, 0), new Vector3(width, height, l), material, BlockType.Hull));
        
        if (style == ModuleStyle.Industrial)
        {
            // Exposed pipes/cables
            module.Blocks.Add(new VoxelBlock(new Vector3(width / 2, 0, 0), new Vector3(bs * 0.25f, bs * 0.25f, l), material, BlockType.Hull));
            module.Blocks.Add(new VoxelBlock(new Vector3(-width / 2, 0, 0), new Vector3(bs * 0.25f, bs * 0.25f, l), material, BlockType.Hull));
        }
        
        module.Size = new Vector3(width + bs * 0.5f, height, l);
        
        module.AttachmentPoints.Add(new AttachmentPoint
        {
            Id = "front",
            Position = new Vector3(0, 0, l / 2 + bs * 0.5f),
            Normal = new Vector3(0, 0, 1),
            CompatibleTypes = new List<ModuleType> { ModuleType.Cockpit, ModuleType.CoreHull, ModuleType.MidSection },
            SizeClass = 1
        });
        module.AttachmentPoints.Add(new AttachmentPoint
        {
            Id = "rear",
            Position = new Vector3(0, 0, -l / 2 - bs * 0.5f),
            Normal = new Vector3(0, 0, -1),
            CompatibleTypes = new List<ModuleType> { ModuleType.CoreHull, ModuleType.EngineNacelle, ModuleType.MidSection },
            SizeClass = 1
        });
    }
    
    private static void CreateSensorArrayBlocks(ShipModule module, ModuleStyle style, string material)
    {
        float bs = 2f;
        
        // Base mount
        module.Blocks.Add(new VoxelBlock(new Vector3(0, 0, 0), new Vector3(bs, bs * 0.5f, bs), material, BlockType.Hull));
        
        // Sensor dish
        module.Blocks.Add(new VoxelBlock(new Vector3(0, bs * 0.5f, 0), new Vector3(bs * 1.5f, bs * 0.25f, bs * 1.5f), "Crystal", BlockType.Hull));
        
        // Antenna spike
        module.Blocks.Add(new VoxelBlock(new Vector3(0, bs, 0), new Vector3(bs * 0.1f, bs, bs * 0.1f), material, BlockType.Hull));
        
        module.Size = new Vector3(bs * 2, bs * 2, bs * 2);
        
        module.AttachmentPoints.Add(new AttachmentPoint
        {
            Id = "base",
            Position = new Vector3(0, -bs * 0.25f, 0),
            Normal = new Vector3(0, -1, 0),
            CompatibleTypes = new List<ModuleType> { ModuleType.CoreHull },
            SizeClass = 1
        });
    }
}
