using System.Numerics;
using AvorionLike.Core.Procedural;
using AvorionLike.Core.ECS;
using AvorionLike.Core.Voxel;
using AvorionLike.Core.Physics;

namespace AvorionLike.Examples;

/// <summary>
/// Showcase example that generates 20 procedural ships and displays them in a grid
/// with numbers so users can pick which generation they like best
/// </summary>
public class ShipShowcaseExample
{
    private readonly EntityManager _entityManager;
    private readonly List<GeneratedShipDisplay> _generatedShips = new();

    public class GeneratedShipDisplay
    {
        public int Number { get; set; }
        public Guid EntityId { get; set; }
        public GeneratedShip ShipData { get; set; } = null!;
        public Vector3 Position { get; set; }
        public string Description { get; set; } = "";
    }

    public ShipShowcaseExample(EntityManager entityManager)
    {
        _entityManager = entityManager;
    }

    /// <summary>
    /// Generate 20 ships with different configurations and display them in a grid
    /// </summary>
    public List<GeneratedShipDisplay> GenerateShowcase(int baseSeed = 0)
    {
        Console.WriteLine("\n=== SHIP GENERATION SHOWCASE ===");
        Console.WriteLine("Generating 20 procedural ships with varied designs...\n");

        _generatedShips.Clear();

        // Define a mix of configurations to showcase variety
        var configurations = new[]
        {
            // Fighters (small, angular)
            new { Size = ShipSize.Fighter, Role = ShipRole.Combat, Hull = ShipHullShape.Angular, Faction = "Military" },
            new { Size = ShipSize.Fighter, Role = ShipRole.Combat, Hull = ShipHullShape.Sleek, Faction = "Military" },
            new { Size = ShipSize.Fighter, Role = ShipRole.Exploration, Hull = ShipHullShape.Sleek, Faction = "Explorers" },
            
            // Corvettes (small to medium)
            new { Size = ShipSize.Corvette, Role = ShipRole.Combat, Hull = ShipHullShape.Angular, Faction = "Military" },
            new { Size = ShipSize.Corvette, Role = ShipRole.Multipurpose, Hull = ShipHullShape.Blocky, Faction = "Traders" },
            new { Size = ShipSize.Corvette, Role = ShipRole.Mining, Hull = ShipHullShape.Blocky, Faction = "Miners" },
            
            // Frigates (medium)
            new { Size = ShipSize.Frigate, Role = ShipRole.Combat, Hull = ShipHullShape.Angular, Faction = "Military" },
            new { Size = ShipSize.Frigate, Role = ShipRole.Multipurpose, Hull = ShipHullShape.Blocky, Faction = "Default" },
            new { Size = ShipSize.Frigate, Role = ShipRole.Trading, Hull = ShipHullShape.Cylindrical, Faction = "Traders" },
            new { Size = ShipSize.Frigate, Role = ShipRole.Exploration, Hull = ShipHullShape.Sleek, Faction = "Explorers" },
            
            // Destroyers (medium to large)
            new { Size = ShipSize.Destroyer, Role = ShipRole.Combat, Hull = ShipHullShape.Angular, Faction = "Military" },
            new { Size = ShipSize.Destroyer, Role = ShipRole.Multipurpose, Hull = ShipHullShape.Blocky, Faction = "Default" },
            new { Size = ShipSize.Destroyer, Role = ShipRole.Trading, Hull = ShipHullShape.Cylindrical, Faction = "Traders" },
            new { Size = ShipSize.Destroyer, Role = ShipRole.Salvage, Hull = ShipHullShape.Irregular, Faction = "Pirates" },
            
            // Cruisers (large)
            new { Size = ShipSize.Cruiser, Role = ShipRole.Combat, Hull = ShipHullShape.Angular, Faction = "Military" },
            new { Size = ShipSize.Cruiser, Role = ShipRole.Multipurpose, Hull = ShipHullShape.Blocky, Faction = "Default" },
            new { Size = ShipSize.Cruiser, Role = ShipRole.Trading, Hull = ShipHullShape.Cylindrical, Faction = "Traders" },
            
            // Battleships (very large)
            new { Size = ShipSize.Battleship, Role = ShipRole.Combat, Hull = ShipHullShape.Angular, Faction = "Military" },
            new { Size = ShipSize.Battleship, Role = ShipRole.Multipurpose, Hull = ShipHullShape.Blocky, Faction = "Default" },
            
            // Carrier (massive)
            new { Size = ShipSize.Carrier, Role = ShipRole.Multipurpose, Hull = ShipHullShape.Blocky, Faction = "Military" },
        };

        // Arrange ships in a 5x4 grid
        int gridWidth = 5;
        float spacing = 150f; // Space between ships

        for (int i = 0; i < configurations.Length && i < 20; i++)
        {
            var config = configurations[i];
            
            // Calculate grid position
            int row = i / gridWidth;
            int col = i % gridWidth;
            Vector3 position = new Vector3(
                col * spacing - (gridWidth - 1) * spacing / 2f,
                0,
                row * spacing
            );

            // Generate the ship
            var generatedShip = GenerateShipAtPosition(
                i + 1,
                position,
                config.Size,
                config.Role,
                config.Hull,
                config.Faction,
                baseSeed + i
            );

            _generatedShips.Add(generatedShip);

            // Progress indicator
            if ((i + 1) % 5 == 0)
            {
                Console.WriteLine($"  Generated {i + 1}/20 ships...");
            }
        }

        Console.WriteLine($"\n✓ Generated all 20 ships!");
        Console.WriteLine("\n=== SHOWCASE READY ===");
        Console.WriteLine("Ships are arranged in a 5x4 grid");
        Console.WriteLine("Each ship is numbered and can be inspected individually");
        Console.WriteLine();

        // Print summary
        PrintShowcaseSummary();

        return _generatedShips;
    }

    /// <summary>
    /// Generate a single ship at a specific position with configuration
    /// </summary>
    private GeneratedShipDisplay GenerateShipAtPosition(
        int number,
        Vector3 position,
        ShipSize size,
        ShipRole role,
        ShipHullShape hullShape,
        string factionName,
        int seed)
    {
        var generator = new ProceduralShipGenerator(seed);
        var style = FactionShipStyle.GetDefaultStyle(factionName);
        style.PreferredHullShape = hullShape; // Override hull shape

        var config = new ShipGenerationConfig
        {
            Size = size,
            Role = role,
            Material = style.PreferredMaterial,
            Style = style,
            Seed = seed,
            RequireHyperdrive = size >= ShipSize.Frigate,
            RequireCargo = true,
            MinimumWeaponMounts = role == ShipRole.Combat ? 4 : 2
        };

        var shipData = generator.GenerateShip(config);

        // Create entity
        var entity = _entityManager.CreateEntity($"Ship #{number} - {size} {hullShape}");
        
        // Add voxel structure
        _entityManager.AddComponent(entity.Id, shipData.Structure);

        // Add physics at specified position
        var physics = new PhysicsComponent
        {
            EntityId = entity.Id,
            Position = position,
            Velocity = Vector3.Zero,
            Mass = shipData.TotalMass,
            MomentOfInertia = shipData.Structure.MomentOfInertia,
            MaxThrust = shipData.TotalThrust,
            MaxTorque = shipData.Structure.TotalTorque
        };
        _entityManager.AddComponent(entity.Id, physics);

        // Create display info
        var display = new GeneratedShipDisplay
        {
            Number = number,
            EntityId = entity.Id,
            ShipData = shipData,
            Position = position,
            Description = $"{size} {hullShape} - {role} ({factionName})"
        };

        Console.WriteLine($"  #{number:D2}: {display.Description} - {shipData.Structure.Blocks.Count} blocks");

        return display;
    }

    /// <summary>
    /// Print a summary of all generated ships
    /// </summary>
    public void PrintShowcaseSummary()
    {
        Console.WriteLine("=== SHIP SHOWCASE SUMMARY ===");
        Console.WriteLine();
        Console.WriteLine("  #  | Size        | Hull      | Role         | Blocks | Faction");
        Console.WriteLine("-----|-------------|-----------|--------------|--------|------------");

        foreach (var ship in _generatedShips)
        {
            var config = ship.ShipData.Config;
            Console.WriteLine($"  {ship.Number:D2} | {config.Size,-11} | {config.Style.PreferredHullShape,-9} | {config.Role,-12} | {ship.ShipData.Structure.Blocks.Count,6} | {config.Style.FactionName}");
        }

        Console.WriteLine();
        Console.WriteLine("Key Features:");
        Console.WriteLine("  • Angular shapes - Sharp, military-style wedges and fighters");
        Console.WriteLine("  • Sleek shapes - Streamlined exploration vessels");
        Console.WriteLine("  • Blocky shapes - Industrial, utilitarian designs");
        Console.WriteLine("  • Cylindrical - Trading and cargo haulers");
        Console.WriteLine("  • Varied block shapes - Non-square blocks for detail");
        Console.WriteLine();
    }

    /// <summary>
    /// Get detailed information about a specific ship
    /// </summary>
    public void PrintShipDetails(int number)
    {
        var ship = _generatedShips.FirstOrDefault(s => s.Number == number);
        if (ship == null)
        {
            Console.WriteLine($"Ship #{number} not found!");
            return;
        }

        Console.WriteLine($"\n=== SHIP #{number} DETAILS ===");
        Console.WriteLine($"Description: {ship.Description}");
        Console.WriteLine($"Position: ({ship.Position.X:F1}, {ship.Position.Y:F1}, {ship.Position.Z:F1})");
        Console.WriteLine();
        Console.WriteLine("Configuration:");
        Console.WriteLine($"  Size: {ship.ShipData.Config.Size}");
        Console.WriteLine($"  Role: {ship.ShipData.Config.Role}");
        Console.WriteLine($"  Hull Shape: {ship.ShipData.Config.Style.PreferredHullShape}");
        Console.WriteLine($"  Faction: {ship.ShipData.Config.Style.FactionName}");
        Console.WriteLine($"  Material: {ship.ShipData.Config.Material}");
        Console.WriteLine();
        Console.WriteLine("Statistics:");
        Console.WriteLine($"  Total Blocks: {ship.ShipData.Structure.Blocks.Count}");
        Console.WriteLine($"  Mass: {ship.ShipData.TotalMass:F2} kg");
        Console.WriteLine($"  Thrust: {ship.ShipData.TotalThrust:F2} N");
        Console.WriteLine($"  Thrust/Mass Ratio: {ship.ShipData.Stats["ThrustToMass"]:F2}");
        Console.WriteLine($"  Power: {ship.ShipData.TotalPowerGeneration:F2} W");
        Console.WriteLine($"  Shields: {ship.ShipData.TotalShieldCapacity:F2}");
        Console.WriteLine($"  Weapons: {ship.ShipData.WeaponMountCount}");
        Console.WriteLine($"  Cargo: {ship.ShipData.CargoBlockCount} bays");
        Console.WriteLine($"  Structural Integrity: {ship.ShipData.Stats.GetValueOrDefault("StructuralIntegrity", 0f):F1}%");
        
        if (ship.ShipData.Warnings.Count > 0)
        {
            Console.WriteLine();
            Console.WriteLine("Warnings:");
            foreach (var warning in ship.ShipData.Warnings)
            {
                Console.WriteLine($"  ⚠ {warning}");
            }
        }
        
        Console.WriteLine();
    }

    /// <summary>
    /// Get all generated ships
    /// </summary>
    public List<GeneratedShipDisplay> GetGeneratedShips()
    {
        return _generatedShips;
    }
}
