using System.Text.Json;
using System.Text.Json.Serialization;

namespace AvorionLike.Core.Voxel;

/// <summary>
/// Comprehensive definition of a block type with all properties needed for AI generation
/// This provides a data-driven approach to block properties for procedural content generation
/// </summary>
public class BlockDefinition
{
    [JsonPropertyName("id")]
    public string Id { get; set; } = "";
    
    [JsonPropertyName("displayName")]
    public string DisplayName { get; set; } = "";
    
    [JsonPropertyName("blockType")]
    public BlockType BlockType { get; set; }
    
    [JsonPropertyName("description")]
    public string Description { get; set; } = "";
    
    /// <summary>
    /// Resource costs for building this block (per unit volume)
    /// </summary>
    [JsonPropertyName("resourceCosts")]
    public Dictionary<string, int> ResourceCosts { get; set; } = new();
    
    /// <summary>
    /// Base hit points per unit volume
    /// </summary>
    [JsonPropertyName("hitPointsPerVolume")]
    public float HitPointsPerVolume { get; set; }
    
    /// <summary>
    /// Mass per unit volume (kg/cubic meter)
    /// </summary>
    [JsonPropertyName("massPerUnitVolume")]
    public float MassPerUnitVolume { get; set; }
    
    /// <summary>
    /// Whether this block can be scaled dynamically
    /// </summary>
    [JsonPropertyName("scalable")]
    public bool Scalable { get; set; } = true;
    
    /// <summary>
    /// Function this block provides
    /// </summary>
    [JsonPropertyName("function")]
    public string Function { get; set; } = "";
    
    /// <summary>
    /// Power generation per unit volume (if applicable)
    /// </summary>
    [JsonPropertyName("powerGenerationPerVolume")]
    public float PowerGenerationPerVolume { get; set; }
    
    /// <summary>
    /// Power consumption per unit volume (if applicable)
    /// </summary>
    [JsonPropertyName("powerConsumptionPerVolume")]
    public float PowerConsumptionPerVolume { get; set; }
    
    /// <summary>
    /// Thrust power per unit volume (for engines/thrusters)
    /// </summary>
    [JsonPropertyName("thrustPowerPerVolume")]
    public float ThrustPowerPerVolume { get; set; }
    
    /// <summary>
    /// Shield capacity per unit volume (for shield generators)
    /// </summary>
    [JsonPropertyName("shieldCapacityPerVolume")]
    public float ShieldCapacityPerVolume { get; set; }
    
    /// <summary>
    /// Cargo capacity per unit volume (for cargo blocks)
    /// </summary>
    [JsonPropertyName("cargoCapacityPerVolume")]
    public float CargoCapacityPerVolume { get; set; }
    
    /// <summary>
    /// Crew capacity (for crew quarters)
    /// </summary>
    [JsonPropertyName("crewCapacityPerVolume")]
    public float CrewCapacityPerVolume { get; set; }
    
    /// <summary>
    /// Priority for AI placement (higher = more important)
    /// </summary>
    [JsonPropertyName("aiPlacementPriority")]
    public int AiPlacementPriority { get; set; }
    
    /// <summary>
    /// Whether this block should be placed internally (protected by armor)
    /// </summary>
    [JsonPropertyName("requiresInternalPlacement")]
    public bool RequiresInternalPlacement { get; set; }
    
    /// <summary>
    /// Whether this block is suitable for external hull
    /// </summary>
    [JsonPropertyName("suitableForExterior")]
    public bool SuitableForExterior { get; set; }
    
    /// <summary>
    /// Minimum tech level required to build this block
    /// </summary>
    [JsonPropertyName("minTechLevel")]
    public int MinTechLevel { get; set; } = 1;
    
    /// <summary>
    /// Default color for this block type (RGB as hex string)
    /// </summary>
    [JsonPropertyName("defaultColor")]
    public string DefaultColor { get; set; } = "#808080";
}

/// <summary>
/// Database/registry of all available block definitions
/// Provides data-driven block properties for AI generation
/// </summary>
public static class BlockDefinitionDatabase
{
    private static Dictionary<BlockType, BlockDefinition>? _definitions;
    
    /// <summary>
    /// Get all block definitions
    /// </summary>
    public static Dictionary<BlockType, BlockDefinition> GetDefinitions()
    {
        if (_definitions == null)
        {
            InitializeDefinitions();
        }
        return _definitions!;
    }
    
    /// <summary>
    /// Get a specific block definition
    /// </summary>
    public static BlockDefinition GetDefinition(BlockType blockType)
    {
        var defs = GetDefinitions();
        return defs.GetValueOrDefault(blockType, defs[BlockType.Hull]);
    }
    
    /// <summary>
    /// Initialize all block definitions with comprehensive properties
    /// </summary>
    private static void InitializeDefinitions()
    {
        _definitions = new Dictionary<BlockType, BlockDefinition>
        {
            [BlockType.Hull] = new BlockDefinition
            {
                Id = "hull_basic",
                DisplayName = "Hull Block",
                BlockType = BlockType.Hull,
                Description = "Basic structural hull block",
                ResourceCosts = new Dictionary<string, int> { ["Iron"] = 10 },
                HitPointsPerVolume = 100f,
                MassPerUnitVolume = 1.0f,
                Scalable = true,
                Function = "structure",
                AiPlacementPriority = 5,
                SuitableForExterior = true,
                RequiresInternalPlacement = false,
                DefaultColor = "#808080"
            },
            
            [BlockType.Armor] = new BlockDefinition
            {
                Id = "armor_plating",
                DisplayName = "Armor Plating",
                BlockType = BlockType.Armor,
                Description = "Heavy armor protection for hull",
                ResourceCosts = new Dictionary<string, int> { ["Iron"] = 15 },
                HitPointsPerVolume = 500f, // 5x more durable than hull
                MassPerUnitVolume = 1.5f,
                Scalable = true,
                Function = "protection",
                AiPlacementPriority = 8,
                SuitableForExterior = true,
                RequiresInternalPlacement = false,
                DefaultColor = "#A0A0A0"
            },
            
            [BlockType.Engine] = new BlockDefinition
            {
                Id = "engine_main",
                DisplayName = "Main Engine",
                BlockType = BlockType.Engine,
                Description = "Primary propulsion engine providing forward thrust",
                ResourceCosts = new Dictionary<string, int> { ["Iron"] = 20, ["Titanium"] = 5 },
                HitPointsPerVolume = 80f,
                MassPerUnitVolume = 1.2f,
                Scalable = true,
                Function = "generateThrust",
                ThrustPowerPerVolume = 50f,
                PowerConsumptionPerVolume = 5f,
                AiPlacementPriority = 10,
                RequiresInternalPlacement = false,
                SuitableForExterior = true,
                DefaultColor = "#FF6600"
            },
            
            [BlockType.Thruster] = new BlockDefinition
            {
                Id = "thruster_maneuvering",
                DisplayName = "Maneuvering Thruster",
                BlockType = BlockType.Thruster,
                Description = "Omnidirectional thrusters for strafing and braking",
                ResourceCosts = new Dictionary<string, int> { ["Iron"] = 18 },
                HitPointsPerVolume = 70f,
                MassPerUnitVolume = 1.0f,
                Scalable = true,
                Function = "applyThrust",
                ThrustPowerPerVolume = 30f,
                PowerConsumptionPerVolume = 3f,
                AiPlacementPriority = 8,
                RequiresInternalPlacement = false,
                SuitableForExterior = true,
                DefaultColor = "#0099FF"
            },
            
            [BlockType.GyroArray] = new BlockDefinition
            {
                Id = "gyro_array",
                DisplayName = "Gyro Array",
                BlockType = BlockType.GyroArray,
                Description = "Gyroscopic stabilizers for rotation control",
                ResourceCosts = new Dictionary<string, int> { ["Iron"] = 15, ["Titanium"] = 3 },
                HitPointsPerVolume = 75f,
                MassPerUnitVolume = 0.9f,
                Scalable = true,
                Function = "applyTorque",
                ThrustPowerPerVolume = 20f, // Torque
                PowerConsumptionPerVolume = 2f,
                AiPlacementPriority = 7,
                RequiresInternalPlacement = true,
                SuitableForExterior = false,
                DefaultColor = "#00FFFF"
            },
            
            [BlockType.Generator] = new BlockDefinition
            {
                Id = "power_generator",
                DisplayName = "Power Generator",
                BlockType = BlockType.Generator,
                Description = "Generates electrical power for ship systems",
                ResourceCosts = new Dictionary<string, int> { ["Iron"] = 25, ["Naonite"] = 5 },
                HitPointsPerVolume = 60f,
                MassPerUnitVolume = 1.3f,
                Scalable = true,
                Function = "generatePower",
                PowerGenerationPerVolume = 100f,
                AiPlacementPriority = 10,
                RequiresInternalPlacement = true,
                SuitableForExterior = false,
                MinTechLevel = 1,
                DefaultColor = "#FFFF00"
            },
            
            [BlockType.ShieldGenerator] = new BlockDefinition
            {
                Id = "shield_generator",
                DisplayName = "Shield Generator",
                BlockType = BlockType.ShieldGenerator,
                Description = "Generates protective energy shields",
                ResourceCosts = new Dictionary<string, int> { ["Titanium"] = 20, ["Naonite"] = 10 },
                HitPointsPerVolume = 50f,
                MassPerUnitVolume = 1.1f,
                Scalable = true,
                Function = "generateShield",
                ShieldCapacityPerVolume = 200f,
                PowerConsumptionPerVolume = 10f,
                AiPlacementPriority = 9,
                RequiresInternalPlacement = true,
                SuitableForExterior = false,
                MinTechLevel = 2,
                DefaultColor = "#00FFAA"
            },
            
            [BlockType.Cargo] = new BlockDefinition
            {
                Id = "cargo_bay",
                DisplayName = "Cargo Bay",
                BlockType = BlockType.Cargo,
                Description = "Storage space for resources and items",
                ResourceCosts = new Dictionary<string, int> { ["Iron"] = 12 },
                HitPointsPerVolume = 80f,
                MassPerUnitVolume = 0.8f,
                Scalable = true,
                Function = "provideCargoSpace",
                CargoCapacityPerVolume = 100f,
                AiPlacementPriority = 7,
                RequiresInternalPlacement = true,
                SuitableForExterior = false,
                DefaultColor = "#8B4513"
            },
            
            [BlockType.CrewQuarters] = new BlockDefinition
            {
                Id = "crew_quarters",
                DisplayName = "Crew Quarters",
                BlockType = BlockType.CrewQuarters,
                Description = "Living space for crew members",
                ResourceCosts = new Dictionary<string, int> { ["Iron"] = 13 },
                HitPointsPerVolume = 70f,
                MassPerUnitVolume = 0.9f,
                Scalable = true,
                Function = "houseCrew",
                CrewCapacityPerVolume = 2f,
                AiPlacementPriority = 6,
                RequiresInternalPlacement = true,
                SuitableForExterior = false,
                DefaultColor = "#4169E1"
            },
            
            [BlockType.TurretMount] = new BlockDefinition
            {
                Id = "turret_mount",
                DisplayName = "Turret Mount",
                BlockType = BlockType.TurretMount,
                Description = "Mounting point for weapon turrets",
                ResourceCosts = new Dictionary<string, int> { ["Iron"] = 20, ["Titanium"] = 5 },
                HitPointsPerVolume = 150f,
                MassPerUnitVolume = 1.4f,
                Scalable = true,
                Function = "mountWeapon",
                PowerConsumptionPerVolume = 8f,
                AiPlacementPriority = 8,
                RequiresInternalPlacement = false,
                SuitableForExterior = true,
                DefaultColor = "#FF0000"
            },
            
            [BlockType.HyperdriveCore] = new BlockDefinition
            {
                Id = "hyperdrive_core",
                DisplayName = "Hyperdrive Core",
                BlockType = BlockType.HyperdriveCore,
                Description = "Enables faster-than-light travel between sectors",
                ResourceCosts = new Dictionary<string, int> { ["Titanium"] = 30, ["Xanion"] = 10 },
                HitPointsPerVolume = 100f,
                MassPerUnitVolume = 1.5f,
                Scalable = true,
                Function = "enableHyperjump",
                PowerConsumptionPerVolume = 50f,
                AiPlacementPriority = 9,
                RequiresInternalPlacement = true,
                SuitableForExterior = false,
                MinTechLevel = 3,
                DefaultColor = "#9370DB"
            },
            
            [BlockType.PodDocking] = new BlockDefinition
            {
                Id = "pod_docking",
                DisplayName = "Pod Docking Port",
                BlockType = BlockType.PodDocking,
                Description = "Docking port for player pods and small craft",
                ResourceCosts = new Dictionary<string, int> { ["Iron"] = 15, ["Titanium"] = 5 },
                HitPointsPerVolume = 120f,
                MassPerUnitVolume = 1.1f,
                Scalable = true,
                Function = "dockPod",
                AiPlacementPriority = 5,
                RequiresInternalPlacement = false,
                SuitableForExterior = true,
                DefaultColor = "#00CED1"
            }
        };
    }
    
    /// <summary>
    /// Export all block definitions to JSON file
    /// </summary>
    public static void ExportToJson(string filePath)
    {
        var definitions = GetDefinitions();
        var options = new JsonSerializerOptions
        {
            WriteIndented = true,
            PropertyNamingPolicy = JsonNamingPolicy.CamelCase
        };
        
        var json = JsonSerializer.Serialize(definitions.Values, options);
        File.WriteAllText(filePath, json);
    }
    
    /// <summary>
    /// Import block definitions from JSON file
    /// </summary>
    public static void ImportFromJson(string filePath)
    {
        if (!File.Exists(filePath))
        {
            throw new FileNotFoundException($"Block definitions file not found: {filePath}");
        }
        
        var json = File.ReadAllText(filePath);
        var options = new JsonSerializerOptions
        {
            PropertyNamingPolicy = JsonNamingPolicy.CamelCase
        };
        
        var definitions = JsonSerializer.Deserialize<List<BlockDefinition>>(json, options);
        if (definitions != null)
        {
            _definitions = new Dictionary<BlockType, BlockDefinition>();
            foreach (var def in definitions)
            {
                _definitions[def.BlockType] = def;
            }
        }
    }
}
