using System.Numerics;
using AvorionLike.Core.Logging;

namespace AvorionLike.Core.Modular;

/// <summary>
/// Configuration for modular ship generation
/// </summary>
public class ModularShipConfig
{
    public string ShipName { get; set; } = "Unnamed Ship";
    public ShipSize Size { get; set; } = ShipSize.Frigate;
    public ShipRole Role { get; set; } = ShipRole.Multipurpose;
    public string Material { get; set; } = "Iron";
    public int Seed { get; set; } = 0;
    
    // Design preferences
    public bool AddWings { get; set; } = true;
    public bool AddWeapons { get; set; } = true;
    public bool AddCargo { get; set; } = true;
    public bool AddHyperdrive { get; set; } = true;
    public int MinimumEngines { get; set; } = 1;
    public int DesiredWeaponMounts { get; set; } = 2;
}

/// <summary>
/// Ship size categories for modular generation
/// </summary>
public enum ShipSize
{
    Fighter,      // Small, agile
    Corvette,     // Light ship
    Frigate,      // Medium ship
    Destroyer,    // Large combat ship
    Cruiser,      // Heavy ship
    Battleship,   // Capital ship
    Carrier       // Massive carrier
}

/// <summary>
/// Ship role determines module selection
/// </summary>
public enum ShipRole
{
    Multipurpose,  // Balanced
    Combat,        // Heavy weapons
    Mining,        // Mining lasers, cargo
    Trading,       // Huge cargo
    Exploration,   // Sensors, hyperdrive
    Salvage        // Salvage beams
}

/// <summary>
/// Result of modular ship generation
/// </summary>
public class ModularGeneratedShip
{
    public ModularShipComponent Ship { get; set; } = new();
    public ModularShipConfig Config { get; set; } = new();
    public List<string> Warnings { get; set; } = new();
    public Dictionary<string, int> ModuleCounts { get; set; } = new();
}

/// <summary>
/// Procedurally generates ships using the modular system
/// Replaces voxel-based ship generation with module assembly
/// </summary>
public class ModularProceduralShipGenerator
{
    private readonly ModuleLibrary _library;
    private Random _random;
    private readonly Logger _logger = Logger.Instance;
    
    public ModularProceduralShipGenerator(ModuleLibrary library, int seed = 0)
    {
        _library = library;
        _random = seed == 0 ? new Random() : new Random(seed);
    }
    
    /// <summary>
    /// Generate a complete modular ship
    /// </summary>
    public ModularGeneratedShip GenerateShip(ModularShipConfig config)
    {
        _random = new Random(config.Seed == 0 ? Environment.TickCount : config.Seed);
        
        var result = new ModularGeneratedShip { Config = config };
        var ship = new ModularShipComponent
        {
            EntityId = Guid.NewGuid(),
            Name = config.ShipName
        };
        
        _logger.Info("ModularShipGen", $"Generating {config.Size} {config.Role} ship: {config.ShipName}");
        
        // Step 1: Create cockpit (core module)
        var cockpit = CreateCockpit(config);
        ship.AddModule(cockpit);
        ship.CoreModuleId = cockpit.Id;
        result.ModuleCounts["Cockpit"] = 1;
        
        // Step 2: Add hull sections based on size
        var hullModules = CreateHullSections(config, cockpit);
        foreach (var hull in hullModules)
        {
            ship.AddModule(hull);
        }
        result.ModuleCounts["Hull"] = hullModules.Count;
        
        // Step 3: Attach hull sections to cockpit and each other
        AttachHullSections(ship, cockpit, hullModules);
        
        // Step 4: Add engines based on size and role
        var engines = CreateEngines(config, hullModules);
        foreach (var engine in engines)
        {
            ship.AddModule(engine);
        }
        result.ModuleCounts["Engine"] = engines.Count;
        
        // Step 5: Attach engines
        AttachEngines(ship, hullModules, engines);
        
        // Step 6: Add wings if desired
        if (config.AddWings && config.Size <= ShipSize.Destroyer)
        {
            var wings = CreateWings(config, hullModules);
            foreach (var wing in wings)
            {
                ship.AddModule(wing);
            }
            result.ModuleCounts["Wing"] = wings.Count;
            AttachWings(ship, hullModules, wings);
        }
        
        // Step 7: Add power core
        var powerCore = CreatePowerCore(config);
        ship.AddModule(powerCore);
        result.ModuleCounts["PowerCore"] = 1;
        AttachUtilityModule(ship, hullModules, powerCore);
        
        // Step 8: Add weapons based on role
        if (config.AddWeapons)
        {
            var weaponMounts = CreateWeaponMounts(config, hullModules);
            foreach (var mount in weaponMounts)
            {
                ship.AddModule(mount);
            }
            result.ModuleCounts["WeaponMount"] = weaponMounts.Count;
            AttachWeaponMounts(ship, hullModules, weaponMounts);
        }
        
        // Step 9: Add utility modules based on role
        if (config.AddCargo)
        {
            var cargo = CreateCargoModule(config);
            ship.AddModule(cargo);
            result.ModuleCounts["Cargo"] = 1;
            AttachUtilityModule(ship, hullModules, cargo);
        }
        
        if (config.AddHyperdrive && config.Size >= ShipSize.Corvette)
        {
            var hyperdrive = CreateHyperdriveModule(config);
            ship.AddModule(hyperdrive);
            result.ModuleCounts["Hyperdrive"] = 1;
            AttachUtilityModule(ship, hullModules, hyperdrive);
        }
        
        // Step 10: Add sensors
        var sensor = CreateSensorModule(config);
        ship.AddModule(sensor);
        result.ModuleCounts["Sensor"] = 1;
        AttachUtilityModule(ship, hullModules, sensor);
        
        // Step 11: Add crew quarters
        var crewQuarters = CreateCrewQuarters(config);
        ship.AddModule(crewQuarters);
        result.ModuleCounts["CrewQuarters"] = 1;
        AttachUtilityModule(ship, hullModules, crewQuarters);
        
        // Step 12: Add shield generator
        var shield = CreateShieldGenerator(config);
        ship.AddModule(shield);
        result.ModuleCounts["Shield"] = 1;
        AttachUtilityModule(ship, hullModules, shield);
        
        // Recalculate final stats
        ship.RecalculateStats();
        
        result.Ship = ship;
        
        _logger.Info("ModularShipGen", 
            $"Generated ship with {ship.Modules.Count} modules, " +
            $"mass: {ship.TotalMass:F0}, thrust: {ship.AggregatedStats.ThrustPower:F0}");
        
        return result;
    }
    
    private ShipModulePart CreateCockpit(ModularShipConfig config)
    {
        var def = _library.GetDefinition("cockpit_basic");
        if (def == null) throw new Exception("Cockpit module definition not found!");
        
        var module = new ShipModulePart("cockpit_basic", Vector3.Zero, config.Material)
        {
            MaxHealth = def.GetHealthForMaterial(config.Material),
            Mass = def.GetMassForMaterial(config.Material),
            FunctionalStats = def.GetStatsForMaterial(config.Material)
        };
        module.Health = module.MaxHealth;
        
        return module;
    }
    
    private List<ShipModulePart> CreateHullSections(ModularShipConfig config, ShipModulePart cockpit)
    {
        var sections = new List<ShipModulePart>();
        
        // Number of hull sections based on ship size
        int hullCount = config.Size switch
        {
            ShipSize.Fighter => 1,
            ShipSize.Corvette => 2,
            ShipSize.Frigate => 3,
            ShipSize.Destroyer => 4,
            ShipSize.Cruiser => 5,
            ShipSize.Battleship => 6,
            ShipSize.Carrier => 8,
            _ => 2
        };
        
        var def = _library.GetDefinition("hull_section_basic");
        if (def == null) return sections;
        
        // Create hull sections behind cockpit
        for (int i = 0; i < hullCount; i++)
        {
            Vector3 position = new Vector3(0, 0, -(i + 1) * 4); // 4 units apart
            
            var hull = new ShipModulePart("hull_section_basic", position, config.Material)
            {
                MaxHealth = def.GetHealthForMaterial(config.Material),
                Mass = def.GetMassForMaterial(config.Material),
                FunctionalStats = def.GetStatsForMaterial(config.Material)
            };
            hull.Health = hull.MaxHealth;
            
            sections.Add(hull);
        }
        
        return sections;
    }
    
    private void AttachHullSections(ModularShipComponent ship, ShipModulePart cockpit, List<ShipModulePart> hullSections)
    {
        if (hullSections.Count == 0) return;
        
        // Attach first hull section to cockpit rear
        ship.AttachModules(cockpit.Id, hullSections[0].Id, "rear", "front", _library);
        
        // Attach subsequent hull sections to previous ones
        for (int i = 1; i < hullSections.Count; i++)
        {
            ship.AttachModules(hullSections[i - 1].Id, hullSections[i].Id, "rear", "front", _library);
        }
    }
    
    private List<ShipModulePart> CreateEngines(ModularShipConfig config, List<ShipModulePart> hullSections)
    {
        var engines = new List<ShipModulePart>();
        
        // Number of engines based on size
        int engineCount = Math.Max(config.MinimumEngines, config.Size switch
        {
            ShipSize.Fighter => 1,
            ShipSize.Corvette => 1,
            ShipSize.Frigate => 2,
            ShipSize.Destroyer => 2,
            ShipSize.Cruiser => 3,
            ShipSize.Battleship => 4,
            ShipSize.Carrier => 4,
            _ => 1
        });
        
        for (int i = 0; i < engineCount; i++)
        {
            string moduleId = engineCount == 1 ? "engine_main" : "engine_nacelle";
            var def = _library.GetDefinition(moduleId);
            if (def == null) continue;
            
            // Position engines at the rear
            Vector3 position = Vector3.Zero;
            if (hullSections.Count > 0)
            {
                var lastHull = hullSections[hullSections.Count - 1];
                
                if (engineCount == 1)
                {
                    position = lastHull.Position + new Vector3(0, 0, -4);
                }
                else
                {
                    // Spread engines across the width
                    float spacing = 3f;
                    float offset = (i - (engineCount - 1) / 2f) * spacing;
                    position = lastHull.Position + new Vector3(offset, 0, -4);
                }
            }
            
            var engine = new ShipModulePart(moduleId, position, config.Material)
            {
                MaxHealth = def.GetHealthForMaterial(config.Material),
                Mass = def.GetMassForMaterial(config.Material),
                FunctionalStats = def.GetStatsForMaterial(config.Material)
            };
            engine.Health = engine.MaxHealth;
            
            engines.Add(engine);
        }
        
        return engines;
    }
    
    private void AttachEngines(ModularShipComponent ship, List<ShipModulePart> hullSections, List<ShipModulePart> engines)
    {
        if (hullSections.Count == 0 || engines.Count == 0) return;
        
        var lastHull = hullSections[hullSections.Count - 1];
        
        // Attach all engines to the last hull section
        foreach (var engine in engines)
        {
            ship.AttachModules(lastHull.Id, engine.Id, "rear", "mount", _library);
        }
    }
    
    private List<ShipModulePart> CreateWings(ModularShipConfig config, List<ShipModulePart> hullSections)
    {
        var wings = new List<ShipModulePart>();
        
        if (hullSections.Count < 2) return wings;
        
        var def = _library.GetDefinition("wing_basic");
        if (def == null) return wings;
        
        // Attach wings to middle hull section
        var middleHull = hullSections[hullSections.Count / 2];
        
        // Left wing
        var leftWing = new ShipModulePart("wing_basic", 
            middleHull.Position + new Vector3(-3, 0, 0), 
            config.Material)
        {
            MaxHealth = def.GetHealthForMaterial(config.Material),
            Mass = def.GetMassForMaterial(config.Material),
            FunctionalStats = def.GetStatsForMaterial(config.Material)
        };
        leftWing.Health = leftWing.MaxHealth;
        wings.Add(leftWing);
        
        // Right wing
        var rightWing = new ShipModulePart("wing_basic", 
            middleHull.Position + new Vector3(3, 0, 0), 
            config.Material)
        {
            MaxHealth = def.GetHealthForMaterial(config.Material),
            Mass = def.GetMassForMaterial(config.Material),
            FunctionalStats = def.GetStatsForMaterial(config.Material)
        };
        rightWing.Health = rightWing.MaxHealth;
        wings.Add(rightWing);
        
        return wings;
    }
    
    private void AttachWings(ModularShipComponent ship, List<ShipModulePart> hullSections, List<ShipModulePart> wings)
    {
        if (hullSections.Count < 2 || wings.Count == 0) return;
        
        var middleHull = hullSections[hullSections.Count / 2];
        
        foreach (var wing in wings)
        {
            ship.AttachModules(middleHull.Id, wing.Id, "left", "mount", _library);
        }
    }
    
    private ShipModulePart CreatePowerCore(ModularShipConfig config)
    {
        var def = _library.GetDefinition("power_core_basic");
        if (def == null) throw new Exception("Power core module definition not found!");
        
        var module = new ShipModulePart("power_core_basic", Vector3.Zero, config.Material)
        {
            MaxHealth = def.GetHealthForMaterial(config.Material),
            Mass = def.GetMassForMaterial(config.Material),
            FunctionalStats = def.GetStatsForMaterial(config.Material)
        };
        module.Health = module.MaxHealth;
        
        return module;
    }
    
    private List<ShipModulePart> CreateWeaponMounts(ModularShipConfig config, List<ShipModulePart> hullSections)
    {
        var mounts = new List<ShipModulePart>();
        
        if (hullSections.Count == 0) return mounts;
        
        var def = _library.GetDefinition("weapon_mount_basic");
        if (def == null) return mounts;
        
        // Add weapon mounts based on role
        int mountCount = config.Role switch
        {
            ShipRole.Combat => config.DesiredWeaponMounts * 2,
            ShipRole.Multipurpose => config.DesiredWeaponMounts,
            _ => Math.Max(1, config.DesiredWeaponMounts / 2)
        };
        
        for (int i = 0; i < mountCount; i++)
        {
            // Distribute weapon mounts along the hull
            int hullIndex = i % hullSections.Count;
            var hull = hullSections[hullIndex];
            
            float side = (i % 2 == 0) ? -1.5f : 1.5f;
            Vector3 position = hull.Position + new Vector3(side, 1, 0);
            
            var mount = new ShipModulePart("weapon_mount_basic", position, config.Material)
            {
                MaxHealth = def.GetHealthForMaterial(config.Material),
                Mass = def.GetMassForMaterial(config.Material),
                FunctionalStats = def.GetStatsForMaterial(config.Material)
            };
            mount.Health = mount.MaxHealth;
            
            mounts.Add(mount);
        }
        
        return mounts;
    }
    
    private void AttachWeaponMounts(ModularShipComponent ship, List<ShipModulePart> hullSections, List<ShipModulePart> mounts)
    {
        if (hullSections.Count == 0 || mounts.Count == 0) return;
        
        for (int i = 0; i < mounts.Count; i++)
        {
            int hullIndex = i % hullSections.Count;
            var hull = hullSections[hullIndex];
            
            ship.AttachModules(hull.Id, mounts[i].Id, "left", "mount", _library);
        }
    }
    
    private ShipModulePart CreateCargoModule(ModularShipConfig config)
    {
        var def = _library.GetDefinition("cargo_bay_basic");
        if (def == null) throw new Exception("Cargo module definition not found!");
        
        var module = new ShipModulePart("cargo_bay_basic", Vector3.Zero, config.Material)
        {
            MaxHealth = def.GetHealthForMaterial(config.Material),
            Mass = def.GetMassForMaterial(config.Material),
            FunctionalStats = def.GetStatsForMaterial(config.Material)
        };
        module.Health = module.MaxHealth;
        
        return module;
    }
    
    private ShipModulePart CreateHyperdriveModule(ModularShipConfig config)
    {
        var def = _library.GetDefinition("hyperdrive_basic");
        if (def == null) throw new Exception("Hyperdrive module definition not found!");
        
        var module = new ShipModulePart("hyperdrive_basic", Vector3.Zero, config.Material)
        {
            MaxHealth = def.GetHealthForMaterial(config.Material),
            Mass = def.GetMassForMaterial(config.Material),
            FunctionalStats = def.GetStatsForMaterial(config.Material)
        };
        module.Health = module.MaxHealth;
        
        return module;
    }
    
    private ShipModulePart CreateSensorModule(ModularShipConfig config)
    {
        var def = _library.GetDefinition("sensor_array_basic");
        if (def == null) throw new Exception("Sensor module definition not found!");
        
        var module = new ShipModulePart("sensor_array_basic", Vector3.Zero, config.Material)
        {
            MaxHealth = def.GetHealthForMaterial(config.Material),
            Mass = def.GetMassForMaterial(config.Material),
            FunctionalStats = def.GetStatsForMaterial(config.Material)
        };
        module.Health = module.MaxHealth;
        
        return module;
    }
    
    private ShipModulePart CreateCrewQuarters(ModularShipConfig config)
    {
        var def = _library.GetDefinition("crew_quarters_basic");
        if (def == null) throw new Exception("Crew quarters module definition not found!");
        
        var module = new ShipModulePart("crew_quarters_basic", Vector3.Zero, config.Material)
        {
            MaxHealth = def.GetHealthForMaterial(config.Material),
            Mass = def.GetMassForMaterial(config.Material),
            FunctionalStats = def.GetStatsForMaterial(config.Material)
        };
        module.Health = module.MaxHealth;
        
        return module;
    }
    
    private ShipModulePart CreateShieldGenerator(ModularShipConfig config)
    {
        var def = _library.GetDefinition("shield_gen_basic");
        if (def == null) throw new Exception("Shield generator module definition not found!");
        
        var module = new ShipModulePart("shield_gen_basic", Vector3.Zero, config.Material)
        {
            MaxHealth = def.GetHealthForMaterial(config.Material),
            Mass = def.GetMassForMaterial(config.Material),
            FunctionalStats = def.GetStatsForMaterial(config.Material)
        };
        module.Health = module.MaxHealth;
        
        return module;
    }
    
    private void AttachUtilityModule(ModularShipComponent ship, List<ShipModulePart> hullSections, ShipModulePart module)
    {
        if (hullSections.Count == 0) return;
        
        // Attach to a middle hull section
        var targetHull = hullSections[hullSections.Count / 2];
        
        // Update module position to be near the hull
        module.Position = targetHull.Position + new Vector3(0, -2, 0);
        
        // Try to attach (may fail if no suitable attachment points)
        ship.AttachModules(targetHull.Id, module.Id, "left", "mount", _library);
    }
}
