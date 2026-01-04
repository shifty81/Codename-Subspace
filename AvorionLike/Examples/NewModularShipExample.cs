using System;
using System.Numerics;
using AvorionLike.Core.Modular;
using AvorionLike.Core.ECS;
using AvorionLike.Core.Logging;

namespace AvorionLike.Examples;

/// <summary>
/// Example demonstrating the NEW modular ship design system (Star Sparrow-inspired)
/// Shows how ships are built from pre-defined modules instead of voxels
/// Voxels are now ONLY used for damage visualization and asteroid mining
/// </summary>
public class NewModularShipExample
{
    private readonly Logger _logger = Logger.Instance;
    
    public void Run()
    {
        Console.WriteLine("\n=== NEW Modular Ship Design System Example ===\n");
        Console.WriteLine("This system replaces voxel-based ship construction with");
        Console.WriteLine("modular parts (inspired by Star Sparrow and similar assets).\n");
        Console.WriteLine("Voxels are NOW ONLY used for:");
        Console.WriteLine("  - Damage visualization on ships");
        Console.WriteLine("  - Asteroid mining and deformation\n");
        
        // Example 1: Building a ship manually
        Console.WriteLine("Example 1: Manual Ship Construction");
        Console.WriteLine("====================================");
        BuildShipManually();
        
        Console.WriteLine("\n");
        
        // Example 2: Generating a ship procedurally
        Console.WriteLine("Example 2: Procedural Ship Generation");
        Console.WriteLine("=====================================");
        GenerateShipProcedurally();
        
        Console.WriteLine("\n");
        
        // Example 3: Damage visualization with voxels
        Console.WriteLine("Example 3: Damage Visualization (Voxels)");
        Console.WriteLine("========================================");
        DemonstrateDamageVisualization();
        
        Console.WriteLine("\n");
        
        // Example 4: Module library exploration
        Console.WriteLine("Example 4: Module Library");
        Console.WriteLine("=========================");
        ExploreModuleLibrary();
        
        Console.WriteLine("\n=== Example Complete ===\n");
    }
    
    private void BuildShipManually()
    {
        // Initialize module library
        var library = new ModuleLibrary();
        library.InitializeBuiltInModules();
        
        Console.WriteLine("Building a fighter ship manually...");
        
        // Create ship
        var ship = new ModularShipComponent
        {
            EntityId = Guid.NewGuid(),
            Name = "Star Fighter Alpha"
        };
        
        // Add cockpit (core module)
        Console.WriteLine("  - Adding cockpit...");
        var cockpit = CreateModule("cockpit_basic", Vector3.Zero, "Titanium", library);
        ship.AddModule(cockpit);
        ship.CoreModuleId = cockpit.Id;
        
        // Add hull section
        Console.WriteLine("  - Adding hull section...");
        var hull = CreateModule("hull_section_basic", new Vector3(0, 0, -4), "Titanium", library);
        ship.AddModule(hull);
        ship.AttachModules(cockpit.Id, hull.Id, "rear", "front", library);
        
        // Add main engine
        Console.WriteLine("  - Adding main engine...");
        var engine = CreateModule("engine_main", new Vector3(0, 0, -8), "Titanium", library);
        ship.AddModule(engine);
        ship.AttachModules(hull.Id, engine.Id, "rear", "mount", library);
        
        // Add wings
        Console.WriteLine("  - Adding wings...");
        var leftWing = CreateModule("wing_basic", new Vector3(-3, 0, -4), "Titanium", library);
        var rightWing = CreateModule("wing_basic", new Vector3(3, 0, -4), "Titanium", library);
        ship.AddModule(leftWing);
        ship.AddModule(rightWing);
        ship.AttachModules(hull.Id, leftWing.Id, "left", "mount", library);
        ship.AttachModules(hull.Id, rightWing.Id, "right", "mount", library);
        
        // Add weapon mounts
        Console.WriteLine("  - Adding weapon mounts...");
        var leftWeapon = CreateModule("weapon_mount_basic", new Vector3(-1.5f, 1, -2), "Titanium", library);
        var rightWeapon = CreateModule("weapon_mount_basic", new Vector3(1.5f, 1, -2), "Titanium", library);
        ship.AddModule(leftWeapon);
        ship.AddModule(rightWeapon);
        ship.AttachModules(hull.Id, leftWeapon.Id, "left", "mount", library);
        ship.AttachModules(hull.Id, rightWeapon.Id, "right", "mount", library);
        
        // Add power core
        Console.WriteLine("  - Adding power core...");
        var powerCore = CreateModule("power_core_basic", new Vector3(0, -2, -4), "Titanium", library);
        ship.AddModule(powerCore);
        ship.AttachModules(hull.Id, powerCore.Id, "left", "mount", library);
        
        // Recalculate stats
        ship.RecalculateStats();
        
        // Display ship info
        Console.WriteLine($"\nShip Built Successfully!");
        DisplayShipStats(ship);
    }
    
    private void GenerateShipProcedurally()
    {
        // Initialize module library
        var library = new ModuleLibrary();
        library.InitializeBuiltInModules();
        
        // Create generator
        var generator = new ModularProceduralShipGenerator(library, seed: 12345);
        
        Console.WriteLine("Generating ships procedurally...\n");
        
        // Generate different ship types
        var configs = new[]
        {
            new ModularShipConfig 
            { 
                ShipName = "Scout Fighter",
                Size = ShipSize.Fighter, 
                Role = ShipRole.Combat,
                Material = "Iron",
                AddWings = true,
                DesiredWeaponMounts = 2
            },
            new ModularShipConfig 
            { 
                ShipName = "Mining Corvette",
                Size = ShipSize.Corvette, 
                Role = ShipRole.Mining,
                Material = "Titanium",
                AddCargo = true,
                DesiredWeaponMounts = 1
            },
            new ModularShipConfig 
            { 
                ShipName = "Heavy Frigate",
                Size = ShipSize.Frigate, 
                Role = ShipRole.Multipurpose,
                Material = "Naonite",
                AddWings = true,
                AddHyperdrive = true,
                DesiredWeaponMounts = 4
            }
        };
        
        foreach (var config in configs)
        {
            Console.WriteLine($"Generating: {config.ShipName}");
            var result = generator.GenerateShip(config);
            
            Console.WriteLine($"  Modules: {result.Ship.Modules.Count}");
            Console.WriteLine($"  Mass: {result.Ship.TotalMass:F0}");
            Console.WriteLine($"  Thrust: {result.Ship.AggregatedStats.ThrustPower:F0}");
            Console.WriteLine($"  Power: {result.Ship.AggregatedStats.PowerGeneration:F0} / {result.Ship.AggregatedStats.PowerConsumption:F0}");
            Console.WriteLine($"  Weapons: {result.Ship.AggregatedStats.WeaponMountPoints}");
            Console.WriteLine($"  Cargo: {result.Ship.AggregatedStats.CargoCapacity:F0}");
            
            Console.WriteLine("  Module breakdown:");
            foreach (var kvp in result.ModuleCounts)
            {
                Console.WriteLine($"    - {kvp.Key}: {kvp.Value}");
            }
            
            Console.WriteLine();
        }
    }
    
    private void DemonstrateDamageVisualization()
    {
        // Initialize module library and systems
        var library = new ModuleLibrary();
        library.InitializeBuiltInModules();
        
        var entityManager = new EntityManager();
        var damageSystem = new VoxelDamageSystem(entityManager);
        
        // Generate a test ship
        var generator = new ModularProceduralShipGenerator(library, seed: 54321);
        var config = new ModularShipConfig
        {
            ShipName = "Test Ship",
            Size = ShipSize.Fighter,
            Role = ShipRole.Combat,
            Material = "Iron"
        };
        
        var result = generator.GenerateShip(config);
        var ship = result.Ship;
        
        // Register ship with entity manager (create entity first)
        var entity = entityManager.CreateEntity(ship.Name);
        ship.EntityId = entity.Id;
        entityManager.AddComponent(ship.EntityId, ship);
        
        Console.WriteLine($"Ship: {ship.Name}");
        Console.WriteLine($"Initial Health: {ship.TotalHealth:F0} / {ship.MaxTotalHealth:F0}");
        Console.WriteLine($"Modules: {ship.Modules.Count}");
        
        // Apply damage to various modules
        Console.WriteLine("\nApplying damage to modules...");
        Console.WriteLine("(Voxels are automatically generated to show damage)");
        
        var wing = ship.Modules.FirstOrDefault(m => m.ModuleDefinitionId == "wing_basic");
        if (wing != null)
        {
            Console.WriteLine($"\nDamaging wing (ID: {wing.Id})");
            Console.WriteLine($"  Before: Health = {wing.Health:F0}, Damage Level = {wing.DamageLevel:F2}");
            
            damageSystem.ApplyDamageToModule(ship.EntityId, wing.Id, 50f);
            
            Console.WriteLine($"  After:  Health = {wing.Health:F0}, Damage Level = {wing.DamageLevel:F2}");
            
            // Check damage voxels
            var damageComp = entityManager.GetComponent<VoxelDamageComponent>(ship.EntityId);
            if (damageComp != null && damageComp.ModuleDamageMap.ContainsKey(wing.Id))
            {
                var damageVoxels = damageComp.ModuleDamageMap[wing.Id];
                Console.WriteLine($"  Damage Voxels: {damageVoxels.Count} voxels created to visualize damage");
                Console.WriteLine($"  (These voxels appear as 'holes' or 'broken sections' on the module)");
            }
        }
        
        var engine = ship.Modules.FirstOrDefault(m => m.ModuleDefinitionId.Contains("engine"));
        if (engine != null)
        {
            Console.WriteLine($"\nDamaging engine (ID: {engine.Id})");
            Console.WriteLine($"  Before: Health = {engine.Health:F0}, Damage Level = {engine.DamageLevel:F2}");
            
            damageSystem.ApplyDamageToModule(ship.EntityId, engine.Id, 75f);
            
            Console.WriteLine($"  After:  Health = {engine.Health:F0}, Damage Level = {engine.DamageLevel:F2}");
            
            // Check damage voxels
            var damageComp = entityManager.GetComponent<VoxelDamageComponent>(ship.EntityId);
            if (damageComp != null && damageComp.ModuleDamageMap.ContainsKey(engine.Id))
            {
                var damageVoxels = damageComp.ModuleDamageMap[engine.Id];
                Console.WriteLine($"  Damage Voxels: {damageVoxels.Count} voxels created to visualize damage");
            }
        }
        
        Console.WriteLine($"\nShip Health After Damage: {ship.TotalHealth:F0} / {ship.MaxTotalHealth:F0}");
        Console.WriteLine($"Ship Status: {(ship.IsDestroyed ? "DESTROYED" : "OPERATIONAL")}");
        
        // Demonstrate repair
        if (wing != null)
        {
            Console.WriteLine($"\nRepairing wing...");
            Console.WriteLine($"  Before Repair: Health = {wing.Health:F0}");
            
            damageSystem.RepairModule(ship.EntityId, wing.Id, 30f);
            
            Console.WriteLine($"  After Repair:  Health = {wing.Health:F0}, Damage Level = {wing.DamageLevel:F2}");
            Console.WriteLine($"  (Damage voxels are automatically updated/removed as module is repaired)");
        }
    }
    
    private void ExploreModuleLibrary()
    {
        // Initialize module library
        var library = new ModuleLibrary();
        library.InitializeBuiltInModules();
        
        Console.WriteLine($"Total modules in library: {library.AllDefinitions.Count}\n");
        
        // Group modules by category
        var categories = Enum.GetValues<ModuleCategory>();
        
        foreach (var category in categories)
        {
            var modules = library.GetDefinitionsByCategory(category);
            if (modules.Count > 0)
            {
                Console.WriteLine($"{category} Modules ({modules.Count}):");
                foreach (var module in modules)
                {
                    Console.WriteLine($"  - {module.Name} (ID: {module.Id})");
                    Console.WriteLine($"    Size: {module.Size.X}x{module.Size.Y}x{module.Size.Z}");
                    Console.WriteLine($"    Mass: {module.BaseMass:F0}, Health: {module.BaseHealth:F0}");
                    Console.WriteLine($"    Tech Level: {module.TechLevel}");
                    Console.WriteLine($"    Attachment Points: {module.AttachmentPoints.Count}");
                    
                    // Show key stats
                    if (module.BaseStats.ThrustPower > 0)
                        Console.WriteLine($"    Thrust: {module.BaseStats.ThrustPower:F0}");
                    if (module.BaseStats.PowerGeneration > 0)
                        Console.WriteLine($"    Power Gen: {module.BaseStats.PowerGeneration:F0}");
                    if (module.BaseStats.ShieldCapacity > 0)
                        Console.WriteLine($"    Shields: {module.BaseStats.ShieldCapacity:F0}");
                    if (module.BaseStats.WeaponMountPoints > 0)
                        Console.WriteLine($"    Weapon Mounts: {module.BaseStats.WeaponMountPoints}");
                    if (module.BaseStats.CargoCapacity > 0)
                        Console.WriteLine($"    Cargo: {module.BaseStats.CargoCapacity:F0}");
                    
                    Console.WriteLine();
                }
            }
        }
        
        // Show material comparison
        Console.WriteLine("\nMaterial Comparison (Power Core):");
        var powerCoreDef = library.GetDefinition("power_core_basic");
        if (powerCoreDef != null)
        {
            foreach (var material in MaterialProperties.GetAllMaterialNames())
            {
                var stats = powerCoreDef.GetStatsForMaterial(material);
                var health = powerCoreDef.GetHealthForMaterial(material);
                var mass = powerCoreDef.GetMassForMaterial(material);
                
                Console.WriteLine($"  {material}:");
                Console.WriteLine($"    Health: {health:F0}, Mass: {mass:F0}");
                Console.WriteLine($"    Power Generation: {stats.PowerGeneration:F0}");
            }
        }
    }
    
    private ShipModulePart CreateModule(string definitionId, Vector3 position, string material, ModuleLibrary library)
    {
        var def = library.GetDefinition(definitionId);
        if (def == null) throw new Exception($"Module definition not found: {definitionId}");
        
        var module = new ShipModulePart(definitionId, position, material)
        {
            MaxHealth = def.GetHealthForMaterial(material),
            Mass = def.GetMassForMaterial(material),
            FunctionalStats = def.GetStatsForMaterial(material)
        };
        module.Health = module.MaxHealth;
        
        return module;
    }
    
    private void DisplayShipStats(ModularShipComponent ship)
    {
        Console.WriteLine($"  Name: {ship.Name}");
        Console.WriteLine($"  Modules: {ship.Modules.Count}");
        Console.WriteLine($"  Mass: {ship.TotalMass:F0}");
        Console.WriteLine($"  Health: {ship.TotalHealth:F0} / {ship.MaxTotalHealth:F0}");
        Console.WriteLine($"  Center of Mass: {ship.CenterOfMass}");
        Console.WriteLine($"  Bounds: {ship.Bounds.Size}");
        Console.WriteLine($"\n  Stats:");
        Console.WriteLine($"    Thrust Power: {ship.AggregatedStats.ThrustPower:F0}");
        Console.WriteLine($"    Max Speed: {ship.AggregatedStats.MaxSpeed:F0}");
        Console.WriteLine($"    Power Gen: {ship.AggregatedStats.PowerGeneration:F0}");
        Console.WriteLine($"    Power Con: {ship.AggregatedStats.PowerConsumption:F0}");
        Console.WriteLine($"    Shield Cap: {ship.AggregatedStats.ShieldCapacity:F0}");
        Console.WriteLine($"    Shield Regen: {ship.AggregatedStats.ShieldRechargeRate:F0}");
        Console.WriteLine($"    Weapon Mounts: {ship.AggregatedStats.WeaponMountPoints}");
        Console.WriteLine($"    Cargo: {ship.AggregatedStats.CargoCapacity:F0}");
        Console.WriteLine($"    Crew Cap: {ship.AggregatedStats.CrewCapacity}");
        Console.WriteLine($"    Crew Req: {ship.AggregatedStats.CrewRequired}");
        Console.WriteLine($"    Sensor Range: {ship.AggregatedStats.SensorRange:F0}");
        Console.WriteLine($"    Hyperdrive: {(ship.AggregatedStats.HasHyperdrive ? $"Yes (Range: {ship.AggregatedStats.HyperdriveRange:F0})" : "No")}");
    }
}
