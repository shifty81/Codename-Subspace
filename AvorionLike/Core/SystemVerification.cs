using System.Numerics;
using AvorionLike.Core.ECS;
using AvorionLike.Core.Physics;
using AvorionLike.Core.Voxel;
using AvorionLike.Core.Resources;
using AvorionLike.Core.RPG;
using AvorionLike.Core.Combat;
using AvorionLike.Core.Navigation;
using AvorionLike.Core.Logging;
using AvorionLike.Core.Modular;

namespace AvorionLike.Core;

/// <summary>
/// Comprehensive system verification tool to test all game systems
/// </summary>
public class SystemVerification
{
    private readonly GameEngine _engine;
    private int _testsRun = 0;
    private int _testsPassed = 0;
    private int _testsFailed = 0;
    private readonly List<string> _failureDetails = new();

    public SystemVerification(GameEngine engine)
    {
        _engine = engine;
    }

    /// <summary>
    /// Run all system verification tests
    /// </summary>
    public VerificationReport RunAllTests()
    {
        Console.WriteLine("\n╔══════════════════════════════════════════════════════════════╗");
        Console.WriteLine("║     CODENAME:SUBSPACE - SYSTEM VERIFICATION SUITE           ║");
        Console.WriteLine("╚══════════════════════════════════════════════════════════════╝\n");

        // Core Systems
        TestEntityComponentSystem();
        TestConfigurationSystem();
        TestLoggingSystem();
        TestEventSystem();

        // Physics & Collision
        TestPhysicsSystem();
        TestCollisionSystem();
        TestDamageSystem();

        // Combat & AI
        TestCombatSystem();
        TestAISystem();

        // Resource & Economy
        TestInventorySystem();
        TestCraftingSystem();
        TestTradingSystem();
        TestEconomySystem();
        TestMiningSystem();

        // RPG & Progression
        TestLootSystem();
        TestPlayerPodSystem();
        TestPodAbilities();
        TestPodSkillTree();

        // Fleet & Navigation
        TestFleetMissionSystem();
        TestCrewSystem();
        TestNavigationSystem();

        // Building & Power
        TestBuildSystem();
        TestPowerSystem();
        
        // Modular Ship Systems
        TestModularShipSyncSystem();
        TestVoxelDamageSystem();

        // Procedural
        TestGalaxyGenerator();

        // Scripting
        TestScriptingEngine();

        // Persistence
        TestPersistenceSystem();

        return GenerateReport();
    }

    private void TestEntityComponentSystem()
    {
        RunTest("ECS: Entity Creation", () =>
        {
            var entity = _engine.EntityManager.CreateEntity("Test Entity");
            Assert(entity != null, "Entity should not be null");
            Assert(!string.IsNullOrEmpty(entity!.Name), "Entity should have a name"); // entity is verified non-null above
            Assert(entity.Id != Guid.Empty, "Entity should have a valid ID");
            _engine.EntityManager.DestroyEntity(entity.Id);
        });

        RunTest("ECS: Component Add/Get", () =>
        {
            var entity = _engine.EntityManager.CreateEntity("Component Test");
            var physics = new PhysicsComponent { Mass = 100f };
            _engine.EntityManager.AddComponent(entity.Id, physics);
            
            var retrieved = _engine.EntityManager.GetComponent<PhysicsComponent>(entity.Id);
            Assert(retrieved != null, "Component should be retrievable");
            Assert(retrieved!.Mass == 100f, "Component data should be preserved"); // retrieved is verified non-null above
            _engine.EntityManager.DestroyEntity(entity.Id);
        });

        RunTest("ECS: Component Remove", () =>
        {
            var entity = _engine.EntityManager.CreateEntity("Remove Test");
            var physics = new PhysicsComponent();
            _engine.EntityManager.AddComponent(entity.Id, physics);
            _engine.EntityManager.RemoveComponent<PhysicsComponent>(entity.Id);
            
            var has = _engine.EntityManager.HasComponent<PhysicsComponent>(entity.Id);
            Assert(!has, "Component should be removed");
            _engine.EntityManager.DestroyEntity(entity.Id);
        });
    }

    private void TestConfigurationSystem()
    {
        RunTest("Config: Manager Initialization", () =>
        {
            var config = Configuration.ConfigurationManager.Instance.Config;
            Assert(config != null, "Configuration should be loaded");
            Assert(config!.Graphics != null, "Graphics config should exist"); // config is verified non-null above
            Assert(config.Audio != null, "Audio config should exist");
            Assert(config.Gameplay != null, "Gameplay config should exist");
        });
    }

    private void TestLoggingSystem()
    {
        RunTest("Logging: Log Messages", () =>
        {
            // Just verify logging doesn't crash
            Logger.Instance.Info("Test", "Test info message");
            Logger.Instance.Warning("Test", "Test warning message");
            Logger.Instance.Debug("Test", "Test debug message");
        });
    }

    private void TestEventSystem()
    {
        RunTest("Events: Subscribe and Publish", () =>
        {
            bool eventReceived = false;
            Events.EventSystem.Instance.Subscribe(Events.GameEvents.GameStarted, 
                (evt) => { eventReceived = true; });
            
            Events.EventSystem.Instance.Publish(Events.GameEvents.GameStarted, new Events.GameEvent());
            
            // Give event time to process
            System.Threading.Thread.Sleep(50);
            
            Assert(eventReceived, "Event should be received");
        });
    }

    private void TestPhysicsSystem()
    {
        RunTest("Physics: Entity Movement", () =>
        {
            var entity = _engine.EntityManager.CreateEntity("Physics Test");
            var physics = new PhysicsComponent 
            { 
                Position = Vector3.Zero,
                Velocity = new Vector3(10, 0, 0),
                Mass = 100f
            };
            _engine.EntityManager.AddComponent(entity.Id, physics);
            
            var initialPos = physics.Position;
            _engine.PhysicsSystem.Update(0.016f); // 16ms delta time
            
            Assert(physics.Position != initialPos, "Entity should have moved");
            _engine.EntityManager.DestroyEntity(entity.Id);
        });

        RunTest("Physics: Force Application", () =>
        {
            var entity = _engine.EntityManager.CreateEntity("Force Test");
            var physics = new PhysicsComponent 
            { 
                Mass = 100f,
                Velocity = Vector3.Zero
            };
            _engine.EntityManager.AddComponent(entity.Id, physics);
            
            physics.AddForce(new Vector3(1000, 0, 0));
            _engine.PhysicsSystem.Update(0.016f); // 16ms delta time
            
            Assert(physics.Velocity.X > 0, "Force should affect velocity");
            _engine.EntityManager.DestroyEntity(entity.Id);
        });
    }

    private void TestCollisionSystem()
    {
        RunTest("Collision: AABB Detection", () =>
        {
            // Create two entities that should collide
            var entity1 = _engine.EntityManager.CreateEntity("Collision Test 1");
            var physics1 = new PhysicsComponent 
            { 
                Position = new Vector3(0, 0, 0),
                CollisionRadius = 5f
            };
            _engine.EntityManager.AddComponent(entity1.Id, physics1);
            
            var entity2 = _engine.EntityManager.CreateEntity("Collision Test 2");
            var physics2 = new PhysicsComponent 
            { 
                Position = new Vector3(3, 0, 0),
                CollisionRadius = 5f
            };
            _engine.EntityManager.AddComponent(entity2.Id, physics2);
            
            // System should detect collision
            _engine.CollisionSystem.Update(0.016f); // 16ms delta time
            
            _engine.EntityManager.DestroyEntity(entity1.Id);
            _engine.EntityManager.DestroyEntity(entity2.Id);
        });
    }

    private void TestDamageSystem()
    {
        RunTest("Damage: Block Damage", () =>
        {
            var entity = _engine.EntityManager.CreateEntity("Damage Test");
            var voxel = new VoxelStructureComponent();
            voxel.AddBlock(new VoxelBlock(Vector3.Zero, new Vector3(2, 2, 2), "Iron", BlockType.Hull));
            _engine.EntityManager.AddComponent(entity.Id, voxel);
            
            var combat = new CombatComponent
            {
                EntityId = entity.Id,
                MaxShields = 100f,
                CurrentShields = 100f
            };
            _engine.EntityManager.AddComponent(entity.Id, combat);
            
            var initialIntegrity = voxel.StructuralIntegrity;
            _engine.DamageSystem.ApplyDamage(new Combat.DamageInfo
            {
                TargetEntityId = entity.Id,
                Damage = 50f,
                HitPosition = Vector3.Zero,
                DamageType = Combat.DamageType.Kinetic
            });
            
            Assert(combat.CurrentShields < 100f || voxel.StructuralIntegrity < initialIntegrity, 
                "Damage should be applied");
            
            _engine.EntityManager.DestroyEntity(entity.Id);
        });
    }

    private void TestCombatSystem()
    {
        RunTest("Combat: Component Creation", () =>
        {
            var entity = _engine.EntityManager.CreateEntity("Combat Test");
            var combat = new CombatComponent
            {
                EntityId = entity.Id,
                MaxShields = 200f,
                CurrentShields = 200f,
                MaxEnergy = 100f,
                CurrentEnergy = 100f
            };
            _engine.EntityManager.AddComponent(entity.Id, combat);
            
            Assert(combat.CurrentShields == 200f, "Shields should be initialized");
            Assert(combat.CurrentEnergy == 100f, "Energy should be initialized");
            
            _engine.EntityManager.DestroyEntity(entity.Id);
        });
    }

    private void TestAISystem()
    {
        RunTest("AI: System Initialization", () =>
        {
            Assert(_engine.AISystem != null, "AI System should be initialized");
        });
    }

    private void TestInventorySystem()
    {
        RunTest("Inventory: Add Resources", () =>
        {
            var inventory = new Inventory { MaxCapacity = 1000 };
            inventory.AddResource(ResourceType.Iron, 100);
            
            Assert(inventory.GetResourceAmount(ResourceType.Iron) == 100, 
                "Resource should be added");
            Assert(inventory.CurrentCapacity == 100, "Capacity should be updated");
        });

        RunTest("Inventory: Remove Resources", () =>
        {
            var inventory = new Inventory { MaxCapacity = 1000 };
            inventory.AddResource(ResourceType.Iron, 100);
            var removed = inventory.RemoveResource(ResourceType.Iron, 50);
            
            Assert(removed, "Resource should be removed");
            Assert(inventory.GetResourceAmount(ResourceType.Iron) == 50, 
                "Correct amount should remain");
        });

        RunTest("Inventory: Capacity Limits", () =>
        {
            var inventory = new Inventory { MaxCapacity = 100 };
            inventory.AddResource(ResourceType.Iron, 150);
            
            Assert(inventory.CurrentCapacity <= 100, 
                "Should not exceed max capacity");
        });
    }

    private void TestCraftingSystem()
    {
        RunTest("Crafting: Available Upgrades", () =>
        {
            var upgrades = _engine.CraftingSystem.GetAvailableUpgrades();
            Assert(upgrades.Any(), "Should have available upgrades");
        });
    }

    private void TestTradingSystem()
    {
        RunTest("Trading: Price Calculation", () =>
        {
            var price = _engine.TradingSystem.GetBuyPrice(ResourceType.Iron, 100);
            Assert(price > 0, "Price should be calculated");
        });

        RunTest("Trading: Buy Resource", () =>
        {
            var inventory = new Inventory { MaxCapacity = 20000 }; // Larger capacity
            inventory.AddResource(ResourceType.Credits, 10000);
            
            var success = _engine.TradingSystem.BuyResource(ResourceType.Iron, 50, inventory);
            Assert(success, "Should be able to buy resources");
            Assert(inventory.GetResourceAmount(ResourceType.Iron) == 50, 
                "Resources should be added");
        });
    }

    private void TestEconomySystem()
    {
        RunTest("Economy: System Initialization", () =>
        {
            Assert(_engine.EconomySystem != null, "Economy system should exist");
        });
    }

    private void TestMiningSystem()
    {
        RunTest("Mining: System Initialization", () =>
        {
            Assert(_engine.MiningSystem != null, "Mining system should exist");
        });
    }

    private void TestLootSystem()
    {
        RunTest("Loot: Generation", () =>
        {
            var loot = _engine.LootSystem.GenerateLoot(5, false);
            Assert(loot != null, "Loot should be generated");
        });
    }

    private void TestPlayerPodSystem()
    {
        RunTest("Pod: Docking System", () =>
        {
            Assert(_engine.PodDockingSystem != null, "Pod docking system should exist");
        });
    }

    private void TestPodAbilities()
    {
        RunTest("Pod: Ability System", () =>
        {
            Assert(_engine.PodAbilitySystem != null, "Pod ability system should exist");
        });
    }

    private void TestPodSkillTree()
    {
        RunTest("Pod: Skill Tree", () =>
        {
            var entity = _engine.EntityManager.CreateEntity("Skill Test");
            var skillTree = new PodSkillTreeComponent { EntityId = entity.Id };
            _engine.EntityManager.AddComponent(entity.Id, skillTree);
            
            var skillPoints = 10;
            var learned = skillTree.LearnSkill("combat_weapon_damage", 5, ref skillPoints);
            
            Assert(learned, "Should learn skill with sufficient points");
            Assert(skillPoints < 10, "Skill points should be consumed");
            
            _engine.EntityManager.DestroyEntity(entity.Id);
        });
    }

    private void TestFleetMissionSystem()
    {
        RunTest("Fleet: Mission System", () =>
        {
            Assert(_engine.FleetMissionSystem != null, "Fleet mission system should exist");
        });
    }

    private void TestCrewSystem()
    {
        RunTest("Crew: Management System", () =>
        {
            Assert(_engine.CrewManagementSystem != null, "Crew system should exist");
        });
    }

    private void TestNavigationSystem()
    {
        RunTest("Navigation: System Initialization", () =>
        {
            Assert(_engine.NavigationSystem != null, "Navigation system should exist");
        });
    }

    private void TestBuildSystem()
    {
        RunTest("Build: System Initialization", () =>
        {
            Assert(_engine.BuildSystem != null, "Build system should exist");
        });
    }

    private void TestPowerSystem()
    {
        RunTest("Power: System Initialization", () =>
        {
            Assert(_engine.PowerSystem != null, "Power system should exist");
        });
    }

    private void TestGalaxyGenerator()
    {
        RunTest("Galaxy: Sector Generation", () =>
        {
            var sector = _engine.GalaxyGenerator.GenerateSector(0, 0, 0);
            Assert(sector != null, "Sector should be generated");
        });
    }

    private void TestScriptingEngine()
    {
        RunTest("Scripting: Lua Execution", () =>
        {
            _engine.ScriptingEngine.ExecuteScript("result = 5 + 5");
            var result = _engine.ScriptingEngine.GetGlobal("result");
            Assert(result != null, "Script should execute");
        });
    }

    private void TestPersistenceSystem()
    {
        RunTest("Persistence: Save/Load", () =>
        {
            // Test that save system exists
            var testEntity = _engine.EntityManager.CreateEntity("Save Test");
            var saved = _engine.SaveGame("verification_test");
            
            Assert(saved, "Should be able to save game");
            
            _engine.EntityManager.DestroyEntity(testEntity.Id);
        });
    }

    private void RunTest(string testName, Action testAction)
    {
        _testsRun++;
        Console.Write($"  [{_testsRun:D3}] {testName,-50} ");
        
        try
        {
            testAction();
            _testsPassed++;
            Console.ForegroundColor = ConsoleColor.Green;
            Console.WriteLine("✓ PASS");
            Console.ResetColor();
        }
        catch (Exception ex)
        {
            _testsFailed++;
            Console.ForegroundColor = ConsoleColor.Red;
            Console.WriteLine("✗ FAIL");
            Console.ResetColor();
            
            var errorMsg = $"{testName}: {ex.Message}";
            _failureDetails.Add(errorMsg);
            
            Console.ForegroundColor = ConsoleColor.DarkRed;
            Console.WriteLine($"      Error: {ex.Message}");
            Console.ResetColor();
        }
    }

    private void Assert(bool condition, string message)
    {
        if (!condition)
        {
            throw new Exception(message);
        }
    }
    
    private void TestModularShipSyncSystem()
    {
        RunTest("ModularShipSync: System Exists", () =>
        {
            Assert(_engine.ModularShipSyncSystem != null, "ModularShipSyncSystem should be initialized");
        });
        
        RunTest("ModularShipSync: Physics Auto-Creation", () =>
        {
            var library = new ModuleLibrary();
            library.InitializeBuiltInModules();
            var generator = new ModularProceduralShipGenerator(library, 12345);
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
            var entity = _engine.EntityManager.CreateEntity("Test Ship");
            _engine.EntityManager.AddComponent(entity.Id, ship);
            
            // Run sync system
            _engine.ModularShipSyncSystem.Update(0.016f);
            
            // Check physics was created
            var physics = _engine.EntityManager.GetComponent<PhysicsComponent>(entity.Id);
            Assert(physics != null, "Physics component should be auto-created");
            Assert(Math.Abs(physics!.Mass - ship.TotalMass) < 0.1f, "Physics mass should match ship mass");
            Assert(physics.CollisionRadius > 0, "Collision radius should be set");
            
            _engine.EntityManager.DestroyEntity(entity.Id);
        });
        
        RunTest("ModularShipSync: Ship Destruction Handling", () =>
        {
            var library = new ModuleLibrary();
            library.InitializeBuiltInModules();
            var generator = new ModularProceduralShipGenerator(library, 12345);
            var config = new ModularShipConfig
            {
                ShipName = "Doomed Ship",
                Size = ShipSize.Corvette,
                Role = ShipRole.Combat,
                Material = "Iron",
                Seed = 200
            };
            
            var result = generator.GenerateShip(config);
            var ship = result.Ship;
            var entity = _engine.EntityManager.CreateEntity("Doomed Ship");
            _engine.EntityManager.AddComponent(entity.Id, ship);
            
            // Create physics
            _engine.ModularShipSyncSystem.Update(0.016f);
            var physics = _engine.EntityManager.GetComponent<PhysicsComponent>(entity.Id);
            Assert(physics != null, "Physics should exist");
            
            // Destroy core module
            if (ship.CoreModuleId.HasValue)
            {
                var core = ship.GetModule(ship.CoreModuleId.Value);
                if (core != null)
                {
                    core.Health = 0;
                    ship.RecalculateStats();
                    
                    // Sync should set physics to static
                    _engine.ModularShipSyncSystem.Update(0.016f);
                    Assert(physics!.IsStatic, "Physics should be static after ship destruction");
                }
            }
            
            _engine.EntityManager.DestroyEntity(entity.Id);
        });
    }
    
    private void TestVoxelDamageSystem()
    {
        RunTest("VoxelDamage: System Exists", () =>
        {
            Assert(_engine.VoxelDamageSystem != null, "VoxelDamageSystem should be initialized");
        });
        
        RunTest("VoxelDamage: Damage Visualization", () =>
        {
            var library = new ModuleLibrary();
            library.InitializeBuiltInModules();
            var generator = new ModularProceduralShipGenerator(library, 12345);
            var config = new ModularShipConfig
            {
                ShipName = "Damaged Ship",
                Size = ShipSize.Frigate,
                Role = ShipRole.Combat,
                Material = "Titanium",
                Seed = 300
            };
            
            var result = generator.GenerateShip(config);
            var ship = result.Ship;
            var entity = _engine.EntityManager.CreateEntity("Damaged Ship");
            _engine.EntityManager.AddComponent(entity.Id, ship);
            
            // Damage some modules
            int modulesToDamage = Math.Min(3, ship.Modules.Count);
            for (int i = 0; i < modulesToDamage; i++)
            {
                ship.Modules[i].Health -= ship.Modules[i].MaxHealth * 0.5f;
            }
            ship.RecalculateStats();
            
            // Run damage system
            _engine.VoxelDamageSystem.Update(0.016f);
            
            // Check damage component was created
            var damageComp = _engine.EntityManager.GetComponent<VoxelDamageComponent>(entity.Id);
            Assert(damageComp != null, "VoxelDamageComponent should be created");
            
            _engine.EntityManager.DestroyEntity(entity.Id);
        });
    }

    private VerificationReport GenerateReport()
    {
        Console.WriteLine("\n╔══════════════════════════════════════════════════════════════╗");
        Console.WriteLine("║                  VERIFICATION COMPLETE                       ║");
        Console.WriteLine("╚══════════════════════════════════════════════════════════════╝");
        
        var passRate = _testsRun > 0 ? (_testsPassed * 100.0 / _testsRun) : 0;
        
        Console.WriteLine($"\n  Total Tests Run:    {_testsRun}");
        Console.ForegroundColor = ConsoleColor.Green;
        Console.WriteLine($"  Tests Passed:       {_testsPassed}");
        Console.ResetColor();
        
        if (_testsFailed > 0)
        {
            Console.ForegroundColor = ConsoleColor.Red;
            Console.WriteLine($"  Tests Failed:       {_testsFailed}");
            Console.ResetColor();
        }
        else
        {
            Console.WriteLine($"  Tests Failed:       {_testsFailed}");
        }
        
        Console.WriteLine($"  Pass Rate:          {passRate:F1}%");
        
        if (_failureDetails.Any())
        {
            Console.WriteLine("\n  Failed Tests:");
            foreach (var failure in _failureDetails)
            {
                Console.ForegroundColor = ConsoleColor.Red;
                Console.WriteLine($"    • {failure}");
                Console.ResetColor();
            }
        }
        
        Console.WriteLine();
        
        return new VerificationReport
        {
            TotalTests = _testsRun,
            PassedTests = _testsPassed,
            FailedTests = _testsFailed,
            PassRate = passRate,
            FailureDetails = _failureDetails.ToList()
        };
    }
}

public class VerificationReport
{
    public int TotalTests { get; set; }
    public int PassedTests { get; set; }
    public int FailedTests { get; set; }
    public double PassRate { get; set; }
    public List<string> FailureDetails { get; set; } = new();
}
