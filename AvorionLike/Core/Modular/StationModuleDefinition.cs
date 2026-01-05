using System.Numerics;

namespace AvorionLike.Core.Modular;

/// <summary>
/// Defines a type of station module (like a prefab or template)
/// Similar to ShipModuleDefinition but for stations
/// </summary>
public class StationModuleDefinition
{
    public string Id { get; set; } = "";
    public string Name { get; set; } = "";
    public string Description { get; set; } = "";
    
    /// <summary>
    /// Category of this module
    /// </summary>
    public StationModuleCategory Category { get; set; } = StationModuleCategory.Hub;
    
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
    public float BaseMass { get; set; } = 100f;
    
    /// <summary>
    /// Base health of the module
    /// </summary>
    public float BaseHealth { get; set; } = 1000f;
    
    /// <summary>
    /// Base cost to build this module
    /// </summary>
    public int BaseCost { get; set; } = 1000;
    
    /// <summary>
    /// Attachment points where other modules can connect
    /// Key = attachment point name, Value = relative position
    /// </summary>
    public Dictionary<string, AttachmentPoint> AttachmentPoints { get; set; } = new();
    
    /// <summary>
    /// Base functional stats for this module type
    /// </summary>
    public StationFunctionalStats BaseStats { get; set; } = new();
    
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
    public StationFunctionalStats GetStatsForMaterial(string materialType)
    {
        var material = MaterialProperties.GetMaterial(materialType);
        var stats = new StationFunctionalStats
        {
            DockingBays = BaseStats.DockingBays,
            TradingCapacity = BaseStats.TradingCapacity,
            ProductionCapacity = BaseStats.ProductionCapacity * material.EnergyEfficiency,
            StorageCapacity = BaseStats.StorageCapacity,
            RepairCapacity = BaseStats.RepairCapacity * material.EnergyEfficiency,
            RefuelCapacity = BaseStats.RefuelCapacity * material.EnergyEfficiency,
            PowerGeneration = BaseStats.PowerGeneration * material.EnergyEfficiency,
            PowerConsumption = BaseStats.PowerConsumption,
            DefenseRating = BaseStats.DefenseRating * material.DurabilityMultiplier,
            ResearchPoints = BaseStats.ResearchPoints,
            CrewCapacity = BaseStats.CrewCapacity,
            ShieldCapacity = BaseStats.ShieldCapacity * material.ShieldMultiplier,
            ShieldRechargeRate = BaseStats.ShieldRechargeRate * material.ShieldMultiplier
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
/// Library of station module definitions
/// Similar to ModuleLibrary but for stations
/// </summary>
public class StationModuleLibrary
{
    private readonly Dictionary<string, StationModuleDefinition> _definitions = new();
    
    public void AddDefinition(StationModuleDefinition definition)
    {
        _definitions[definition.Id] = definition;
    }
    
    public StationModuleDefinition? GetDefinition(string id)
    {
        _definitions.TryGetValue(id, out var definition);
        return definition;
    }
    
    public List<StationModuleDefinition> GetAllDefinitions()
    {
        return _definitions.Values.ToList();
    }
    
    public List<StationModuleDefinition> GetDefinitionsByCategory(StationModuleCategory category)
    {
        return _definitions.Values.Where(d => d.Category == category).ToList();
    }
    
    /// <summary>
    /// Initialize built-in station module definitions
    /// </summary>
    public void InitializeBuiltInModules()
    {
        // Hub/Command Modules
        AddDefinition(new StationModuleDefinition
        {
            Id = "station_hub_basic",
            Name = "Basic Command Hub",
            Description = "Central command module for station control",
            Category = StationModuleCategory.Hub,
            SubCategory = "Command",
            Size = new Vector3(10, 10, 10),
            BaseMass = 500f,
            BaseHealth = 5000f,
            BaseCost = 10000,
            TechLevel = 1,
            BaseStats = new StationFunctionalStats
            {
                PowerGeneration = 100f,
                PowerConsumption = 20f,
                CrewCapacity = 50
            },
            AttachmentPoints = CreateCubicAttachmentPoints(10, "hub")
        });
        
        AddDefinition(new StationModuleDefinition
        {
            Id = "station_hub_advanced",
            Name = "Advanced Command Center",
            Description = "Enhanced command module with improved systems",
            Category = StationModuleCategory.Hub,
            SubCategory = "Command",
            Size = new Vector3(15, 15, 15),
            BaseMass = 1000f,
            BaseHealth = 10000f,
            BaseCost = 25000,
            TechLevel = 3,
            BaseStats = new StationFunctionalStats
            {
                PowerGeneration = 250f,
                PowerConsumption = 40f,
                CrewCapacity = 100,
                ResearchPoints = 10
            },
            AttachmentPoints = CreateCubicAttachmentPoints(15, "hub")
        });
        
        // Docking Modules
        AddDefinition(new StationModuleDefinition
        {
            Id = "station_docking_small",
            Name = "Small Docking Bay",
            Description = "Basic docking facility for small ships",
            Category = StationModuleCategory.Docking,
            SubCategory = "Hangar",
            Size = new Vector3(15, 8, 20),
            BaseMass = 300f,
            BaseHealth = 3000f,
            BaseCost = 5000,
            TechLevel = 1,
            BaseStats = new StationFunctionalStats
            {
                DockingBays = 2,
                PowerConsumption = 10f,
                RepairCapacity = 50f,
                RefuelCapacity = 100f
            },
            AttachmentPoints = CreateDockingAttachmentPoints()
        });
        
        AddDefinition(new StationModuleDefinition
        {
            Id = "station_docking_large",
            Name = "Large Hangar Bay",
            Description = "Spacious hangar for large ships",
            Category = StationModuleCategory.Docking,
            SubCategory = "Hangar",
            Size = new Vector3(30, 15, 40),
            BaseMass = 800f,
            BaseHealth = 8000f,
            BaseCost = 15000,
            TechLevel = 2,
            BaseStats = new StationFunctionalStats
            {
                DockingBays = 5,
                PowerConsumption = 25f,
                RepairCapacity = 150f,
                RefuelCapacity = 300f
            },
            AttachmentPoints = CreateDockingAttachmentPoints()
        });
        
        // Production Modules
        AddDefinition(new StationModuleDefinition
        {
            Id = "station_factory_basic",
            Name = "Basic Factory Module",
            Description = "Simple production facility",
            Category = StationModuleCategory.Production,
            SubCategory = "Factory",
            Size = new Vector3(12, 10, 12),
            BaseMass = 400f,
            BaseHealth = 4000f,
            BaseCost = 8000,
            TechLevel = 1,
            BaseStats = new StationFunctionalStats
            {
                ProductionCapacity = 100f,
                PowerConsumption = 50f,
                StorageCapacity = 500f,
                CrewCapacity = 20
            },
            AttachmentPoints = CreateCubicAttachmentPoints(12, "factory")
        });
        
        AddDefinition(new StationModuleDefinition
        {
            Id = "station_refinery",
            Name = "Ore Refinery",
            Description = "Processes raw ore into refined materials",
            Category = StationModuleCategory.Production,
            SubCategory = "Refinery",
            Size = new Vector3(15, 12, 15),
            BaseMass = 600f,
            BaseHealth = 5000f,
            BaseCost = 12000,
            TechLevel = 2,
            BaseStats = new StationFunctionalStats
            {
                ProductionCapacity = 200f,
                PowerConsumption = 75f,
                StorageCapacity = 1000f,
                CrewCapacity = 30
            },
            AttachmentPoints = CreateCubicAttachmentPoints(15, "refinery")
        });
        
        // Storage Modules
        AddDefinition(new StationModuleDefinition
        {
            Id = "station_storage_basic",
            Name = "Cargo Storage",
            Description = "Basic cargo storage facility",
            Category = StationModuleCategory.Storage,
            SubCategory = "Warehouse",
            Size = new Vector3(10, 8, 15),
            BaseMass = 200f,
            BaseHealth = 2000f,
            BaseCost = 3000,
            TechLevel = 1,
            BaseStats = new StationFunctionalStats
            {
                StorageCapacity = 2000f,
                PowerConsumption = 5f
            },
            AttachmentPoints = CreateCubicAttachmentPoints(10, "storage")
        });
        
        AddDefinition(new StationModuleDefinition
        {
            Id = "station_storage_large",
            Name = "Large Warehouse",
            Description = "Massive storage facility for bulk goods",
            Category = StationModuleCategory.Storage,
            SubCategory = "Warehouse",
            Size = new Vector3(20, 15, 30),
            BaseMass = 500f,
            BaseHealth = 4000f,
            BaseCost = 8000,
            TechLevel = 2,
            BaseStats = new StationFunctionalStats
            {
                StorageCapacity = 10000f,
                PowerConsumption = 15f
            },
            AttachmentPoints = CreateCubicAttachmentPoints(20, "storage")
        });
        
        // Defense Modules
        AddDefinition(new StationModuleDefinition
        {
            Id = "station_defense_turret",
            Name = "Defense Turret",
            Description = "Automated defense turret",
            Category = StationModuleCategory.Defense,
            SubCategory = "Weapon",
            Size = new Vector3(5, 5, 8),
            BaseMass = 150f,
            BaseHealth = 1500f,
            BaseCost = 5000,
            TechLevel = 2,
            BaseStats = new StationFunctionalStats
            {
                DefenseRating = 100f,
                PowerConsumption = 30f
            },
            AttachmentPoints = CreateTurretAttachmentPoints()
        });
        
        AddDefinition(new StationModuleDefinition
        {
            Id = "station_shield_generator",
            Name = "Shield Generator",
            Description = "Protective shield system",
            Category = StationModuleCategory.Defense,
            SubCategory = "Shield",
            Size = new Vector3(8, 8, 8),
            BaseMass = 300f,
            BaseHealth = 3000f,
            BaseCost = 8000,
            TechLevel = 2,
            BaseStats = new StationFunctionalStats
            {
                ShieldCapacity = 5000f,
                ShieldRechargeRate = 50f,
                PowerConsumption = 100f
            },
            AttachmentPoints = CreateCubicAttachmentPoints(8, "shield")
        });
        
        // Utility Modules
        AddDefinition(new StationModuleDefinition
        {
            Id = "station_power_generator",
            Name = "Power Generator",
            Description = "Generates power for station systems",
            Category = StationModuleCategory.Utility,
            SubCategory = "Power",
            Size = new Vector3(8, 10, 8),
            BaseMass = 400f,
            BaseHealth = 4000f,
            BaseCost = 6000,
            TechLevel = 1,
            BaseStats = new StationFunctionalStats
            {
                PowerGeneration = 500f
            },
            AttachmentPoints = CreateCubicAttachmentPoints(8, "power")
        });
        
        AddDefinition(new StationModuleDefinition
        {
            Id = "station_sensor_array",
            Name = "Sensor Array",
            Description = "Long-range detection and scanning",
            Category = StationModuleCategory.Utility,
            SubCategory = "Sensors",
            Size = new Vector3(6, 12, 6),
            BaseMass = 200f,
            BaseHealth = 2000f,
            BaseCost = 7000,
            TechLevel = 2,
            BaseStats = new StationFunctionalStats
            {
                PowerConsumption = 25f
            },
            AttachmentPoints = CreateCubicAttachmentPoints(6, "sensor")
        });
        
        // Habitat Modules
        AddDefinition(new StationModuleDefinition
        {
            Id = "station_habitat_basic",
            Name = "Crew Quarters",
            Description = "Living quarters for station personnel",
            Category = StationModuleCategory.Habitat,
            SubCategory = "Quarters",
            Size = new Vector3(12, 8, 15),
            BaseMass = 250f,
            BaseHealth = 2500f,
            BaseCost = 4000,
            TechLevel = 1,
            BaseStats = new StationFunctionalStats
            {
                CrewCapacity = 100,
                PowerConsumption = 15f
            },
            AttachmentPoints = CreateCubicAttachmentPoints(12, "habitat")
        });
        
        // Trade Modules
        AddDefinition(new StationModuleDefinition
        {
            Id = "station_trade_market",
            Name = "Trading Market",
            Description = "Marketplace for buying and selling goods",
            Category = StationModuleCategory.Trade,
            SubCategory = "Market",
            Size = new Vector3(15, 10, 15),
            BaseMass = 300f,
            BaseHealth = 3000f,
            BaseCost = 10000,
            TechLevel = 1,
            BaseStats = new StationFunctionalStats
            {
                TradingCapacity = 1000f,
                StorageCapacity = 1000f,
                PowerConsumption = 20f,
                CrewCapacity = 20
            },
            AttachmentPoints = CreateCubicAttachmentPoints(15, "trade")
        });
        
        // Research Modules
        AddDefinition(new StationModuleDefinition
        {
            Id = "station_research_lab",
            Name = "Research Laboratory",
            Description = "Advanced research facility",
            Category = StationModuleCategory.Research,
            SubCategory = "Lab",
            Size = new Vector3(12, 10, 12),
            BaseMass = 350f,
            BaseHealth = 3500f,
            BaseCost = 15000,
            TechLevel = 3,
            BaseStats = new StationFunctionalStats
            {
                ResearchPoints = 50,
                PowerConsumption = 75f,
                CrewCapacity = 40
            },
            AttachmentPoints = CreateCubicAttachmentPoints(12, "research")
        });
        
        // Structural Modules
        AddDefinition(new StationModuleDefinition
        {
            Id = "station_connector",
            Name = "Connector Module",
            Description = "Structural connector between modules",
            Category = StationModuleCategory.Structural,
            SubCategory = "Connector",
            Size = new Vector3(5, 5, 10),
            BaseMass = 50f,
            BaseHealth = 1000f,
            BaseCost = 500,
            TechLevel = 1,
            AttachmentPoints = CreateConnectorAttachmentPoints()
        });
    }
    
    private Dictionary<string, AttachmentPoint> CreateCubicAttachmentPoints(float size, string prefix)
    {
        var points = new Dictionary<string, AttachmentPoint>();
        float half = size / 2f;
        
        points[$"{prefix}_front"] = new AttachmentPoint
        {
            Name = $"{prefix}_front",
            Position = new Vector3(0, 0, half),
            Direction = Vector3.UnitZ
        };
        points[$"{prefix}_back"] = new AttachmentPoint
        {
            Name = $"{prefix}_back",
            Position = new Vector3(0, 0, -half),
            Direction = -Vector3.UnitZ
        };
        points[$"{prefix}_left"] = new AttachmentPoint
        {
            Name = $"{prefix}_left",
            Position = new Vector3(-half, 0, 0),
            Direction = -Vector3.UnitX
        };
        points[$"{prefix}_right"] = new AttachmentPoint
        {
            Name = $"{prefix}_right",
            Position = new Vector3(half, 0, 0),
            Direction = Vector3.UnitX
        };
        points[$"{prefix}_top"] = new AttachmentPoint
        {
            Name = $"{prefix}_top",
            Position = new Vector3(0, half, 0),
            Direction = Vector3.UnitY
        };
        points[$"{prefix}_bottom"] = new AttachmentPoint
        {
            Name = $"{prefix}_bottom",
            Position = new Vector3(0, -half, 0),
            Direction = -Vector3.UnitY
        };
        
        return points;
    }
    
    private Dictionary<string, AttachmentPoint> CreateDockingAttachmentPoints()
    {
        var points = new Dictionary<string, AttachmentPoint>();
        
        points["dock_rear"] = new AttachmentPoint
        {
            Name = "dock_rear",
            Position = new Vector3(0, 0, -10),
            Direction = -Vector3.UnitZ
        };
        points["dock_top"] = new AttachmentPoint
        {
            Name = "dock_top",
            Position = new Vector3(0, 4, 0),
            Direction = Vector3.UnitY
        };
        points["dock_bottom"] = new AttachmentPoint
        {
            Name = "dock_bottom",
            Position = new Vector3(0, -4, 0),
            Direction = -Vector3.UnitY
        };
        
        return points;
    }
    
    private Dictionary<string, AttachmentPoint> CreateTurretAttachmentPoints()
    {
        var points = new Dictionary<string, AttachmentPoint>();
        
        points["turret_base"] = new AttachmentPoint
        {
            Name = "turret_base",
            Position = new Vector3(0, -2.5f, 0),
            Direction = -Vector3.UnitY
        };
        
        return points;
    }
    
    private Dictionary<string, AttachmentPoint> CreateConnectorAttachmentPoints()
    {
        var points = new Dictionary<string, AttachmentPoint>();
        
        points["connector_front"] = new AttachmentPoint
        {
            Name = "connector_front",
            Position = new Vector3(0, 0, 5),
            Direction = Vector3.UnitZ
        };
        points["connector_back"] = new AttachmentPoint
        {
            Name = "connector_back",
            Position = new Vector3(0, 0, -5),
            Direction = -Vector3.UnitZ
        };
        
        return points;
    }
}
