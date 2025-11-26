using System.Numerics;
using AvorionLike.Core.Procedural;
using AvorionLike.Core.ECS;
using AvorionLike.Core.Voxel;
using AvorionLike.Core.Physics;

namespace AvorionLike.Examples;

/// <summary>
/// Showcase example for industrial mining ships
/// Demonstrates the new IndustrialMiningShipGenerator with angular, blocky voxel-based designs
/// </summary>
public class IndustrialMiningShipShowcase
{
    private readonly EntityManager _entityManager;
    private readonly List<MiningShipDisplay> _generatedShips = new();

    public class MiningShipDisplay
    {
        public int Number { get; set; }
        public Guid EntityId { get; set; }
        public GeneratedMiningShip ShipData { get; set; } = null!;
        public Vector3 Position { get; set; }
        public string Description { get; set; } = "";
    }

    public IndustrialMiningShipShowcase(EntityManager entityManager)
    {
        _entityManager = entityManager;
    }

    /// <summary>
    /// Generate showcase of industrial mining ships
    /// </summary>
    public List<MiningShipDisplay> GenerateShowcase(int baseSeed = 0)
    {
        Console.WriteLine("\n╔══════════════════════════════════════════════════════════════╗");
        Console.WriteLine("║           INDUSTRIAL MINING SHIP SHOWCASE                     ║");
        Console.WriteLine("║  Angular & Blocky Voxel-Based Mining Vessels                 ║");
        Console.WriteLine("╚══════════════════════════════════════════════════════════════╝\n");
        
        _generatedShips.Clear();

        // Define varied mining ship configurations
        var configurations = new[]
        {
            // Small mining drones
            new MiningShipVariant(ShipSize.Fighter, "Mining Drone Mk1", false, false, true, 2, 0, 2),
            new MiningShipVariant(ShipSize.Fighter, "Mining Drone Mk2", true, false, true, 3, 1, 3),
            
            // Light mining vessels
            new MiningShipVariant(ShipSize.Corvette, "Light Prospector", true, true, true, 4, 1, 4),
            new MiningShipVariant(ShipSize.Corvette, "Scout Miner", false, true, true, 3, 1, 3),
            
            // Standard mining ships
            new MiningShipVariant(ShipSize.Frigate, "Industrial Miner", true, true, true, 5, 2, 6),
            new MiningShipVariant(ShipSize.Frigate, "Heavy Extractor", true, true, true, 6, 2, 8),
            new MiningShipVariant(ShipSize.Frigate, "Ore Harvester", true, true, true, 4, 3, 6),
            
            // Heavy mining ships
            new MiningShipVariant(ShipSize.Destroyer, "Mining Destroyer", true, true, true, 6, 3, 10),
            new MiningShipVariant(ShipSize.Destroyer, "Asteroid Crusher", true, true, true, 8, 2, 8),
            
            // Mining cruisers
            new MiningShipVariant(ShipSize.Cruiser, "Mining Operations Cruiser", true, true, true, 8, 4, 12),
            
            // Industrial behemoths
            new MiningShipVariant(ShipSize.Battleship, "Industrial Behemoth", true, true, true, 10, 5, 16),
            
            // Massive mining operation vessels
            new MiningShipVariant(ShipSize.Carrier, "Mining Fleet Carrier", true, true, true, 12, 6, 20),
        };

        // Arrange ships in a grid
        int gridWidth = 4;
        float spacing = 200f;

        Console.WriteLine("Generating industrial mining ships...\n");
        Console.WriteLine("Design Features:");
        Console.WriteLine("  • Angular, blocky industrial hull shapes");
        Console.WriteLine("  • Exposed framework/gantry structures");
        Console.WriteLine("  • Large cargo modules on sides");
        Console.WriteLine("  • Forward-mounted mining equipment");
        Console.WriteLine("  • Asymmetric mining arms");
        Console.WriteLine("  • Industrial color scheme (gray, rust, orange)\n");

        for (int i = 0; i < configurations.Length; i++)
        {
            var variant = configurations[i];
            
            // Calculate grid position
            int row = i / gridWidth;
            int col = i % gridWidth;
            Vector3 position = new Vector3(
                col * spacing - (gridWidth - 1) * spacing / 2f,
                0,
                row * spacing
            );

            // Generate the ship
            var generatedShip = GenerateMiningShipAtPosition(
                i + 1,
                position,
                variant,
                baseSeed + i
            );

            _generatedShips.Add(generatedShip);

            // Progress indicator
            Console.WriteLine($"  [{i + 1:D2}/{configurations.Length}] {variant.Name}");
            Console.WriteLine($"       Size: {variant.Size} | Blocks: {generatedShip.ShipData.Structure.Blocks.Count}");
            Console.WriteLine($"       Mining Lasers: {generatedShip.ShipData.MiningLaserCount} | Cargo Bays: {generatedShip.ShipData.CargoCapacity}");
            Console.WriteLine();
        }

        Console.WriteLine($"✓ Generated {configurations.Length} industrial mining ships!\n");

        // Print summary
        PrintShowcaseSummary();

        return _generatedShips;
    }

    private class MiningShipVariant
    {
        public ShipSize Size { get; }
        public string Name { get; }
        public bool UseExposedFramework { get; }
        public bool UseAsymmetricMiningArms { get; }
        public bool UseAngularPanels { get; }
        public int MiningLaserCount { get; }
        public int OreProcessorCount { get; }
        public int CargoModuleCount { get; }

        public MiningShipVariant(ShipSize size, string name, bool framework, bool asymArms, bool angular, 
            int lasers, int processors, int cargo)
        {
            Size = size;
            Name = name;
            UseExposedFramework = framework;
            UseAsymmetricMiningArms = asymArms;
            UseAngularPanels = angular;
            MiningLaserCount = lasers;
            OreProcessorCount = processors;
            CargoModuleCount = cargo;
        }
    }

    /// <summary>
    /// Generate a single mining ship at a specific position
    /// </summary>
    private MiningShipDisplay GenerateMiningShipAtPosition(
        int number,
        Vector3 position,
        MiningShipVariant variant,
        int seed)
    {
        var generator = new IndustrialMiningShipGenerator(seed);

        var config = new IndustrialMiningShipConfig
        {
            Size = variant.Size,
            Material = "Iron",
            Seed = seed,
            MiningLaserCount = variant.MiningLaserCount,
            OreProcessorCount = variant.OreProcessorCount,
            CargoModuleCount = variant.CargoModuleCount,
            UseExposedFramework = variant.UseExposedFramework,
            UseAsymmetricMiningArms = variant.UseAsymmetricMiningArms,
            UseAngularPanels = variant.UseAngularPanels,
            IndustrialComplexity = 0.7f
        };

        var shipData = generator.GenerateMiningShip(config);
        shipData.ShipName = variant.Name;

        // Create entity
        var entity = _entityManager.CreateEntity($"Mining Ship #{number} - {variant.Name}");
        
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
        var display = new MiningShipDisplay
        {
            Number = number,
            EntityId = entity.Id,
            ShipData = shipData,
            Position = position,
            Description = $"{variant.Size} {variant.Name}"
        };

        return display;
    }

    /// <summary>
    /// Print a summary of all generated ships
    /// </summary>
    public void PrintShowcaseSummary()
    {
        Console.WriteLine("╔══════════════════════════════════════════════════════════════════════════════════════╗");
        Console.WriteLine("║                        INDUSTRIAL MINING SHIP SUMMARY                                ║");
        Console.WriteLine("╠══════════════════════════════════════════════════════════════════════════════════════╣");
        Console.WriteLine("║  #  │ Size        │ Ship Name                    │ Blocks │ Lasers │ Cargo │ Mass   ║");
        Console.WriteLine("╠═════╪═════════════╪══════════════════════════════╪════════╪════════╪═══════╪════════╣");

        foreach (var ship in _generatedShips)
        {
            string sizeName = ship.ShipData.Config.Size.ToString().PadRight(11);
            string shipName = ship.ShipData.ShipName.PadRight(28);
            if (shipName.Length > 28) shipName = shipName.Substring(0, 25) + "...";
            
            Console.WriteLine($"║ {ship.Number,2} │ {sizeName} │ {shipName} │ {ship.ShipData.Structure.Blocks.Count,6} │ {ship.ShipData.MiningLaserCount,6} │ {ship.ShipData.CargoCapacity,5} │ {ship.ShipData.TotalMass,6:F0} ║");
        }

        Console.WriteLine("╚══════════════════════════════════════════════════════════════════════════════════════╝");
        Console.WriteLine();
        Console.WriteLine("Design Characteristics:");
        Console.WriteLine("  ┌─────────────────────────────────────────────────────────────────┐");
        Console.WriteLine("  │ ANGULAR & BLOCKY:  All ships feature industrial box-like hulls │");
        Console.WriteLine("  │ EXPOSED FRAMEWORK: Visible gantry structures and support beams │");
        Console.WriteLine("  │ CARGO MODULES:     Large boxy containers on ship sides         │");
        Console.WriteLine("  │ MINING EQUIPMENT:  Forward-facing laser mounts and drill arms  │");
        Console.WriteLine("  │ ORE PROCESSING:    Dorsal processing units with pipe networks  │");
        Console.WriteLine("  │ INDUSTRIAL COLORS: Gray hulls, rust accents, orange highlights │");
        Console.WriteLine("  └─────────────────────────────────────────────────────────────────┘");
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

        Console.WriteLine($"\n╔══════════════════════════════════════════════════════════════╗");
        Console.WriteLine($"║  MINING SHIP #{number} - {ship.ShipData.ShipName,-37} ║");
        Console.WriteLine($"╚══════════════════════════════════════════════════════════════╝");
        Console.WriteLine();
        Console.WriteLine($"Configuration:");
        Console.WriteLine($"  Size: {ship.ShipData.Config.Size}");
        Console.WriteLine($"  Material: {ship.ShipData.Config.Material}");
        Console.WriteLine($"  Exposed Framework: {(ship.ShipData.Config.UseExposedFramework ? "Yes" : "No")}");
        Console.WriteLine($"  Asymmetric Mining Arms: {(ship.ShipData.Config.UseAsymmetricMiningArms ? "Yes" : "No")}");
        Console.WriteLine($"  Angular Panels: {(ship.ShipData.Config.UseAngularPanels ? "Yes" : "No")}");
        Console.WriteLine();
        Console.WriteLine("Mining Statistics:");
        Console.WriteLine($"  Mining Lasers: {ship.ShipData.MiningLaserCount}");
        Console.WriteLine($"  Ore Processors: {ship.ShipData.Config.OreProcessorCount}");
        Console.WriteLine($"  Cargo Capacity: {ship.ShipData.CargoCapacity} bays");
        Console.WriteLine($"  Mining Capacity Rating: {ship.ShipData.MiningCapacity:F0}");
        Console.WriteLine();
        Console.WriteLine("Ship Statistics:");
        Console.WriteLine($"  Total Blocks: {ship.ShipData.Structure.Blocks.Count}");
        Console.WriteLine($"  Total Mass: {ship.ShipData.TotalMass:F2} kg");
        Console.WriteLine($"  Total Thrust: {ship.ShipData.TotalThrust:F2} N");
        Console.WriteLine($"  Thrust/Mass Ratio: {ship.ShipData.Stats.GetValueOrDefault("ThrustToMass", 0f):F2}");
        Console.WriteLine($"  Power Generation: {ship.ShipData.TotalPowerGeneration:F2} W");
        Console.WriteLine($"  Position: ({ship.Position.X:F1}, {ship.Position.Y:F1}, {ship.Position.Z:F1})");
        
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
    public List<MiningShipDisplay> GetGeneratedShips()
    {
        return _generatedShips;
    }

    /// <summary>
    /// Interactive menu for exploring the showcase
    /// </summary>
    public void RunInteractiveMenu()
    {
        while (true)
        {
            Console.WriteLine("\n═══════════════════════════════════════════════════════════");
            Console.WriteLine("INDUSTRIAL MINING SHIP SHOWCASE - Options:");
            Console.WriteLine("═══════════════════════════════════════════════════════════");
            Console.WriteLine("  1  - View ship summary table");
            Console.WriteLine("  2  - View specific ship details (enter ship number)");
            Console.WriteLine("  3  - Launch 3D viewer with all ships");
            Console.WriteLine("  0  - Return to main menu");
            Console.WriteLine();
            Console.Write("Enter choice: ");
            
            string? input = Console.ReadLine();
            
            if (string.IsNullOrWhiteSpace(input))
                continue;
                
            if (input == "0")
            {
                Console.WriteLine("Returning to main menu...");
                break;
            }
            
            if (input == "1")
            {
                PrintShowcaseSummary();
            }
            else if (input == "2")
            {
                Console.Write("Enter ship number (1-{0}): ", _generatedShips.Count);
                string? shipNumInput = Console.ReadLine();
                if (int.TryParse(shipNumInput, out int shipNum))
                {
                    PrintShipDetails(shipNum);
                }
                else
                {
                    Console.WriteLine("Invalid ship number.");
                }
            }
            else if (input == "3")
            {
                Console.WriteLine("3D Viewer would launch here - ships are positioned in a grid.");
                Console.WriteLine("(Integration with GraphicsWindow needed)");
            }
        }
    }
}
