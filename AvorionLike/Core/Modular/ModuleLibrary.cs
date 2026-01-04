using System.Numerics;
using System.Text.Json;
using AvorionLike.Core.Logging;

namespace AvorionLike.Core.Modular;

/// <summary>
/// Library/registry of all available ship module definitions
/// This is like a catalog of parts that can be used to build ships
/// </summary>
public class ModuleLibrary
{
    private readonly Dictionary<string, ShipModuleDefinition> _definitions = new();
    private readonly Logger _logger = Logger.Instance;
    
    public IReadOnlyCollection<ShipModuleDefinition> AllDefinitions => _definitions.Values;
    
    /// <summary>
    /// Initialize with built-in module definitions
    /// These are basic placeholder definitions that will be replaced with actual 3D models
    /// </summary>
    public void InitializeBuiltInModules()
    {
        _logger.Info("ModuleLibrary", "Initializing built-in ship modules");
        
        // Core hull modules
        AddDefinition(CreateCockpitModule());
        AddDefinition(CreateHullSectionModule());
        AddDefinition(CreateHullCornerModule());
        
        // Engine modules
        AddDefinition(CreateMainEngineModule());
        AddDefinition(CreateEngineNacelleModule());
        AddDefinition(CreateThrusterModule());
        
        // Wing modules
        AddDefinition(CreateWingModule());
        AddDefinition(CreateStabilizerModule());
        
        // Weapon modules
        AddDefinition(CreateWeaponMountModule());
        AddDefinition(CreateTurretModule());
        
        // Utility modules
        AddDefinition(CreatePowerCoreModule());
        AddDefinition(CreateShieldGeneratorModule());
        AddDefinition(CreateCargoModule());
        AddDefinition(CreateCrewQuartersModule());
        AddDefinition(CreateHyperdriveModule());
        AddDefinition(CreateSensorModule());
        AddDefinition(CreateMiningModule());
        
        // Decorative modules
        AddDefinition(CreateAntennaModule());
        
        _logger.Info("ModuleLibrary", $"Loaded {_definitions.Count} built-in modules");
    }
    
    /// <summary>
    /// Add a module definition to the library
    /// </summary>
    public void AddDefinition(ShipModuleDefinition definition)
    {
        if (string.IsNullOrEmpty(definition.Id))
        {
            _logger.Warning("ModuleLibrary", "Attempted to add module with empty ID");
            return;
        }
        
        _definitions[definition.Id] = definition;
    }
    
    /// <summary>
    /// Get a module definition by ID
    /// </summary>
    public ShipModuleDefinition? GetDefinition(string id)
    {
        return _definitions.TryGetValue(id, out var definition) ? definition : null;
    }
    
    /// <summary>
    /// Get all definitions of a specific category
    /// </summary>
    public List<ShipModuleDefinition> GetDefinitionsByCategory(ModuleCategory category)
    {
        return _definitions.Values.Where(d => d.Category == category).ToList();
    }
    
    /// <summary>
    /// Get all definitions with a specific tag
    /// </summary>
    public List<ShipModuleDefinition> GetDefinitionsByTag(string tag)
    {
        return _definitions.Values.Where(d => d.Tags.Contains(tag)).ToList();
    }
    
    /// <summary>
    /// Load module definitions from a JSON file
    /// </summary>
    public bool LoadFromFile(string filePath)
    {
        try
        {
            if (!File.Exists(filePath))
            {
                _logger.Warning("ModuleLibrary", $"Module definition file not found: {filePath}");
                return false;
            }
            
            var json = File.ReadAllText(filePath);
            var definitions = JsonSerializer.Deserialize<List<ShipModuleDefinition>>(json);
            
            if (definitions != null)
            {
                foreach (var def in definitions)
                {
                    AddDefinition(def);
                }
                _logger.Info("ModuleLibrary", $"Loaded {definitions.Count} modules from {filePath}");
                return true;
            }
        }
        catch (Exception ex)
        {
            _logger.Error("ModuleLibrary", $"Failed to load modules from {filePath}: {ex.Message}");
        }
        
        return false;
    }
    
    /// <summary>
    /// Save module definitions to a JSON file
    /// </summary>
    public bool SaveToFile(string filePath)
    {
        try
        {
            var json = JsonSerializer.Serialize(_definitions.Values, new JsonSerializerOptions 
            { 
                WriteIndented = true 
            });
            File.WriteAllText(filePath, json);
            _logger.Info("ModuleLibrary", $"Saved {_definitions.Count} modules to {filePath}");
            return true;
        }
        catch (Exception ex)
        {
            _logger.Error("ModuleLibrary", $"Failed to save modules to {filePath}: {ex.Message}");
            return false;
        }
    }
    
    // Factory methods for built-in modules
    
    private ShipModuleDefinition CreateCockpitModule()
    {
        var module = new ShipModuleDefinition
        {
            Id = "cockpit_basic",
            Name = "Basic Cockpit",
            Description = "Standard ship cockpit module",
            Category = ModuleCategory.Hull,
            SubCategory = "Cockpit",
            Size = new Vector3(3, 2, 4),
            BaseMass = 15f,
            BaseHealth = 150f,
            BaseCost = 500,
            TechLevel = 1,
            Tags = new List<string> { "core", "cockpit", "essential" }
        };
        
        // Attachment points
        module.AttachmentPoints["rear"] = new AttachmentPoint
        {
            Name = "rear",
            Position = new Vector3(0, 0, -2),
            Direction = new Vector3(0, 0, -1),
            Size = AttachmentSize.Medium
        };
        
        module.BaseStats.CrewCapacity = 2;
        module.BaseStats.CrewRequired = 1;
        module.BaseStats.SensorRange = 1000f;
        
        return module;
    }
    
    private ShipModuleDefinition CreateHullSectionModule()
    {
        var module = new ShipModuleDefinition
        {
            Id = "hull_section_basic",
            Name = "Hull Section",
            Description = "Standard hull section for connecting modules",
            Category = ModuleCategory.Hull,
            SubCategory = "Section",
            Size = new Vector3(3, 3, 4),
            BaseMass = 20f,
            BaseHealth = 200f,
            BaseCost = 200,
            TechLevel = 1,
            Tags = new List<string> { "hull", "structural" }
        };
        
        // Attachment points on all sides
        module.AttachmentPoints["front"] = new AttachmentPoint
        {
            Name = "front",
            Position = new Vector3(0, 0, 2),
            Direction = new Vector3(0, 0, 1),
            Size = AttachmentSize.Medium
        };
        module.AttachmentPoints["rear"] = new AttachmentPoint
        {
            Name = "rear",
            Position = new Vector3(0, 0, -2),
            Direction = new Vector3(0, 0, -1),
            Size = AttachmentSize.Medium
        };
        module.AttachmentPoints["left"] = new AttachmentPoint
        {
            Name = "left",
            Position = new Vector3(-1.5f, 0, 0),
            Direction = new Vector3(-1, 0, 0),
            Size = AttachmentSize.Small
        };
        module.AttachmentPoints["right"] = new AttachmentPoint
        {
            Name = "right",
            Position = new Vector3(1.5f, 0, 0),
            Direction = new Vector3(1, 0, 0),
            Size = AttachmentSize.Small
        };
        
        return module;
    }
    
    private ShipModuleDefinition CreateHullCornerModule()
    {
        var module = new ShipModuleDefinition
        {
            Id = "hull_corner_basic",
            Name = "Hull Corner",
            Description = "Corner hull section for angled connections",
            Category = ModuleCategory.Hull,
            SubCategory = "Corner",
            Size = new Vector3(3, 3, 3),
            BaseMass = 15f,
            BaseHealth = 150f,
            BaseCost = 150,
            TechLevel = 1,
            Tags = new List<string> { "hull", "structural", "corner" }
        };
        
        module.AttachmentPoints["side1"] = new AttachmentPoint
        {
            Name = "side1",
            Position = new Vector3(-1.5f, 0, 0),
            Direction = new Vector3(-1, 0, 0),
            Size = AttachmentSize.Medium
        };
        module.AttachmentPoints["side2"] = new AttachmentPoint
        {
            Name = "side2",
            Position = new Vector3(0, 0, 1.5f),
            Direction = new Vector3(0, 0, 1),
            Size = AttachmentSize.Medium
        };
        
        return module;
    }
    
    private ShipModuleDefinition CreateMainEngineModule()
    {
        var module = new ShipModuleDefinition
        {
            Id = "engine_main",
            Name = "Main Engine",
            Description = "Primary propulsion engine",
            Category = ModuleCategory.Engine,
            SubCategory = "Main",
            Size = new Vector3(2, 2, 3),
            BaseMass = 25f,
            BaseHealth = 100f,
            BaseCost = 800,
            TechLevel = 1,
            Tags = new List<string> { "engine", "propulsion" }
        };
        
        module.AttachmentPoints["mount"] = new AttachmentPoint
        {
            Name = "mount",
            Position = new Vector3(0, 0, 1.5f),
            Direction = new Vector3(0, 0, 1),
            Size = AttachmentSize.Medium
        };
        
        module.BaseStats.ThrustPower = 1000f;
        module.BaseStats.MaxSpeed = 100f;
        module.BaseStats.PowerConsumption = 50f;
        
        return module;
    }
    
    private ShipModuleDefinition CreateEngineNacelleModule()
    {
        var module = new ShipModuleDefinition
        {
            Id = "engine_nacelle",
            Name = "Engine Nacelle",
            Description = "Secondary engine nacelle for additional thrust",
            Category = ModuleCategory.Engine,
            SubCategory = "Nacelle",
            Size = new Vector3(1.5f, 1.5f, 4),
            BaseMass = 18f,
            BaseHealth = 80f,
            BaseCost = 600,
            TechLevel = 1,
            Tags = new List<string> { "engine", "nacelle" }
        };
        
        module.AttachmentPoints["mount"] = new AttachmentPoint
        {
            Name = "mount",
            Position = new Vector3(0, 0, 2),
            Direction = new Vector3(0, 0, 1),
            Size = AttachmentSize.Small
        };
        
        module.BaseStats.ThrustPower = 600f;
        module.BaseStats.MaxSpeed = 80f;
        module.BaseStats.PowerConsumption = 35f;
        
        return module;
    }
    
    private ShipModuleDefinition CreateThrusterModule()
    {
        var module = new ShipModuleDefinition
        {
            Id = "thruster_maneuver",
            Name = "Maneuvering Thruster",
            Description = "Small thruster for maneuvering",
            Category = ModuleCategory.Thruster,
            SubCategory = "Maneuver",
            Size = new Vector3(1, 1, 1.5f),
            BaseMass = 8f,
            BaseHealth = 50f,
            BaseCost = 300,
            TechLevel = 1,
            Tags = new List<string> { "thruster", "maneuver" }
        };
        
        module.AttachmentPoints["mount"] = new AttachmentPoint
        {
            Name = "mount",
            Position = new Vector3(0, 0, 0.75f),
            Direction = new Vector3(0, 0, 1),
            Size = AttachmentSize.Small
        };
        
        module.BaseStats.ThrustPower = 300f;
        module.BaseStats.PowerConsumption = 20f;
        
        return module;
    }
    
    private ShipModuleDefinition CreateWingModule()
    {
        var module = new ShipModuleDefinition
        {
            Id = "wing_basic",
            Name = "Wing Section",
            Description = "Wing section for aerodynamics and mounting points",
            Category = ModuleCategory.Wing,
            SubCategory = "Standard",
            Size = new Vector3(6, 0.5f, 3),
            BaseMass = 12f,
            BaseHealth = 100f,
            BaseCost = 250,
            TechLevel = 1,
            Tags = new List<string> { "wing", "structural" }
        };
        
        module.AttachmentPoints["mount"] = new AttachmentPoint
        {
            Name = "mount",
            Position = new Vector3(0, 0, 0),
            Direction = new Vector3(0, 1, 0),
            Size = AttachmentSize.Small
        };
        module.AttachmentPoints["tip"] = new AttachmentPoint
        {
            Name = "tip",
            Position = new Vector3(3, 0, 0),
            Direction = new Vector3(1, 0, 0),
            Size = AttachmentSize.Small
        };
        
        return module;
    }
    
    private ShipModuleDefinition CreateStabilizerModule()
    {
        var module = new ShipModuleDefinition
        {
            Id = "stabilizer_basic",
            Name = "Stabilizer Fin",
            Description = "Tail stabilizer fin",
            Category = ModuleCategory.Tail,
            SubCategory = "Stabilizer",
            Size = new Vector3(0.5f, 3, 2),
            BaseMass = 8f,
            BaseHealth = 75f,
            BaseCost = 200,
            TechLevel = 1,
            Tags = new List<string> { "tail", "stabilizer" }
        };
        
        module.AttachmentPoints["mount"] = new AttachmentPoint
        {
            Name = "mount",
            Position = new Vector3(0, 0, 0),
            Direction = new Vector3(0, -1, 0),
            Size = AttachmentSize.Small
        };
        
        return module;
    }
    
    private ShipModuleDefinition CreateWeaponMountModule()
    {
        var module = new ShipModuleDefinition
        {
            Id = "weapon_mount_basic",
            Name = "Weapon Mount",
            Description = "Mounting point for weapons",
            Category = ModuleCategory.WeaponMount,
            SubCategory = "Hardpoint",
            Size = new Vector3(1, 1, 1),
            BaseMass = 5f,
            BaseHealth = 50f,
            BaseCost = 400,
            TechLevel = 1,
            Tags = new List<string> { "weapon", "mount" }
        };
        
        module.AttachmentPoints["mount"] = new AttachmentPoint
        {
            Name = "mount",
            Position = new Vector3(0, 0, 0),
            Direction = new Vector3(0, 1, 0),
            Size = AttachmentSize.Small
        };
        
        module.BaseStats.WeaponMountPoints = 1;
        
        return module;
    }
    
    private ShipModuleDefinition CreateTurretModule()
    {
        var module = new ShipModuleDefinition
        {
            Id = "turret_basic",
            Name = "Basic Turret",
            Description = "Basic weapon turret",
            Category = ModuleCategory.Weapon,
            SubCategory = "Turret",
            Size = new Vector3(1.5f, 1.5f, 1.5f),
            BaseMass = 10f,
            BaseHealth = 75f,
            BaseCost = 1000,
            TechLevel = 1,
            Tags = new List<string> { "weapon", "turret" }
        };
        
        module.AttachmentPoints["mount"] = new AttachmentPoint
        {
            Name = "mount",
            Position = new Vector3(0, -0.75f, 0),
            Direction = new Vector3(0, -1, 0),
            Size = AttachmentSize.Small,
            AllowedCategories = new List<ModuleCategory> { ModuleCategory.WeaponMount }
        };
        
        module.BaseStats.WeaponDamage = 50f;
        module.BaseStats.WeaponRange = 1000f;
        module.BaseStats.PowerConsumption = 30f;
        
        return module;
    }
    
    private ShipModuleDefinition CreatePowerCoreModule()
    {
        var module = new ShipModuleDefinition
        {
            Id = "power_core_basic",
            Name = "Power Core",
            Description = "Main power generator",
            Category = ModuleCategory.PowerCore,
            SubCategory = "Generator",
            Size = new Vector3(2, 2, 2),
            BaseMass = 20f,
            BaseHealth = 100f,
            BaseCost = 1500,
            TechLevel = 1,
            Tags = new List<string> { "power", "generator" }
        };
        
        module.AttachmentPoints["mount"] = new AttachmentPoint
        {
            Name = "mount",
            Position = new Vector3(0, 0, 0),
            Direction = new Vector3(0, 1, 0),
            Size = AttachmentSize.Medium
        };
        
        module.BaseStats.PowerGeneration = 500f;
        module.BaseStats.PowerStorage = 1000f;
        
        return module;
    }
    
    private ShipModuleDefinition CreateShieldGeneratorModule()
    {
        var module = new ShipModuleDefinition
        {
            Id = "shield_gen_basic",
            Name = "Shield Generator",
            Description = "Energy shield generator",
            Category = ModuleCategory.Shield,
            SubCategory = "Generator",
            Size = new Vector3(2, 2, 2),
            BaseMass = 18f,
            BaseHealth = 80f,
            BaseCost = 2000,
            TechLevel = 2,
            Tags = new List<string> { "shield", "defense" }
        };
        
        module.AttachmentPoints["mount"] = new AttachmentPoint
        {
            Name = "mount",
            Position = new Vector3(0, 0, 0),
            Direction = new Vector3(0, 1, 0),
            Size = AttachmentSize.Medium
        };
        
        module.BaseStats.ShieldCapacity = 1000f;
        module.BaseStats.ShieldRechargeRate = 50f;
        module.BaseStats.PowerConsumption = 80f;
        
        return module;
    }
    
    private ShipModuleDefinition CreateCargoModule()
    {
        var module = new ShipModuleDefinition
        {
            Id = "cargo_bay_basic",
            Name = "Cargo Bay",
            Description = "Storage bay for cargo",
            Category = ModuleCategory.Cargo,
            SubCategory = "Bay",
            Size = new Vector3(4, 3, 4),
            BaseMass = 15f,
            BaseHealth = 100f,
            BaseCost = 800,
            TechLevel = 1,
            Tags = new List<string> { "cargo", "storage" }
        };
        
        module.AttachmentPoints["mount"] = new AttachmentPoint
        {
            Name = "mount",
            Position = new Vector3(0, 0, 0),
            Direction = new Vector3(0, 1, 0),
            Size = AttachmentSize.Medium
        };
        
        module.BaseStats.CargoCapacity = 500f;
        
        return module;
    }
    
    private ShipModuleDefinition CreateCrewQuartersModule()
    {
        var module = new ShipModuleDefinition
        {
            Id = "crew_quarters_basic",
            Name = "Crew Quarters",
            Description = "Living quarters for crew",
            Category = ModuleCategory.CrewQuarters,
            SubCategory = "Quarters",
            Size = new Vector3(3, 2, 3),
            BaseMass = 12f,
            BaseHealth = 80f,
            BaseCost = 600,
            TechLevel = 1,
            Tags = new List<string> { "crew", "quarters" }
        };
        
        module.AttachmentPoints["mount"] = new AttachmentPoint
        {
            Name = "mount",
            Position = new Vector3(0, 0, 0),
            Direction = new Vector3(0, 1, 0),
            Size = AttachmentSize.Medium
        };
        
        module.BaseStats.CrewCapacity = 10;
        
        return module;
    }
    
    private ShipModuleDefinition CreateHyperdriveModule()
    {
        var module = new ShipModuleDefinition
        {
            Id = "hyperdrive_basic",
            Name = "Hyperdrive Core",
            Description = "FTL hyperdrive system",
            Category = ModuleCategory.Hyperdrive,
            SubCategory = "Core",
            Size = new Vector3(2.5f, 2.5f, 3),
            BaseMass = 30f,
            BaseHealth = 120f,
            BaseCost = 5000,
            TechLevel = 3,
            Tags = new List<string> { "hyperdrive", "ftl" }
        };
        
        module.AttachmentPoints["mount"] = new AttachmentPoint
        {
            Name = "mount",
            Position = new Vector3(0, 0, 0),
            Direction = new Vector3(0, 1, 0),
            Size = AttachmentSize.Large
        };
        
        module.BaseStats.HasHyperdrive = true;
        module.BaseStats.HyperdriveRange = 5000f;
        module.BaseStats.PowerConsumption = 200f;
        module.BaseStats.CrewRequired = 2;
        
        return module;
    }
    
    private ShipModuleDefinition CreateSensorModule()
    {
        var module = new ShipModuleDefinition
        {
            Id = "sensor_array_basic",
            Name = "Sensor Array",
            Description = "Long-range sensor system",
            Category = ModuleCategory.Sensor,
            SubCategory = "Array",
            Size = new Vector3(1.5f, 2, 1.5f),
            BaseMass = 10f,
            BaseHealth = 60f,
            BaseCost = 1200,
            TechLevel = 2,
            Tags = new List<string> { "sensor", "radar" }
        };
        
        module.AttachmentPoints["mount"] = new AttachmentPoint
        {
            Name = "mount",
            Position = new Vector3(0, -1, 0),
            Direction = new Vector3(0, -1, 0),
            Size = AttachmentSize.Small
        };
        
        module.BaseStats.SensorRange = 5000f;
        module.BaseStats.PowerConsumption = 25f;
        
        return module;
    }
    
    private ShipModuleDefinition CreateMiningModule()
    {
        var module = new ShipModuleDefinition
        {
            Id = "mining_laser_basic",
            Name = "Mining Laser",
            Description = "Mining laser for resource extraction",
            Category = ModuleCategory.Mining,
            SubCategory = "Laser",
            Size = new Vector3(1, 1, 2),
            BaseMass = 15f,
            BaseHealth = 70f,
            BaseCost = 1500,
            TechLevel = 2,
            Tags = new List<string> { "mining", "laser" }
        };
        
        module.AttachmentPoints["mount"] = new AttachmentPoint
        {
            Name = "mount",
            Position = new Vector3(0, 0, 1),
            Direction = new Vector3(0, 0, 1),
            Size = AttachmentSize.Small
        };
        
        module.BaseStats.MiningPower = 100f;
        module.BaseStats.PowerConsumption = 50f;
        
        return module;
    }
    
    private ShipModuleDefinition CreateAntennaModule()
    {
        var module = new ShipModuleDefinition
        {
            Id = "antenna_basic",
            Name = "Communication Antenna",
            Description = "Decorative antenna for communication",
            Category = ModuleCategory.Antenna,
            SubCategory = "Decorative",
            Size = new Vector3(0.5f, 3, 0.5f),
            BaseMass = 3f,
            BaseHealth = 30f,
            BaseCost = 100,
            TechLevel = 1,
            Tags = new List<string> { "decorative", "antenna" }
        };
        
        module.AttachmentPoints["mount"] = new AttachmentPoint
        {
            Name = "mount",
            Position = new Vector3(0, -1.5f, 0),
            Direction = new Vector3(0, -1, 0),
            Size = AttachmentSize.Small
        };
        
        return module;
    }
}
