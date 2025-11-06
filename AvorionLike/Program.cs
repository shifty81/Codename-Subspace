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
        Console.WriteLine("  Codename:Subspace - Space Game Engine");
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
            Console.WriteLine("1. NEW GAME - Start Full Gameplay Experience [NEW!]");
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
        
        Console.WriteLine("Building your starter ship...");
        
        // Core hull (center)
        voxelComponent.AddBlock(new VoxelBlock(
            new Vector3(0, 0, 0),
            new Vector3(3, 3, 3),
            "Titanium",
            BlockType.Hull
        ));
        
        // Main engines (rear)
        voxelComponent.AddBlock(new VoxelBlock(
            new Vector3(-5, 0, 0),
            new Vector3(2, 2, 2),
            "Iron",
            BlockType.Engine
        ));
        
        voxelComponent.AddBlock(new VoxelBlock(
            new Vector3(-5, 2, 0),
            new Vector3(2, 2, 2),
            "Iron",
            BlockType.Engine
        ));
        
        // Maneuvering thrusters
        voxelComponent.AddBlock(new VoxelBlock(
            new Vector3(0, 4, 0),
            new Vector3(1.5f, 1.5f, 1.5f),
            "Iron",
            BlockType.Thruster
        ));
        
        voxelComponent.AddBlock(new VoxelBlock(
            new Vector3(0, -4, 0),
            new Vector3(1.5f, 1.5f, 1.5f),
            "Iron",
            BlockType.Thruster
        ));
        
        // Generator
        voxelComponent.AddBlock(new VoxelBlock(
            new Vector3(3, 0, 0),
            new Vector3(2, 2, 2),
            "Iron",
            BlockType.Generator
        ));
        
        // Shield generator
        voxelComponent.AddBlock(new VoxelBlock(
            new Vector3(0, 0, 3),
            new Vector3(2, 2, 2),
            "Titanium",
            BlockType.ShieldGenerator
        ));
        
        // Gyro arrays for rotation
        voxelComponent.AddBlock(new VoxelBlock(
            new Vector3(0, 2, 2),
            new Vector3(1.5f, 1.5f, 1.5f),
            "Iron",
            BlockType.GyroArray
        ));
        
        voxelComponent.AddBlock(new VoxelBlock(
            new Vector3(0, -2, -2),
            new Vector3(1.5f, 1.5f, 1.5f),
            "Iron",
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
        
        // Create some nearby asteroids for mining
        Console.WriteLine("\nPopulating nearby space...");
        var random = new Random();
        for (int i = 0; i < 5; i++)
        {
            var asteroid = _gameEngine.EntityManager.CreateEntity($"Asteroid {i + 1}");
            
            var asteroidVoxel = new VoxelStructureComponent();
            asteroidVoxel.AddBlock(new VoxelBlock(
                new Vector3(0, 0, 0),
                new Vector3(5, 5, 5),
                "Iron",
                BlockType.Armor
            ));
            _gameEngine.EntityManager.AddComponent(asteroid.Id, asteroidVoxel);
            
            var asteroidPhysics = new PhysicsComponent
            {
                Position = new Vector3(
                    random.Next(-100, 100),
                    random.Next(-100, 100),
                    random.Next(-100, 100)
                ),
                Mass = 10000f
            };
            _gameEngine.EntityManager.AddComponent(asteroid.Id, asteroidPhysics);
            
            Console.WriteLine($"  ✓ Created {asteroid.Name} at position ({asteroidPhysics.Position.X:F0}, {asteroidPhysics.Position.Y:F0}, {asteroidPhysics.Position.Z:F0})");
        }
        
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
}


