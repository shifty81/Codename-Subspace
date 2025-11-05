using AvorionLike.Core;
using AvorionLike.Core.ECS;
using AvorionLike.Core.Voxel;
using AvorionLike.Core.Physics;
using AvorionLike.Core.Resources;
using AvorionLike.Core.RPG;
using AvorionLike.Core.Graphics;
using AvorionLike.Core.Combat;
using AvorionLike.Core.Navigation;
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
        Console.WriteLine("   AvorionLike - Custom Game Engine Demo");
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
        Console.WriteLine("\nThank you for using AvorionLike!");
    }

    static void ShowMainMenu()
    {
        while (_running)
        {
            Console.WriteLine("\n=== Main Menu ===");
            Console.WriteLine("1. Engine Demo - Create Test Ship");
            Console.WriteLine("2. Voxel System Demo - Build Ship Structure");
            Console.WriteLine("3. Physics Demo - Simulate Movement");
            Console.WriteLine("4. Procedural Generation - Generate Galaxy Sector");
            Console.WriteLine("5. Resource Management Demo");
            Console.WriteLine("6. RPG Systems Demo - Trading & Progression");
            Console.WriteLine("7. Scripting Demo - Execute Lua Script");
            Console.WriteLine("8. Multiplayer - Start Server");
            Console.WriteLine("9. View Statistics");
            Console.WriteLine("10. 3D Graphics Demo - Visualize Voxel Ships [NEW]");
            Console.WriteLine("11. Persistence Demo - Save/Load Game [NEW]");
            Console.WriteLine("0. Exit");
            Console.Write("\nSelect option: ");

            var choice = Console.ReadLine();

            switch (choice)
            {
                case "1":
                    CreateTestShipDemo();
                    break;
                case "2":
                    VoxelSystemDemo();
                    break;
                case "3":
                    PhysicsDemo();
                    break;
                case "4":
                    ProceduralGenerationDemo();
                    break;
                case "5":
                    ResourceManagementDemo();
                    break;
                case "6":
                    RPGSystemsDemo();
                    break;
                case "7":
                    ScriptingDemo();
                    break;
                case "8":
                    MultiplayerDemo();
                    break;
                case "9":
                    ShowStatistics();
                    break;
                case "10":
                    GraphicsDemo();
                    break;
                case "11":
                    PersistenceDemo();
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

    static void CreateTestShipDemo()
    {
        Console.WriteLine("\n=== Create Test Ship Demo ===");
        
        // Create a test ship entity
        var ship = _gameEngine!.EntityManager.CreateEntity("Player Ship");
        
        // Add voxel structure
        var voxelComponent = new VoxelStructureComponent();
        Console.WriteLine("Building ship structure with voxel blocks...");
        
        // Create a ship with different functional blocks
        // Core hull
        voxelComponent.AddBlock(new VoxelBlock(
            new Vector3(0, 0, 0),
            new Vector3(3, 3, 3),
            "Titanium",
            BlockType.Hull
        ));
        
        // Engines for forward thrust
        voxelComponent.AddBlock(new VoxelBlock(
            new Vector3(-4, 0, 0),
            new Vector3(2, 2, 2),
            "Iron",
            BlockType.Engine
        ));
        
        // Thrusters for maneuvering
        voxelComponent.AddBlock(new VoxelBlock(
            new Vector3(4, 0, 0),
            new Vector3(1.5f, 1.5f, 1.5f),
            "Iron",
            BlockType.Thruster
        ));
        
        // Generator
        voxelComponent.AddBlock(new VoxelBlock(
            new Vector3(0, -3, 0),
            new Vector3(2, 2, 2),
            "Iron",
            BlockType.Generator
        ));
        
        // Shield generator
        voxelComponent.AddBlock(new VoxelBlock(
            new Vector3(0, 3, 0),
            new Vector3(2, 2, 2),
            "Titanium",
            BlockType.ShieldGenerator
        ));
        
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
        
        Console.WriteLine($"\n✓ Ship created successfully!");
        Console.WriteLine($"  Name: {ship.Name}");
        Console.WriteLine($"  ID: {ship.Id}");
        Console.WriteLine($"  Voxel blocks: {voxelComponent.Blocks.Count}");
        Console.WriteLine($"  Total mass: {voxelComponent.TotalMass:F2} kg");
        Console.WriteLine($"  Moment of inertia: {voxelComponent.MomentOfInertia:F2}");
        Console.WriteLine($"  Center of mass: {voxelComponent.CenterOfMass}");
        Console.WriteLine($"  Total thrust: {voxelComponent.TotalThrust:F2} N");
        Console.WriteLine($"  Total torque: {voxelComponent.TotalTorque:F2} Nm");
        Console.WriteLine($"  Power generation: {voxelComponent.PowerGeneration:F2} W");
        Console.WriteLine($"  Shield capacity: {voxelComponent.ShieldCapacity:F2}");
        Console.WriteLine($"  Structural integrity: {voxelComponent.StructuralIntegrity:F1}%");
        Console.WriteLine($"  Position: {physicsComponent.Position}");
        Console.WriteLine($"  Velocity: {physicsComponent.Velocity}");
        Console.WriteLine($"  Credits: {inventoryComponent.Inventory.GetResourceAmount(ResourceType.Credits)}");
    }

    static void VoxelSystemDemo()
    {
        Console.WriteLine("\n=== Voxel System Demo ===");
        Console.WriteLine("Building a custom ship structure with functional blocks...\n");

        var structure = new VoxelStructureComponent();
        
        // Create a functional ship design
        Console.WriteLine("Creating advanced ship design:");
        
        // Core (Titanium for better durability)
        structure.AddBlock(new VoxelBlock(new Vector3(0, 0, 0), new Vector3(3, 3, 3), "Titanium", BlockType.Hull));
        
        // Engines (back)
        structure.AddBlock(new VoxelBlock(new Vector3(-4, 0, 0), new Vector3(2, 2, 2), "Iron", BlockType.Engine));
        structure.AddBlock(new VoxelBlock(new Vector3(-4, 2, 0), new Vector3(2, 2, 2), "Iron", BlockType.Engine));
        
        // Thrusters (sides for maneuvering)
        structure.AddBlock(new VoxelBlock(new Vector3(0, 4, 0), new Vector3(1.5f, 1.5f, 1.5f), "Iron", BlockType.Thruster));
        structure.AddBlock(new VoxelBlock(new Vector3(0, -4, 0), new Vector3(1.5f, 1.5f, 1.5f), "Iron", BlockType.Thruster));
        
        // Gyro arrays for rotation
        structure.AddBlock(new VoxelBlock(new Vector3(0, 0, 3), new Vector3(2, 2, 2), "Iron", BlockType.GyroArray));
        structure.AddBlock(new VoxelBlock(new Vector3(0, 0, -3), new Vector3(2, 2, 2), "Iron", BlockType.GyroArray));
        
        // Generator
        structure.AddBlock(new VoxelBlock(new Vector3(4, 0, 0), new Vector3(2, 2, 2), "Titanium", BlockType.Generator));
        
        // Shield generator
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
        var loot = lootSystem.GenerateLoot(5);
        
        Console.WriteLine("Loot dropped from level 5 enemy:");
        foreach (var drop in loot)
        {
            Console.WriteLine($"  {drop.Resource}: {drop.Amount} (Chance: {drop.DropChance * 100:F0}%)");
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
                
                // Create different ship designs
                switch (i)
                {
                    case 0: // Fighter with engines
                        // Core
                        voxelComponent.AddBlock(new VoxelBlock(
                            new Vector3(0, 0, 0),
                            new Vector3(3, 2, 2),
                            "Titanium",
                            BlockType.Hull
                        ));
                        // Engines
                        voxelComponent.AddBlock(new VoxelBlock(
                            new Vector3(-4, 0, 0),
                            new Vector3(2, 2, 2),
                            "Iron",
                            BlockType.Engine
                        ));
                        // Wings
                        voxelComponent.AddBlock(new VoxelBlock(new Vector3(0, 3, 0), new Vector3(2, 1, 2), "Iron", BlockType.Armor));
                        voxelComponent.AddBlock(new VoxelBlock(new Vector3(0, -3, 0), new Vector3(2, 1, 2), "Iron", BlockType.Armor));
                        break;
                    
                    case 1: // Cross-shaped ship with shields
                        voxelComponent.AddBlock(new VoxelBlock(new Vector3(0, 0, 0), new Vector3(3, 3, 3), "Titanium", BlockType.Hull));
                        voxelComponent.AddBlock(new VoxelBlock(new Vector3(4, 0, 0), new Vector3(2, 2, 2), "Iron", BlockType.Generator));
                        voxelComponent.AddBlock(new VoxelBlock(new Vector3(-4, 0, 0), new Vector3(2, 2, 2), "Iron", BlockType.Engine));
                        voxelComponent.AddBlock(new VoxelBlock(new Vector3(0, 4, 0), new Vector3(2, 2, 2), "Naonite", BlockType.ShieldGenerator));
                        voxelComponent.AddBlock(new VoxelBlock(new Vector3(0, -4, 0), new Vector3(2, 2, 2), "Naonite", BlockType.Thruster));
                        break;
                    
                    case 2: // Compact cargo ship
                        for (int x = 0; x < 3; x++)
                        {
                            for (int y = 0; y < 2; y++)
                            {
                                var blockType = (x == 1 && y == 0) ? BlockType.Generator : BlockType.Cargo;
                                voxelComponent.AddBlock(new VoxelBlock(
                                    new Vector3(x * 3, y * 3, 0),
                                    new Vector3(2.5f, 2.5f, 2.5f),
                                    x == 1 ? "Trinium" : "Xanion",
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
                            Console.WriteLine($"    Physics: Position=({physics.Position.X:F1}, {physics.Position.Y:F1}, {physics.Position.Z:F1})");
                        }
                        if (_gameEngine.EntityManager.HasComponent<VoxelStructureComponent>(entity.Id))
                        {
                            var voxel = _gameEngine.EntityManager.GetComponent<VoxelStructureComponent>(entity.Id);
                            Console.WriteLine($"    Voxels: {voxel.Blocks.Count} blocks, Mass={voxel.TotalMass:F1}kg");
                        }
                        if (_gameEngine.EntityManager.HasComponent<InventoryComponent>(entity.Id))
                        {
                            var inventory = _gameEngine.EntityManager.GetComponent<InventoryComponent>(entity.Id);
                            Console.WriteLine($"    Inventory: {inventory.Inventory.CurrentCapacity}/{inventory.Inventory.MaxCapacity}");
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
                                Console.WriteLine($"    Physics: Position=({physics.Position.X:F1}, {physics.Position.Y:F1}, {physics.Position.Z:F1})");
                            }
                            if (_gameEngine.EntityManager.HasComponent<VoxelStructureComponent>(entity.Id))
                            {
                                var voxel = _gameEngine.EntityManager.GetComponent<VoxelStructureComponent>(entity.Id);
                                Console.WriteLine($"    Voxels: {voxel.Blocks.Count} blocks, Mass={voxel.TotalMass:F1}kg");
                            }
                            if (_gameEngine.EntityManager.HasComponent<InventoryComponent>(entity.Id))
                            {
                                var inventory = _gameEngine.EntityManager.GetComponent<InventoryComponent>(entity.Id);
                                Console.WriteLine($"    Inventory: {inventory.Inventory.CurrentCapacity}/{inventory.Inventory.MaxCapacity}");
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
}


