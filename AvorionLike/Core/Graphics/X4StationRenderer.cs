using System.Numerics;

namespace AvorionLike.Core.Graphics;

// Note: Using local enum to avoid conflict with multiple X4StationType enums in different namespaces
// This mirrors AvorionLike.Core.Modular.X4StationType
public enum X4StationType
{
    TradingPost,
    Shipyard,
    Factory,
    MiningStation,
    ResearchStation,
    DefensePlatform,
    RefuelingDepot,
    CommandCenter,
    Habitat
}

/// <summary>
/// X4-inspired station renderer with detailed modular architecture
/// Creates complex, realistic space stations with X4-style components
/// </summary>
public class X4StationRenderer
{
    private readonly Random _random;
    
    public X4StationRenderer(int seed = 0)
    {
        _random = seed == 0 ? new Random() : new Random(seed);
    }
    
    /// <summary>
    /// Generate detailed station model with X4-style modular design
    /// </summary>
    public DetailedStation GenerateDetailedStation(X4StationType stationType, string material, int complexity = 3)
    {
        var station = new DetailedStation
        {
            Type = stationType,
            Material = material,
            Complexity = complexity
        };
        
        // Generate core hub module
        station.CoreHub = GenerateCoreHub(stationType);
        
        // Add type-specific modules
        station.Modules = GenerateStationModules(stationType, complexity);
        
        // Add structural connectors between modules
        station.Connectors = GenerateConnectors(station.Modules);
        
        // Add docking bays
        station.DockingBays = GenerateDockingBays(stationType, complexity);
        
        // Add external details (antennas, solar panels, lights)
        station.ExternalDetails = GenerateExternalDetails(stationType, complexity);
        
        // Add defensive turrets for military stations
        if (ShouldHaveDefenses(stationType))
        {
            station.DefensiveTurrets = GenerateDefensiveTurrets(complexity);
        }
        
        // Generate lighting system
        station.LightingSystems = GenerateStationLighting(stationType, complexity);
        
        return station;
    }
    
    /// <summary>
    /// Generate central core hub module
    /// </summary>
    private StationModule GenerateCoreHub(X4StationType stationType)
    {
        var coreSize = stationType switch
        {
            X4StationType.TradingPost => new Vector3(40f, 40f, 40f),
            X4StationType.Shipyard => new Vector3(60f, 50f, 60f),
            X4StationType.Factory => new Vector3(80f, 40f, 80f),
            X4StationType.MiningStation => new Vector3(50f, 50f, 50f),
            X4StationType.ResearchStation => new Vector3(45f, 60f, 45f),
            X4StationType.DefensePlatform => new Vector3(50f, 30f, 50f),
            X4StationType.CommandCenter => new Vector3(70f, 70f, 70f),
            _ => new Vector3(40f, 40f, 40f)
        };
        
        return new StationModule
        {
            Type = StationModuleType.CoreHub,
            Position = Vector3.Zero,
            Scale = coreSize,
            Rotation = Vector3.Zero,
            ModelType = stationType switch
            {
                X4StationType.TradingPost => StationModelType.RingHub,
                X4StationType.Shipyard => StationModelType.TowerHub,
                X4StationType.Factory => StationModelType.IndustrialHub,
                X4StationType.ResearchStation => StationModelType.SphericalHub,
                X4StationType.CommandCenter => StationModelType.CommandHub,
                _ => StationModelType.CylindricalHub
            },
            MaterialProperties = new ModuleMaterialProperties
            {
                BaseColor = new Vector3(0.6f, 0.65f, 0.7f),
                Metallic = 0.7f,
                Roughness = 0.4f,
                Emissive = 0.1f
            }
        };
    }
    
    /// <summary>
    /// Generate type-specific station modules
    /// </summary>
    private List<StationModule> GenerateStationModules(X4StationType stationType, int complexity)
    {
        var modules = new List<StationModule>();
        
        switch (stationType)
        {
            case X4StationType.TradingPost:
                modules.AddRange(GenerateTradingModules(complexity));
                break;
                
            case X4StationType.Shipyard:
                modules.AddRange(GenerateShipyardModules(complexity));
                break;
                
            case X4StationType.Factory:
                modules.AddRange(GenerateFactoryModules(complexity));
                break;
                
            case X4StationType.MiningStation:
                modules.AddRange(GenerateMiningModules(complexity));
                break;
                
            case X4StationType.ResearchStation:
                modules.AddRange(GenerateResearchModules(complexity));
                break;
                
            case X4StationType.DefensePlatform:
                modules.AddRange(GenerateDefenseModules(complexity));
                break;
                
            case X4StationType.RefuelingDepot:
                modules.AddRange(GenerateRefuelingModules(complexity));
                break;
                
            case X4StationType.CommandCenter:
                modules.AddRange(GenerateCommandModules(complexity));
                break;
        }
        
        return modules;
    }
    
    /// <summary>
    /// Generate trading post modules (cargo bays, market halls)
    /// </summary>
    private List<StationModule> GenerateTradingModules(int complexity)
    {
        var modules = new List<StationModule>();
        int moduleCount = 4 + complexity * 2;
        
        for (int i = 0; i < moduleCount; i++)
        {
            float angle = i * (360f / moduleCount);
            float radius = 80f + (float)_random.NextDouble() * 40f;
            
            modules.Add(new StationModule
            {
                Type = i % 2 == 0 ? StationModuleType.CargoBay : StationModuleType.MarketHall,
                Position = new Vector3(
                    (float)Math.Cos(angle * Math.PI / 180) * radius,
                    (float)(_random.NextDouble() - 0.5) * 30f,
                    (float)Math.Sin(angle * Math.PI / 180) * radius
                ),
                Scale = new Vector3(30f, 25f, 35f),
                Rotation = new Vector3(0, angle, 0),
                ModelType = StationModelType.CargoContainer
            });
        }
        
        return modules;
    }
    
    /// <summary>
    /// Generate shipyard modules (construction frames, assembly bays)
    /// </summary>
    private List<StationModule> GenerateShipyardModules(int complexity)
    {
        var modules = new List<StationModule>();
        
        // Main construction frame
        modules.Add(new StationModule
        {
            Type = StationModuleType.ConstructionFrame,
            Position = new Vector3(0, -60f, 100f),
            Scale = new Vector3(150f, 80f, 100f),
            Rotation = Vector3.Zero,
            ModelType = StationModelType.ConstructionFrame
        });
        
        // Assembly bays
        int bayCount = 2 + complexity;
        for (int i = 0; i < bayCount; i++)
        {
            modules.Add(new StationModule
            {
                Type = StationModuleType.AssemblyBay,
                Position = new Vector3(i * 60f - (bayCount - 1) * 30f, 0, -80f),
                Scale = new Vector3(40f, 40f, 60f),
                Rotation = Vector3.Zero,
                ModelType = StationModelType.HangarBay
            });
        }
        
        return modules;
    }
    
    /// <summary>
    /// Generate factory modules (production lines, refineries)
    /// </summary>
    private List<StationModule> GenerateFactoryModules(int complexity)
    {
        var modules = new List<StationModule>();
        int moduleCount = 6 + complexity * 3;
        
        // Create grid of industrial modules
        int gridSize = (int)Math.Ceiling(Math.Sqrt(moduleCount));
        
        for (int i = 0; i < moduleCount; i++)
        {
            int x = i % gridSize;
            int z = i / gridSize;
            
            modules.Add(new StationModule
            {
                Type = StationModuleType.ProductionLine,
                Position = new Vector3(x * 70f - gridSize * 35f, 0, z * 70f - gridSize * 35f),
                Scale = new Vector3(50f, 40f, 50f),
                Rotation = new Vector3(0, _random.Next(4) * 90f, 0),
                ModelType = StationModelType.IndustrialModule
            });
        }
        
        return modules;
    }
    
    /// <summary>
    /// Generate mining station modules (ore processors, storage silos)
    /// </summary>
    private List<StationModule> GenerateMiningModules(int complexity)
    {
        var modules = new List<StationModule>();
        
        // Ore processing units
        int processorCount = 3 + complexity;
        for (int i = 0; i < processorCount; i++)
        {
            float angle = i * (360f / processorCount);
            
            modules.Add(new StationModule
            {
                Type = StationModuleType.OreProcessor,
                Position = new Vector3(
                    (float)Math.Cos(angle * Math.PI / 180) * 70f,
                    0,
                    (float)Math.Sin(angle * Math.PI / 180) * 70f
                ),
                Scale = new Vector3(35f, 50f, 35f),
                Rotation = new Vector3(0, angle, 0),
                ModelType = StationModelType.ProcessingUnit
            });
        }
        
        // Storage silos
        int siloCount = 4 + complexity * 2;
        for (int i = 0; i < siloCount; i++)
        {
            float angle = i * (360f / siloCount) + 180f / siloCount;
            
            modules.Add(new StationModule
            {
                Type = StationModuleType.StorageSilo,
                Position = new Vector3(
                    (float)Math.Cos(angle * Math.PI / 180) * 100f,
                    0,
                    (float)Math.Sin(angle * Math.PI / 180) * 100f
                ),
                Scale = new Vector3(25f, 60f, 25f),
                Rotation = Vector3.Zero,
                ModelType = StationModelType.Silo
            });
        }
        
        return modules;
    }
    
    /// <summary>
    /// Generate research station modules (laboratories, sensor arrays)
    /// </summary>
    private List<StationModule> GenerateResearchModules(int complexity)
    {
        var modules = new List<StationModule>();
        
        // Research laboratories arranged in a ring
        int labCount = 4 + complexity;
        for (int i = 0; i < labCount; i++)
        {
            float angle = i * (360f / labCount);
            
            modules.Add(new StationModule
            {
                Type = StationModuleType.Laboratory,
                Position = new Vector3(
                    (float)Math.Cos(angle * Math.PI / 180) * 90f,
                    (float)Math.Sin(i * 0.5f) * 20f,
                    (float)Math.Sin(angle * Math.PI / 180) * 90f
                ),
                Scale = new Vector3(30f, 35f, 40f),
                Rotation = new Vector3(0, angle, 0),
                ModelType = StationModelType.LabModule
            });
        }
        
        // Central sensor array tower
        modules.Add(new StationModule
        {
            Type = StationModuleType.SensorArray,
            Position = new Vector3(0, 80f, 0),
            Scale = new Vector3(20f, 60f, 20f),
            Rotation = Vector3.Zero,
            ModelType = StationModelType.SensorTower
        });
        
        return modules;
    }
    
    /// <summary>
    /// Generate defense platform modules (weapon batteries, shields)
    /// </summary>
    private List<StationModule> GenerateDefenseModules(int complexity)
    {
        var modules = new List<StationModule>();
        
        // Weapon batteries
        int batteryCount = 6 + complexity * 2;
        for (int i = 0; i < batteryCount; i++)
        {
            float angle = i * (360f / batteryCount);
            float radius = 60f;
            
            modules.Add(new StationModule
            {
                Type = StationModuleType.WeaponBattery,
                Position = new Vector3(
                    (float)Math.Cos(angle * Math.PI / 180) * radius,
                    (float)Math.Sin(i * 0.3f) * 15f,
                    (float)Math.Sin(angle * Math.PI / 180) * radius
                ),
                Scale = new Vector3(25f, 20f, 30f),
                Rotation = new Vector3(0, angle, 0),
                ModelType = StationModelType.WeaponPlatform
            });
        }
        
        // Shield generators
        modules.Add(new StationModule
        {
            Type = StationModuleType.ShieldGenerator,
            Position = new Vector3(0, 40f, 0),
            Scale = new Vector3(30f, 30f, 30f),
            Rotation = Vector3.Zero,
            ModelType = StationModelType.ShieldArray
        });
        
        return modules;
    }
    
    /// <summary>
    /// Generate refueling depot modules (fuel tanks, pumping stations)
    /// </summary>
    private List<StationModule> GenerateRefuelingModules(int complexity)
    {
        var modules = new List<StationModule>();
        
        // Fuel storage tanks
        int tankCount = 8 + complexity * 3;
        for (int i = 0; i < tankCount; i++)
        {
            float angle = i * (360f / tankCount);
            
            modules.Add(new StationModule
            {
                Type = StationModuleType.FuelTank,
                Position = new Vector3(
                    (float)Math.Cos(angle * Math.PI / 180) * 80f,
                    0,
                    (float)Math.Sin(angle * Math.PI / 180) * 80f
                ),
                Scale = new Vector3(30f, 50f, 30f),
                Rotation = Vector3.Zero,
                ModelType = StationModelType.Cylinder
            });
        }
        
        return modules;
    }
    
    /// <summary>
    /// Generate command center modules (communication arrays, control rooms)
    /// </summary>
    private List<StationModule> GenerateCommandModules(int complexity)
    {
        var modules = new List<StationModule>();
        
        // Communication towers
        int towerCount = 4 + complexity;
        for (int i = 0; i < towerCount; i++)
        {
            float angle = i * (360f / towerCount);
            
            modules.Add(new StationModule
            {
                Type = StationModuleType.CommunicationTower,
                Position = new Vector3(
                    (float)Math.Cos(angle * Math.PI / 180) * 100f,
                    40f,
                    (float)Math.Sin(angle * Math.PI / 180) * 100f
                ),
                Scale = new Vector3(15f, 80f, 15f),
                Rotation = Vector3.Zero,
                ModelType = StationModelType.CommsTower
            });
        }
        
        // Control modules
        int controlCount = 6 + complexity * 2;
        for (int i = 0; i < controlCount; i++)
        {
            float angle = i * (360f / controlCount) + 180f / controlCount;
            
            modules.Add(new StationModule
            {
                Type = StationModuleType.ControlRoom,
                Position = new Vector3(
                    (float)Math.Cos(angle * Math.PI / 180) * 70f,
                    0,
                    (float)Math.Sin(angle * Math.PI / 180) * 70f
                ),
                Scale = new Vector3(30f, 25f, 30f),
                Rotation = new Vector3(0, angle, 0),
                ModelType = StationModelType.ControlModule
            });
        }
        
        return modules;
    }
    
    /// <summary>
    /// Generate structural connectors between modules
    /// </summary>
    private List<StationConnector> GenerateConnectors(List<StationModule> modules)
    {
        var connectors = new List<StationConnector>();
        
        // Connect each module to its nearest neighbors
        foreach (var module in modules)
        {
            var nearest = modules
                .Where(m => m != module)
                .OrderBy(m => Vector3.Distance(m.Position, module.Position))
                .Take(2);
            
            foreach (var target in nearest)
            {
                // Avoid duplicate connectors
                if (connectors.Any(c => 
                    (c.StartPosition == module.Position && c.EndPosition == target.Position) ||
                    (c.StartPosition == target.Position && c.EndPosition == module.Position)))
                    continue;
                
                connectors.Add(new StationConnector
                {
                    StartPosition = module.Position,
                    EndPosition = target.Position,
                    Radius = 5f + (float)_random.NextDouble() * 3f,
                    Type = ConnectorType.Tube
                });
            }
        }
        
        return connectors;
    }
    
    /// <summary>
    /// Generate docking bays for ships
    /// </summary>
    private List<DockingBay> GenerateDockingBays(X4StationType stationType, int complexity)
    {
        var bays = new List<DockingBay>();
        
        int bayCount = stationType switch
        {
            X4StationType.TradingPost => 8 + complexity * 2,
            X4StationType.Shipyard => 6 + complexity * 2,
            X4StationType.CommandCenter => 10 + complexity * 3,
            _ => 4 + complexity
        };
        
        for (int i = 0; i < bayCount; i++)
        {
            float angle = i * (360f / bayCount);
            float radius = 120f;
            
            bays.Add(new DockingBay
            {
                Position = new Vector3(
                    (float)Math.Cos(angle * Math.PI / 180) * radius,
                    0,
                    (float)Math.Sin(angle * Math.PI / 180) * radius
                ),
                Rotation = new Vector3(0, angle + 180f, 0),
                Size = DockingBaySize.Medium,
                LightColor = new Vector3(0.3f, 0.8f, 1.0f),
                Active = true
            });
        }
        
        return bays;
    }
    
    /// <summary>
    /// Generate external station details
    /// </summary>
    private List<ExternalDetail> GenerateExternalDetails(X4StationType stationType, int complexity)
    {
        var details = new List<ExternalDetail>();
        int detailCount = 20 + complexity * 10;
        
        for (int i = 0; i < detailCount; i++)
        {
            var detailType = (ExternalDetailType)_random.Next((int)ExternalDetailType.Count);
            
            details.Add(new ExternalDetail
            {
                Type = detailType,
                Position = new Vector3(
                    (float)(_random.NextDouble() - 0.5) * 200f,
                    (float)(_random.NextDouble() - 0.5) * 100f,
                    (float)(_random.NextDouble() - 0.5) * 200f
                ),
                Scale = detailType switch
                {
                    ExternalDetailType.Antenna => new Vector3(2f, 15f, 2f),
                    ExternalDetailType.SolarPanel => new Vector3(20f, 0.5f, 10f),
                    ExternalDetailType.Light => new Vector3(3f, 3f, 3f),
                    ExternalDetailType.Dish => new Vector3(10f, 10f, 5f),
                    _ => new Vector3(5f, 5f, 5f)
                },
                Rotation = new Vector3(
                    (float)_random.NextDouble() * 360f,
                    (float)_random.NextDouble() * 360f,
                    (float)_random.NextDouble() * 360f
                )
            });
        }
        
        return details;
    }
    
    /// <summary>
    /// Generate defensive turrets for military stations
    /// </summary>
    private List<DefensiveTurret> GenerateDefensiveTurrets(int complexity)
    {
        var turrets = new List<DefensiveTurret>();
        int turretCount = 8 + complexity * 4;
        
        for (int i = 0; i < turretCount; i++)
        {
            float angle = i * (360f / turretCount);
            float radius = 100f + (float)_random.NextDouble() * 50f;
            
            turrets.Add(new DefensiveTurret
            {
                Position = new Vector3(
                    (float)Math.Cos(angle * Math.PI / 180) * radius,
                    (float)(_random.NextDouble() - 0.5) * 40f,
                    (float)Math.Sin(angle * Math.PI / 180) * radius
                ),
                TurretType = (TurretType)_random.Next((int)TurretType.Count),
                Size = TurretSize.Medium
            });
        }
        
        return turrets;
    }
    
    /// <summary>
    /// Generate station lighting systems
    /// </summary>
    private List<StationLight> GenerateStationLighting(X4StationType stationType, int complexity)
    {
        var lights = new List<StationLight>();
        int lightCount = 30 + complexity * 15;
        
        // Navigation lights
        for (int i = 0; i < 8; i++)
        {
            float angle = i * 45f;
            lights.Add(new StationLight
            {
                Position = new Vector3(
                    (float)Math.Cos(angle * Math.PI / 180) * 150f,
                    0,
                    (float)Math.Sin(angle * Math.PI / 180) * 150f
                ),
                Color = i % 2 == 0 ? new Vector3(1f, 0f, 0f) : new Vector3(0f, 1f, 0f),
                Intensity = 2.0f,
                Range = 100f,
                BlinkPattern = BlinkPattern.Slow
            });
        }
        
        // Work lights
        for (int i = 8; i < lightCount; i++)
        {
            lights.Add(new StationLight
            {
                Position = new Vector3(
                    (float)(_random.NextDouble() - 0.5) * 200f,
                    (float)(_random.NextDouble() - 0.5) * 100f,
                    (float)(_random.NextDouble() - 0.5) * 200f
                ),
                Color = new Vector3(1f, 0.9f, 0.8f),
                Intensity = 1.0f + (float)_random.NextDouble(),
                Range = 50f,
                BlinkPattern = BlinkPattern.Steady
            });
        }
        
        return lights;
    }
    
    /// <summary>
    /// Check if station type should have defensive systems
    /// </summary>
    private bool ShouldHaveDefenses(X4StationType type)
    {
        return type switch
        {
            X4StationType.DefensePlatform => true,
            X4StationType.CommandCenter => true,
            X4StationType.MiningStation => true,
            _ => false
        };
    }
}

/// <summary>
/// Detailed station with full rendering data
/// </summary>
public class DetailedStation
{
    public X4StationType Type { get; set; }
    public string Material { get; set; } = "";
    public int Complexity { get; set; }
    public StationModule CoreHub { get; set; } = null!;
    public List<StationModule> Modules { get; set; } = new();
    public List<StationConnector> Connectors { get; set; } = new();
    public List<DockingBay> DockingBays { get; set; } = new();
    public List<ExternalDetail> ExternalDetails { get; set; } = new();
    public List<DefensiveTurret>? DefensiveTurrets { get; set; }
    public List<StationLight> LightingSystems { get; set; } = new();
}

/// <summary>
/// Station module component
/// </summary>
public class StationModule
{
    public StationModuleType Type { get; set; }
    public Vector3 Position { get; set; }
    public Vector3 Scale { get; set; }
    public Vector3 Rotation { get; set; }
    public StationModelType ModelType { get; set; }
    public ModuleMaterialProperties MaterialProperties { get; set; } = new();
}

/// <summary>
/// Connector between modules
/// </summary>
public class StationConnector
{
    public Vector3 StartPosition { get; set; }
    public Vector3 EndPosition { get; set; }
    public float Radius { get; set; }
    public ConnectorType Type { get; set; }
}

/// <summary>
/// Docking bay for ships
/// </summary>
public class DockingBay
{
    public Vector3 Position { get; set; }
    public Vector3 Rotation { get; set; }
    public DockingBaySize Size { get; set; }
    public Vector3 LightColor { get; set; }
    public bool Active { get; set; }
}

/// <summary>
/// External station detail
/// </summary>
public class ExternalDetail
{
    public ExternalDetailType Type { get; set; }
    public Vector3 Position { get; set; }
    public Vector3 Scale { get; set; }
    public Vector3 Rotation { get; set; }
}

/// <summary>
/// Defensive turret
/// </summary>
public class DefensiveTurret
{
    public Vector3 Position { get; set; }
    public TurretType TurretType { get; set; }
    public TurretSize Size { get; set; }
}

/// <summary>
/// Station lighting
/// </summary>
public class StationLight
{
    public Vector3 Position { get; set; }
    public Vector3 Color { get; set; }
    public float Intensity { get; set; }
    public float Range { get; set; }
    public BlinkPattern BlinkPattern { get; set; }
}

/// <summary>
/// Module material properties
/// </summary>
public class ModuleMaterialProperties
{
    public Vector3 BaseColor { get; set; }
    public float Metallic { get; set; }
    public float Roughness { get; set; }
    public float Emissive { get; set; }
}

// Enums
public enum StationModuleType
{
    CoreHub, CargoBay, MarketHall, ConstructionFrame, AssemblyBay,
    ProductionLine, OreProcessor, StorageSilo, Laboratory, SensorArray,
    WeaponBattery, ShieldGenerator, FuelTank, CommunicationTower, ControlRoom
}

public enum StationModelType
{
    RingHub, TowerHub, IndustrialHub, SphericalHub, CommandHub, CylindricalHub,
    CargoContainer, ConstructionFrame, HangarBay, IndustrialModule, ProcessingUnit,
    Silo, LabModule, SensorTower, WeaponPlatform, ShieldArray, Cylinder,
    CommsTower, ControlModule
}

public enum ConnectorType
{
    Tube, Truss, Umbilical
}

public enum DockingBaySize
{
    Small, Medium, Large, Capital
}

public enum ExternalDetailType
{
    Antenna, SolarPanel, Light, Dish, Vent, Count
}

public enum TurretType
{
    Laser, Plasma, Missile, Count
}

public enum TurretSize
{
    Small, Medium, Large
}
