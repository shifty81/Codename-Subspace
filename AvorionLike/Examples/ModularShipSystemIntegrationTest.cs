using System.Numerics;
using AvorionLike.Core;
using AvorionLike.Core.ECS;
using AvorionLike.Core.Modular;
using AvorionLike.Core.Physics;
using AvorionLike.Core.Logging;

namespace AvorionLike.Examples;

/// <summary>
/// Test to verify that modular ship systems are properly integrated with the game engine
/// Tests ModularShipSyncSystem and VoxelDamageSystem
/// </summary>
public class ModularShipSystemIntegrationTest
{
    private readonly Logger _logger = Logger.Instance;
    
    public void RunTest()
    {
        Console.WriteLine("\n" + new string('=', 70));
        Console.WriteLine("      MODULAR SHIP SYSTEM INTEGRATION TEST");
        Console.WriteLine(new string('=', 70));
        Console.WriteLine();
        
        // Test 1: System Registration
        TestSystemRegistration();
        
        // Test 2: Physics Synchronization
        TestPhysicsSynchronization();
        
        // Test 3: Damage Visualization
        TestDamageVisualization();
        
        // Test 4: Ship Destruction Handling
        TestShipDestruction();
        
        Console.WriteLine();
        Console.WriteLine(new string('=', 70));
        Console.WriteLine("All tests completed!");
        Console.WriteLine(new string('=', 70));
    }
    
    private void TestSystemRegistration()
    {
        Console.WriteLine("TEST 1: System Registration");
        Console.WriteLine("----------------------------");
        
        var engine = new GameEngine(12345);
        
        // Verify systems are created
        var syncSystemExists = engine.ModularShipSyncSystem != null;
        var damageSystemExists = engine.VoxelDamageSystem != null;
        
        Console.WriteLine($"  ModularShipSyncSystem registered: {(syncSystemExists ? "✓ PASS" : "✗ FAIL")}");
        Console.WriteLine($"  VoxelDamageSystem registered: {(damageSystemExists ? "✓ PASS" : "✗ FAIL")}");
        Console.WriteLine();
    }
    
    private void TestPhysicsSynchronization()
    {
        Console.WriteLine("TEST 2: Physics Synchronization");
        Console.WriteLine("--------------------------------");
        
        // Use same tolerance as ModularShipSyncSystem for consistency
        const float MassSyncTolerance = 0.1f; // Matches ModularShipSyncSystem.MassSyncThreshold
        
        var engine = new GameEngine(12345);
        var library = new ModuleLibrary();
        library.InitializeBuiltInModules();
        
        var generator = new ModularProceduralShipGenerator(library, 42);
        
        // Generate a test ship
        var config = new ModularShipConfig
        {
            ShipName = "Test Ship",
            Size = ShipSize.Corvette,
            Role = ShipRole.Combat,
            Material = "Iron",
            Seed = 100
        };
        
        var result = generator.GenerateShip(config);
        var ship = result.Ship;
        
        // Create an entity for the ship
        var entity = engine.EntityManager.CreateEntity("Test Ship");
        engine.EntityManager.AddComponent(entity.Id, ship);
        
        Console.WriteLine($"  Created ship with {ship.Modules.Count} modules");
        Console.WriteLine($"  Ship mass: {ship.TotalMass:F2} kg");
        
        // Run the sync system
        engine.ModularShipSyncSystem.Update(0.016f);
        
        // Check if physics component was created
        var physics = engine.EntityManager.GetComponent<PhysicsComponent>(entity.Id);
        var physicsCreated = physics != null;
        
        Console.WriteLine($"  Physics component created: {(physicsCreated ? "✓ PASS" : "✗ FAIL")}");
        
        if (physics != null)
        {
            var massMatches = Math.Abs(physics.Mass - ship.TotalMass) < MassSyncTolerance;
            var hasCollisionRadius = physics.CollisionRadius > 0;
            
            Console.WriteLine($"  Physics mass matches ship mass: {(massMatches ? "✓ PASS" : "✗ FAIL")}");
            Console.WriteLine($"    - Ship mass: {ship.TotalMass:F2}");
            Console.WriteLine($"    - Physics mass: {physics.Mass:F2}");
            Console.WriteLine($"  Collision radius set: {(hasCollisionRadius ? "✓ PASS" : "✗ FAIL")}");
            Console.WriteLine($"    - Collision radius: {physics.CollisionRadius:F2}");
        }
        
        Console.WriteLine();
    }
    
    private void TestDamageVisualization()
    {
        Console.WriteLine("TEST 3: Damage Visualization");
        Console.WriteLine("-----------------------------");
        
        var engine = new GameEngine(12345);
        var library = new ModuleLibrary();
        library.InitializeBuiltInModules();
        
        var generator = new ModularProceduralShipGenerator(library, 42);
        
        // Generate a test ship
        var config = new ModularShipConfig
        {
            ShipName = "Damaged Ship",
            Size = ShipSize.Frigate,
            Role = ShipRole.Combat,
            Material = "Titanium",
            Seed = 200
        };
        
        var result = generator.GenerateShip(config);
        var ship = result.Ship;
        
        // Create an entity for the ship
        var entity = engine.EntityManager.CreateEntity("Damaged Ship");
        engine.EntityManager.AddComponent(entity.Id, ship);
        
        // Damage some modules
        int modulesToDamage = Math.Min(3, ship.Modules.Count);
        for (int i = 0; i < modulesToDamage; i++)
        {
            var module = ship.Modules[i];
            module.Health -= module.MaxHealth * 0.5f; // 50% damage
        }
        
        ship.RecalculateStats();
        
        Console.WriteLine($"  Damaged {modulesToDamage} modules to 50% health");
        
        // Run the damage visualization system
        engine.VoxelDamageSystem.Update(0.016f);
        
        // Check if damage component was created
        var damageComponent = engine.EntityManager.GetComponent<VoxelDamageComponent>(entity.Id);
        var damageComponentExists = damageComponent != null;
        
        Console.WriteLine($"  VoxelDamageComponent created: {(damageComponentExists ? "✓ PASS" : "✗ FAIL")}");
        
        if (damageComponent != null)
        {
            var hasDamageVoxels = damageComponent.DamageVoxels.Count > 0;
            var hasModuleDamageMap = damageComponent.ModuleDamageMap.Count > 0;
            
            Console.WriteLine($"  Damage voxels generated: {(hasDamageVoxels ? "✓ PASS" : "✗ FAIL")}");
            Console.WriteLine($"    - Voxel count: {damageComponent.DamageVoxels.Count}");
            Console.WriteLine($"  Module damage mapping created: {(hasModuleDamageMap ? "✓ PASS" : "✗ FAIL")}");
            Console.WriteLine($"    - Mapped modules: {damageComponent.ModuleDamageMap.Count}");
        }
        
        Console.WriteLine();
    }
    
    private void TestShipDestruction()
    {
        Console.WriteLine("TEST 4: Ship Destruction Handling");
        Console.WriteLine("----------------------------------");
        
        var engine = new GameEngine(12345);
        var library = new ModuleLibrary();
        library.InitializeBuiltInModules();
        
        var generator = new ModularProceduralShipGenerator(library, 42);
        
        // Generate a test ship
        var config = new ModularShipConfig
        {
            ShipName = "Doomed Ship",
            Size = ShipSize.Corvette,
            Role = ShipRole.Exploration,
            Material = "Iron",
            Seed = 300
        };
        
        var result = generator.GenerateShip(config);
        var ship = result.Ship;
        
        // Create an entity for the ship
        var entity = engine.EntityManager.CreateEntity("Doomed Ship");
        engine.EntityManager.AddComponent(entity.Id, ship);
        
        // Create physics component first
        engine.ModularShipSyncSystem.Update(0.016f);
        
        var physics = engine.EntityManager.GetComponent<PhysicsComponent>(entity.Id);
        
        if (physics != null)
        {
            var wasStatic = physics.IsStatic;
            
            // Destroy the core module
            if (ship.CoreModuleId.HasValue)
            {
                var coreModule = ship.GetModule(ship.CoreModuleId.Value);
                if (coreModule != null)
                {
                    coreModule.Health = 0;
                    ship.RecalculateStats();
                    
                    Console.WriteLine($"  Core module destroyed");
                    Console.WriteLine($"  Ship destroyed: {ship.IsDestroyed}");
                    
                    // Run sync system to update physics
                    engine.ModularShipSyncSystem.Update(0.016f);
                    
                    var nowStatic = physics.IsStatic;
                    var velocityZero = physics.Velocity.Length() < 0.01f;
                    
                    Console.WriteLine($"  Physics set to static after destruction: {(nowStatic && !wasStatic ? "✓ PASS" : "✗ FAIL")}");
                    Console.WriteLine($"  Velocity cleared: {(velocityZero ? "✓ PASS" : "✗ FAIL")}");
                }
            }
        }
        else
        {
            Console.WriteLine("  ✗ FAIL - Physics component not created");
        }
        
        Console.WriteLine();
    }
}
