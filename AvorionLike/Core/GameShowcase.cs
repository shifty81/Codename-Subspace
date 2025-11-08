using System.Numerics;
using AvorionLike.Core.ECS;
using AvorionLike.Core.Voxel;
using AvorionLike.Core.Physics;
using AvorionLike.Core.Resources;
using AvorionLike.Core.Combat;
using AvorionLike.Core.RPG;
using AvorionLike.Core.Navigation;

namespace AvorionLike.Core;

/// <summary>
/// Showcase system that creates a visually impressive demo of the game's capabilities
/// </summary>
public class GameShowcase
{
    private readonly GameEngine _gameEngine;
    private readonly Random _random;

    public GameShowcase(GameEngine gameEngine, int seed = 42)
    {
        _gameEngine = gameEngine;
        _random = new Random(seed);
    }

    /// <summary>
    /// Create an impressive showcase scene with multiple ships of varying designs
    /// </summary>
    public Guid CreateShowcaseScene()
    {
        Console.WriteLine("\n=== Creating Visual Showcase ===");
        Console.WriteLine("Building a stunning display of game capabilities...\n");

        // Create player ship as centerpiece
        var playerShip = CreateShowcasePlayerShip();
        
        // Create surrounding fleet of different ship types
        CreateShowcaseFleet(playerShip);
        
        // Populate with asteroids and stations
        var playerPhysics = _gameEngine.EntityManager.GetComponent<PhysicsComponent>(playerShip);
        if (playerPhysics != null)
        {
            var worldPopulator = new GameWorldPopulator(_gameEngine, seed: 12345);
            worldPopulator.PopulateStarterArea(playerPhysics.Position, radius: 1200f);
        }

        Console.WriteLine("✓ Showcase scene created!");
        Console.WriteLine("  - 1 Hero Ship (Player)");
        Console.WriteLine("  - Multiple escort ships");
        Console.WriteLine("  - Asteroids and stations");
        Console.WriteLine("  - Ready for exploration!\n");

        return playerShip;
    }

    /// <summary>
    /// Create an impressive player ship with all systems and colorful materials
    /// </summary>
    private Guid CreateShowcasePlayerShip()
    {
        Console.WriteLine("Building showcase player ship...");
        
        var playerShip = _gameEngine.EntityManager.CreateEntity("Hero-class Battlecruiser");
        var voxelComponent = new VoxelStructureComponent();

        // Core command section - Avorion (purple, most advanced)
        voxelComponent.AddBlock(new VoxelBlock(
            new Vector3(0, 0, 0),
            new Vector3(4, 4, 4),
            "Avorion",
            BlockType.Hull
        ));

        // Bridge tower - Xanion (gold)
        voxelComponent.AddBlock(new VoxelBlock(
            new Vector3(0, 5, 0),
            new Vector3(3, 3, 3),
            "Xanion",
            BlockType.Hull
        ));

        // Main hull sections - Titanium (silver-blue)
        for (int i = -12; i <= 8; i += 4)
        {
            voxelComponent.AddBlock(new VoxelBlock(
                new Vector3(i, 0, 0),
                new Vector3(4, 3, 4),
                "Titanium",
                BlockType.Hull
            ));
        }

        // Port and starboard wings - Trinium (blue)
        for (int z = -8; z <= 8; z += 8)
        {
            voxelComponent.AddBlock(new VoxelBlock(
                new Vector3(-8, 0, z),
                new Vector3(16, 2, 3),
                "Trinium",
                BlockType.Hull
            ));
        }

        // Main engines (rear) - Ogonite (red/orange with glow)
        for (int z = -6; z <= 6; z += 6)
        {
            voxelComponent.AddBlock(new VoxelBlock(
                new Vector3(-14, 0, z),
                new Vector3(3, 3, 3),
                "Ogonite",
                BlockType.Engine
            ));
        }

        // Additional maneuvering thrusters
        Vector3[] thrusterPositions = new[]
        {
            new Vector3(10, 2, 0),
            new Vector3(0, 2, 8),
            new Vector3(0, 2, -8),
            new Vector3(0, -3, 0)
        };

        foreach (var pos in thrusterPositions)
        {
            voxelComponent.AddBlock(new VoxelBlock(
                pos,
                new Vector3(2, 2, 2),
                "Trinium",
                BlockType.Thruster
            ));
        }

        // Power generators - Xanion (gold with glow)
        voxelComponent.AddBlock(new VoxelBlock(
            new Vector3(4, 0, 0),
            new Vector3(3, 3, 3),
            "Xanion",
            BlockType.Generator
        ));

        // Shield generators - Naonite (green with glow)
        for (int z = -4; z <= 4; z += 8)
        {
            voxelComponent.AddBlock(new VoxelBlock(
                new Vector3(0, 0, z),
                new Vector3(2.5f, 2.5f, 2.5f),
                "Naonite",
                BlockType.ShieldGenerator
            ));
        }

        // Gyro arrays - Avorion (purple with strong glow)
        for (int i = 0; i < 4; i++)
        {
            float angle = i * MathF.PI / 2f;
            float x = MathF.Cos(angle) * 6f;
            float z = MathF.Sin(angle) * 6f;
            
            voxelComponent.AddBlock(new VoxelBlock(
                new Vector3(x, 1, z),
                new Vector3(2, 2, 2),
                "Avorion",
                BlockType.GyroArray
            ));
        }

        // Weapon hardpoints - Iron (gray)
        Vector3[] weaponPositions = new[]
        {
            new Vector3(8, 1, 6),
            new Vector3(8, 1, -6),
            new Vector3(-10, 1, 6),
            new Vector3(-10, 1, -6)
        };

        foreach (var pos in weaponPositions)
        {
            voxelComponent.AddBlock(new VoxelBlock(
                pos,
                new Vector3(2, 1.5f, 2),
                "Iron",
                BlockType.TurretMount
            ));
        }

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

        // Add inventory with resources
        var inventoryComponent = new InventoryComponent(2000);
        inventoryComponent.Inventory.AddResource(ResourceType.Credits, 50000);
        inventoryComponent.Inventory.AddResource(ResourceType.Iron, 1000);
        inventoryComponent.Inventory.AddResource(ResourceType.Titanium, 800);
        inventoryComponent.Inventory.AddResource(ResourceType.Naonite, 600);
        inventoryComponent.Inventory.AddResource(ResourceType.Trinium, 400);
        inventoryComponent.Inventory.AddResource(ResourceType.Xanion, 200);
        inventoryComponent.Inventory.AddResource(ResourceType.Ogonite, 150);
        inventoryComponent.Inventory.AddResource(ResourceType.Avorion, 100);
        _gameEngine.EntityManager.AddComponent(playerShip.Id, inventoryComponent);

        // Add progression
        var progressionComponent = new ProgressionComponent
        {
            EntityId = playerShip.Id,
            Level = 5,
            Experience = 12500,
            SkillPoints = 10
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
            JumpRange = 10f
        };
        _gameEngine.EntityManager.AddComponent(playerShip.Id, hyperdriveComponent);

        // Add sector location
        var locationComponent = new SectorLocationComponent
        {
            EntityId = playerShip.Id,
            CurrentSector = new SectorCoordinate(0, 0, 0)
        };
        _gameEngine.EntityManager.AddComponent(playerShip.Id, locationComponent);

        Console.WriteLine($"  ✓ Hero ship: {voxelComponent.Blocks.Count} blocks, {voxelComponent.TotalMass:F0} kg");
        
        return playerShip.Id;
    }

    /// <summary>
    /// Create a fleet of escort ships around the player
    /// </summary>
    private void CreateShowcaseFleet(Guid playerShipId)
    {
        Console.WriteLine("Creating escort fleet...");
        
        var playerPhysics = _gameEngine.EntityManager.GetComponent<PhysicsComponent>(playerShipId);
        if (playerPhysics == null) return;

        // Create different ship types at various positions
        CreateScoutShip(playerPhysics.Position + new Vector3(50, 20, 30));
        CreateScoutShip(playerPhysics.Position + new Vector3(-50, -20, -30));
        CreateFrigateShip(playerPhysics.Position + new Vector3(80, 0, 60));
        CreateFrigateShip(playerPhysics.Position + new Vector3(-80, 0, -60));

        Console.WriteLine("  ✓ Fleet created: 2 scouts, 2 frigates");
    }

    private Guid CreateScoutShip(Vector3 position)
    {
        var ship = _gameEngine.EntityManager.CreateEntity("Scout Ship");
        var voxelComponent = new VoxelStructureComponent();

        // Small, fast design
        voxelComponent.AddBlock(new VoxelBlock(new Vector3(0, 0, 0), new Vector3(2, 2, 3), "Trinium", BlockType.Hull));
        voxelComponent.AddBlock(new VoxelBlock(new Vector3(-3, 0, 0), new Vector3(2, 1.5f, 2), "Ogonite", BlockType.Engine));
        voxelComponent.AddBlock(new VoxelBlock(new Vector3(0, 1.5f, 0), new Vector3(1.5f, 1.5f, 1.5f), "Naonite", BlockType.ShieldGenerator));

        _gameEngine.EntityManager.AddComponent(ship.Id, voxelComponent);

        var physicsComponent = new PhysicsComponent
        {
            Position = position,
            Velocity = Vector3.Zero,
            Mass = voxelComponent.TotalMass,
            MomentOfInertia = voxelComponent.MomentOfInertia,
            MaxThrust = voxelComponent.TotalThrust,
            MaxTorque = voxelComponent.TotalTorque
        };
        _gameEngine.EntityManager.AddComponent(ship.Id, physicsComponent);

        return ship.Id;
    }

    private Guid CreateFrigateShip(Vector3 position)
    {
        var ship = _gameEngine.EntityManager.CreateEntity("Frigate");
        var voxelComponent = new VoxelStructureComponent();

        // Medium-sized combat ship
        voxelComponent.AddBlock(new VoxelBlock(new Vector3(0, 0, 0), new Vector3(3, 3, 4), "Titanium", BlockType.Hull));
        voxelComponent.AddBlock(new VoxelBlock(new Vector3(-5, 0, 0), new Vector3(2.5f, 2.5f, 2.5f), "Ogonite", BlockType.Engine));
        voxelComponent.AddBlock(new VoxelBlock(new Vector3(0, 0, 3), new Vector3(2, 2, 2), "Naonite", BlockType.ShieldGenerator));
        voxelComponent.AddBlock(new VoxelBlock(new Vector3(4, 1, 2), new Vector3(2, 1, 2), "Iron", BlockType.TurretMount));
        voxelComponent.AddBlock(new VoxelBlock(new Vector3(4, 1, -2), new Vector3(2, 1, 2), "Iron", BlockType.TurretMount));

        _gameEngine.EntityManager.AddComponent(ship.Id, voxelComponent);

        var physicsComponent = new PhysicsComponent
        {
            Position = position,
            Velocity = Vector3.Zero,
            Mass = voxelComponent.TotalMass,
            MomentOfInertia = voxelComponent.MomentOfInertia,
            MaxThrust = voxelComponent.TotalThrust,
            MaxTorque = voxelComponent.TotalTorque
        };
        _gameEngine.EntityManager.AddComponent(ship.Id, physicsComponent);

        return ship.Id;
    }
}
