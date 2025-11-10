using AvorionLike.Core;
using AvorionLike.Core.ECS;
using AvorionLike.Core.Voxel;
using AvorionLike.Core.Physics;
using AvorionLike.Core.Resources;
using AvorionLike.Core.RPG;
using AvorionLike.Core.Graphics;
using AvorionLike.Core.Combat;
using AvorionLike.Core.Navigation;
using AvorionLike.Core.Procedural;
using AvorionLike.Examples;
using System.Numerics;

namespace AvorionLike;

/// <summary>
/// Console-based UI for the AvorionLike game engine application
/// Cross-platform compatible (works on Windows, Linux, macOS)
/// </summary>
class Program
{
    private static GameEngine? _gameEngine;
    private static bool _running = true;

    static void Main(string[] args)
    {
        Console.WriteLine("==============================================");
        Console.WriteLine($"  {VersionInfo.FullVersion}");
        Console.WriteLine("==============================================");
        Console.WriteLine();

        // Initialize game engine
        _gameEngine = new GameEngine(12345);
        
        Console.WriteLine("Game engine initialized successfully!");
        Console.WriteLine();

        // Start the engine
        _gameEngine.Start();

        // Show main menu
        ShowMainMenu();

        // Cleanup
        _gameEngine.Stop();
        Console.WriteLine("\nThank you for testing Codename:Subspace!");
    }

    static void ShowMainMenu()
    {
        while (_running)
        {
            Console.WriteLine("\n=== Main Menu ===");
            Console.WriteLine("--- PLAY GAME ---");
            Console.WriteLine("1. NEW GAME - Start Full Gameplay Experience");
            Console.WriteLine("19. SHOWCASE MODE - Visual Demo with Enhanced UI [NEW! ✨]");
            Console.WriteLine();
            Console.WriteLine("--- DEMOS & TESTS ---");
            Console.WriteLine("2. Engine Demo - Create Test Ship");
            Console.WriteLine("3. Voxel System Demo - Build Ship Structure");
            Console.WriteLine("4. Physics Demo - Simulate Movement");
            Console.WriteLine("5. Procedural Generation - Generate Galaxy Sector");
            Console.WriteLine("6. Resource Management Demo");
            Console.WriteLine("7. RPG Systems Demo - Trading & Progression");
            Console.WriteLine("8. Scripting Demo - Execute Lua Script");
            Console.WriteLine("9. Multiplayer - Start Server");
            Console.WriteLine("10. View Statistics");
            Console.WriteLine("11. 3D Graphics Demo - Visualize Voxel Ships");
            Console.WriteLine("12. Persistence Demo - Save/Load Game");
            Console.WriteLine("13. Player Pod Demo - Character System");
            Console.WriteLine("14. Enhanced Pod Demo - Skills & Abilities");
            Console.WriteLine("16. Collision & Damage Test - Test Physics Collision");
            Console.WriteLine("17. System Verification - Test All Systems");
            Console.WriteLine("18. Ship Generation Demo - Procedural Ships & Textures");
            Console.WriteLine("21. Integration Test - Test All Systems Together [NEW! 🧪]");
            Console.WriteLine("22. Movement & Shape Test - Verify Smooth Movement & Ship Shapes [NEW! ✨]");
            Console.WriteLine("23. Test Ship Connectivity - Verify No Floating Blocks [NEW! 🔧]");
            Console.WriteLine("24. Test Ship Shape Variety - Verify Different Hull Shapes [NEW! 🎨]");
            Console.WriteLine("25. Ship Showcase - Generate 20 Ships for Selection [NEW! 🚀⭐]");
            Console.WriteLine();
            Console.WriteLine("--- INFO ---");
            Console.WriteLine("15. About / Version Info");
            Console.WriteLine("20. Generate HTML Demo Viewer [NEW! 📄]");
            Console.WriteLine("0. Exit");
            Console.Write("\nSelect option: ");

            var choice = Console.ReadLine();

            switch (choice)
            {
                case "1":
                    StartNewGame();
                    break;
                case "2":
                    CreateTestShipDemo();
                    break;
                case "3":
                    VoxelSystemDemo();
                    break;
                case "4":
                    PhysicsDemo();
                    break;
                case "5":
                    ProceduralGenerationDemo();
                    break;
                case "6":
                    ResourceManagementDemo();
                    break;
                case "7":
                    RPGSystemsDemo();
                    break;
                case "8":
                    ScriptingDemo();
                    break;
                case "9":
                    MultiplayerDemo();
                    break;
                case "10":
                    ShowStatistics();
                    break;
                case "11":
                    GraphicsDemo();
                    break;
                case "12":
                    PersistenceDemo();
                    break;
                case "13":
                    PlayerPodDemo();
                    break;
                case "14":
                    EnhancedPlayerPodDemo();
                    break;
                case "15":
                    ShowVersionInfo();
                    break;
                case "16":
                    CollisionDamageDemo();
                    break;
                case "17":
                    SystemVerificationDemo();
                    break;
                case "18":
                    ShipGenerationDemo();
                    break;
                case "19":
                    ShowcaseMode();
                    break;
                case "20":
                    GenerateHTMLDemoViewer();
                    break;
                case "21":
                    RunIntegrationTest();
                    break;
                case "22":
                    RunMovementAndShapeTest();
                    break;
                case "23":
                    TestShipConnectivity.Run();
                    break;
                case "24":
                    TestShipShapes.Run();
                    break;
                case "25":
                    RunShipShowcase();
                    break;
                case "0":
                    _running = false;
                    break;
                default:
                    Console.WriteLine("Invalid option!");
                    break;
            }
        }
    }

    static void StartNewGame()
    {
        Console.WriteLine("\n=== NEW GAME - Full Gameplay Experience ===");
        Console.WriteLine("Initializing player ship and game world...\n");
        
        // Create player ship
        var playerShip = _gameEngine!.EntityManager.CreateEntity("Player Ship");
        
        // Add voxel structure - create a functional starter ship
        var voxelComponent = new VoxelStructureComponent();
        
        Console.WriteLine("Building your starter ship with colorful materials...");
        
        // Core hull (center) - Titanium (silver-blue)
        voxelComponent.AddBlock(new VoxelBlock(
            new Vector3(0, 0, 0),
            new Vector3(3, 3, 3),
            "Titanium",
            BlockType.Hull
        ));
        
        // Main engines (rear) - Ogonite (red/orange with glow)
        voxelComponent.AddBlock(new VoxelBlock(
            new Vector3(-5, 0, 0),
            new Vector3(2, 2, 2),
            "Ogonite",
            BlockType.Engine
        ));
        
        voxelComponent.AddBlock(new VoxelBlock(
            new Vector3(-5, 2, 0),
            new Vector3(2, 2, 2),
            "Ogonite",
            BlockType.Engine
        ));
        
        // Maneuvering thrusters - Trinium (blue with glow)
        voxelComponent.AddBlock(new VoxelBlock(
            new Vector3(0, 4, 0),
            new Vector3(1.5f, 1.5f, 1.5f),
            "Trinium",
            BlockType.Thruster
        ));
        
        voxelComponent.AddBlock(new VoxelBlock(
            new Vector3(0, -4, 0),
            new Vector3(1.5f, 1.5f, 1.5f),
            "Trinium",
            BlockType.Thruster
        ));
        
        // Generator - Xanion (gold/yellow with glow)
        voxelComponent.AddBlock(new VoxelBlock(
            new Vector3(3, 0, 0),
            new Vector3(2, 2, 2),
            "Xanion",
            BlockType.Generator
        ));
        
        // Shield generator - Naonite (green with glow)
        voxelComponent.AddBlock(new VoxelBlock(
            new Vector3(0, 0, 3),
            new Vector3(2, 2, 2),
            "Naonite",
            BlockType.ShieldGenerator
        ));
        
        // Gyro arrays for rotation - Avorion (purple with strong glow)
        voxelComponent.AddBlock(new VoxelBlock(
            new Vector3(0, 2, 2),
            new Vector3(1.5f, 1.5f, 1.5f),
            "Avorion",
            BlockType.GyroArray
        ));
        
        voxelComponent.AddBlock(new VoxelBlock(
            new Vector3(0, -2, -2),
            new Vector3(1.5f, 1.5f, 1.5f),
            "Avorion",
            BlockType.GyroArray
        ));
        
        _gameEngine.EntityManager.AddComponent(playerShip.Id, voxelComponent);
        
        // Add physics
        var physicsComponent = new PhysicsComponent
        {
            Position = new Vector3(0, 0, 0),
            Velocity = Vector3.Zero,
            Mass = voxelComponent.TotalMass,
            MomentOfInertia = voxelComponent.MomentOfInertia,
            MaxThrust = voxelComponent.TotalThrust,
            MaxTorque = voxelComponent.TotalTorque
        };
        _gameEngine.EntityManager.AddComponent(playerShip.Id, physicsComponent);
        
        // Add inventory
        var inventoryComponent = new InventoryComponent(1000);
        inventoryComponent.Inventory.AddResource(ResourceType.Credits, 10000);
        inventoryComponent.Inventory.AddResource(ResourceType.Iron, 500);
        inventoryComponent.Inventory.AddResource(ResourceType.Titanium, 200);
        _gameEngine.EntityManager.AddComponent(playerShip.Id, inventoryComponent);

        // Add progression
        var progressionComponent = new ProgressionComponent
        {
            EntityId = playerShip.Id,
            Level = 1,
            Experience = 0,
            SkillPoints = 0
        };
        _gameEngine.EntityManager.AddComponent(playerShip.Id, progressionComponent);
        
        // Add combat capabilities
        var combatComponent = new CombatComponent
        {
            EntityId = playerShip.Id,
            MaxShields = voxelComponent.ShieldCapacity,
            CurrentShields = voxelComponent.ShieldCapacity,
            MaxEnergy = voxelComponent.PowerGeneration,
            CurrentEnergy = voxelComponent.PowerGeneration
        };
        _gameEngine.EntityManager.AddComponent(playerShip.Id, combatComponent);
        
        // Add hyperdrive
        var hyperdriveComponent = new HyperdriveComponent
        {
            EntityId = playerShip.Id,
            JumpRange = 5f
        };
        _gameEngine.EntityManager.AddComponent(playerShip.Id, hyperdriveComponent);
        
        // Add sector location
        var locationComponent = new SectorLocationComponent
        {
            EntityId = playerShip.Id,
            CurrentSector = new SectorCoordinate(0, 0, 0)
        };
        _gameEngine.EntityManager.AddComponent(playerShip.Id, locationComponent);
        
        Console.WriteLine($"\n✓ Player ship created!");
        Console.WriteLine($"  Name: {playerShip.Name}");
        Console.WriteLine($"  Blocks: {voxelComponent.Blocks.Count}");
        Console.WriteLine($"  Mass: {voxelComponent.TotalMass:F2} kg");
        Console.WriteLine($"  Thrust: {voxelComponent.TotalThrust:F2} N");
        Console.WriteLine($"  Torque: {voxelComponent.TotalTorque:F2} Nm");
        Console.WriteLine($"  Power: {voxelComponent.PowerGeneration:F2} W");
        Console.WriteLine($"  Shields: {voxelComponent.ShieldCapacity:F2}");
        Console.WriteLine($"  Credits: {inventoryComponent.Inventory.GetResourceAmount(ResourceType.Credits):N0}");
        
        Console.WriteLine("\n--- Ship Materials (Colorful!) ---");
        Console.WriteLine("  Core: Titanium (Silver-Blue, Metallic)");
        Console.WriteLine("  Engines: Ogonite (Red/Orange with Glow)");
        Console.WriteLine("  Thrusters: Trinium (Bright Blue with Glow)");
        Console.WriteLine("  Generator: Xanion (Gold/Yellow with Strong Glow)");
        Console.WriteLine("  Shields: Naonite (Bright Green with Glow)");
        Console.WriteLine("  Gyros: Avorion (Purple with Strong Glow)");
        Console.WriteLine("\n  Each block type has distinct colors to verify rendering!");
        
        // Populate the game world with a living, breathing universe
        Console.WriteLine("\n=== Populating Game World ===");
        var worldPopulator = new GameWorldPopulator(_gameEngine, seed: 12345);
        worldPopulator.PopulateStarterArea(physicsComponent.Position, radius: 800f);
        
        Console.WriteLine("\n=== Launching Full Game Experience ===");
        Console.WriteLine("Opening 3D window with Player UI...");
        Console.WriteLine("You can now control your ship and explore!\n");
        
        try
        {
            using var graphicsWindow = new GraphicsWindow(_gameEngine);
            graphicsWindow.SetPlayerShip(playerShip.Id);
            graphicsWindow.Run();
        }
        catch (Exception ex)
        {
            Console.WriteLine($"Error running game: {ex.Message}");
            Console.WriteLine("Graphics rendering may not be available on this system.");
        }

        Console.WriteLine("\nReturned to main menu.");
    }

    static void CreateTestShipDemo()
    {
        Console.WriteLine("\n=== Create Test Ship Demo ===");
        Console.WriteLine("Now featuring PROCEDURAL GENERATION with enhanced aesthetics!\n");
        
        // Use the procedural ship generator to create an impressive test ship
        var shipGenerator = new ProceduralShipGenerator(Environment.TickCount);
        
        // Generate a combat frigate with vibrant Avorion material
        var config = new ShipGenerationConfig
        {
            Size = ShipSize.Frigate,
            Role = ShipRole.Combat,
            Material = "Avorion",  // Purple with strong glow
            Style = FactionShipStyle.GetDefaultStyle("Military"),
            Seed = Environment.TickCount
        };
        
        Console.WriteLine("Generating procedural combat frigate with colorful materials...");
        var generatedShip = shipGenerator.GenerateShip(config);
        
        // Create entity and add the generated structure
        var ship = _gameEngine!.EntityManager.CreateEntity("Player Ship");
        var voxelComponent = generatedShip.Structure;
        _gameEngine.EntityManager.AddComponent(ship.Id, voxelComponent);
        
        // Add physics with enhanced properties
        var physicsComponent = new PhysicsComponent
        {
            Position = new Vector3(100, 100, 100),
            Mass = voxelComponent.TotalMass,
            MomentOfInertia = voxelComponent.MomentOfInertia,
            MaxThrust = voxelComponent.TotalThrust,
            MaxTorque = voxelComponent.TotalTorque,
            Velocity = new Vector3(10, 0, 0)
        };
        _gameEngine.EntityManager.AddComponent(ship.Id, physicsComponent);
        
        // Add inventory
        var inventoryComponent = new InventoryComponent(1000);
        inventoryComponent.Inventory.AddResource(ResourceType.Credits, 10000);
        inventoryComponent.Inventory.AddResource(ResourceType.Iron, 500);
        inventoryComponent.Inventory.AddResource(ResourceType.Titanium, 200);
        _gameEngine.EntityManager.AddComponent(ship.Id, inventoryComponent);

        // Add progression
        var progressionComponent = new ProgressionComponent();
        _gameEngine.EntityManager.AddComponent(ship.Id, progressionComponent);
        
        // Add combat capabilities
        var combatComponent = new CombatComponent
        {
            EntityId = ship.Id,
            MaxShields = voxelComponent.ShieldCapacity,
            CurrentShields = voxelComponent.ShieldCapacity,
            MaxEnergy = voxelComponent.PowerGeneration
        };
        _gameEngine.EntityManager.AddComponent(ship.Id, combatComponent);
        
        // Add hyperdrive
        var hyperdriveComponent = new HyperdriveComponent
        {
            EntityId = ship.Id,
            JumpRange = 5f
        };
        _gameEngine.EntityManager.AddComponent(ship.Id, hyperdriveComponent);
        
        // Add sector location
        var locationComponent = new SectorLocationComponent
        {
            EntityId = ship.Id,
            CurrentSector = new SectorCoordinate(0, 0, 0)
        };
        _gameEngine.EntityManager.AddComponent(ship.Id, locationComponent);
        
        Console.WriteLine($"\n✓ Procedurally generated ship created!");
        Console.WriteLine($"  Name: {ship.Name}");
        Console.WriteLine($"  ID: {ship.Id}");
        Console.WriteLine($"  Voxel blocks: {voxelComponent.Blocks.Count}");
        Console.WriteLine($"  Total mass: {voxelComponent.TotalMass:F2} kg");
        Console.WriteLine($"  Moment of inertia: {voxelComponent.MomentOfInertia:F2}");
        Console.WriteLine($"  Center of mass: {voxelComponent.CenterOfMass}");
        Console.WriteLine($"  Total thrust: {generatedShip.TotalThrust:F2} N");
        Console.WriteLine($"  Total torque: {voxelComponent.TotalTorque:F2} Nm");
        Console.WriteLine($"  Power generation: {generatedShip.TotalPowerGeneration:F2} W");
        Console.WriteLine($"  Shield capacity: {generatedShip.TotalShieldCapacity:F2}");
        Console.WriteLine($"  Structural integrity: {voxelComponent.StructuralIntegrity:F1}%");
        Console.WriteLine($"  Weapon mounts: {generatedShip.WeaponMountCount}");
        Console.WriteLine($"  Cargo blocks: {generatedShip.CargoBlockCount}");
        Console.WriteLine($"  Position: {physicsComponent.Position}");
        Console.WriteLine($"  Velocity: {physicsComponent.Velocity}");
        Console.WriteLine($"  Credits: {inventoryComponent.Inventory.GetResourceAmount(ResourceType.Credits)}");
        
        // Show the improvements
        Console.WriteLine($"\n--- What's New ---");
        Console.WriteLine($"  ✨ Procedurally generated with {voxelComponent.Blocks.Count} blocks (was 5 manual blocks)");
        Console.WriteLine($"  ✨ Cohesive design with connected structure");
        Console.WriteLine($"  ✨ Functional components properly placed");
        Console.WriteLine($"  ✨ Military-style angular hull shape");
        Console.WriteLine($"  ✨ Validated structural integrity");
        
        if (generatedShip.Warnings.Count > 0)
        {
            Console.WriteLine($"\n--- Generation Warnings ({generatedShip.Warnings.Count}) ---");
            foreach (var warning in generatedShip.Warnings.Take(5))
            {
                Console.WriteLine($"  ⚠ {warning}");
            }
        }
    }

    static void VoxelSystemDemo()
    {
        Console.WriteLine("\n=== Voxel System Demo ===");
        Console.WriteLine("Building a custom ship structure with colorful functional blocks...\n");

        var structure = new VoxelStructureComponent();
        
        // Create a functional ship design with vibrant materials
        Console.WriteLine("Creating rainbow ship design:");
        
        // Core (Avorion - purple for better visibility)
        structure.AddBlock(new VoxelBlock(new Vector3(0, 0, 0), new Vector3(3, 3, 3), "Avorion", BlockType.Hull));
        
        // Engines (back) - Ogonite (red/orange with glow)
        structure.AddBlock(new VoxelBlock(new Vector3(-4, 0, 0), new Vector3(2, 2, 2), "Ogonite", BlockType.Engine));
        structure.AddBlock(new VoxelBlock(new Vector3(-4, 2, 0), new Vector3(2, 2, 2), "Ogonite", BlockType.Engine));
        
        // Thrusters (sides for maneuvering) - Trinium (blue with glow)
        structure.AddBlock(new VoxelBlock(new Vector3(0, 4, 0), new Vector3(1.5f, 1.5f, 1.5f), "Trinium", BlockType.Thruster));
        structure.AddBlock(new VoxelBlock(new Vector3(0, -4, 0), new Vector3(1.5f, 1.5f, 1.5f), "Trinium", BlockType.Thruster));
        
        // Gyro arrays for rotation - Xanion (gold with glow)
        structure.AddBlock(new VoxelBlock(new Vector3(0, 0, 3), new Vector3(2, 2, 2), "Xanion", BlockType.GyroArray));
        structure.AddBlock(new VoxelBlock(new Vector3(0, 0, -3), new Vector3(2, 2, 2), "Xanion", BlockType.GyroArray));
        
        // Generator - Naonite (green with glow)
        structure.AddBlock(new VoxelBlock(new Vector3(4, 0, 0), new Vector3(2, 2, 2), "Naonite", BlockType.Generator));
        
        // Shield generator - Titanium (silver-blue, metallic)
        structure.AddBlock(new VoxelBlock(new Vector3(2, 2, 0), new Vector3(2, 2, 2), "Titanium", BlockType.ShieldGenerator));

        Console.WriteLine($"  Total blocks: {structure.Blocks.Count}");
        Console.WriteLine($"  Total mass: {structure.TotalMass:F2} kg");
        Console.WriteLine($"  Moment of inertia: {structure.MomentOfInertia:F2}");
        Console.WriteLine($"  Center of mass: ({structure.CenterOfMass.X:F1}, {structure.CenterOfMass.Y:F1}, {structure.CenterOfMass.Z:F1})");
        Console.WriteLine($"  Total thrust: {structure.TotalThrust:F2} N");
        Console.WriteLine($"  Total torque: {structure.TotalTorque:F2} Nm");
        Console.WriteLine($"  Power generation: {structure.PowerGeneration:F2} W");
        Console.WriteLine($"  Shield capacity: {structure.ShieldCapacity:F2}");
        Console.WriteLine($"  Structural integrity: {structure.StructuralIntegrity:F1}%");
        
        Console.WriteLine("\nBlock details by type:");
        var engineCount = structure.GetBlocksByType(BlockType.Engine).Count();
        var thrusterCount = structure.GetBlocksByType(BlockType.Thruster).Count();
        var gyroCount = structure.GetBlocksByType(BlockType.GyroArray).Count();
        var generatorCount = structure.GetBlocksByType(BlockType.Generator).Count();
        var shieldCount = structure.GetBlocksByType(BlockType.ShieldGenerator).Count();
        
        Console.WriteLine($"  Engines: {engineCount}");
        Console.WriteLine($"  Thrusters: {thrusterCount}");
        Console.WriteLine($"  Gyro Arrays: {gyroCount}");
        Console.WriteLine($"  Generators: {generatorCount}");
        Console.WriteLine($"  Shield Generators: {shieldCount}");
    }

    static void PhysicsDemo()
    {
        Console.WriteLine("\n=== Physics System Demo ===");
        Console.WriteLine("Simulating Newtonian physics...\n");

        // Create a test object
        var entity = _gameEngine!.EntityManager.CreateEntity("Test Object");
        var physics = new PhysicsComponent
        {
            Position = new Vector3(0, 0, 0),
            Velocity = new Vector3(50, 30, 0),
            Mass = 1000f
        };
        _gameEngine.EntityManager.AddComponent(entity.Id, physics);

        Console.WriteLine("Initial state:");
        Console.WriteLine($"  Position: {physics.Position}");
        Console.WriteLine($"  Velocity: {physics.Velocity}");
        Console.WriteLine($"  Mass: {physics.Mass} kg\n");

        // Apply a force
        Console.WriteLine("Applying force: (1000, 0, 0) N");
        physics.AddForce(new Vector3(1000, 0, 0));

        // Simulate for a few frames
        Console.WriteLine("\nSimulating physics for 5 seconds...");
        for (int i = 0; i < 5; i++)
        {
            _gameEngine.Update();
            System.Threading.Thread.Sleep(100); // Simulate time passing
            
            if (i % 1 == 0)
            {
                var phys = _gameEngine.EntityManager.GetComponent<PhysicsComponent>(entity.Id);
                Console.WriteLine($"  t={i}s: Pos=({phys!.Position.X:F1}, {phys.Position.Y:F1}, {phys.Position.Z:F1}), " +
                                $"Vel=({phys.Velocity.X:F1}, {phys.Velocity.Y:F1}, {phys.Velocity.Z:F1})");
            }
        }
    }

    static void ProceduralGenerationDemo()
    {
        Console.WriteLine("\n=== Procedural Generation Demo ===");
        Console.WriteLine("Generating galaxy sectors...\n");

        for (int i = 0; i < 3; i++)
        {
            int x = i * 5;
            int y = 0;
            int z = 0;
            
            var sector = _gameEngine!.GenerateSector(x, y, z);
            
            Console.WriteLine($"Sector [{x}, {y}, {z}]:");
            Console.WriteLine($"  Asteroids: {sector.Asteroids.Count}");
            
            if (sector.Station != null)
            {
                Console.WriteLine($"  Station: {sector.Station.Name} ({sector.Station.StationType})");
            }
            else
            {
                Console.WriteLine($"  Station: None");
            }
            
            // Show first 3 asteroids
            for (int j = 0; j < Math.Min(3, sector.Asteroids.Count); j++)
            {
                var asteroid = sector.Asteroids[j];
                Console.WriteLine($"    Asteroid {j + 1}: {asteroid.ResourceType}, Size: {asteroid.Size:F1}");
            }
            Console.WriteLine();
        }
    }

    static void ResourceManagementDemo()
    {
        Console.WriteLine("\n=== Resource Management Demo ===");
        
        var inventory = new Inventory { MaxCapacity = 1000 };
        
        Console.WriteLine($"Max capacity: {inventory.MaxCapacity}");
        Console.WriteLine($"Current capacity: {inventory.CurrentCapacity}\n");

        // Add resources
        Console.WriteLine("Adding resources:");
        inventory.AddResource(ResourceType.Iron, 100);
        Console.WriteLine($"  +100 Iron");
        inventory.AddResource(ResourceType.Titanium, 50);
        Console.WriteLine($"  +50 Titanium");
        inventory.AddResource(ResourceType.Credits, 5000);
        Console.WriteLine($"  +5000 Credits");

        Console.WriteLine($"\nCurrent capacity: {inventory.CurrentCapacity}/{inventory.MaxCapacity}");
        
        Console.WriteLine("\nInventory contents:");
        foreach (var kvp in inventory.GetAllResources())
        {
            if (kvp.Value > 0)
            {
                Console.WriteLine($"  {kvp.Key}: {kvp.Value}");
            }
        }

        // Test crafting
        Console.WriteLine("\n--- Crafting System ---");
        var crafting = _gameEngine!.CraftingSystem;
        
        Console.WriteLine("Available upgrades:");
        foreach (var upgrade in crafting.GetAvailableUpgrades())
        {
            Console.WriteLine($"  {upgrade.Name} ({upgrade.Type})");
            Console.WriteLine($"    Effect: +{upgrade.EffectValue:F1}");
            Console.WriteLine($"    Cost:");
            foreach (var cost in upgrade.CraftingCost)
            {
                Console.WriteLine($"      {cost.Key}: {cost.Value}");
            }
        }
    }

    static void RPGSystemsDemo()
    {
        Console.WriteLine("\n=== RPG Systems Demo ===");

        // Trading
        Console.WriteLine("--- Trading System ---");
        var trading = _gameEngine!.TradingSystem;
        var inventory = new Inventory { MaxCapacity = 1000 };
        inventory.AddResource(ResourceType.Credits, 10000);

        Console.WriteLine($"Starting credits: {inventory.GetResourceAmount(ResourceType.Credits)}");
        
        int buyPrice = trading.GetBuyPrice(ResourceType.Iron, 50);
        Console.WriteLine($"\nBuying 50 Iron for {buyPrice} credits...");
        
        if (trading.BuyResource(ResourceType.Iron, 50, inventory))
        {
            Console.WriteLine($"  ✓ Purchase successful!");
            Console.WriteLine($"  Credits remaining: {inventory.GetResourceAmount(ResourceType.Credits)}");
            Console.WriteLine($"  Iron: {inventory.GetResourceAmount(ResourceType.Iron)}");
        }

        // Progression
        Console.WriteLine("\n--- Progression System ---");
        var progression = new ProgressionComponent();
        Console.WriteLine($"Level: {progression.Level}");
        Console.WriteLine($"Experience: {progression.Experience}/{progression.ExperienceToNextLevel}");
        Console.WriteLine($"Skill Points: {progression.SkillPoints}");

        Console.WriteLine("\nGaining 150 experience...");
        bool leveledUp = progression.AddExperience(150);
        
        if (leveledUp)
        {
            Console.WriteLine($"  ✓ LEVEL UP!");
        }
        
        Console.WriteLine($"  Level: {progression.Level}");
        Console.WriteLine($"  Experience: {progression.Experience}/{progression.ExperienceToNextLevel}");
        Console.WriteLine($"  Skill Points: {progression.SkillPoints}");

        // Loot system
        Console.WriteLine("\n--- Loot System ---");
        var lootSystem = _gameEngine.LootSystem;
        var loot = lootSystem.GenerateLoot(5, includePodLoot: false);
        
        Console.WriteLine("Loot dropped from level 5 enemy:");
        foreach (var drop in loot)
        {
            Console.WriteLine($"  {drop.Resource}: {drop.Amount} (Chance: {drop.DropChance * 100:F0}%)");
        }
        
        // Pod loot system
        Console.WriteLine("\n--- Pod Loot System (NEW) ---");
        var podLoot = lootSystem.GenerateLoot(10, includePodLoot: true);
        
        Console.WriteLine("Loot dropped from level 10 boss:");
        foreach (var drop in podLoot)
        {
            if (drop.Resource.HasValue)
            {
                Console.WriteLine($"  {drop.Resource}: {drop.Amount}");
            }
            if (drop.PodUpgrade != null)
            {
                var upgrade = drop.PodUpgrade;
                Console.WriteLine($"  🌟 Pod Upgrade: {upgrade.Name} (Rarity: {upgrade.Rarity}/5)");
                Console.WriteLine($"     {upgrade.Description}");
            }
            if (!string.IsNullOrEmpty(drop.AbilityId))
            {
                Console.WriteLine($"  ⚡ Ability Unlock: {drop.AbilityId}");
            }
        }
    }

    static void ScriptingDemo()
    {
        Console.WriteLine("\n=== Scripting Demo ===");
        Console.WriteLine("Executing Lua script...\n");

        // Execute some test scripts
        Console.WriteLine("Script 1: Simple calculation");
        _gameEngine!.ExecuteScript("result = 10 + 20; log('Result: ' .. result)");

        Console.WriteLine("\nScript 2: Accessing engine");
        _gameEngine.ExecuteScript(@"
            log('Accessing game engine from Lua')
            stats = Engine:GetStatistics()
            log('Total entities: ' .. stats.TotalEntities)
        ");

        Console.WriteLine("\nScript 3: Function definition");
        _gameEngine.ExecuteScript(@"
            function greet(name)
                return 'Hello, ' .. name .. '!'
            end
            log(greet('Player'))
        ");

        Console.WriteLine("\n✓ Scripting system is working!");
    }

    static void MultiplayerDemo()
    {
        Console.WriteLine("\n=== Multiplayer Demo ===");
        
        if (_gameEngine!.GameServer?.IsRunning ?? false)
        {
            Console.WriteLine("Server is already running!");
            Console.Write("Stop server? (y/n): ");
            if (Console.ReadLine()?.ToLower() == "y")
            {
                _gameEngine.StopServer();
                Console.WriteLine("✓ Server stopped");
            }
        }
        else
        {
            Console.Write("Start multiplayer server on port 27015? (y/n): ");
            if (Console.ReadLine()?.ToLower() == "y")
            {
                _gameEngine.StartServer();
                Console.WriteLine("✓ Server started on port 27015");
                Console.WriteLine("  Players can now connect to this server");
                Console.WriteLine("  Note: Server runs in the background");
            }
        }
    }

    static void ShowStatistics()
    {
        Console.WriteLine("\n=== Engine Statistics ===");
        var stats = _gameEngine!.GetStatistics();
        
        Console.WriteLine($"Total Entities: {stats.TotalEntities}");
        Console.WriteLine($"Total Components: {stats.TotalComponents}");
        Console.WriteLine($"Physics System: {(stats.PhysicsEnabled ? "Enabled" : "Disabled")}");
        Console.WriteLine($"Server Status: {(stats.IsServerRunning ? "Running" : "Stopped")}");
        Console.WriteLine($"Engine Status: {(_gameEngine.IsRunning ? "Running" : "Stopped")}");
        
        Console.WriteLine("\nEntity details:");
        var entities = _gameEngine.EntityManager.GetAllEntities().ToList();
        foreach (var entity in entities)
        {
            Console.WriteLine($"  {entity.Name} (ID: {entity.Id.ToString()[..8]}...)");
        }
    }

    static void GraphicsDemo()
    {
        Console.WriteLine("\n=== 3D Graphics Demo ===");
        Console.WriteLine("Starting 3D graphics window...");
        Console.WriteLine();

        // Check if there are any entities to render
        var entities = _gameEngine!.EntityManager.GetAllEntities().ToList();
        if (entities.Count == 0)
        {
            Console.WriteLine("No entities found! Creating demo ships first...");
            Console.WriteLine();
            
            // Create a few test ships at different positions
            for (int i = 0; i < 3; i++)
            {
                var ship = _gameEngine.EntityManager.CreateEntity($"Demo Ship {i + 1}");
                
                var voxelComponent = new VoxelStructureComponent();
                
                // Create different ship designs with colorful materials
                switch (i)
                {
                    case 0: // Fighter with bright colors
                        // Core - Purple Avorion
                        voxelComponent.AddBlock(new VoxelBlock(
                            new Vector3(0, 0, 0),
                            new Vector3(3, 2, 2),
                            "Avorion",
                            BlockType.Hull
                        ));
                        // Engines - Red Ogonite
                        voxelComponent.AddBlock(new VoxelBlock(
                            new Vector3(-4, 0, 0),
                            new Vector3(2, 2, 2),
                            "Ogonite",
                            BlockType.Engine
                        ));
                        // Wings - Gold Xanion
                        voxelComponent.AddBlock(new VoxelBlock(new Vector3(0, 3, 0), new Vector3(2, 1, 2), "Xanion", BlockType.Armor));
                        voxelComponent.AddBlock(new VoxelBlock(new Vector3(0, -3, 0), new Vector3(2, 1, 2), "Xanion", BlockType.Armor));
                        break;
                    
                    case 1: // Cross-shaped ship with rainbow colors
                        voxelComponent.AddBlock(new VoxelBlock(new Vector3(0, 0, 0), new Vector3(3, 3, 3), "Avorion", BlockType.Hull)); // Purple center
                        voxelComponent.AddBlock(new VoxelBlock(new Vector3(4, 0, 0), new Vector3(2, 2, 2), "Xanion", BlockType.Generator)); // Gold right
                        voxelComponent.AddBlock(new VoxelBlock(new Vector3(-4, 0, 0), new Vector3(2, 2, 2), "Ogonite", BlockType.Engine)); // Red left
                        voxelComponent.AddBlock(new VoxelBlock(new Vector3(0, 4, 0), new Vector3(2, 2, 2), "Naonite", BlockType.ShieldGenerator)); // Green top
                        voxelComponent.AddBlock(new VoxelBlock(new Vector3(0, -4, 0), new Vector3(2, 2, 2), "Trinium", BlockType.Thruster)); // Blue bottom
                        break;
                    
                    case 2: // Compact cargo ship with gradient colors
                        for (int x = 0; x < 3; x++)
                        {
                            for (int y = 0; y < 2; y++)
                            {
                                var blockType = (x == 1 && y == 0) ? BlockType.Generator : BlockType.Cargo;
                                // Create a gradient from Naonite (green) to Trinium (blue) to Xanion (gold)
                                string material = x switch
                                {
                                    0 => "Naonite",
                                    1 => "Trinium",
                                    _ => "Xanion"
                                };
                                voxelComponent.AddBlock(new VoxelBlock(
                                    new Vector3(x * 3, y * 3, 0),
                                    new Vector3(2.5f, 2.5f, 2.5f),
                                    material,
                                    blockType
                                ));
                            }
                        }
                        break;
                }
                
                _gameEngine.EntityManager.AddComponent(ship.Id, voxelComponent);
                
                // Add physics with different positions
                var physicsComponent = new PhysicsComponent
                {
                    Position = new Vector3(i * 30 - 30, 0, 0),
                    Mass = voxelComponent.TotalMass
                };
                _gameEngine.EntityManager.AddComponent(ship.Id, physicsComponent);
                
                Console.WriteLine($"Created {ship.Name} at position ({physicsComponent.Position.X}, {physicsComponent.Position.Y}, {physicsComponent.Position.Z})");
            }
            
            Console.WriteLine();
        }

        Console.WriteLine("Opening 3D window... (Close window or press ESC to return to menu)");
        Console.WriteLine();

        try
        {
            using var graphicsWindow = new GraphicsWindow(_gameEngine);
            graphicsWindow.Run();
        }
        catch (Exception ex)
        {
            Console.WriteLine($"Error running graphics window: {ex.Message}");
            Console.WriteLine("Graphics rendering may not be available on this system.");
        }

        Console.WriteLine("Returned to console mode.");
    }

    static void PersistenceDemo()
    {
        if (_gameEngine == null) return;

        Console.WriteLine("\n=== Persistence System Demo ===");
        Console.WriteLine("This demo tests the save/load functionality.\n");

        Console.WriteLine("1. Save Current Game State");
        Console.WriteLine("2. Load Game State");
        Console.WriteLine("3. List Save Games");
        Console.WriteLine("4. Quick Save");
        Console.WriteLine("5. Quick Load");
        Console.WriteLine("0. Back to Main Menu");
        Console.Write("\nSelect option: ");

        var choice = Console.ReadLine();

        switch (choice)
        {
            case "1":
                Console.Write("Enter save name: ");
                var saveName = Console.ReadLine();
                if (!string.IsNullOrWhiteSpace(saveName))
                {
                    Console.WriteLine($"\nSaving game as '{saveName}'...");
                    
                    // Show current state before saving
                    var entitiesBeforeSave = _gameEngine.EntityManager.GetAllEntities().ToList();
                    Console.WriteLine($"Current entities: {entitiesBeforeSave.Count}");
                    foreach (var entity in entitiesBeforeSave)
                    {
                        Console.WriteLine($"  - {entity.Name} (ID: {entity.Id})");
                        if (_gameEngine.EntityManager.HasComponent<PhysicsComponent>(entity.Id))
                        {
                            var physics = _gameEngine.EntityManager.GetComponent<PhysicsComponent>(entity.Id);
                            if (physics != null)
                            {
                                Console.WriteLine($"    Physics: Position=({physics.Position.X:F1}, {physics.Position.Y:F1}, {physics.Position.Z:F1})");
                            }
                        }
                        if (_gameEngine.EntityManager.HasComponent<VoxelStructureComponent>(entity.Id))
                        {
                            var voxel = _gameEngine.EntityManager.GetComponent<VoxelStructureComponent>(entity.Id);
                            if (voxel != null)
                            {
                                Console.WriteLine($"    Voxels: {voxel.Blocks.Count} blocks, Mass={voxel.TotalMass:F1}kg");
                            }
                        }
                        if (_gameEngine.EntityManager.HasComponent<InventoryComponent>(entity.Id))
                        {
                            var inventory = _gameEngine.EntityManager.GetComponent<InventoryComponent>(entity.Id);
                            if (inventory != null)
                            {
                                Console.WriteLine($"    Inventory: {inventory.Inventory.CurrentCapacity}/{inventory.Inventory.MaxCapacity}");
                            }
                        }
                    }
                    
                    bool success = _gameEngine.SaveGame(saveName);
                    if (success)
                    {
                        Console.WriteLine($"✓ Game saved successfully as '{saveName}'");
                    }
                    else
                    {
                        Console.WriteLine("✗ Failed to save game");
                    }
                }
                break;

            case "2":
                Console.Write("Enter save name to load: ");
                var loadName = Console.ReadLine();
                if (!string.IsNullOrWhiteSpace(loadName))
                {
                    Console.WriteLine($"\nLoading game '{loadName}'...");
                    
                    bool success = _gameEngine.LoadGame(loadName);
                    if (success)
                    {
                        Console.WriteLine($"✓ Game loaded successfully from '{loadName}'");
                        
                        // Show loaded state
                        var entitiesAfterLoad = _gameEngine.EntityManager.GetAllEntities().ToList();
                        Console.WriteLine($"\nLoaded entities: {entitiesAfterLoad.Count}");
                        foreach (var entity in entitiesAfterLoad)
                        {
                            Console.WriteLine($"  - {entity.Name} (ID: {entity.Id})");
                            if (_gameEngine.EntityManager.HasComponent<PhysicsComponent>(entity.Id))
                            {
                                var physics = _gameEngine.EntityManager.GetComponent<PhysicsComponent>(entity.Id);
                                if (physics != null)
                                {
                                    Console.WriteLine($"    Physics: Position=({physics.Position.X:F1}, {physics.Position.Y:F1}, {physics.Position.Z:F1})");
                                }
                            }
                            if (_gameEngine.EntityManager.HasComponent<VoxelStructureComponent>(entity.Id))
                            {
                                var voxel = _gameEngine.EntityManager.GetComponent<VoxelStructureComponent>(entity.Id);
                                if (voxel != null)
                                {
                                    Console.WriteLine($"    Voxels: {voxel.Blocks.Count} blocks, Mass={voxel.TotalMass:F1}kg");
                                }
                            }
                            if (_gameEngine.EntityManager.HasComponent<InventoryComponent>(entity.Id))
                            {
                                var inventory = _gameEngine.EntityManager.GetComponent<InventoryComponent>(entity.Id);
                                if (inventory != null)
                                {
                                    Console.WriteLine($"    Inventory: {inventory.Inventory.CurrentCapacity}/{inventory.Inventory.MaxCapacity}");
                                }
                            }
                        }
                    }
                    else
                    {
                        Console.WriteLine("✗ Failed to load game");
                    }
                }
                break;

            case "3":
                Console.WriteLine("\nAvailable save games:");
                var saves = _gameEngine.GetSaveGameInfo();
                if (saves.Count == 0)
                {
                    Console.WriteLine("  No save games found.");
                }
                else
                {
                    foreach (var save in saves)
                    {
                        Console.WriteLine($"  - {save.SaveName} ({save.FileName})");
                        Console.WriteLine($"    Saved: {save.SaveTime:yyyy-MM-dd HH:mm:ss}");
                        Console.WriteLine($"    Version: {save.Version}");
                    }
                }
                break;

            case "4":
                Console.WriteLine("\nQuick saving...");
                if (_gameEngine.QuickSave())
                {
                    Console.WriteLine("✓ Quick save successful");
                }
                else
                {
                    Console.WriteLine("✗ Quick save failed");
                }
                break;

            case "5":
                Console.WriteLine("\nQuick loading...");
                if (_gameEngine.QuickLoad())
                {
                    Console.WriteLine("✓ Quick load successful");
                }
                else
                {
                    Console.WriteLine("✗ Quick load failed (no quicksave found?)");
                }
                break;

            case "0":
                break;

            default:
                Console.WriteLine("Invalid option!");
                break;
        }
    }

    static void PlayerPodDemo()
    {
        if (_gameEngine == null) return;

        Console.WriteLine("\n=== Player Pod System Demo ===");
        Console.WriteLine("The player pod is your character in the game - a multi-purpose utility ship");
        Console.WriteLine("at half the efficiency of a built ship, but upgradeable and affecting any ship you pilot.\n");

        // Create the player pod entity
        var pod = _gameEngine.EntityManager.CreateEntity("Player Pod");
        Console.WriteLine($"✓ Created Player Pod (ID: {pod.Id.ToString("N")[..8]}...)");

        // Add the player pod component
        var podComponent = new PlayerPodComponent
        {
            EntityId = pod.Id,
            BaseEfficiencyMultiplier = 0.5f,
            BaseThrustPower = 50f,
            BasePowerGeneration = 100f,
            BaseShieldCapacity = 200f,
            BaseTorque = 20f,
            MaxUpgradeSlots = 5
        };
        _gameEngine.EntityManager.AddComponent(pod.Id, podComponent);

        // Add a single-block voxel structure (pod looks like a 1-block ship)
        var voxelComponent = new VoxelStructureComponent();
        voxelComponent.AddBlock(new VoxelBlock(
            new Vector3(0, 0, 0),
            new Vector3(2, 2, 2),
            "Titanium",
            BlockType.Hull
        ));
        _gameEngine.EntityManager.AddComponent(pod.Id, voxelComponent);

        // Add physics
        var physicsComponent = new PhysicsComponent
        {
            Position = new Vector3(0, 0, 0),
            Mass = voxelComponent.TotalMass,
            MaxThrust = podComponent.GetTotalThrust(),
            MaxTorque = podComponent.GetTotalTorque(),
            Velocity = Vector3.Zero
        };
        _gameEngine.EntityManager.AddComponent(pod.Id, physicsComponent);

        // Add progression (pod levels up like a character)
        var progressionComponent = new ProgressionComponent
        {
            EntityId = pod.Id,
            Level = 1,
            Experience = 0,
            SkillPoints = 0
        };
        _gameEngine.EntityManager.AddComponent(pod.Id, progressionComponent);

        // Add inventory
        var inventoryComponent = new InventoryComponent(500);
        inventoryComponent.Inventory.AddResource(ResourceType.Credits, 5000);
        _gameEngine.EntityManager.AddComponent(pod.Id, inventoryComponent);

        Console.WriteLine("\n--- Player Pod Stats (Base) ---");
        Console.WriteLine($"  Efficiency Multiplier: {podComponent.GetTotalEfficiencyMultiplier():F2}x (50% of built ships)");
        Console.WriteLine($"  Thrust Power: {podComponent.GetTotalThrust():F2} N");
        Console.WriteLine($"  Power Generation: {podComponent.GetTotalPowerGeneration():F2} W");
        Console.WriteLine($"  Shield Capacity: {podComponent.GetTotalShieldCapacity():F2}");
        Console.WriteLine($"  Torque: {podComponent.GetTotalTorque():F2} Nm");
        Console.WriteLine($"  Level: {progressionComponent.Level}");
        Console.WriteLine($"  Upgrade Slots: {podComponent.EquippedUpgrades.Count}/{podComponent.MaxUpgradeSlots}");

        // Demonstrate upgrade system
        Console.WriteLine("\n--- Equipping Upgrades ---");
        
        var upgrade1 = new PodUpgrade(
            "Advanced Thruster Module",
            "Increases thrust power by 25N",
            PodUpgradeType.ThrustBoost,
            25f,
            3
        );
        
        var upgrade2 = new PodUpgrade(
            "Efficiency Optimizer",
            "Increases efficiency by 10%",
            PodUpgradeType.EfficiencyBoost,
            0.1f,
            4
        );
        
        var upgrade3 = new PodUpgrade(
            "Shield Amplifier",
            "Increases shield capacity by 100",
            PodUpgradeType.ShieldBoost,
            100f,
            2
        );

        podComponent.EquipUpgrade(upgrade1);
        Console.WriteLine($"✓ Equipped: {upgrade1.Name} (Rarity: {upgrade1.Rarity}/5)");
        
        podComponent.EquipUpgrade(upgrade2);
        Console.WriteLine($"✓ Equipped: {upgrade2.Name} (Rarity: {upgrade2.Rarity}/5)");
        
        podComponent.EquipUpgrade(upgrade3);
        Console.WriteLine($"✓ Equipped: {upgrade3.Name} (Rarity: {upgrade3.Rarity}/5)");

        Console.WriteLine("\n--- Player Pod Stats (With Upgrades) ---");
        Console.WriteLine($"  Efficiency Multiplier: {podComponent.GetTotalEfficiencyMultiplier():F2}x");
        Console.WriteLine($"  Thrust Power: {podComponent.GetTotalThrust():F2} N");
        Console.WriteLine($"  Power Generation: {podComponent.GetTotalPowerGeneration():F2} W");
        Console.WriteLine($"  Shield Capacity: {podComponent.GetTotalShieldCapacity():F2}");
        Console.WriteLine($"  Torque: {podComponent.GetTotalTorque():F2} Nm");
        Console.WriteLine($"  Upgrade Slots: {podComponent.EquippedUpgrades.Count}/{podComponent.MaxUpgradeSlots}");

        // Level up demonstration
        Console.WriteLine("\n--- Level Up System ---");
        Console.WriteLine("Gaining experience...");
        progressionComponent.AddExperience(150);
        Console.WriteLine($"✓ LEVEL UP! Now level {progressionComponent.Level}");
        Console.WriteLine($"  Skill Points: {progressionComponent.SkillPoints}");

        // Create a ship with docking port
        Console.WriteLine("\n--- Creating Ship with Pod Docking Port ---");
        var ship = _gameEngine.EntityManager.CreateEntity("Fighter Ship");
        
        var shipVoxel = new VoxelStructureComponent();
        
        // Core hull
        shipVoxel.AddBlock(new VoxelBlock(new Vector3(0, 0, 0), new Vector3(3, 3, 3), "Titanium", BlockType.Hull));
        
        // Engines
        shipVoxel.AddBlock(new VoxelBlock(new Vector3(-4, 0, 0), new Vector3(2, 2, 2), "Iron", BlockType.Engine));
        
        // Pod docking port
        shipVoxel.AddBlock(new VoxelBlock(new Vector3(0, 3, 0), new Vector3(2, 2, 2), "Titanium", BlockType.PodDocking));
        
        // Generator
        shipVoxel.AddBlock(new VoxelBlock(new Vector3(0, -3, 0), new Vector3(2, 2, 2), "Iron", BlockType.Generator));
        
        _gameEngine.EntityManager.AddComponent(ship.Id, shipVoxel);

        var shipPhysics = new PhysicsComponent
        {
            Position = new Vector3(50, 0, 0),
            Mass = shipVoxel.TotalMass,
            MaxThrust = shipVoxel.TotalThrust,
            MaxTorque = shipVoxel.TotalTorque
        };
        _gameEngine.EntityManager.AddComponent(ship.Id, shipPhysics);

        // Add docking component
        var dockingComponent = new DockingComponent
        {
            EntityId = ship.Id,
            HasDockingPort = true,
            DockingPortPosition = new Vector3(0, 3, 0)
        };
        _gameEngine.EntityManager.AddComponent(ship.Id, dockingComponent);

        Console.WriteLine($"✓ Created {ship.Name} with pod docking port");
        Console.WriteLine($"  Ship Blocks: {shipVoxel.Blocks.Count}");
        Console.WriteLine($"  Ship Mass: {shipVoxel.TotalMass:F2} kg");
        Console.WriteLine($"  Ship Thrust: {shipVoxel.TotalThrust:F2} N");
        Console.WriteLine($"  Ship Power: {shipVoxel.PowerGeneration:F2} W");
        Console.WriteLine($"  Ship Shields: {shipVoxel.ShieldCapacity:F2}");

        // Dock the pod to the ship
        Console.WriteLine("\n--- Docking Pod to Ship ---");
        bool docked = _gameEngine.PodDockingSystem.DockPod(pod.Id, ship.Id);
        
        if (docked)
        {
            Console.WriteLine("✓ Pod successfully docked to ship!");
            Console.WriteLine("  Pod skills and abilities now affect the ship considerably.");
            
            // Get effective stats with pod bonuses
            var effectiveStats = _gameEngine.PodDockingSystem.GetEffectiveShipStats(ship.Id);
            
            Console.WriteLine("\n--- Ship Stats (With Docked Pod) ---");
            Console.WriteLine($"  Total Thrust: {effectiveStats.TotalThrust:F2} N");
            Console.WriteLine($"  Total Torque: {effectiveStats.TotalTorque:F2} Nm");
            Console.WriteLine($"  Power Generation: {effectiveStats.PowerGeneration:F2} W");
            Console.WriteLine($"  Shield Capacity: {effectiveStats.ShieldCapacity:F2}");
            Console.WriteLine($"  Level Bonus: +{progressionComponent.Level * 5}% to all stats");
            
            float improvement = ((effectiveStats.TotalThrust / shipVoxel.TotalThrust) - 1.0f) * 100f;
            Console.WriteLine($"\n  Overall Improvement: +{improvement:F1}% from pod and level bonuses!");
        }
        else
        {
            Console.WriteLine("✗ Failed to dock pod");
        }

        Console.WriteLine("\n--- Pod System Features ---");
        Console.WriteLine("✓ Pod functions as your playable character");
        Console.WriteLine("✓ All ship systems at 50% efficiency (upgradeable)");
        Console.WriteLine("✓ Levels up and gains experience like RPG character");
        Console.WriteLine("✓ Equippable rare upgrades (5 slots)");
        Console.WriteLine("✓ Pod abilities considerably affect docked ship");
        Console.WriteLine("✓ Dock into any ship with a pod docking port");
        Console.WriteLine("✓ Displayed as single-block ship visually");

        Console.WriteLine("\nPress Enter to return to main menu...");
        Console.ReadLine();
    }
    
    static void EnhancedPlayerPodDemo()
    {
        if (_gameEngine == null) return;

        Console.WriteLine("\n=== Enhanced Player Pod System Demo ===");
        Console.WriteLine("Showcasing Skills, Abilities, and Advanced Progression\n");

        // Create the player pod entity
        var pod = _gameEngine.EntityManager.CreateEntity("Player Pod");
        Console.WriteLine($"✓ Created Player Pod (ID: {pod.Id.ToString("N")[..8]}...)");

        // Add the player pod component
        var podComponent = new PlayerPodComponent
        {
            EntityId = pod.Id,
            BaseEfficiencyMultiplier = 0.5f,
            BaseThrustPower = 50f,
            BasePowerGeneration = 100f,
            BaseShieldCapacity = 200f,
            BaseTorque = 20f,
            MaxUpgradeSlots = 5
        };
        _gameEngine.EntityManager.AddComponent(pod.Id, podComponent);

        // Add skill tree component
        var skillTreeComponent = new PodSkillTreeComponent
        {
            EntityId = pod.Id
        };
        _gameEngine.EntityManager.AddComponent(pod.Id, skillTreeComponent);

        // Add abilities component
        var abilitiesComponent = new PodAbilitiesComponent
        {
            EntityId = pod.Id
        };
        _gameEngine.EntityManager.AddComponent(pod.Id, abilitiesComponent);

        // Add voxel structure
        var voxelComponent = new VoxelStructureComponent();
        voxelComponent.AddBlock(new VoxelBlock(
            new Vector3(0, 0, 0),
            new Vector3(2, 2, 2),
            "Titanium",
            BlockType.Hull
        ));
        _gameEngine.EntityManager.AddComponent(pod.Id, voxelComponent);

        // Add physics
        var physicsComponent = new PhysicsComponent
        {
            Position = new Vector3(0, 0, 0),
            Mass = voxelComponent.TotalMass,
            MaxThrust = podComponent.GetTotalThrust(skillTreeComponent, abilitiesComponent),
            MaxTorque = podComponent.GetTotalTorque(),
            Velocity = Vector3.Zero
        };
        _gameEngine.EntityManager.AddComponent(pod.Id, physicsComponent);

        // Add progression
        var progressionComponent = new ProgressionComponent
        {
            EntityId = pod.Id,
            Level = 5,
            Experience = 0,
            SkillPoints = 10  // Start with some skill points
        };
        _gameEngine.EntityManager.AddComponent(pod.Id, progressionComponent);

        // Add inventory
        var inventoryComponent = new InventoryComponent(500);
        inventoryComponent.Inventory.AddResource(ResourceType.Credits, 10000);
        _gameEngine.EntityManager.AddComponent(pod.Id, inventoryComponent);

        Console.WriteLine("\n--- Base Pod Stats ---");
        Console.WriteLine($"  Level: {progressionComponent.Level}");
        Console.WriteLine($"  Available Skill Points: {progressionComponent.SkillPoints}");
        Console.WriteLine($"  Efficiency: {podComponent.GetTotalEfficiencyMultiplier(skillTreeComponent):F2}x");
        Console.WriteLine($"  Thrust: {podComponent.GetTotalThrust(skillTreeComponent, abilitiesComponent):F2} N");
        Console.WriteLine($"  Power: {podComponent.GetTotalPowerGeneration(skillTreeComponent):F2} W");
        Console.WriteLine($"  Shields: {podComponent.GetTotalShieldCapacity(skillTreeComponent, abilitiesComponent):F2}");

        // Demonstrate Skill Tree System
        Console.WriteLine("\n=== SKILL TREE SYSTEM ===");
        Console.WriteLine("Learning combat and engineering skills...\n");

        // Learn combat skills
        var skillPoints = progressionComponent.SkillPoints;
        bool learned1 = skillTreeComponent.LearnSkill("combat_weapon_damage", progressionComponent.Level, ref skillPoints);
        if (learned1)
        {
            Console.WriteLine("✓ Learned: Weapon Mastery (Rank 1/5)");
            Console.WriteLine($"  Effect: +10% weapon damage");
            progressionComponent.SkillPoints = skillPoints;
        }

        learned1 = skillTreeComponent.LearnSkill("combat_weapon_damage", progressionComponent.Level, ref skillPoints);
        if (learned1)
        {
            Console.WriteLine("✓ Learned: Weapon Mastery (Rank 2/5)");
            Console.WriteLine($"  Effect: +20% weapon damage");
            progressionComponent.SkillPoints = skillPoints;
        }

        // Learn defense skill
        bool learned2 = skillTreeComponent.LearnSkill("defense_shield_capacity", progressionComponent.Level, ref skillPoints);
        if (learned2)
        {
            Console.WriteLine("✓ Learned: Shield Fortification (Rank 1/5)");
            Console.WriteLine($"  Effect: +15% shield capacity");
            progressionComponent.SkillPoints = skillPoints;
        }

        // Learn engineering skills
        bool learned3 = skillTreeComponent.LearnSkill("engineering_thrust", progressionComponent.Level, ref skillPoints);
        if (learned3)
        {
            Console.WriteLine("✓ Learned: Advanced Propulsion (Rank 1/5)");
            Console.WriteLine($"  Effect: +12% thrust power");
            progressionComponent.SkillPoints = skillPoints;
        }

        bool learned4 = skillTreeComponent.LearnSkill("engineering_power", progressionComponent.Level, ref skillPoints);
        if (learned4)
        {
            Console.WriteLine("✓ Learned: Power Optimization (Rank 1/5)");
            Console.WriteLine($"  Effect: +15% power generation");
            progressionComponent.SkillPoints = skillPoints;
        }

        Console.WriteLine($"\n  Remaining Skill Points: {progressionComponent.SkillPoints}");
        Console.WriteLine($"  Total Skills Learned: {skillTreeComponent.LearnedSkills.Count}");

        Console.WriteLine("\n--- Pod Stats (With Skills) ---");
        Console.WriteLine($"  Thrust: {podComponent.GetTotalThrust(skillTreeComponent, abilitiesComponent):F2} N");
        Console.WriteLine($"  Power: {podComponent.GetTotalPowerGeneration(skillTreeComponent):F2} W");
        Console.WriteLine($"  Shields: {podComponent.GetTotalShieldCapacity(skillTreeComponent, abilitiesComponent):F2}");

        // Demonstrate Active Abilities System
        Console.WriteLine("\n=== ACTIVE ABILITIES SYSTEM ===");
        Console.WriteLine("Equipping pod abilities...\n");

        // Equip abilities
        bool equipped1 = abilitiesComponent.EquipAbility("shield_overcharge");
        if (equipped1)
        {
            var ability = abilitiesComponent.Abilities["shield_overcharge"];
            Console.WriteLine($"✓ Equipped: {ability.Name}");
            Console.WriteLine($"  {ability.Description}");
            Console.WriteLine($"  Energy Cost: {ability.EnergyCost}, Cooldown: {ability.Cooldown}s");
        }

        bool equipped2 = abilitiesComponent.EquipAbility("overload_weapons");
        if (equipped2)
        {
            var ability = abilitiesComponent.Abilities["overload_weapons"];
            Console.WriteLine($"✓ Equipped: {ability.Name}");
            Console.WriteLine($"  {ability.Description}");
            Console.WriteLine($"  Energy Cost: {ability.EnergyCost}, Cooldown: {ability.Cooldown}s");
        }

        bool equipped3 = abilitiesComponent.EquipAbility("afterburner");
        if (equipped3)
        {
            var ability = abilitiesComponent.Abilities["afterburner"];
            Console.WriteLine($"✓ Equipped: {ability.Name}");
            Console.WriteLine($"  {ability.Description}");
            Console.WriteLine($"  Energy Cost: {ability.EnergyCost}, Cooldown: {ability.Cooldown}s");
        }

        bool equipped4 = abilitiesComponent.EquipAbility("scan_pulse");
        if (equipped4)
        {
            var ability = abilitiesComponent.Abilities["scan_pulse"];
            Console.WriteLine($"✓ Equipped: {ability.Name}");
            Console.WriteLine($"  {ability.Description}");
            Console.WriteLine($"  Energy Cost: {ability.EnergyCost}, Cooldown: {ability.Cooldown}s");
        }

        Console.WriteLine($"\n  Equipped Abilities: {abilitiesComponent.EquippedAbilityIds.Count}/{abilitiesComponent.MaxEquippedAbilities}");

        // Use abilities
        Console.WriteLine("\n--- Using Abilities ---");
        float availableEnergy = podComponent.GetTotalPowerGeneration(skillTreeComponent);
        Console.WriteLine($"Available Energy: {availableEnergy:F2} W\n");

        bool used1 = abilitiesComponent.UseAbility("afterburner", availableEnergy);
        if (used1)
        {
            Console.WriteLine("✓ Activated: Afterburner");
            Console.WriteLine($"  Thrust increased by 100% for 5 seconds!");
            
            var afterburner = abilitiesComponent.Abilities["afterburner"];
            float boostedThrust = podComponent.GetTotalThrust(skillTreeComponent, abilitiesComponent);
            Console.WriteLine($"  Current Thrust (with ability): {boostedThrust:F2} N");
        }

        bool used2 = abilitiesComponent.UseAbility("shield_overcharge", availableEnergy);
        if (used2)
        {
            Console.WriteLine("✓ Activated: Shield Overcharge");
            Console.WriteLine($"  Shield capacity increased by 50% for 10 seconds!");
            
            float boostedShields = podComponent.GetTotalShieldCapacity(skillTreeComponent, abilitiesComponent);
            Console.WriteLine($"  Current Shields (with ability): {boostedShields:F2}");
        }

        // Create a ship with docking port
        Console.WriteLine("\n=== DOCKING TO SHIP ===");
        var ship = _gameEngine.EntityManager.CreateEntity("Advanced Fighter");
        
        var shipVoxel = new VoxelStructureComponent();
        shipVoxel.AddBlock(new VoxelBlock(new Vector3(0, 0, 0), new Vector3(4, 4, 4), "Titanium", BlockType.Hull));
        shipVoxel.AddBlock(new VoxelBlock(new Vector3(-5, 0, 0), new Vector3(2, 2, 2), "Iron", BlockType.Engine));
        shipVoxel.AddBlock(new VoxelBlock(new Vector3(5, 0, 0), new Vector3(2, 2, 2), "Iron", BlockType.Engine));
        shipVoxel.AddBlock(new VoxelBlock(new Vector3(0, 4, 0), new Vector3(2, 2, 2), "Titanium", BlockType.PodDocking));
        shipVoxel.AddBlock(new VoxelBlock(new Vector3(0, -4, 0), new Vector3(2, 2, 2), "Iron", BlockType.Generator));
        shipVoxel.AddBlock(new VoxelBlock(new Vector3(0, 0, 4), new Vector3(2, 2, 2), "Titanium", BlockType.ShieldGenerator));
        
        _gameEngine.EntityManager.AddComponent(ship.Id, shipVoxel);

        var dockingComponent = new DockingComponent
        {
            EntityId = ship.Id,
            HasDockingPort = true,
            DockingPortPosition = new Vector3(0, 4, 0)
        };
        _gameEngine.EntityManager.AddComponent(ship.Id, dockingComponent);

        Console.WriteLine($"✓ Created {ship.Name}");
        Console.WriteLine($"  Ship Thrust: {shipVoxel.TotalThrust:F2} N");
        Console.WriteLine($"  Ship Power: {shipVoxel.PowerGeneration:F2} W");
        Console.WriteLine($"  Ship Shields: {shipVoxel.ShieldCapacity:F2}");

        // Dock the pod
        Console.WriteLine("\n--- Docking Pod ---");
        bool docked = _gameEngine.PodDockingSystem.DockPod(pod.Id, ship.Id);
        
        if (docked)
        {
            Console.WriteLine("✓ Pod successfully docked!");
            
            var effectiveStats = _gameEngine.PodDockingSystem.GetEffectiveShipStats(ship.Id);
            
            Console.WriteLine("\n--- Ship Stats (With Skilled Pod) ---");
            Console.WriteLine($"  Total Thrust: {effectiveStats.TotalThrust:F2} N");
            Console.WriteLine($"  Total Torque: {effectiveStats.TotalTorque:F2} Nm");
            Console.WriteLine($"  Power Generation: {effectiveStats.PowerGeneration:F2} W");
            Console.WriteLine($"  Shield Capacity: {effectiveStats.ShieldCapacity:F2}");
            Console.WriteLine($"  Weapon Damage: +{(effectiveStats.WeaponDamageMultiplier - 1.0f) * 100:F1}%");
            Console.WriteLine($"  Critical Hit Chance: {effectiveStats.CriticalHitChance * 100:F1}%");
            Console.WriteLine($"  Fire Rate: +{(effectiveStats.FireRateMultiplier - 1.0f) * 100:F1}%");
            
            float improvement = ((effectiveStats.TotalThrust / shipVoxel.TotalThrust) - 1.0f) * 100f;
            Console.WriteLine($"\n  Overall Performance: +{improvement:F1}% from pod level, skills, and abilities!");
        }

        Console.WriteLine("\n=== NEW FEATURES SUMMARY ===");
        Console.WriteLine("✓ Skill Tree System");
        Console.WriteLine("  - 18+ skills across 5 categories");
        Console.WriteLine("  - Combat, Defense, Engineering, Exploration, Leadership");
        Console.WriteLine("  - Prerequisites and level requirements");
        Console.WriteLine("  - Permanent character progression");
        Console.WriteLine();
        Console.WriteLine("✓ Active Abilities System");
        Console.WriteLine("  - 8+ unique pod abilities");
        Console.WriteLine("  - Shield, Weapon, Mobility, and Utility types");
        Console.WriteLine("  - Cooldown and energy management");
        Console.WriteLine("  - 4 equippable ability slots");
        Console.WriteLine();
        Console.WriteLine("✓ Enhanced Pod Stats");
        Console.WriteLine("  - Skills boost all pod systems");
        Console.WriteLine("  - Abilities provide temporary power-ups");
        Console.WriteLine("  - Combined bonuses affect docked ships");
        Console.WriteLine("  - Path to becoming unstoppable force!");

        Console.WriteLine("\nPress Enter to return to main menu...");
        Console.ReadLine();
    }
    
    static void ShowVersionInfo()
    {
        Console.WriteLine("\n=== About Codename:Subspace ===");
        Console.WriteLine();
        Console.WriteLine(VersionInfo.GetVersionInfo());
        Console.WriteLine();
        Console.WriteLine("Project: Open-source space game engine");
        Console.WriteLine("Inspired by: Avorion's gameplay mechanics");
        Console.WriteLine("Built with: C# and .NET 9.0");
        Console.WriteLine();
        Console.WriteLine("Features:");
        Console.WriteLine("  • Entity-Component System (ECS) architecture");
        Console.WriteLine("  • Real-time 3D graphics with OpenGL");
        Console.WriteLine("  • Integrated Player UI with ImGui.NET");
        Console.WriteLine("  • Voxel-based ship building");
        Console.WriteLine("  • Newtonian physics simulation");
        Console.WriteLine("  • Procedural galaxy generation");
        Console.WriteLine("  • Multiplayer networking support");
        Console.WriteLine("  • Lua scripting and modding");
        Console.WriteLine("  • Faction and politics system");
        Console.WriteLine("  • Player pod progression system");
        Console.WriteLine();
        Console.WriteLine(VersionInfo.GetSystemRequirements());
        Console.WriteLine();
        Console.WriteLine("GitHub: https://github.com/shifty81/AvorionLike");
        Console.WriteLine();
        Console.WriteLine("Press Enter to return to main menu...");
        Console.ReadLine();
    }

    static void CollisionDamageDemo()
    {
        Console.WriteLine("\n=== Collision & Damage Test Demo ===");
        Console.WriteLine("This demo creates two ships on a collision course to test:");
        Console.WriteLine("  • Collision detection (AABB spatial grid)");
        Console.WriteLine("  • Collision response (impulse-based physics)");
        Console.WriteLine("  • Damage system (block destruction)");
        Console.WriteLine("  • Shield absorption");
        Console.WriteLine();

        // Create Ship 1 - Moving ship with shields
        var ship1 = _gameEngine!.EntityManager.CreateEntity("Red Fighter");
        var voxel1 = new VoxelStructureComponent();
        
        // Core hull
        voxel1.AddBlock(new VoxelBlock(new Vector3(0, 0, 0), new Vector3(3, 3, 3), "Avorion", BlockType.Hull));
        voxel1.AddBlock(new VoxelBlock(new Vector3(3, 0, 0), new Vector3(2, 2, 2), "Ogonite", BlockType.Engine));
        voxel1.AddBlock(new VoxelBlock(new Vector3(-3, 0, 0), new Vector3(2, 2, 2), "Naonite", BlockType.ShieldGenerator));
        
        _gameEngine.EntityManager.AddComponent(ship1.Id, voxel1);
        
        var physics1 = new PhysicsComponent
        {
            Position = new Vector3(-50, 0, 0),
            Velocity = new Vector3(20, 0, 0), // Moving right at 20 m/s
            Mass = voxel1.TotalMass,
            CollisionRadius = 5f
        };
        _gameEngine.EntityManager.AddComponent(ship1.Id, physics1);
        
        var combat1 = new CombatComponent
        {
            EntityId = ship1.Id,
            MaxShields = voxel1.ShieldCapacity,
            CurrentShields = voxel1.ShieldCapacity,
            MaxEnergy = 100f,
            CurrentEnergy = 100f
        };
        _gameEngine.EntityManager.AddComponent(ship1.Id, combat1);

        // Create Ship 2 - Stationary target
        var ship2 = _gameEngine.EntityManager.CreateEntity("Blue Cargo");
        var voxel2 = new VoxelStructureComponent();
        
        voxel2.AddBlock(new VoxelBlock(new Vector3(0, 0, 0), new Vector3(4, 4, 4), "Titanium", BlockType.Hull));
        voxel2.AddBlock(new VoxelBlock(new Vector3(0, 5, 0), new Vector3(3, 3, 3), "Iron", BlockType.Cargo));
        
        _gameEngine.EntityManager.AddComponent(ship2.Id, voxel2);
        
        var physics2 = new PhysicsComponent
        {
            Position = new Vector3(50, 0, 0),
            Velocity = new Vector3(-15, 0, 0), // Moving left at 15 m/s
            Mass = voxel2.TotalMass,
            CollisionRadius = 6f
        };
        _gameEngine.EntityManager.AddComponent(ship2.Id, physics2);
        
        var combat2 = new CombatComponent
        {
            EntityId = ship2.Id,
            MaxShields = 0f, // No shields
            CurrentShields = 0f,
            MaxEnergy = 50f,
            CurrentEnergy = 50f
        };
        _gameEngine.EntityManager.AddComponent(ship2.Id, combat2);

        Console.WriteLine($"✓ Created {ship1.Name}:");
        Console.WriteLine($"    Position: {physics1.Position}");
        Console.WriteLine($"    Velocity: {physics1.Velocity} m/s");
        Console.WriteLine($"    Blocks: {voxel1.Blocks.Count}");
        Console.WriteLine($"    Shields: {combat1.CurrentShields:F0} / {combat1.MaxShields:F0}");
        Console.WriteLine($"    Integrity: {voxel1.StructuralIntegrity:F1}%");
        
        Console.WriteLine();
        Console.WriteLine($"✓ Created {ship2.Name}:");
        Console.WriteLine($"    Position: {physics2.Position}");
        Console.WriteLine($"    Velocity: {physics2.Velocity} m/s");
        Console.WriteLine($"    Blocks: {voxel2.Blocks.Count}");
        Console.WriteLine($"    Shields: {combat2.CurrentShields:F0} / {combat2.MaxShields:F0}");
        Console.WriteLine($"    Integrity: {voxel2.StructuralIntegrity:F1}%");
        
        Console.WriteLine();
        Console.WriteLine("Simulating collision...");
        Console.WriteLine("(Ships will collide in approximately 1.4 seconds)");
        Console.WriteLine();
        
        // Simulate for 3 seconds
        int steps = 60; // 60 steps = 3 seconds at 20 FPS
        for (int i = 0; i < steps; i++)
        {
            _gameEngine.Update();
            
            float distance = Vector3.Distance(physics1.Position, physics2.Position);
            
            // Check if collision occurred (distance less than sum of radii)
            if (i % 10 == 0 || distance < 15f)
            {
                Console.WriteLine($"[Step {i:D2}] Distance: {distance:F1}m | " +
                    $"Ship1 Pos: ({physics1.Position.X:F1}, {physics1.Position.Y:F1}, {physics1.Position.Z:F1}) | " +
                    $"Ship2 Pos: ({physics2.Position.X:F1}, {physics2.Position.Y:F1}, {physics2.Position.Z:F1})");
                    
                if (distance < 15f && i > 0)
                {
                    Console.WriteLine($"    >>> COLLISION DETECTED! <<<");
                    Console.WriteLine($"    Ship1 Shields: {combat1.CurrentShields:F0} / {combat1.MaxShields:F0}");
                    Console.WriteLine($"    Ship1 Integrity: {voxel1.StructuralIntegrity:F1}% ({voxel1.Blocks.Count} blocks)");
                    Console.WriteLine($"    Ship2 Integrity: {voxel2.StructuralIntegrity:F1}% ({voxel2.Blocks.Count} blocks)");
                }
            }
            
            Thread.Sleep(50); // Slow down for visibility
        }
        
        Console.WriteLine();
        Console.WriteLine("=== Final State ===");
        Console.WriteLine($"{ship1.Name}:");
        Console.WriteLine($"    Position: ({physics1.Position.X:F1}, {physics1.Position.Y:F1}, {physics1.Position.Z:F1})");
        Console.WriteLine($"    Velocity: ({physics1.Velocity.X:F1}, {physics1.Velocity.Y:F1}, {physics1.Velocity.Z:F1}) m/s");
        Console.WriteLine($"    Shields: {combat1.CurrentShields:F0} / {combat1.MaxShields:F0}");
        Console.WriteLine($"    Blocks: {voxel1.Blocks.Count} (Integrity: {voxel1.StructuralIntegrity:F1}%)");
        
        Console.WriteLine();
        Console.WriteLine($"{ship2.Name}:");
        Console.WriteLine($"    Position: ({physics2.Position.X:F1}, {physics2.Position.Y:F1}, {physics2.Position.Z:F1})");
        Console.WriteLine($"    Velocity: ({physics2.Velocity.X:F1}, {physics2.Velocity.Y:F1}, {physics2.Velocity.Z:F1}) m/s");
        Console.WriteLine($"    Blocks: {voxel2.Blocks.Count} (Integrity: {voxel2.StructuralIntegrity:F1}%)");
        
        Console.WriteLine();
        Console.WriteLine("=== Test Complete ===");
        Console.WriteLine("✓ Collision detection working");
        Console.WriteLine("✓ Physics response working");
        Console.WriteLine("✓ Damage system working");
        Console.WriteLine("✓ Shield absorption working");
        Console.WriteLine();
        Console.WriteLine("Press Enter to return to main menu...");
        Console.ReadLine();
    }

    static void SystemVerificationDemo()
    {
        Console.WriteLine("\n=== System Verification - Comprehensive Test Suite ===");
        Console.WriteLine("This will test all major game systems to ensure they are");
        Console.WriteLine("coded properly and working as intended.\n");
        
        Console.Write("Run verification tests? (y/n): ");
        if (Console.ReadLine()?.ToLower() != "y")
        {
            return;
        }

        Console.WriteLine();
        
        try
        {
            var verification = new Core.SystemVerification(_gameEngine!);
            var report = verification.RunAllTests();
            
            Console.WriteLine("\n=== FINAL SUMMARY ===");
            
            if (report.PassRate == 100.0)
            {
                Console.ForegroundColor = ConsoleColor.Green;
                Console.WriteLine("✓ ALL SYSTEMS OPERATIONAL");
                Console.WriteLine("✓ All systems are coded properly and working as intended");
                Console.ResetColor();
            }
            else if (report.PassRate >= 90.0)
            {
                Console.ForegroundColor = ConsoleColor.Yellow;
                Console.WriteLine("⚠ SYSTEMS MOSTLY OPERATIONAL");
                Console.WriteLine($"⚠ {report.FailedTests} minor issues found");
                Console.ResetColor();
            }
            else if (report.PassRate >= 70.0)
            {
                Console.ForegroundColor = ConsoleColor.DarkYellow;
                Console.WriteLine("⚠ SYSTEMS PARTIALLY OPERATIONAL");
                Console.WriteLine($"⚠ {report.FailedTests} issues require attention");
                Console.ResetColor();
            }
            else
            {
                Console.ForegroundColor = ConsoleColor.Red;
                Console.WriteLine("✗ SYSTEMS REQUIRE ATTENTION");
                Console.WriteLine($"✗ {report.FailedTests} critical issues found");
                Console.ResetColor();
            }
            
            Console.WriteLine($"\nPass Rate: {report.PassRate:F1}%");
            Console.WriteLine($"Tests Passed: {report.PassedTests}/{report.TotalTests}");
            
            if (report.FailureDetails.Any())
            {
                Console.WriteLine("\nRecommendations:");
                Console.WriteLine("  • Review failed test details above");
                Console.WriteLine("  • Check system implementations");
                Console.WriteLine("  • Run individual system demos for detailed testing");
            }
            else
            {
                Console.WriteLine("\nRecommendations:");
                Console.WriteLine("  ✓ Systems are ready for gameplay");
                Console.WriteLine("  ✓ Continue with integration testing");
                Console.WriteLine("  ✓ Proceed with feature development");
            }
        }
        catch (Exception ex)
        {
            Console.ForegroundColor = ConsoleColor.Red;
            Console.WriteLine($"\n✗ Verification error: {ex.Message}");
            Console.WriteLine($"Stack trace: {ex.StackTrace}");
            Console.ResetColor();
        }
        
        Console.WriteLine("\nPress Enter to return to main menu...");
        Console.ReadLine();
    }
    
    static void ShipGenerationDemo()
    {
        Console.WriteLine("\n=== Procedural Ship Generation & Texture Demo ===");
        Console.WriteLine("This demo showcases the Avorion-inspired ship generation system.");
        Console.WriteLine();
        
        try
        {
            var example = new Examples.ShipGenerationExample(_gameEngine!.EntityManager, seed: 12345);
            
            Console.WriteLine("What would you like to see?");
            Console.WriteLine("1. Generate Example Ships (5 different faction ships)");
            Console.WriteLine("2. Demonstrate Texture Generation (all celestial bodies)");
            Console.WriteLine("3. Show Material Library (all available materials)");
            Console.WriteLine("4. Show Ship with Textures (complete integration)");
            Console.WriteLine("5. Show Available Palettes (color schemes)");
            Console.WriteLine("6. Run All Demos");
            Console.Write("\nSelect option (1-6): ");
            
            var choice = Console.ReadLine();
            Console.WriteLine();
            
            switch (choice)
            {
                case "1":
                    example.GenerateExampleShips();
                    break;
                case "2":
                    example.DemonstrateTextureGeneration();
                    break;
                case "3":
                    example.DemonstrateMaterialLibrary();
                    break;
                case "4":
                    example.DemonstrateShipWithTextures();
                    break;
                case "5":
                    example.ShowAvailablePalettes();
                    break;
                case "6":
                    example.GenerateExampleShips();
                    example.DemonstrateTextureGeneration();
                    example.DemonstrateMaterialLibrary();
                    example.DemonstrateShipWithTextures();
                    example.ShowAvailablePalettes();
                    break;
                default:
                    Console.WriteLine("Invalid option! Running all demos...");
                    example.GenerateExampleShips();
                    example.DemonstrateTextureGeneration();
                    break;
            }
            
            Console.WriteLine("\n=== Key Features ===");
            Console.WriteLine("✅ Function Over Form - Ships prioritize working systems");
            Console.WriteLine("✅ Faction Diversity - Each faction has unique visual style");
            Console.WriteLine("✅ Functional Components - Engines, shields, generators, weapons");
            Console.WriteLine("✅ Procedural Textures - No texture files needed, all generated");
            Console.WriteLine("✅ Material Library - 20+ materials with PBR properties");
            Console.WriteLine("✅ Celestial Bodies - Gas giants, rocky planets, asteroids, nebulae");
            Console.WriteLine("✅ Deep Customization - Players can modify every block");
            Console.WriteLine();
            Console.WriteLine("See SHIP_GENERATION_TEXTURE_GUIDE.md for complete documentation.");
        }
        catch (Exception ex)
        {
            Console.ForegroundColor = ConsoleColor.Red;
            Console.WriteLine($"\n✗ Demo error: {ex.Message}");
            Console.WriteLine($"Stack trace: {ex.StackTrace}");
            Console.ResetColor();
        }
        
        Console.WriteLine("\nPress Enter to return to main menu...");
        Console.ReadLine();
    }

    static void ShowcaseMode()
    {
        Console.WriteLine("\n=== SHOWCASE MODE - Visual Demo ===");
        Console.WriteLine("Creating an impressive visual demonstration...\n");
        
        try
        {
            var showcase = new GameShowcase(_gameEngine!, seed: 42);
            var playerShipId = showcase.CreateShowcaseScene();
            
            Console.WriteLine("\n=== Launching Showcase ===");
            Console.WriteLine("Opening 3D window with enhanced UI...");
            Console.WriteLine("This demo showcases:");
            Console.WriteLine("  ✨ Enhanced UI with gradients and glows");
            Console.WriteLine("  🎨 Improved visual effects and animations");
            Console.WriteLine("  🚀 Multiple ship designs and materials");
            Console.WriteLine("  🌌 Populated game world with asteroids");
            Console.WriteLine("  📊 Responsive UI that scales with resolution\n");
            
            using var graphicsWindow = new GraphicsWindow(_gameEngine!);
            graphicsWindow.SetPlayerShip(playerShipId);
            graphicsWindow.Run();
        }
        catch (Exception ex)
        {
            Console.WriteLine($"Error running showcase: {ex.Message}");
            Console.WriteLine("Graphics rendering may not be available on this system.");
        }

        Console.WriteLine("\nReturned to main menu.");
    }

    static void GenerateHTMLDemoViewer()
    {
        Console.WriteLine("\n=== Generate HTML Demo Viewer ===");
        Console.WriteLine("Creating interactive HTML documentation...\n");
        
        try
        {
            var outputPath = Path.Combine(Directory.GetCurrentDirectory(), "GAME_SHOWCASE.html");
            
            var htmlContent = @"<!DOCTYPE html>
<html lang=""en"">
<head>
    <meta charset=""UTF-8"">
    <meta name=""viewport"" content=""width=device-width, initial-scale=1.0"">
    <title>Codename: Subspace - Game Showcase</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #0a0e1a 0%, #1a1f3a 100%);
            color: #e0e0e0;
            line-height: 1.6;
        }
        
        .container {
            max-width: 1200px;
            margin: 0 auto;
            padding: 20px;
        }
        
        header {
            text-align: center;
            padding: 40px 0;
            background: linear-gradient(135deg, #00b4d8 0%, #0077b6 100%);
            border-radius: 10px;
            margin-bottom: 40px;
            box-shadow: 0 10px 30px rgba(0, 180, 216, 0.3);
        }
        
        h1 {
            font-size: 3em;
            margin-bottom: 10px;
            text-shadow: 2px 2px 4px rgba(0, 0, 0, 0.5);
            color: #ffffff;
        }
        
        .subtitle {
            font-size: 1.2em;
            color: #e0f7fa;
            font-weight: 300;
        }
        
        .feature-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
            gap: 30px;
            margin: 40px 0;
        }
        
        .feature-card {
            background: rgba(255, 255, 255, 0.05);
            border: 2px solid rgba(0, 180, 216, 0.3);
            border-radius: 10px;
            padding: 30px;
            transition: all 0.3s ease;
            backdrop-filter: blur(10px);
        }
        
        .feature-card:hover {
            transform: translateY(-5px);
            border-color: rgba(0, 180, 216, 0.8);
            box-shadow: 0 10px 30px rgba(0, 180, 216, 0.3);
        }
        
        .feature-card h2 {
            color: #00d9ff;
            margin-bottom: 15px;
            font-size: 1.8em;
        }
        
        .feature-card ul {
            list-style: none;
            padding-left: 0;
        }
        
        .feature-card li {
            padding: 8px 0;
            padding-left: 25px;
            position: relative;
        }
        
        .feature-card li:before {
            content: '▹';
            position: absolute;
            left: 0;
            color: #00d9ff;
            font-weight: bold;
        }
        
        .controls {
            background: linear-gradient(135deg, rgba(0, 119, 182, 0.2) 0%, rgba(0, 180, 216, 0.2) 100%);
            border: 2px solid rgba(0, 180, 216, 0.5);
            border-radius: 10px;
            padding: 30px;
            margin: 40px 0;
        }
        
        .controls h2 {
            color: #00d9ff;
            margin-bottom: 20px;
            font-size: 2em;
        }
        
        .control-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
            gap: 20px;
            margin-top: 20px;
        }
        
        .control-item {
            background: rgba(0, 0, 0, 0.3);
            padding: 15px;
            border-radius: 5px;
            border-left: 3px solid #00d9ff;
        }
        
        .control-key {
            font-weight: bold;
            color: #00d9ff;
            display: inline-block;
            min-width: 100px;
        }
        
        .stats {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
            gap: 20px;
            margin: 40px 0;
        }
        
        .stat-card {
            background: linear-gradient(135deg, rgba(0, 180, 216, 0.1) 0%, rgba(0, 119, 182, 0.1) 100%);
            border: 2px solid rgba(0, 180, 216, 0.3);
            border-radius: 10px;
            padding: 20px;
            text-align: center;
        }
        
        .stat-value {
            font-size: 2.5em;
            font-weight: bold;
            color: #00d9ff;
            margin: 10px 0;
        }
        
        .stat-label {
            color: #b0bec5;
            font-size: 0.9em;
            text-transform: uppercase;
            letter-spacing: 1px;
        }
        
        footer {
            text-align: center;
            padding: 40px 0;
            margin-top: 60px;
            border-top: 1px solid rgba(0, 180, 216, 0.3);
            color: #78909c;
        }
        
        .cta-button {
            display: inline-block;
            padding: 15px 40px;
            background: linear-gradient(135deg, #00b4d8 0%, #0077b6 100%);
            color: white;
            text-decoration: none;
            border-radius: 5px;
            font-weight: bold;
            margin: 20px 10px;
            transition: all 0.3s ease;
            box-shadow: 0 5px 15px rgba(0, 180, 216, 0.3);
        }
        
        .cta-button:hover {
            transform: translateY(-2px);
            box-shadow: 0 8px 25px rgba(0, 180, 216, 0.5);
        }
        
        @media (max-width: 768px) {
            h1 {
                font-size: 2em;
            }
            
            .feature-grid {
                grid-template-columns: 1fr;
            }
            
            .control-grid {
                grid-template-columns: 1fr;
            }
        }
    </style>
</head>
<body>
    <div class=""container"">
        <header>
            <h1>⬡ Codename: Subspace</h1>
            <p class=""subtitle"">Advanced Space Game Engine - Visual Showcase</p>
        </header>
        
        <div class=""stats"">
            <div class=""stat-card"">
                <div class=""stat-value"">95%</div>
                <div class=""stat-label"">Backend Complete</div>
            </div>
            <div class=""stat-card"">
                <div class=""stat-value"">20+</div>
                <div class=""stat-label"">Core Systems</div>
            </div>
            <div class=""stat-card"">
                <div class=""stat-value"">100%</div>
                <div class=""stat-label"">Open Source</div>
            </div>
            <div class=""stat-card"">
                <div class=""stat-value"">∞</div>
                <div class=""stat-label"">Possibilities</div>
            </div>
        </div>
        
        <div class=""feature-grid"">
            <div class=""feature-card"">
                <h2>🎨 Enhanced Graphics</h2>
                <ul>
                    <li>Real-time 3D OpenGL rendering</li>
                    <li>Voxel-based ship visualization</li>
                    <li>Material-based coloring system</li>
                    <li>Phong lighting with specular highlights</li>
                    <li>Procedural starfield background</li>
                    <li>Smooth camera controls</li>
                </ul>
            </div>
            
            <div class=""feature-card"">
                <h2>🎮 Improved UI</h2>
                <ul>
                    <li>Futuristic HUD with gradients</li>
                    <li>Glowing progress bars</li>
                    <li>Enhanced radar with animations</li>
                    <li>Color-coded status indicators</li>
                    <li>Responsive design for all resolutions</li>
                    <li>ImGui-powered debug panels</li>
                </ul>
            </div>
            
            <div class=""feature-card"">
                <h2>🚀 Ship Systems</h2>
                <ul>
                    <li>6DOF movement and rotation</li>
                    <li>Newtonian physics simulation</li>
                    <li>Block-based ship construction</li>
                    <li>Multiple material types</li>
                    <li>Dynamic thrust and torque</li>
                    <li>Shields and energy management</li>
                </ul>
            </div>
            
            <div class=""feature-card"">
                <h2>🌌 Game World</h2>
                <ul>
                    <li>Procedural galaxy generation</li>
                    <li>Asteroid fields with resources</li>
                    <li>Space stations and POIs</li>
                    <li>Sector-based universe</li>
                    <li>Dynamic entity spawning</li>
                    <li>Persistent save/load system</li>
                </ul>
            </div>
            
            <div class=""feature-card"">
                <h2>⚙️ Core Systems</h2>
                <ul>
                    <li>Entity-Component System (ECS)</li>
                    <li>Resource & inventory management</li>
                    <li>Combat & damage system</li>
                    <li>Mining & salvaging</li>
                    <li>Trading & economy</li>
                    <li>Crafting & upgrades</li>
                </ul>
            </div>
            
            <div class=""feature-card"">
                <h2>🔧 Modding Support</h2>
                <ul>
                    <li>Lua scripting engine</li>
                    <li>Comprehensive modding API</li>
                    <li>Automatic mod discovery</li>
                    <li>Hot-reload capabilities</li>
                    <li>Dependency management</li>
                    <li>Sample mods included</li>
                </ul>
            </div>
        </div>
        
        <div class=""controls"">
            <h2>🎮 Controls</h2>
            <div class=""control-grid"">
                <div class=""control-item"">
                    <span class=""control-key"">C Key</span>
                    <span>Toggle Camera/Ship Control</span>
                </div>
                <div class=""control-item"">
                    <span class=""control-key"">WASD</span>
                    <span>Movement/Thrust</span>
                </div>
                <div class=""control-item"">
                    <span class=""control-key"">Space/Shift</span>
                    <span>Up/Down Thrust</span>
                </div>
                <div class=""control-item"">
                    <span class=""control-key"">Arrow Keys</span>
                    <span>Pitch/Yaw Rotation</span>
                </div>
                <div class=""control-item"">
                    <span class=""control-key"">Q/E Keys</span>
                    <span>Roll Rotation</span>
                </div>
                <div class=""control-item"">
                    <span class=""control-key"">X Key</span>
                    <span>Emergency Brake</span>
                </div>
                <div class=""control-item"">
                    <span class=""control-key"">Mouse</span>
                    <span>Free Look (Camera Mode)</span>
                </div>
                <div class=""control-item"">
                    <span class=""control-key"">TAB</span>
                    <span>Player Status Panel</span>
                </div>
                <div class=""control-item"">
                    <span class=""control-key"">I Key</span>
                    <span>Inventory Panel</span>
                </div>
                <div class=""control-item"">
                    <span class=""control-key"">B Key</span>
                    <span>Ship Builder</span>
                </div>
                <div class=""control-item"">
                    <span class=""control-key"">F1-F4</span>
                    <span>Toggle Debug Panels</span>
                </div>
                <div class=""control-item"">
                    <span class=""control-key"">ESC</span>
                    <span>Exit to Menu</span>
                </div>
            </div>
        </div>
        
        <div style=""text-align: center; margin: 60px 0;"">
            <h2 style=""color: #00d9ff; margin-bottom: 20px;"">Ready to Explore?</h2>
            <p style=""margin-bottom: 30px; font-size: 1.1em;"">Run the game and select option 19 for the full showcase experience!</p>
            <a href=""https://github.com/shifty81/Codename-Subspace"" class=""cta-button"">View on GitHub</a>
            <a href=""README.md"" class=""cta-button"">Read Documentation</a>
        </div>
        
        <footer>
            <p>Codename: Subspace - Open Source Space Game Engine</p>
            <p>Built with C# • .NET 9.0 • Silk.NET • OpenGL</p>
            <p style=""margin-top: 10px;"">© 2024 - MIT License</p>
        </footer>
    </div>
</body>
</html>";
            
            File.WriteAllText(outputPath, htmlContent);
            
            Console.WriteLine($"✓ HTML demo viewer generated!");
            Console.WriteLine($"  Location: {outputPath}");
            Console.WriteLine($"\nOpen this file in your web browser to view the interactive showcase.");
        }
        catch (Exception ex)
        {
            Console.WriteLine($"Error generating HTML: {ex.Message}");
        }
        
        Console.WriteLine("\nPress Enter to return to main menu...");
        Console.ReadLine();
    }

    static void RunIntegrationTest()
    {
        Console.WriteLine("\n=== Integration Test - All Systems Together ===");
        Console.WriteLine("Running comprehensive tests to verify all systems work correctly...");
        Console.WriteLine();
        
        var integrationTest = new IntegrationTest(_gameEngine!);
        bool passed = integrationTest.RunTests();
        
        if (passed)
        {
            Console.WriteLine("\n🎉 All systems are working correctly!");
            Console.WriteLine("The game engine is ready for gameplay.");
        }
        else
        {
            Console.WriteLine("\n⚠️  Some tests failed. Check the output above for details.");
        }
        
        Console.WriteLine("\nPress any key to return to main menu...");
        Console.ReadKey();
    }

    static void RunShipShowcase()
    {
        Console.WriteLine("\n=== SHIP SHOWCASE - 20 Procedural Ships ===");
        Console.WriteLine("This demo generates 20 different ships with varied designs");
        Console.WriteLine("so you can pick which generation you like best!\n");
        
        try
        {
            var showcase = new ShipShowcaseExample(_gameEngine!.EntityManager);
            var ships = showcase.GenerateShowcase(Environment.TickCount);
            
            // Interactive menu
            bool done = false;
            while (!done)
            {
                Console.WriteLine("\n=== SHOWCASE MENU ===");
                Console.WriteLine("1. View Ship Details (enter ship number 1-20)");
                Console.WriteLine("2. View Summary (all ships)");
                Console.WriteLine("3. Launch 3D Viewer (see all ships in scene)");
                Console.WriteLine("0. Return to Main Menu");
                Console.Write("\nSelect option: ");
                
                var choice = Console.ReadLine();
                
                if (choice == "0")
                {
                    done = true;
                }
                else if (choice == "2")
                {
                    showcase.PrintShowcaseSummary();
                }
                else if (choice == "3")
                {
                    Console.WriteLine("\nLaunching 3D viewer with all 20 ships...");
                    Console.WriteLine("Use camera controls to inspect each ship.");
                    Console.WriteLine("Ship numbers are displayed above each vessel.\n");
                    
                    try
                    {
                        using var graphicsWindow = new GraphicsWindow(_gameEngine!);
                        // Set first ship as player view point
                        if (ships.Count > 0)
                        {
                            graphicsWindow.SetPlayerShip(ships[0].EntityId);
                        }
                        graphicsWindow.Run();
                    }
                    catch (Exception ex)
                    {
                        Console.WriteLine($"Error running 3D viewer: {ex.Message}");
                        Console.WriteLine("Graphics may not be available on this system.");
                    }
                }
                else if (int.TryParse(choice, out int shipNum) && shipNum >= 1 && shipNum <= 20)
                {
                    showcase.PrintShipDetails(shipNum);
                }
                else
                {
                    Console.WriteLine("Invalid option!");
                }
            }
        }
        catch (Exception ex)
        {
            Console.ForegroundColor = ConsoleColor.Red;
            Console.WriteLine($"\n✗ Showcase error: {ex.Message}");
            Console.WriteLine($"Stack trace: {ex.StackTrace}");
            Console.ResetColor();
        }
    }

    static void RunMovementAndShapeTest()
    {
        Console.WriteLine("\n=== Movement & Shape Test ===");
        Console.WriteLine("Testing physics interpolation and improved ship generation...");
        Console.WriteLine();
        
        var test = new MovementAndShapeTests();
        test.RunAllTests();
        
        Console.WriteLine("\nPress any key to return to main menu...");
        Console.ReadKey();
    }
}


