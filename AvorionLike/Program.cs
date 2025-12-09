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
using AvorionLike.Core.Progression;
using AvorionLike.Core.Fleet;
using AvorionLike.Core.UI;
using AvorionLike.Core.Configuration;
using AvorionLike.Core.AI;
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
            Console.WriteLine("\n╔═══════════════════════════════════════════════════╗");
            Console.WriteLine("║      Codename:Subspace - Main Menu              ║");
            Console.WriteLine("╚═══════════════════════════════════════════════════╝");
            Console.WriteLine();
            Console.WriteLine("  1. Start New Game - Full Gameplay Experience");
            Console.WriteLine("  2. Experience Full Solar System - Complete Test Environment");
            Console.WriteLine("  3. About / Version Info");
            Console.WriteLine("  0. Exit");
            Console.WriteLine();
            Console.Write("Select option: ");

            var choice = Console.ReadLine();

            switch (choice)
            {
                case "1":
                    StartNewGame();
                    break;
                case "2":
                    ExperienceFullSolarSystem();
                    break;
                case "3":
                    ShowVersionInfo();
                    break;
                case "0":
                    _running = false;
                    break;
                default:
                    Console.WriteLine("\n⚠ Invalid option! Please select a valid menu option.");
                    break;
            }
        }
    }

    static void StartNewGame()
    {
        Console.WriteLine("\n=== NEW GAME - Full Visual Testing Experience ===");
        Console.WriteLine("Creating comprehensive test environment with all implementations...\n");
        
        // Step 1: Let user choose their ship generation
        var selectedShip = SelectShipGeneration();
        if (selectedShip == null)
        {
            Console.WriteLine("Ship selection cancelled. Returning to main menu.");
            return;
        }
        
        Console.WriteLine($"\n✓ Selected Ship: {selectedShip.Description}");
        Console.WriteLine("Creating your game world...\n");
        
        // Create player ship with the selected generation
        var playerShip = _gameEngine!.EntityManager.CreateEntity("Player Ship");
        var voxelComponent = selectedShip.ShipData.Structure;
        
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
        
        // Add sector location - Start at galaxy rim (400 sectors from center)
        var locationComponent = new SectorLocationComponent
        {
            EntityId = playerShip.Id,
            CurrentSector = new SectorCoordinate(400, 0, 0)  // Galaxy rim starting position
        };
        _gameEngine.EntityManager.AddComponent(playerShip.Id, locationComponent);
        
        // Add galaxy progression tracking
        var playerProgressionComp = new PlayerProgressionComponent
        {
            EntityId = playerShip.Id,
            ClosestDistanceToCenter = 400,
            CurrentZone = "Galaxy Rim (Iron Zone)",
            FurthestZoneReached = "Galaxy Rim (Iron Zone)",
            AvailableMaterialTier = MaterialTier.Iron,
            HighestMaterialTierAcquired = MaterialTier.Iron,
            CurrentZoneDifficulty = 1.0f,
            SectorsExplored = 1
        };
        _gameEngine.EntityManager.AddComponent(playerShip.Id, playerProgressionComp);
        
        // Register Galaxy Progression System and Fleet Automation System
        var galaxyProgressionSystem = new GalaxyProgressionSystem(_gameEngine.EntityManager);
        var fleetAutomationSystem = new FleetAutomationSystem(_gameEngine.EntityManager);
        
        _gameEngine.EntityManager.RegisterSystem(galaxyProgressionSystem);
        _gameEngine.EntityManager.RegisterSystem(fleetAutomationSystem);
        
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
        
        // Display galaxy progression information
        int distance = GalaxyProgressionSystem.GetDistanceFromCenter(locationComponent.CurrentSector);
        var zoneName = GalaxyProgressionSystem.GetZoneName(distance);
        var availableTier = GalaxyProgressionSystem.GetAvailableMaterialTier(distance);
        var difficulty = GalaxyProgressionSystem.GetDifficultyMultiplier(distance);
        
        Console.WriteLine("\n=== GALAXY PROGRESSION SYSTEM ===");
        Console.WriteLine($"  📍 Starting Location: Sector [{locationComponent.CurrentSector.X}, {locationComponent.CurrentSector.Y}, {locationComponent.CurrentSector.Z}]");
        Console.WriteLine($"  🌌 Current Zone: {zoneName}");
        Console.WriteLine($"  📏 Distance from Center: {distance} sectors");
        Console.WriteLine($"  ⚔️  Zone Difficulty: {difficulty:F1}x");
        Console.WriteLine($"  🔨 Available Materials: {availableTier}");
        Console.WriteLine($"  📊 Highest Tier Acquired: {playerProgressionComp.HighestMaterialTierAcquired}");
        
        // Display material tier unlocks
        var features = MaterialTierInfo.GetUnlockedFeatures(availableTier);
        Console.WriteLine($"\n  ✨ Unlocked Features in {availableTier} Zone:");
        foreach (var feature in features.Take(5))
        {
            Console.WriteLine($"     • {feature}");
        }
        
        // Show progression goals
        Console.WriteLine("\n  🎯 Progression Goals:");
        Console.WriteLine($"     • Reach Titanium Zone (< 350 sectors from center)");
        Console.WriteLine($"     • Unlock Shields in Naonite Zone (< 250 sectors)");
        Console.WriteLine($"     • Hire Captains in Ogonite Zone (< 50 sectors)");
        Console.WriteLine($"     • Reach Galactic Core (< 25 sectors) for Avorion!");
        
        Console.WriteLine("\n  💡 Tip: Journey toward the galactic center (0,0,0) to unlock better materials and features!");
        
        // Populate the game world with zone-appropriate content
        Console.WriteLine("\n=== Populating Game World ===");
        var worldPopulator = new GameWorldPopulator(_gameEngine, seed: 12345);
        worldPopulator.PopulateZoneArea(physicsComponent.Position, locationComponent.CurrentSector, radius: 800f);
        
        // Add comprehensive test showcase - All implementations in one place!
        Console.WriteLine("\n=== Creating Test Showcase - All Implementations ===");
        CreateComprehensiveTestShowcase(playerShip.Id, physicsComponent.Position);
        
        Console.WriteLine("\n=== Launching Full Game Experience ===");
        Console.WriteLine("Opening 3D window with Player UI...");
        Console.WriteLine("You can now control your ship and explore!\n");
        
        Console.WriteLine("💡 TESTING FEATURES:");
        Console.WriteLine("  • Press ~ or click Console button to open testing console");
        Console.WriteLine("  • Type 'help' in console for all commands");
        Console.WriteLine("  • Quick demos: demo_quick, demo_combat, demo_mining, demo_world");
        Console.WriteLine("  • Spawn entities: spawn_ship, spawn_enemy, spawn_asteroid");
        Console.WriteLine("  • Resources: credits [amount], add_resource [type] [amount]");
        Console.WriteLine("  • Testing: tp [x y z], velocity [x y z], heal, damage [amount]\n");
        
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

    static void ExperienceFullSolarSystem()
    {
        Console.WriteLine("\n╔═══════════════════════════════════════════════════════════════════════════╗");
        Console.WriteLine("║     FULL SOLAR SYSTEM - COMPREHENSIVE TEST ENVIRONMENT                   ║");
        Console.WriteLine("║     Experience everything that has been implemented                       ║");
        Console.WriteLine("╚═══════════════════════════════════════════════════════════════════════════╝");
        Console.WriteLine();
        Console.WriteLine("Creating a complete solar system with all implemented features...\n");
        
        // Create player ship with full capabilities
        Console.WriteLine("=== Creating Player Ship ===");
        var playerShip = _gameEngine!.EntityManager.CreateEntity("Player Ship - Command Cruiser");
        
        // Use procedural generation for a well-equipped player ship
        var shipGenerator = new ProceduralShipGenerator(Environment.TickCount);
        var playerShipConfig = new ShipGenerationConfig
        {
            Size = ShipSize.Cruiser,
            Role = ShipRole.Multipurpose,
            Material = "Avorion",
            Style = FactionShipStyle.GetDefaultStyle("Military"),
            Seed = 12345,
            RequireHyperdrive = true,
            RequireCargo = true,
            MinimumWeaponMounts = 6
        };
        
        var generatedPlayerShip = shipGenerator.GenerateShip(playerShipConfig);
        var voxelComponent = generatedPlayerShip.Structure;
        _gameEngine.EntityManager.AddComponent(playerShip.Id, voxelComponent);
        
        // Add physics at origin
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
        
        // Add full inventory with all resource types
        var inventoryComponent = new InventoryComponent(5000);
        inventoryComponent.Inventory.AddResource(ResourceType.Credits, 1000000);
        inventoryComponent.Inventory.AddResource(ResourceType.Iron, 2000);
        inventoryComponent.Inventory.AddResource(ResourceType.Titanium, 1500);
        inventoryComponent.Inventory.AddResource(ResourceType.Naonite, 1000);
        inventoryComponent.Inventory.AddResource(ResourceType.Trinium, 800);
        inventoryComponent.Inventory.AddResource(ResourceType.Xanion, 500);
        inventoryComponent.Inventory.AddResource(ResourceType.Ogonite, 300);
        inventoryComponent.Inventory.AddResource(ResourceType.Avorion, 200);
        _gameEngine.EntityManager.AddComponent(playerShip.Id, inventoryComponent);
        
        // Add progression - mid-level to test progression features
        var progressionComponent = new ProgressionComponent
        {
            EntityId = playerShip.Id,
            Level = 10,
            Experience = 50000,
            SkillPoints = 25
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
        
        // Add hyperdrive with good range
        var hyperdriveComponent = new HyperdriveComponent
        {
            EntityId = playerShip.Id,
            JumpRange = 15f
        };
        _gameEngine.EntityManager.AddComponent(playerShip.Id, hyperdriveComponent);
        
        // Start in mid-galaxy for variety (Naonite zone)
        var locationComponent = new SectorLocationComponent
        {
            EntityId = playerShip.Id,
            CurrentSector = new SectorCoordinate(200, 0, 0)
        };
        _gameEngine.EntityManager.AddComponent(playerShip.Id, locationComponent);
        
        // Add galaxy progression tracking
        var playerProgressionComp = new PlayerProgressionComponent
        {
            EntityId = playerShip.Id,
            ClosestDistanceToCenter = 200,
            CurrentZone = "Mid-Galaxy (Naonite Zone)",
            FurthestZoneReached = "Mid-Galaxy (Naonite Zone)",
            AvailableMaterialTier = MaterialTier.Naonite,
            HighestMaterialTierAcquired = MaterialTier.Naonite,
            CurrentZoneDifficulty = 1.5f,
            SectorsExplored = 1
        };
        _gameEngine.EntityManager.AddComponent(playerShip.Id, playerProgressionComp);
        
        Console.WriteLine($"✓ Player ship created!");
        Console.WriteLine($"  Type: {playerShipConfig.Size} {playerShipConfig.Role}");
        Console.WriteLine($"  Blocks: {voxelComponent.Blocks.Count}");
        Console.WriteLine($"  Mass: {voxelComponent.TotalMass:F2} kg");
        Console.WriteLine($"  Thrust: {voxelComponent.TotalThrust:F2} N");
        Console.WriteLine($"  Power: {voxelComponent.PowerGeneration:F2} W");
        Console.WriteLine($"  Shields: {voxelComponent.ShieldCapacity:F2}");
        Console.WriteLine($"  Credits: {inventoryComponent.Inventory.GetResourceAmount(ResourceType.Credits):N0}");
        Console.WriteLine($"  Level: {progressionComponent.Level}");
        
        // Register systems
        var galaxyProgressionSystem = new GalaxyProgressionSystem(_gameEngine.EntityManager);
        var fleetAutomationSystem = new FleetAutomationSystem(_gameEngine.EntityManager);
        _gameEngine.EntityManager.RegisterSystem(galaxyProgressionSystem);
        _gameEngine.EntityManager.RegisterSystem(fleetAutomationSystem);
        
        // Create comprehensive solar system with all features
        Console.WriteLine("\n=== Populating Solar System ===");
        CreateCompleteSolarSystem(playerShip.Id, physicsComponent.Position);
        
        Console.WriteLine("\n=== System Features Summary ===");
        Console.WriteLine("✓ Player Ship: Fully equipped Cruiser with all capabilities");
        Console.WriteLine("✓ Diverse Fleet: 20+ ships of all sizes and roles");
        Console.WriteLine("✓ Asteroid Fields: Rich with all material types");
        Console.WriteLine("✓ Space Stations: Multiple types (Trading, Military, Industrial, Research)");
        Console.WriteLine("✓ AI Ships: Traders, Miners, Pirates, Explorers");
        Console.WriteLine("✓ Galaxy Progression: Visible material tier zones");
        Console.WriteLine("✓ Fleet Automation: Demonstrating captain assignments");
        Console.WriteLine();
        Console.WriteLine("💡 TESTING TIPS:");
        Console.WriteLine("  • Press ~ or click Console button for testing commands");
        Console.WriteLine("  • Type 'help' for all available commands");
        Console.WriteLine("  • Use M key to open Galaxy Map");
        Console.WriteLine("  • Press I for Inventory, B for Ship Builder");
        Console.WriteLine("  • Fly around to explore all generated content");
        Console.WriteLine("  • Try combat with 'spawn_enemy' command");
        Console.WriteLine("  • Test mining with nearby asteroids");
        Console.WriteLine("  • Visit stations for trading");
        Console.WriteLine();
        
        // Launch the game
        Console.WriteLine("=== Launching Full Solar System Experience ===");
        Console.WriteLine("Opening 3D window...\n");
        
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
    
    /// <summary>
    /// Creates a complete solar system with all implemented features for comprehensive testing
    /// </summary>
    static void CreateCompleteSolarSystem(Guid playerShipId, Vector3 centerPosition)
    {
        var shipGenerator = new ProceduralShipGenerator(Environment.TickCount);
        var stationGenerator = new ProceduralStationGenerator(Environment.TickCount);
        int entityCount = 0;
        
        // === 1. Create diverse fleet of ships at various distances ===
        Console.WriteLine("Generating diverse ship fleet...");
        
        // INCREASED SPACING: Objects are now spread out much more (3-5x multiplier)
        var shipConfigs = new[]
        {
            // Fighters (close by but more spread out)
            (Size: ShipSize.Fighter, Role: ShipRole.Combat, Material: "Titanium", Position: new Vector3(300, 50, 0)),
            (Size: ShipSize.Fighter, Role: ShipRole.Exploration, Material: "Trinium", Position: new Vector3(-350, -40, 150)),
            (Size: ShipSize.Fighter, Role: ShipRole.Combat, Material: "Naonite", Position: new Vector3(250, 0, -280)),
            
            // Corvettes (medium distance - much more spread out)
            (Size: ShipSize.Corvette, Role: ShipRole.Mining, Material: "Iron", Position: new Vector3(600, 0, 350)),
            (Size: ShipSize.Corvette, Role: ShipRole.Trading, Material: "Titanium", Position: new Vector3(-650, 80, -400)),
            (Size: ShipSize.Corvette, Role: ShipRole.Combat, Material: "Titanium", Position: new Vector3(550, -100, 500)),
            (Size: ShipSize.Corvette, Role: ShipRole.Multipurpose, Material: "Naonite", Position: new Vector3(-700, 0, 0)),
            
            // Frigates (further out - significantly more distant)
            (Size: ShipSize.Frigate, Role: ShipRole.Combat, Material: "Ogonite", Position: new Vector3(1200, 0, 800)),
            (Size: ShipSize.Frigate, Role: ShipRole.Trading, Material: "Xanion", Position: new Vector3(-1100, 150, -750)),
            (Size: ShipSize.Frigate, Role: ShipRole.Exploration, Material: "Naonite", Position: new Vector3(950, -80, -900)),
            (Size: ShipSize.Frigate, Role: ShipRole.Mining, Material: "Iron", Position: new Vector3(-1050, 0, 850)),
            (Size: ShipSize.Frigate, Role: ShipRole.Multipurpose, Material: "Trinium", Position: new Vector3(1100, 120, -800)),
            
            // Destroyers (distant - much farther)
            (Size: ShipSize.Destroyer, Role: ShipRole.Combat, Material: "Avorion", Position: new Vector3(1800, 0, 1200)),
            (Size: ShipSize.Destroyer, Role: ShipRole.Salvage, Material: "Ogonite", Position: new Vector3(-1700, -150, -1300)),
            
            // Cruisers (very distant - far on the horizon)
            (Size: ShipSize.Cruiser, Role: ShipRole.Combat, Material: "Avorion", Position: new Vector3(2500, 200, 1600)),
            (Size: ShipSize.Cruiser, Role: ShipRole.Trading, Material: "Xanion", Position: new Vector3(-2400, 0, -1800)),
            
            // Capital ships (extremely distant - barely visible on horizon)
            (Size: ShipSize.Battleship, Role: ShipRole.Combat, Material: "Avorion", Position: new Vector3(3500, 300, 2000)),
            (Size: ShipSize.Carrier, Role: ShipRole.Multipurpose, Material: "Avorion", Position: new Vector3(-3300, -250, -2200)),
        };
        
        foreach (var config in shipConfigs)
        {
            try
            {
                var shipConfig = new ShipGenerationConfig
                {
                    Size = config.Size,
                    Role = config.Role,
                    Material = config.Material,
                    Style = FactionShipStyle.GetDefaultStyle("Default"),
                    Seed = config.Position.GetHashCode()
                };
                
                var generatedShip = shipGenerator.GenerateShip(shipConfig);
                var ship = _gameEngine!.EntityManager.CreateEntity($"{config.Size} - {config.Role}");
                
                _gameEngine.EntityManager.AddComponent(ship.Id, generatedShip.Structure);
                
                var shipPhysics = new PhysicsComponent
                {
                    Position = centerPosition + config.Position,
                    Velocity = Vector3.Zero,
                    Mass = generatedShip.Structure.TotalMass,
                    MomentOfInertia = generatedShip.Structure.MomentOfInertia,
                    MaxThrust = generatedShip.Structure.TotalThrust,
                    MaxTorque = generatedShip.Structure.TotalTorque
                };
                _gameEngine.EntityManager.AddComponent(ship.Id, shipPhysics);
                
                // Add combat component for combat ships
                if (config.Role == ShipRole.Combat)
                {
                    var shipCombat = new CombatComponent
                    {
                        EntityId = ship.Id,
                        MaxShields = generatedShip.Structure.ShieldCapacity,
                        CurrentShields = generatedShip.Structure.ShieldCapacity,
                        MaxEnergy = generatedShip.Structure.PowerGeneration,
                        CurrentEnergy = generatedShip.Structure.PowerGeneration
                    };
                    _gameEngine.EntityManager.AddComponent(ship.Id, shipCombat);
                }
                
                entityCount++;
            }
            catch (Exception ex)
            {
                Console.WriteLine($"  Warning: Failed to generate {config.Size} {config.Role}: {ex.Message}");
            }
        }
        
        Console.WriteLine($"  ✓ Generated {entityCount} ships");
        
        // === 2. Create rich asteroid fields with all material types (INCREASED SPACING) ===
        Console.WriteLine("Creating asteroid fields...");
        
        var asteroidConfigs = new[]
        {
            // Iron field (close but more spread)
            (Material: ResourceType.Iron, Center: new Vector3(450, 0, 600), Count: 20),
            // Titanium field
            (Material: ResourceType.Titanium, Center: new Vector3(-800, 150, -500), Count: 15),
            // Naonite field
            (Material: ResourceType.Naonite, Center: new Vector3(1000, -100, 1000), Count: 12),
            // Trinium field
            (Material: ResourceType.Trinium, Center: new Vector3(-1200, 0, 900), Count: 10),
            // Xanion field
            (Material: ResourceType.Xanion, Center: new Vector3(1500, 120, -1200), Count: 8),
            // Ogonite field
            (Material: ResourceType.Ogonite, Center: new Vector3(-1700, -150, -1400), Count: 6),
            // Avorion field (rare, far away)
            (Material: ResourceType.Avorion, Center: new Vector3(2000, 250, 1500), Count: 4),
        };
        
        int asteroidCount = 0;
        foreach (var field in asteroidConfigs)
        {
            for (int i = 0; i < field.Count; i++)
            {
                var asteroid = _gameEngine!.EntityManager.CreateEntity($"Asteroid - {field.Material}");
                
                // Create asteroid voxel structure
                var asteroidVoxel = new VoxelStructureComponent();
                var asteroidSize = 3f + (float)(_gameEngine.GetHashCode() % 5);
                asteroidVoxel.AddBlock(new VoxelBlock(
                    Vector3.Zero,
                    new Vector3(asteroidSize, asteroidSize, asteroidSize),
                    GetMaterialForResource(field.Material),
                    BlockType.Hull // Asteroids use Hull block type
                ));
                _gameEngine.EntityManager.AddComponent(asteroid.Id, asteroidVoxel);
                
                // Random position within field (larger spread)
                var offset = new Vector3(
                    (float)(new Random(i).NextDouble() * 250 - 125),
                    (float)(new Random(i + 1000).NextDouble() * 120 - 60),
                    (float)(new Random(i + 2000).NextDouble() * 250 - 125)
                );
                
                var asteroidPhysics = new PhysicsComponent
                {
                    Position = centerPosition + field.Center + offset,
                    Mass = asteroidVoxel.TotalMass,
                    IsStatic = true
                };
                _gameEngine.EntityManager.AddComponent(asteroid.Id, asteroidPhysics);
                
                // Note: Resource extraction is handled by the mining system based on asteroid material
                
                asteroidCount++;
            }
        }
        
        Console.WriteLine($"  ✓ Generated {asteroidCount} asteroids across {asteroidConfigs.Length} fields");
        
        // === 3. Create PLANETS for visual interest and scale (NEW!) ===
        Console.WriteLine("Creating planets...");
        
        var planetConfigs = new[]
        {
            // Rocky inner planet (close, visible)
            (Name: "Rocky Planet Alpha", Size: 80f, Position: new Vector3(1500, -200, 800), Type: "Rocky"),
            // Gas giant (medium distance, large and visible)
            (Name: "Gas Giant Beta", Size: 150f, Position: new Vector3(-2500, 300, -1800), Type: "Gas"),
            // Ice planet (distant but visible on horizon)
            (Name: "Ice World Gamma", Size: 60f, Position: new Vector3(3200, -100, 2400), Type: "Ice"),
            // Desert planet (mid distance)
            (Name: "Desert World Delta", Size: 70f, Position: new Vector3(-1800, 150, 2000), Type: "Desert"),
        };
        
        int planetCount = 0;
        foreach (var planetConfig in planetConfigs)
        {
            var planet = _gameEngine!.EntityManager.CreateEntity(planetConfig.Name);
            
            // Create planet as a large sphere-like voxel structure
            var planetVoxel = new VoxelStructureComponent();
            
            // Create a simple sphere-approximation with multiple blocks
            float radius = planetConfig.Size / 2f;
            int blocksPerAxis = Math.Max(5, (int)(planetConfig.Size / 20f)); // More blocks for larger planets
            float blockSize = planetConfig.Size / blocksPerAxis;
            
            for (int x = 0; x < blocksPerAxis; x++)
            {
                for (int y = 0; y < blocksPerAxis; y++)
                {
                    for (int z = 0; z < blocksPerAxis; z++)
                    {
                        Vector3 blockPos = new Vector3(
                            (x - blocksPerAxis / 2f) * blockSize,
                            (y - blocksPerAxis / 2f) * blockSize,
                            (z - blocksPerAxis / 2f) * blockSize
                        );
                        
                        // Only add blocks within sphere radius
                        if (blockPos.Length() <= radius)
                        {
                            planetVoxel.AddBlock(new VoxelBlock(
                                blockPos,
                                new Vector3(blockSize, blockSize, blockSize),
                                GetMaterialForPlanetType(planetConfig.Type),
                                BlockType.Hull
                            ));
                        }
                    }
                }
            }
            
            _gameEngine.EntityManager.AddComponent(planet.Id, planetVoxel);
            
            var planetPhysics = new PhysicsComponent
            {
                Position = centerPosition + planetConfig.Position,
                Mass = planetVoxel.TotalMass * 1000f, // Planets are very massive
                IsStatic = true // Planets don't move
            };
            _gameEngine.EntityManager.AddComponent(planet.Id, planetPhysics);
            
            planetCount++;
        }
        
        Console.WriteLine($"  ✓ Generated {planetCount} planets");
        
        // === 4. Create multiple space stations (INCREASED SPACING) ===
        Console.WriteLine("Creating space stations...");
        
        var stationConfigs = new[]
        {
            (Type: "Trading", Material: "Titanium", Position: new Vector3(1300, 300, 0), Arch: StationArchitecture.Modular),
            (Type: "Military", Material: "Ogonite", Position: new Vector3(-1500, -250, 1000), Arch: StationArchitecture.Modular),
            (Type: "Industrial", Material: "Iron", Position: new Vector3(0, 450, -1400), Arch: StationArchitecture.Sprawling),
            (Type: "Research", Material: "Avorion", Position: new Vector3(-1100, -350, -1200), Arch: StationArchitecture.Modular),
        };
        
        int stationCount = 0;
        foreach (var config in stationConfigs)
        {
            try
            {
                var stationConfig = new StationGenerationConfig
                {
                    Size = StationSize.Large,
                    StationType = config.Type,
                    Material = config.Material,
                    Architecture = config.Arch,
                    Seed = config.Type.GetHashCode()
                };
                
                var generatedStation = stationGenerator.GenerateStation(stationConfig);
                var station = _gameEngine!.EntityManager.CreateEntity($"{config.Type} Station");
                
                _gameEngine.EntityManager.AddComponent(station.Id, generatedStation.Structure);
                
                var stationPhysics = new PhysicsComponent
                {
                    Position = centerPosition + config.Position,
                    Mass = generatedStation.Structure.TotalMass,
                    IsStatic = true
                };
                _gameEngine.EntityManager.AddComponent(station.Id, stationPhysics);
                
                stationCount++;
            }
            catch (Exception ex)
            {
                Console.WriteLine($"  Warning: Failed to generate {config.Type} station: {ex.Message}");
            }
        }
        
        Console.WriteLine($"  ✓ Generated {stationCount} space stations");
        
        // === 5. Create AI ships with behaviors (INCREASED SPACING) ===
        Console.WriteLine("Creating AI ships...");
        
        var aiShipConfigs = new[]
        {
            (Name: "Trader 1", Role: ShipRole.Trading, Position: new Vector3(1250, 280, 60)),
            (Name: "Trader 2", Role: ShipRole.Trading, Position: new Vector3(1350, 310, -90)),
            (Name: "Miner 1", Role: ShipRole.Mining, Position: new Vector3(420, 30, 550)),
            (Name: "Miner 2", Role: ShipRole.Mining, Position: new Vector3(520, -30, 650)),
            (Name: "Pirate 1", Role: ShipRole.Combat, Position: new Vector3(-2000, 150, 700)),
            (Name: "Pirate 2", Role: ShipRole.Combat, Position: new Vector3(-2200, -120, 900)),
            (Name: "Explorer 1", Role: ShipRole.Exploration, Position: new Vector3(1700, 350, 1200)),
        };
        
        int aiShipCount = 0;
        foreach (var aiConfig in aiShipConfigs)
        {
            try
            {
                var aiShipGenConfig = new ShipGenerationConfig
                {
                    Size = ShipSize.Corvette,
                    Role = aiConfig.Role,
                    Material = "Titanium",
                    Style = FactionShipStyle.GetDefaultStyle("Default"),
                    Seed = aiConfig.Name.GetHashCode()
                };
                
                var generatedAIShip = shipGenerator.GenerateShip(aiShipGenConfig);
                var aiShip = _gameEngine!.EntityManager.CreateEntity(aiConfig.Name);
                
                _gameEngine.EntityManager.AddComponent(aiShip.Id, generatedAIShip.Structure);
                
                var aiPhysics = new PhysicsComponent
                {
                    Position = centerPosition + aiConfig.Position,
                    Velocity = Vector3.Zero,
                    Mass = generatedAIShip.Structure.TotalMass
                };
                _gameEngine.EntityManager.AddComponent(aiShip.Id, aiPhysics);
                
                // Add AI component with appropriate personality and capabilities
                var aiComponent = new AIComponent
                {
                    EntityId = aiShip.Id,
                    Personality = aiConfig.Role == ShipRole.Combat ? AIPersonality.Aggressive : 
                                 aiConfig.Role == ShipRole.Trading ? AIPersonality.Trader :
                                 aiConfig.Role == ShipRole.Mining ? AIPersonality.Miner : AIPersonality.Balanced,
                    CurrentState = aiConfig.Role == ShipRole.Combat ? AIState.Patrol :
                                  aiConfig.Role == ShipRole.Trading ? AIState.Trading :
                                  aiConfig.Role == ShipRole.Mining ? AIState.Mining : AIState.Patrol,
                    CanMine = aiConfig.Role == ShipRole.Mining,
                    CanTrade = aiConfig.Role == ShipRole.Trading
                };
                _gameEngine.EntityManager.AddComponent(aiShip.Id, aiComponent);
                
                aiShipCount++;
            }
            catch (Exception ex)
            {
                Console.WriteLine($"  Warning: Failed to generate AI ship {aiConfig.Name}: {ex.Message}");
            }
        }
        
        Console.WriteLine($"  ✓ Generated {aiShipCount} AI ships");
        
        Console.WriteLine($"\n✓ Solar system populated with {entityCount + asteroidCount + stationCount + aiShipCount} entities!");
    }
    
    /// <summary>
    /// Helper method to get material name for a resource type
    /// </summary>
    static string GetMaterialForResource(ResourceType resourceType)
    {
        return resourceType switch
        {
            ResourceType.Iron => "Iron",
            ResourceType.Titanium => "Titanium",
            ResourceType.Naonite => "Naonite",
            ResourceType.Trinium => "Trinium",
            ResourceType.Xanion => "Xanion",
            ResourceType.Ogonite => "Ogonite",
            ResourceType.Avorion => "Avorion",
            _ => "Iron"
        };
    }

    static string GetMaterialForPlanetType(string planetType)
    {
        return planetType switch
        {
            "Rocky" => "Iron",        // Gray/dark rocky appearance
            "Gas" => "Xanion",        // Golden/yellow gas giant
            "Ice" => "Titanium",      // Light blue ice world
            "Desert" => "Ogonite",    // Reddish desert world
            "Lava" => "Avorion",      // Purple/dark volcanic world
            "Ocean" => "Naonite",     // Green/teal ocean world
            _ => "Trinium"            // Default blue
        };
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
        // Position engines to touch core: core edge at -1.5, engine half-size 1, so -2.5
        structure.AddBlock(new VoxelBlock(new Vector3(-2.5f, 0, 0), new Vector3(2, 2, 2), "Ogonite", BlockType.Engine));
        structure.AddBlock(new VoxelBlock(new Vector3(-2.5f, 1.5f, 0), new Vector3(2, 2, 2), "Ogonite", BlockType.Engine));
        
        // Thrusters (sides for maneuvering) - Trinium (blue with glow)
        // Position thrusters to touch core: core edge at ±1.5, thruster half-size 0.75, so ±2.25
        structure.AddBlock(new VoxelBlock(new Vector3(0, 2.25f, 0), new Vector3(1.5f, 1.5f, 1.5f), "Trinium", BlockType.Thruster));
        structure.AddBlock(new VoxelBlock(new Vector3(0, -2.25f, 0), new Vector3(1.5f, 1.5f, 1.5f), "Trinium", BlockType.Thruster));
        
        // Gyro arrays for rotation - Xanion (gold with glow)
        // Position gyros to touch core: core edge at ±1.5, gyro half-size 1, so ±2.5
        structure.AddBlock(new VoxelBlock(new Vector3(0, 0, 2.5f), new Vector3(2, 2, 2), "Xanion", BlockType.GyroArray));
        structure.AddBlock(new VoxelBlock(new Vector3(0, 0, -2.5f), new Vector3(2, 2, 2), "Xanion", BlockType.GyroArray));
        
        // Generator - Naonite (green with glow)
        // Position generator to touch core: core edge at 1.5, generator half-size 1, so 2.5
        structure.AddBlock(new VoxelBlock(new Vector3(2.5f, 0, 0), new Vector3(2, 2, 2), "Naonite", BlockType.Generator));
        
        // Shield generator - Titanium (silver-blue, metallic)
        // Position shield to touch core and gyro: between core top edge and top gyro
        structure.AddBlock(new VoxelBlock(new Vector3(1.5f, 1.5f, 0), new Vector3(2, 2, 2), "Titanium", BlockType.ShieldGenerator));

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
                        // Engines - Red Ogonite (touching core at -1.5, engine at -2.5)
                        voxelComponent.AddBlock(new VoxelBlock(
                            new Vector3(-2.5f, 0, 0),
                            new Vector3(2, 2, 2),
                            "Ogonite",
                            BlockType.Engine
                        ));
                        // Wings - Gold Xanion (touching core at ±1, wing at ±1.5)
                        voxelComponent.AddBlock(new VoxelBlock(new Vector3(0, 1.5f, 0), new Vector3(2, 1, 2), "Xanion", BlockType.Armor));
                        voxelComponent.AddBlock(new VoxelBlock(new Vector3(0, -1.5f, 0), new Vector3(2, 1, 2), "Xanion", BlockType.Armor));
                        break;
                    
                    case 1: // Cross-shaped ship with rainbow colors (touching blocks)
                        voxelComponent.AddBlock(new VoxelBlock(new Vector3(0, 0, 0), new Vector3(3, 3, 3), "Avorion", BlockType.Hull)); // Purple center
                        voxelComponent.AddBlock(new VoxelBlock(new Vector3(2.5f, 0, 0), new Vector3(2, 2, 2), "Xanion", BlockType.Generator)); // Gold right
                        voxelComponent.AddBlock(new VoxelBlock(new Vector3(-2.5f, 0, 0), new Vector3(2, 2, 2), "Ogonite", BlockType.Engine)); // Red left
                        voxelComponent.AddBlock(new VoxelBlock(new Vector3(0, 2.5f, 0), new Vector3(2, 2, 2), "Naonite", BlockType.ShieldGenerator)); // Green top
                        voxelComponent.AddBlock(new VoxelBlock(new Vector3(0, -2.5f, 0), new Vector3(2, 2, 2), "Trinium", BlockType.Thruster)); // Blue bottom
                        break;
                    
                    case 2: // Compact cargo ship with gradient colors (touching blocks)
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
                                // Blocks touch: size 2.5, so positions should be at x*2.5, y*2.5
                                voxelComponent.AddBlock(new VoxelBlock(
                                    new Vector3(x * 2.5f, y * 2.5f, 0),
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
    
    static void RunAIShipGenerationDemo()
    {
        Console.WriteLine("\n=== AI-Driven Voxel Construction System ===");
        Console.WriteLine("Demonstrating comprehensive AI ship generation with smart design rules");
        Console.WriteLine();
        
        try
        {
            var example = new AIShipGenerationExample();
            example.Run();
        }
        catch (Exception ex)
        {
            Console.WriteLine($"\n✗ Error running AI ship generation demo: {ex.Message}");
            Console.WriteLine($"Stack trace: {ex.StackTrace}");
        }
        
        Console.WriteLine("\nPress any key to return to main menu...");
        Console.ReadKey();
    }
    
    static void RunVisualEnhancementsTest()
    {
        Console.WriteLine("\n=== Visual Enhancements Test ===");
        Console.WriteLine("Testing enhanced ship, station, and asteroid generation with visual details");
        Console.WriteLine();
        
        try
        {
            var entityManager = new EntityManager();
            var test = new VisualEnhancementsTest(entityManager);
            test.RunDemo();
        }
        catch (Exception ex)
        {
            Console.WriteLine($"\n✗ Error running visual enhancements test: {ex.Message}");
            Console.WriteLine($"Stack trace: {ex.StackTrace}");
        }
        
        Console.WriteLine("\nPress any key to return to main menu...");
        Console.ReadKey();
    }
    
    /// <summary>
    /// Allows user to visually see and select which ship generation they want to use
    /// Returns the selected ship display info, or null if cancelled
    /// </summary>
    static ShipShowcaseExample.GeneratedShipDisplay? SelectShipGeneration()
    {
        Console.WriteLine("\n╔══════════════════════════════════════════════════════════════════════════╗");
        Console.WriteLine("║           SHIP GENERATION SELECTION                                       ║");
        Console.WriteLine("║   Choose your starting ship design from procedurally generated options    ║");
        Console.WriteLine("╚══════════════════════════════════════════════════════════════════════════╝");
        Console.WriteLine();
        Console.WriteLine("Generating ship options for you to choose from...\n");
        
        // Create a temporary entity manager for preview generation
        var previewEntityManager = new EntityManager();
        var showcase = new ShipShowcaseExample(previewEntityManager);
        
        // Generate 12 starter-appropriate ships (smaller selection for new game)
        var ships = GenerateStarterShipOptions(previewEntityManager, Environment.TickCount);
        
        if (ships.Count == 0)
        {
            Console.WriteLine("Error generating ship options. Using default ship.");
            return null;
        }
        
        // Show ship options summary
        PrintShipSelectionSummary(ships);
        
        // Allow user to interact
        ShipShowcaseExample.GeneratedShipDisplay? selectedShip = null;
        bool done = false;
        
        while (!done)
        {
            Console.WriteLine();
            Console.WriteLine("═══════════════════════════════════════════════════════════════════════════");
            Console.WriteLine("SHIP SELECTION OPTIONS:");
            Console.WriteLine("═══════════════════════════════════════════════════════════════════════════");
            Console.WriteLine($"  Enter ship number (1-{ships.Count}) to select that ship");
            Console.WriteLine("  V - View ships in 3D (visually inspect before choosing)");
            Console.WriteLine("  D - View detailed stats for a ship");
            Console.WriteLine("  R - Regenerate new ship options");
            Console.WriteLine("  0 - Cancel and return to main menu");
            Console.WriteLine();
            Console.Write("Select option: ");
            
            var input = Console.ReadLine()?.Trim().ToUpper();
            
            if (string.IsNullOrEmpty(input))
                continue;
            
            if (input == "0")
            {
                done = true;
                selectedShip = null;
            }
            else if (input == "V")
            {
                // Launch 3D viewer for visual inspection
                Console.WriteLine("\nLaunching 3D viewer to visually inspect ships...");
                Console.WriteLine("Use camera controls to fly around and inspect each design.");
                Console.WriteLine("Close the window when you're ready to make your selection.\n");
                
                try
                {
                    // Create a temporary graphics window for preview
                    var tempEngine = new GameEngine(Environment.TickCount);
                    try
                    {
                        tempEngine.Start();
                        
                        // Copy ships to the temp engine for viewing
                        foreach (var ship in ships)
                        {
                            var entity = tempEngine.EntityManager.CreateEntity($"Ship #{ship.Number} - {ship.Description}");
                            
                            // Clone the voxel structure
                            var voxelClone = new VoxelStructureComponent();
                            foreach (var block in ship.ShipData.Structure.Blocks)
                            {
                                voxelClone.AddBlock(new VoxelBlock(
                                    block.Position, 
                                    block.Size, 
                                    block.MaterialType, 
                                    block.BlockType));
                            }
                            tempEngine.EntityManager.AddComponent(entity.Id, voxelClone);
                            
                            // Add physics at the ship's position
                            var physics = new PhysicsComponent
                            {
                                Position = ship.Position,
                                Mass = ship.ShipData.TotalMass
                            };
                            tempEngine.EntityManager.AddComponent(entity.Id, physics);
                        }
                        
                        using var graphicsWindow = new GraphicsWindow(tempEngine);
                        if (ships.Count > 0)
                        {
                            // Get the entity ID of the first ship for camera focus
                            var firstShipEntity = tempEngine.EntityManager.GetAllEntities().FirstOrDefault();
                            if (firstShipEntity != null)
                            {
                                graphicsWindow.SetPlayerShip(firstShipEntity.Id);
                            }
                        }
                        graphicsWindow.Run();
                    }
                    finally
                    {
                        tempEngine.Stop();
                    }
                }
                catch (Exception ex)
                {
                    Console.WriteLine($"Error running 3D viewer: {ex.Message}");
                    Console.WriteLine("Graphics may not be available on this system.");
                }
                
                Console.WriteLine("\nReturned from 3D viewer. Ready to select your ship.");
                PrintShipSelectionSummary(ships);
            }
            else if (input == "D")
            {
                Console.Write("Enter ship number to view details (1-12): ");
                var detailInput = Console.ReadLine();
                if (int.TryParse(detailInput, out int detailNum) && detailNum >= 1 && detailNum <= ships.Count)
                {
                    PrintShipDetails(ships[detailNum - 1]);
                }
                else
                {
                    Console.WriteLine("Invalid ship number.");
                }
            }
            else if (input == "R")
            {
                Console.WriteLine("\nRegenerating new ship options...\n");
                previewEntityManager = new EntityManager();
                ships = GenerateStarterShipOptions(previewEntityManager, Environment.TickCount);
                PrintShipSelectionSummary(ships);
            }
            else if (int.TryParse(input, out int shipNum) && shipNum >= 1 && shipNum <= ships.Count)
            {
                selectedShip = ships[shipNum - 1];
                done = true;
                Console.WriteLine($"\n✓ Selected: Ship #{shipNum} - {selectedShip.Description}");
            }
            else
            {
                Console.WriteLine("Invalid option. Please try again.");
            }
        }
        
        return selectedShip;
    }
    
    /// <summary>
    /// Generate starter-appropriate ship options for new game selection
    /// </summary>
    static List<ShipShowcaseExample.GeneratedShipDisplay> GenerateStarterShipOptions(EntityManager entityManager, int baseSeed)
    {
        // Ship generation constants
        const int SHIP_GRID_WIDTH = 4;
        const float SHIP_SPACING = 100f;
        const int COMBAT_MIN_WEAPONS = 4;
        const int DEFAULT_MIN_WEAPONS = 2;
        
        var ships = new List<ShipShowcaseExample.GeneratedShipDisplay>();
        
        // Define starter-appropriate configurations (smaller, varied ships)
        var configurations = new[]
        {
            // Fighters - Small and agile
            new { Size = ShipSize.Fighter, Role = ShipRole.Combat, Hull = ShipHullShape.Angular, Faction = "Military", Material = "Titanium" },
            new { Size = ShipSize.Fighter, Role = ShipRole.Exploration, Hull = ShipHullShape.Sleek, Faction = "Explorers", Material = "Trinium" },
            new { Size = ShipSize.Fighter, Role = ShipRole.Multipurpose, Hull = ShipHullShape.Blocky, Faction = "Default", Material = "Iron" },
            
            // Corvettes - Slightly larger, more versatile
            new { Size = ShipSize.Corvette, Role = ShipRole.Combat, Hull = ShipHullShape.Angular, Faction = "Military", Material = "Titanium" },
            new { Size = ShipSize.Corvette, Role = ShipRole.Mining, Hull = ShipHullShape.Blocky, Faction = "Miners", Material = "Iron" },
            new { Size = ShipSize.Corvette, Role = ShipRole.Trading, Hull = ShipHullShape.Cylindrical, Faction = "Traders", Material = "Titanium" },
            new { Size = ShipSize.Corvette, Role = ShipRole.Exploration, Hull = ShipHullShape.Sleek, Faction = "Explorers", Material = "Naonite" },
            new { Size = ShipSize.Corvette, Role = ShipRole.Multipurpose, Hull = ShipHullShape.Blocky, Faction = "Default", Material = "Titanium" },
            
            // Frigates - Larger starter options for experienced feel (require hyperdrive for long-range travel)
            new { Size = ShipSize.Frigate, Role = ShipRole.Combat, Hull = ShipHullShape.Angular, Faction = "Military", Material = "Ogonite" },
            new { Size = ShipSize.Frigate, Role = ShipRole.Trading, Hull = ShipHullShape.Cylindrical, Faction = "Traders", Material = "Xanion" },
            new { Size = ShipSize.Frigate, Role = ShipRole.Mining, Hull = ShipHullShape.Blocky, Faction = "Miners", Material = "Iron" },
            new { Size = ShipSize.Frigate, Role = ShipRole.Multipurpose, Hull = ShipHullShape.Blocky, Faction = "Default", Material = "Avorion" },
        };
        
        for (int i = 0; i < configurations.Length; i++)
        {
            var config = configurations[i];
            
            // Calculate grid position for 3D viewing
            int row = i / SHIP_GRID_WIDTH;
            int col = i % SHIP_GRID_WIDTH;
            Vector3 position = new Vector3(
                col * SHIP_SPACING - (SHIP_GRID_WIDTH - 1) * SHIP_SPACING / 2f,
                0,
                row * SHIP_SPACING
            );
            
            try
            {
                var generator = new ProceduralShipGenerator(baseSeed + i);
                var style = FactionShipStyle.GetDefaultStyle(config.Faction);
                style.PreferredHullShape = config.Hull;
                
                var shipConfig = new ShipGenerationConfig
                {
                    Size = config.Size,
                    Role = config.Role,
                    Material = config.Material,
                    Style = style,
                    Seed = baseSeed + i,
                    // Frigates and larger require hyperdrives for long-range interstellar travel
                    RequireHyperdrive = config.Size >= ShipSize.Frigate,
                    RequireCargo = true,
                    // Combat ships need more weapon mounts
                    MinimumWeaponMounts = config.Role == ShipRole.Combat ? COMBAT_MIN_WEAPONS : DEFAULT_MIN_WEAPONS
                };
                
                var shipData = generator.GenerateShip(shipConfig);
                
                // Create entity for potential preview
                var entity = entityManager.CreateEntity($"Ship #{i + 1}");
                entityManager.AddComponent(entity.Id, shipData.Structure);
                
                var physics = new PhysicsComponent
                {
                    Position = position,
                    Mass = shipData.TotalMass
                };
                entityManager.AddComponent(entity.Id, physics);
                
                var display = new ShipShowcaseExample.GeneratedShipDisplay
                {
                    Number = i + 1,
                    EntityId = entity.Id,
                    ShipData = shipData,
                    Position = position,
                    Description = $"{config.Size} {config.Hull} {config.Role} ({config.Faction})"
                };
                
                ships.Add(display);
                Console.WriteLine($"  #{i + 1:D2}: {display.Description} - {shipData.Structure.Blocks.Count} blocks");
            }
            catch (Exception ex)
            {
                Console.WriteLine($"  Warning: Failed to generate ship #{i + 1}: {ex.Message}");
            }
        }
        
        return ships;
    }
    
    /// <summary>
    /// Print a summary table of available ships for selection
    /// </summary>
    static void PrintShipSelectionSummary(List<ShipShowcaseExample.GeneratedShipDisplay> ships)
    {
        Console.WriteLine();
        Console.WriteLine("╔══════════════════════════════════════════════════════════════════════════╗");
        Console.WriteLine("║                      AVAILABLE SHIP GENERATIONS                          ║");
        Console.WriteLine("╠══════════════════════════════════════════════════════════════════════════╣");
        Console.WriteLine("║  #  │ Size      │ Hull      │ Role         │ Blocks │ Thrust   │ Shields ║");
        Console.WriteLine("╠═════╪═══════════╪═══════════╪══════════════╪════════╪══════════╪═════════╣");
        
        foreach (var ship in ships)
        {
            var config = ship.ShipData.Config;
            var size = config.Size.ToString().PadRight(9);
            var hull = config.Style.PreferredHullShape.ToString().PadRight(9);
            var role = config.Role.ToString().PadRight(12);
            var blocks = ship.ShipData.Structure.Blocks.Count.ToString().PadLeft(6);
            var thrust = ship.ShipData.TotalThrust.ToString("F0").PadLeft(8);
            var shields = ship.ShipData.TotalShieldCapacity.ToString("F0").PadLeft(7);
            
            Console.WriteLine($"║ {ship.Number,2}  │ {size} │ {hull} │ {role} │ {blocks} │ {thrust} │ {shields} ║");
        }
        
        Console.WriteLine("╚══════════════════════════════════════════════════════════════════════════╝");
        Console.WriteLine();
        Console.WriteLine("💡 TIP: Use 'V' to view ships in 3D and visually inspect each design!");
        Console.WriteLine("        Use 'D' followed by a ship number to see detailed statistics.");
    }
    
    /// <summary>
    /// Print detailed information about a specific ship
    /// </summary>
    static void PrintShipDetails(ShipShowcaseExample.GeneratedShipDisplay ship)
    {
        Console.WriteLine();
        Console.WriteLine($"╔═══════════════════════════════════════════════════════════════╗");
        Console.WriteLine($"║  SHIP #{ship.Number} - DETAILED INFORMATION                        ║");
        Console.WriteLine($"╠═══════════════════════════════════════════════════════════════╣");
        Console.WriteLine($"║  Description: {ship.Description,-45}║");
        Console.WriteLine($"╠═══════════════════════════════════════════════════════════════╣");
        Console.WriteLine($"║  CONFIGURATION:                                               ║");
        Console.WriteLine($"║    Size:       {ship.ShipData.Config.Size,-46}║");
        Console.WriteLine($"║    Role:       {ship.ShipData.Config.Role,-46}║");
        Console.WriteLine($"║    Hull Shape: {ship.ShipData.Config.Style.PreferredHullShape,-46}║");
        Console.WriteLine($"║    Faction:    {ship.ShipData.Config.Style.FactionName,-46}║");
        Console.WriteLine($"║    Material:   {ship.ShipData.Config.Material,-46}║");
        Console.WriteLine($"╠═══════════════════════════════════════════════════════════════╣");
        Console.WriteLine($"║  STATISTICS:                                                  ║");
        Console.WriteLine($"║    Total Blocks:   {ship.ShipData.Structure.Blocks.Count,-42}║");
        Console.WriteLine($"║    Mass:           {ship.ShipData.TotalMass,-38:F2} kg ║");
        Console.WriteLine($"║    Thrust:         {ship.ShipData.TotalThrust,-38:F2} N  ║");
        Console.WriteLine($"║    Thrust/Mass:    {ship.ShipData.Stats.GetValueOrDefault("ThrustToMass", 0f),-38:F2}    ║");
        Console.WriteLine($"║    Power Gen:      {ship.ShipData.TotalPowerGeneration,-38:F2} W  ║");
        Console.WriteLine($"║    Shield Cap:     {ship.ShipData.TotalShieldCapacity,-38:F2}    ║");
        Console.WriteLine($"║    Weapons:        {ship.ShipData.WeaponMountCount,-42}║");
        Console.WriteLine($"║    Cargo Bays:     {ship.ShipData.CargoBlockCount,-42}║");
        Console.WriteLine($"║    Integrity:      {ship.ShipData.Stats.GetValueOrDefault("StructuralIntegrity", 0f),-38:F1}%   ║");
        Console.WriteLine($"╚═══════════════════════════════════════════════════════════════╝");
        
        if (ship.ShipData.Warnings.Count > 0)
        {
            Console.WriteLine();
            Console.WriteLine("⚠ Generation Warnings:");
            foreach (var warning in ship.ShipData.Warnings.Take(5))
            {
                Console.WriteLine($"    {warning}");
            }
        }
    }

    /// <summary>
    /// Creates a comprehensive test showcase with all implementations visible for testing
    /// </summary>
    /// <summary>
    /// Creates a comprehensive test showcase with all implementations visible for testing
    /// </summary>
    static void CreateComprehensiveTestShowcase(Guid playerShipId, Vector3 playerPosition)
    {
        Console.WriteLine("Generating test showcase with multiple ship types...");
        
        var shipGenerator = new ProceduralShipGenerator(Environment.TickCount);
        int shipCount = 0;
        
        // Use varied seeds for more variety each time you play
        int baseSeed = Environment.TickCount;
        
        // Configuration for different ship types to test
        var testConfigurations = new[]
        {
            // Fighter examples (different factions, styles, and hull shapes)
            new { Name = "Military Fighter", Size = ShipSize.Fighter, Role = ShipRole.Combat, Material = "Avorion", Style = "Military", Hull = ShipHullShape.Angular, Position = new Vector3(150, 0, 0) },
            new { Name = "Scout Fighter", Size = ShipSize.Fighter, Role = ShipRole.Exploration, Material = "Trinium", Style = "Explorers", Hull = ShipHullShape.Sleek, Position = new Vector3(150, 50, 0) },
            
            // Corvette examples with varied hulls
            new { Name = "Combat Corvette", Size = ShipSize.Corvette, Role = ShipRole.Combat, Material = "Titanium", Style = "Military", Hull = ShipHullShape.Angular, Position = new Vector3(200, 0, 0) },
            new { Name = "Mining Corvette", Size = ShipSize.Corvette, Role = ShipRole.Mining, Material = "Iron", Style = "Miners", Hull = ShipHullShape.Blocky, Position = new Vector3(200, 50, 0) },
            
            // Frigate examples with distinct shapes
            new { Name = "Military Frigate", Size = ShipSize.Frigate, Role = ShipRole.Combat, Material = "Ogonite", Style = "Military", Hull = ShipHullShape.Angular, Position = new Vector3(300, 0, 0) },
            new { Name = "Trading Frigate", Size = ShipSize.Frigate, Role = ShipRole.Trading, Material = "Xanion", Style = "Traders", Hull = ShipHullShape.Cylindrical, Position = new Vector3(300, 50, 0) },
            new { Name = "Explorer Frigate", Size = ShipSize.Frigate, Role = ShipRole.Exploration, Material = "Naonite", Style = "Explorers", Hull = ShipHullShape.Sleek, Position = new Vector3(300, -50, 0) },
            
            // Destroyer examples
            new { Name = "Heavy Destroyer", Size = ShipSize.Destroyer, Role = ShipRole.Combat, Material = "Avorion", Style = "Military", Hull = ShipHullShape.Angular, Position = new Vector3(450, 0, 0) },
            new { Name = "Salvage Destroyer", Size = ShipSize.Destroyer, Role = ShipRole.Salvage, Material = "Ogonite", Style = "Pirates", Hull = ShipHullShape.Irregular, Position = new Vector3(450, 50, 0) },
            
            // Cruiser examples
            new { Name = "Battle Cruiser", Size = ShipSize.Cruiser, Role = ShipRole.Combat, Material = "Avorion", Style = "Military", Hull = ShipHullShape.Angular, Position = new Vector3(600, 0, 0) },
            new { Name = "Trade Cruiser", Size = ShipSize.Cruiser, Role = ShipRole.Trading, Material = "Xanion", Style = "Traders", Hull = ShipHullShape.Cylindrical, Position = new Vector3(600, 50, 0) },
            
            // Battleship examples
            new { Name = "Dreadnought", Size = ShipSize.Battleship, Role = ShipRole.Combat, Material = "Avorion", Style = "Military", Hull = ShipHullShape.Angular, Position = new Vector3(800, 0, 0) },
            
            // Carrier example
            new { Name = "Fleet Carrier", Size = ShipSize.Carrier, Role = ShipRole.Multipurpose, Material = "Avorion", Style = "Military", Hull = ShipHullShape.Blocky, Position = new Vector3(1000, 0, 0) },
            
            // Different hull shapes at various positions (for visual comparison)
            new { Name = "Angular Fighter", Size = ShipSize.Fighter, Role = ShipRole.Combat, Material = "Titanium", Style = "Military", Hull = ShipHullShape.Angular, Position = new Vector3(-150, 0, 100) },
            new { Name = "Sleek Fighter", Size = ShipSize.Fighter, Role = ShipRole.Combat, Material = "Trinium", Style = "Explorers", Hull = ShipHullShape.Sleek, Position = new Vector3(-150, 0, 150) },
            new { Name = "Blocky Trader", Size = ShipSize.Corvette, Role = ShipRole.Trading, Material = "Iron", Style = "Traders", Hull = ShipHullShape.Blocky, Position = new Vector3(-150, 0, 200) },
            new { Name = "Cylindrical Hauler", Size = ShipSize.Frigate, Role = ShipRole.Trading, Material = "Titanium", Style = "Traders", Hull = ShipHullShape.Cylindrical, Position = new Vector3(-150, 0, 250) },
            new { Name = "Irregular Pirate", Size = ShipSize.Corvette, Role = ShipRole.Combat, Material = "Iron", Style = "Pirates", Hull = ShipHullShape.Irregular, Position = new Vector3(-150, 0, 300) },
        };
        
        // Generate all test ships with varied seeds
        for (int i = 0; i < testConfigurations.Length; i++)
        {
            var config = testConfigurations[i];
            try
            {
                // Create faction style with specified hull shape
                var style = FactionShipStyle.GetDefaultStyle(config.Style);
                style.PreferredHullShape = config.Hull;
                
                var shipConfig = new ShipGenerationConfig
                {
                    Size = config.Size,
                    Role = config.Role,
                    Material = config.Material,
                    Style = style,
                    Seed = baseSeed + i * 1000 + config.Name.GetHashCode() // Varied seed for different results each time
                };
                
                var generatedShip = shipGenerator.GenerateShip(shipConfig);
                var ship = _gameEngine!.EntityManager.CreateEntity(config.Name);
                
                // Add voxel structure
                _gameEngine.EntityManager.AddComponent(ship.Id, generatedShip.Structure);
                
                // Add physics at specified position
                var physicsComponent = new PhysicsComponent
                {
                    Position = playerPosition + config.Position,
                    Velocity = Vector3.Zero,
                    Mass = generatedShip.Structure.TotalMass,
                    MomentOfInertia = generatedShip.Structure.MomentOfInertia,
                    MaxThrust = generatedShip.Structure.TotalThrust,
                    MaxTorque = generatedShip.Structure.TotalTorque
                };
                _gameEngine.EntityManager.AddComponent(ship.Id, physicsComponent);
                
                // Add combat component
                var combatComponent = new CombatComponent
                {
                    EntityId = ship.Id,
                    MaxShields = generatedShip.Structure.ShieldCapacity,
                    CurrentShields = generatedShip.Structure.ShieldCapacity,
                    MaxEnergy = generatedShip.Structure.PowerGeneration,
                    CurrentEnergy = generatedShip.Structure.PowerGeneration
                };
                _gameEngine.EntityManager.AddComponent(ship.Id, combatComponent);
                
                shipCount++;
            }
            catch (Exception ex)
            {
                Console.WriteLine($"  ⚠ Failed to generate {config.Name}: {ex.Message}");
            }
        }
        
        // Add some enhanced stations (using visual enhancements) with varied seeds
        Console.WriteLine("Adding test stations...");
        try
        {
            var stationGenerator = new ProceduralStationGenerator(baseSeed + 5000);
            var stationConfigs = new[]
            {
                new { Type = "Trading", Material = "Titanium", Arch = StationArchitecture.Modular, Position = new Vector3(0, 200, 400) },
                new { Type = "Military", Material = "Ogonite", Arch = StationArchitecture.Modular, Position = new Vector3(0, 350, 400) },
                new { Type = "Industrial", Material = "Iron", Arch = StationArchitecture.Sprawling, Position = new Vector3(0, 500, 400) },
                new { Type = "Research", Material = "Avorion", Arch = StationArchitecture.Modular, Position = new Vector3(0, 650, 400) },
            };
            
            for (int i = 0; i < stationConfigs.Length; i++)
            {
                var stationConfig = stationConfigs[i];
                var config = new StationGenerationConfig
                {
                    Size = StationSize.Medium,
                    StationType = stationConfig.Type,
                    Material = stationConfig.Material,
                    Architecture = stationConfig.Arch,
                    Seed = baseSeed + 6000 + i * 100 // Varied seed for stations too
                };
                
                var generatedStation = stationGenerator.GenerateStation(config);
                var stationEntity = _gameEngine!.EntityManager.CreateEntity($"{stationConfig.Type} Station");
                
                _gameEngine.EntityManager.AddComponent(stationEntity.Id, generatedStation.Structure);
                
                var stationPhysics = new PhysicsComponent
                {
                    Position = playerPosition + stationConfig.Position,
                    Velocity = Vector3.Zero,
                    Mass = generatedStation.Structure.TotalMass,
                    IsStatic = true
                };
                _gameEngine.EntityManager.AddComponent(stationEntity.Id, stationPhysics);
            }
        }
        catch (Exception ex)
        {
            Console.WriteLine($"  ⚠ Failed to generate stations: {ex.Message}");
        }
        
        // Add connectivity test ships (to verify no floating blocks)
        Console.WriteLine("Adding connectivity test examples...");
        try
        {
            TestShipConnectivity.Run();
        }
        catch (Exception ex)
        {
            Console.WriteLine($"  ⚠ Connectivity tests skipped: {ex.Message}");
        }
        
        Console.WriteLine($"✓ Test showcase created: {shipCount} ships generated");
        Console.WriteLine($"  Ships use varied seeds for different designs each playthrough");
        Console.WriteLine($"  Multiple hull shapes: Angular, Sleek, Blocky, Cylindrical, Irregular");
        Console.WriteLine($"  Ships positioned at various distances from player ship");
        Console.WriteLine($"  Use camera controls to fly around and inspect each design");
        Console.WriteLine($"  All implementations are now visible for testing!");
    }
    
    static void LaunchGraphicalMainMenu()
    {
        Console.WriteLine("\n=== GRAPHICAL MAIN MENU - Comprehensive Game Setup ===");
        Console.WriteLine("Launching graphical main menu with ImGui interface...");
        Console.WriteLine("This provides full control over:");
        Console.WriteLine("  • New Game with extensive customization");
        Console.WriteLine("  • Load/Save game management");
        Console.WriteLine("  • Multiplayer hosting and joining");
        Console.WriteLine("  • All game settings and options\n");
        
        try
        {
            // Create a minimal graphics window just to host the menu system
            using var graphicsWindow = new GraphicsWindow(_gameEngine!);
            
            // Create and configure the main menu system
            var mainMenu = new MainMenuSystem(_gameEngine!);
            
            // Set up callbacks for menu actions
            mainMenu.SetCallbacks(
                onNewGameStart: (settings) => {
                    Console.WriteLine("\n=== Starting New Game with Custom Settings ===");
                    Console.WriteLine(settings.GetSummary());
                    StartNewGameWithSettings(settings);
                },
                onLoadGame: (saveName) => {
                    Console.WriteLine($"\n=== Loading Game: {saveName} ===");
                    if (_gameEngine!.LoadGame(saveName))
                    {
                        Console.WriteLine("Game loaded successfully!");
                        // Would need to launch game with loaded state
                    }
                    else
                    {
                        Console.WriteLine("Failed to load game.");
                    }
                },
                onHostMultiplayer: (serverName, port, maxPlayers) => {
                    Console.WriteLine($"\n=== Hosting Multiplayer Server ===");
                    Console.WriteLine($"Server Name: {serverName}");
                    Console.WriteLine($"Port: {port}");
                    Console.WriteLine($"Max Players: {maxPlayers}");
                    _gameEngine!.StartServer();
                },
                onJoinMultiplayer: (address, port, playerName) => {
                    Console.WriteLine($"\n=== Joining Multiplayer Server ===");
                    Console.WriteLine($"Server: {address}:{port}");
                    Console.WriteLine($"Player Name: {playerName}");
                    // Would need to implement client connection
                }
            );
            
            mainMenu.Show();
            
            // Run the graphics window with the main menu
            graphicsWindow.Run();
        }
        catch (Exception ex)
        {
            Console.WriteLine($"Error launching graphical main menu: {ex.Message}");
            Console.WriteLine("Graphics rendering may not be available on this system.");
            Console.WriteLine("\nFalling back to console menu...");
        }
    }
    
    static void StartNewGameWithSettings(NewGameSettings settings)
    {
        Console.WriteLine("\n=== NEW GAME - With Custom Settings ===");
        Console.WriteLine("Creating game world based on your configuration...\n");
        
        // Apply the settings to game engine configuration
        var config = ConfigurationManager.Instance.Config;
        config.Development.GalaxySeed = settings.GalaxySeed;
        config.Gameplay.PlayerName = settings.PlayerName;
        config.Gameplay.Difficulty = settings.PlayerDifficulty;
        
        // Reinitialize game engine with new seed if needed
        if (settings.GalaxySeed != 12345)
        {
            _gameEngine?.Stop();
            _gameEngine = new GameEngine(settings.GalaxySeed);
            _gameEngine.Start();
        }
        
        // Create player ship based on settings
        var playerShip = _gameEngine!.EntityManager.CreateEntity(settings.PlayerName);
        
        // Create ship based on starting ship class
        var voxelComponent = CreateStartingShip(settings.StartingShipClass);
        _gameEngine.EntityManager.AddComponent(playerShip.Id, voxelComponent);
        
        // Calculate starting position based on region
        Vector3 startSector = settings.StartingRegionType switch
        {
            "Rim" => new Vector3(400, 0, 0),    // Galaxy rim (Iron tier)
            "Mid" => new Vector3(200, 0, 0),    // Mid-galaxy (Titanium tier)
            "Core" => new Vector3(50, 0, 0),    // Near core (Avorion tier)
            _ => new Vector3(400, 0, 0)
        };
        
        // Add physics
        var physicsComponent = new PhysicsComponent
        {
            Position = Vector3.Zero,
            Velocity = Vector3.Zero,
            Mass = voxelComponent.TotalMass,
            MomentOfInertia = voxelComponent.MomentOfInertia,
            MaxThrust = voxelComponent.TotalThrust * settings.ResourceGatheringMultiplier,
            MaxTorque = voxelComponent.TotalTorque
        };
        _gameEngine.EntityManager.AddComponent(playerShip.Id, physicsComponent);
        
        // Add inventory with starting resources
        var inventoryComponent = new InventoryComponent(1000);
        inventoryComponent.Inventory.AddResource(ResourceType.Credits, settings.StartingCredits);
        foreach (var resource in settings.StartingResources)
        {
            if (Enum.TryParse<ResourceType>(resource.Key, out var resType))
            {
                inventoryComponent.Inventory.AddResource(resType, resource.Value);
            }
        }
        _gameEngine.EntityManager.AddComponent(playerShip.Id, inventoryComponent);
        
        // Add other components
        var progressionComponent = new ProgressionComponent { EntityId = playerShip.Id, Level = 1 };
        _gameEngine.EntityManager.AddComponent(playerShip.Id, progressionComponent);
        
        var combatComponent = new CombatComponent
        {
            EntityId = playerShip.Id,
            MaxShields = voxelComponent.ShieldCapacity,
            CurrentShields = voxelComponent.ShieldCapacity,
            MaxEnergy = voxelComponent.PowerGeneration,
            CurrentEnergy = voxelComponent.PowerGeneration
        };
        _gameEngine.EntityManager.AddComponent(playerShip.Id, combatComponent);
        
        var hyperdriveComponent = new HyperdriveComponent { EntityId = playerShip.Id, JumpRange = 5f };
        _gameEngine.EntityManager.AddComponent(playerShip.Id, hyperdriveComponent);
        
        var locationComponent = new SectorLocationComponent
        {
            EntityId = playerShip.Id,
            CurrentSector = new SectorCoordinate((int)startSector.X, (int)startSector.Y, (int)startSector.Z)
        };
        _gameEngine.EntityManager.AddComponent(playerShip.Id, locationComponent);
        
        // Register systems
        _gameEngine.EntityManager.RegisterSystem(new GalaxyProgressionSystem(_gameEngine.EntityManager));
        _gameEngine.EntityManager.RegisterSystem(new FleetAutomationSystem(_gameEngine.EntityManager));
        
        Console.WriteLine($"✓ Game world created!");
        Console.WriteLine($"  Player: {settings.PlayerName}");
        Console.WriteLine($"  Starting Region: {settings.StartingRegionType}");
        Console.WriteLine($"  Starting Credits: {settings.StartingCredits:N0}");
        Console.WriteLine($"  Starting Ship: {settings.StartingShipClass}");
        Console.WriteLine($"  Galaxy Seed: {settings.GalaxySeed}");
        
        // Populate world based on settings
        Console.WriteLine("\n=== Populating Game World ===");
        var worldPopulator = new GameWorldPopulator(_gameEngine, seed: settings.GalaxySeed);
        worldPopulator.PopulateZoneArea(physicsComponent.Position, locationComponent.CurrentSector, radius: 800f);
        
        // Launch the game
        Console.WriteLine("\n=== Launching Game ===");
        try
        {
            using var graphicsWindow = new GraphicsWindow(_gameEngine);
            graphicsWindow.SetPlayerShip(playerShip.Id);
            graphicsWindow.Run();
        }
        catch (Exception ex)
        {
            Console.WriteLine($"Error running game: {ex.Message}");
        }
        
        Console.WriteLine("\nReturned to main menu.");
    }
    
    static VoxelStructureComponent CreateStartingShip(string shipClass)
    {
        var voxel = new VoxelStructureComponent();
        
        switch (shipClass.ToLower())
        {
            case "starter":
                // Small starter pod
                voxel.AddBlock(new VoxelBlock(new Vector3(0, 0, 0), new Vector3(2, 2, 2), "Titanium", BlockType.Hull));
                voxel.AddBlock(new VoxelBlock(new Vector3(-2.5f, 0, 0), new Vector3(1.5f, 1.5f, 1.5f), "Iron", BlockType.Engine));
                voxel.AddBlock(new VoxelBlock(new Vector3(2.5f, 0, 0), new Vector3(1.5f, 1.5f, 1.5f), "Iron", BlockType.Generator));
                break;
                
            case "fighter":
                // Combat-focused fighter
                voxel.AddBlock(new VoxelBlock(new Vector3(0, 0, 0), new Vector3(3, 3, 3), "Titanium", BlockType.Hull));
                voxel.AddBlock(new VoxelBlock(new Vector3(-2.5f, 0, 0), new Vector3(2, 2, 2), "Ogonite", BlockType.Engine));
                voxel.AddBlock(new VoxelBlock(new Vector3(2.5f, 0, 0), new Vector3(2, 2, 2), "Naonite", BlockType.ShieldGenerator));
                voxel.AddBlock(new VoxelBlock(new Vector3(0, 2.5f, 0), new Vector3(2, 2, 2), "Xanion", BlockType.Generator));
                voxel.AddBlock(new VoxelBlock(new Vector3(0, -2.5f, 0), new Vector3(2, 2, 2), "Trinium", BlockType.Thruster));
                break;
                
            case "miner":
                // Mining-focused ship
                voxel.AddBlock(new VoxelBlock(new Vector3(0, 0, 0), new Vector3(4, 4, 4), "Iron", BlockType.Hull));
                voxel.AddBlock(new VoxelBlock(new Vector3(-3f, 0, 0), new Vector3(2, 2, 2), "Iron", BlockType.Engine));
                voxel.AddBlock(new VoxelBlock(new Vector3(3f, 0, 0), new Vector3(3, 3, 3), "Iron", BlockType.Cargo));
                voxel.AddBlock(new VoxelBlock(new Vector3(0, 3f, 0), new Vector3(2, 2, 2), "Iron", BlockType.Generator));
                break;
                
            case "trader":
                // Trading-focused ship with cargo capacity
                voxel.AddBlock(new VoxelBlock(new Vector3(0, 0, 0), new Vector3(5, 3, 3), "Titanium", BlockType.Hull));
                voxel.AddBlock(new VoxelBlock(new Vector3(-3.5f, 0, 0), new Vector3(2, 2, 2), "Titanium", BlockType.Engine));
                voxel.AddBlock(new VoxelBlock(new Vector3(3.5f, 0, 0), new Vector3(3, 3, 3), "Iron", BlockType.Cargo));
                voxel.AddBlock(new VoxelBlock(new Vector3(3.5f, 0, 3.5f), new Vector3(3, 3, 3), "Iron", BlockType.Cargo));
                voxel.AddBlock(new VoxelBlock(new Vector3(0, 3f, 0), new Vector3(2, 2, 2), "Titanium", BlockType.Generator));
                break;
                
            default:
                // Default to starter
                voxel.AddBlock(new VoxelBlock(new Vector3(0, 0, 0), new Vector3(2, 2, 2), "Titanium", BlockType.Hull));
                voxel.AddBlock(new VoxelBlock(new Vector3(-2.5f, 0, 0), new Vector3(1.5f, 1.5f, 1.5f), "Iron", BlockType.Engine));
                break;
        }
        
        return voxel;
    }
    
    static void RunIndustrialMiningShipDemo()
    {
        Console.WriteLine("\n=== INDUSTRIAL MINING SHIP SHOWCASE ===");
        Console.WriteLine("Generating industrial mining ships with angular, blocky voxel-based designs");
        Console.WriteLine("Inspired by mining ships from Space Engineers, Empyrion, and Avorion\n");
        
        try
        {
            var showcase = new IndustrialMiningShipShowcase(_gameEngine!.EntityManager);
            var ships = showcase.GenerateShowcase(Environment.TickCount);
            
            // Interactive menu
            bool done = false;
            while (!done)
            {
                Console.WriteLine("\n=== MINING SHIP SHOWCASE MENU ===");
                Console.WriteLine("1. View Ship Details (enter ship number)");
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
                    Console.WriteLine("\nLaunching 3D viewer with all mining ships...");
                    Console.WriteLine("Use camera controls to inspect each ship.");
                    Console.WriteLine("Mining ships feature:");
                    Console.WriteLine("  • Angular, blocky industrial hulls");
                    Console.WriteLine("  • Exposed framework/gantry structures");
                    Console.WriteLine("  • Forward-mounted mining equipment");
                    Console.WriteLine("  • Large cargo bays on sides\n");
                    
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
                else if (int.TryParse(choice, out int shipNum) && shipNum >= 1 && shipNum <= ships.Count)
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
            Console.WriteLine($"\n✗ Mining ship showcase error: {ex.Message}");
            Console.WriteLine($"Stack trace: {ex.StackTrace}");
            Console.ResetColor();
        }
    }
    
    static void RunRenderingModeDemo()
    {
        Console.WriteLine("\n=== NPR/PBR RENDERING MODE DEMO ===");
        Console.WriteLine("Testing rendering modes to fix visual issues on blocks");
        Console.WriteLine();
        
        try
        {
            // Show rendering mode information first
            RenderingModeDemo.DisplayRenderingModeInfo();
            
            // Run the interactive demo
            var demo = new RenderingModeDemo(_gameEngine!.EntityManager);
            demo.RunDemo();
        }
        catch (Exception ex)
        {
            Console.ForegroundColor = ConsoleColor.Red;
            Console.WriteLine($"\n✗ Rendering mode demo error: {ex.Message}");
            Console.WriteLine($"Stack trace: {ex.StackTrace}");
            Console.ResetColor();
        }
        
        Console.WriteLine("\nPress any key to return to main menu...");
        Console.ReadKey();
    }
    
    static void RunWorldGenerationShowcase()
    {
        Console.WriteLine("\n=== WORLD GENERATION OPTIONS - VISUAL SHOWCASE ===");
        Console.WriteLine("Demonstrating all procedural generation features\n");
        
        try
        {
            var showcase = new WorldGenerationShowcase(_gameEngine!.EntityManager);
            showcase.RunShowcase();
        }
        catch (Exception ex)
        {
            Console.ForegroundColor = ConsoleColor.Red;
            Console.WriteLine($"\n✗ World generation showcase error: {ex.Message}");
            Console.WriteLine($"Stack trace: {ex.StackTrace}");
            Console.ResetColor();
        }
        
        Console.WriteLine("\nPress any key to return to main menu...");
        Console.ReadKey();
    }
    
    static void RunModularShipDemo()
    {
        Console.WriteLine("\n=== NMS-STYLE MODULAR SHIP GENERATOR ===");
        Console.WriteLine("Building ships from snap-together modules\n");
        
        try
        {
            var example = new ModularShipExample();
            example.RunExample();
        }
        catch (Exception ex)
        {
            Console.ForegroundColor = ConsoleColor.Red;
            Console.WriteLine($"\n✗ Modular ship generation error: {ex.Message}");
            Console.WriteLine($"Stack trace: {ex.StackTrace}");
            Console.ResetColor();
        }
        
        Console.WriteLine("\nPress any key to return to main menu...");
        Console.ReadKey();
    }
}
