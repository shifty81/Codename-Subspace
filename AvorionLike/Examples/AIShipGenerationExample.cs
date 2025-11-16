using System.Numerics;
using AvorionLike.Core.Voxel;
using AvorionLike.Core.Logging;

namespace AvorionLike.Examples;

/// <summary>
/// Comprehensive demonstration of AI-driven voxel-based ship construction
/// Shows block definitions, ship aggregation, and AI generation
/// </summary>
public class AIShipGenerationExample
{
    private readonly Logger _logger = Logger.Instance;
    
    public void Run()
    {
        Console.WriteLine("=== AI-Driven Ship Generation Demo ===\n");
        
        // Part 1: Demonstrate block definition system
        DemonstrateBlockDefinitions();
        
        // Part 2: Demonstrate ship aggregate calculation
        DemonstrateShipAggregate();
        
        // Part 3: Demonstrate AI ship generation for different goals
        DemonstrateAIGeneration();
        
        Console.WriteLine("\n=== Demo Complete ===");
    }
    
    /// <summary>
    /// Part 1: Show block definition system with JSON export
    /// </summary>
    private void DemonstrateBlockDefinitions()
    {
        Console.WriteLine("\n--- Part 1: Block Definition System ---\n");
        
        // Get all block definitions
        var definitions = BlockDefinitionDatabase.GetDefinitions();
        
        Console.WriteLine($"Total block types defined: {definitions.Count}\n");
        
        // Show details for a few key block types
        var keyTypes = new[] { BlockType.Hull, BlockType.Armor, BlockType.Engine, BlockType.Generator };
        
        foreach (var blockType in keyTypes)
        {
            var def = definitions[blockType];
            Console.WriteLine($"{def.DisplayName} ({def.BlockType}):");
            Console.WriteLine($"  Description: {def.Description}");
            Console.WriteLine($"  Function: {def.Function}");
            Console.WriteLine($"  HP per volume: {def.HitPointsPerVolume}");
            Console.WriteLine($"  Mass per volume: {def.MassPerUnitVolume}");
            Console.WriteLine($"  Scalable: {def.Scalable}");
            
            if (def.ResourceCosts.Any())
            {
                Console.Write("  Resource costs: ");
                Console.WriteLine(string.Join(", ", def.ResourceCosts.Select(kv => $"{kv.Key}: {kv.Value}")));
            }
            
            if (def.PowerGenerationPerVolume > 0)
                Console.WriteLine($"  Power generation: {def.PowerGenerationPerVolume}/volume");
            if (def.PowerConsumptionPerVolume > 0)
                Console.WriteLine($"  Power consumption: {def.PowerConsumptionPerVolume}/volume");
            if (def.ThrustPowerPerVolume > 0)
                Console.WriteLine($"  Thrust power: {def.ThrustPowerPerVolume}/volume");
            
            Console.WriteLine($"  AI Priority: {def.AiPlacementPriority}");
            Console.WriteLine($"  Internal placement required: {def.RequiresInternalPlacement}");
            Console.WriteLine();
        }
        
        // Export to JSON
        string jsonPath = "block_definitions.json";
        try
        {
            BlockDefinitionDatabase.ExportToJson(jsonPath);
            Console.WriteLine($"✓ Block definitions exported to {jsonPath}");
            
            // Show a snippet of the JSON
            var jsonContent = File.ReadAllText(jsonPath);
            var snippet = jsonContent.Substring(0, Math.Min(500, jsonContent.Length));
            Console.WriteLine($"\nJSON snippet:\n{snippet}...\n");
        }
        catch (Exception ex)
        {
            Console.WriteLine($"✗ Error exporting JSON: {ex.Message}");
        }
    }
    
    /// <summary>
    /// Part 2: Demonstrate ship aggregate calculation
    /// </summary>
    private void DemonstrateShipAggregate()
    {
        Console.WriteLine("\n--- Part 2: Ship Aggregate System ---\n");
        
        // Create a simple ship structure
        var structure = new VoxelStructureComponent();
        
        // Add various blocks to demonstrate aggregate calculation
        Console.WriteLine("Building test ship with various block types...\n");
        
        // Hull structure (10 blocks)
        for (int i = 0; i < 10; i++)
        {
            structure.AddBlock(new VoxelBlock(
                new Vector3(i * 2, 0, 0),
                new Vector3(2, 2, 2),
                "Iron",
                BlockType.Hull
            ));
        }
        
        // Add engines (2 blocks at rear)
        structure.AddBlock(new VoxelBlock(new Vector3(0, 0, -5), new Vector3(2, 2, 2), "Iron", BlockType.Engine));
        structure.AddBlock(new VoxelBlock(new Vector3(2, 0, -5), new Vector3(2, 2, 2), "Iron", BlockType.Engine));
        
        // Add generator (1 block, internal)
        structure.AddBlock(new VoxelBlock(new Vector3(8, 0, 0), new Vector3(2, 2, 2), "Titanium", BlockType.Generator));
        
        // Add thrusters (2 blocks)
        structure.AddBlock(new VoxelBlock(new Vector3(4, 2, 0), new Vector3(2, 2, 2), "Iron", BlockType.Thruster));
        structure.AddBlock(new VoxelBlock(new Vector3(4, -2, 0), new Vector3(2, 2, 2), "Iron", BlockType.Thruster));
        
        // Add gyro array
        structure.AddBlock(new VoxelBlock(new Vector3(10, 0, 0), new Vector3(2, 2, 2), "Iron", BlockType.GyroArray));
        
        // Add shields
        structure.AddBlock(new VoxelBlock(new Vector3(12, 0, 0), new Vector3(2, 2, 2), "Naonite", BlockType.ShieldGenerator));
        
        // Add cargo bay
        structure.AddBlock(new VoxelBlock(new Vector3(14, 0, 0), new Vector3(4, 2, 2), "Iron", BlockType.Cargo));
        
        // Add crew quarters
        structure.AddBlock(new VoxelBlock(new Vector3(16, 0, 0), new Vector3(2, 2, 2), "Iron", BlockType.CrewQuarters));
        
        // Add weapons
        structure.AddBlock(new VoxelBlock(new Vector3(6, 2, 2), new Vector3(2, 2, 2), "Iron", BlockType.TurretMount));
        structure.AddBlock(new VoxelBlock(new Vector3(6, -2, 2), new Vector3(2, 2, 2), "Iron", BlockType.TurretMount));
        
        // Add armor
        structure.AddBlock(new VoxelBlock(new Vector3(18, 0, 0), new Vector3(2, 2, 2), "Titanium", BlockType.Armor));
        
        // Add hyperdrive
        structure.AddBlock(new VoxelBlock(new Vector3(8, 0, 2), new Vector3(2, 2, 2), "Xanion", BlockType.HyperdriveCore));
        
        Console.WriteLine($"Created ship with {structure.Blocks.Count} blocks\n");
        
        // Create aggregate and calculate
        var aggregate = new ShipAggregate(structure);
        
        // Display comprehensive statistics
        Console.WriteLine(aggregate.GetStatsSummary());
        
        // Validate requirements
        Console.WriteLine("\n=== Validation ===");
        var warnings = aggregate.ValidateRequirements();
        if (warnings.Any())
        {
            Console.WriteLine("Warnings:");
            foreach (var warning in warnings)
            {
                Console.WriteLine($"  ⚠ {warning}");
            }
        }
        else
        {
            Console.WriteLine("✓ All requirements met!");
        }
    }
    
    /// <summary>
    /// Part 3: Demonstrate AI ship generation for different design goals
    /// </summary>
    private void DemonstrateAIGeneration()
    {
        Console.WriteLine("\n--- Part 3: AI-Driven Ship Generation ---\n");
        
        var generator = new AIShipGenerator(12345);
        
        // Test different ship design goals
        var goals = new[]
        {
            ShipDesignGoal.Scout,
            ShipDesignGoal.CargoHauler,
            ShipDesignGoal.Battleship,
            ShipDesignGoal.Frigate
        };
        
        foreach (var goal in goals)
        {
            Console.WriteLine($"\n=== Generating {goal} ===\n");
            
            var parameters = new AIShipGenerationParameters
            {
                Goal = goal,
                Material = "Iron",
                TargetBlockCount = goal == ShipDesignGoal.Battleship ? 150 : 100,
                Seed = 12345 + (int)goal,
                MinWeaponMounts = goal == ShipDesignGoal.Battleship ? 6 : 2,
                RequireHyperdrive = true,
                RequireShields = goal != ShipDesignGoal.Scout,
                MinCrewCapacity = 5,
                AvoidSimpleBoxes = true,
                DesiredAspectRatio = goal == ShipDesignGoal.Scout ? 3.0f : 2.0f,
                UseAngularDesign = true
            };
            
            try
            {
                var result = generator.GenerateShip(parameters);
                
                Console.WriteLine($"Generated {goal} ship:");
                Console.WriteLine($"  Total blocks: {result.Structure.Blocks.Count}");
                Console.WriteLine($"  Design quality: {result.DesignQuality:F1}/100");
                Console.WriteLine();
                
                // Show aggregate stats
                Console.WriteLine(result.Aggregate.GetStatsSummary());
                
                // Show design decisions
                Console.WriteLine("\n--- Design Decisions ---");
                foreach (var decision in result.DesignDecisions.Take(5))
                {
                    Console.WriteLine($"  • {decision}");
                }
                if (result.DesignDecisions.Count > 5)
                {
                    Console.WriteLine($"  ... and {result.DesignDecisions.Count - 5} more");
                }
                
                // Show warnings
                if (result.Warnings.Any())
                {
                    Console.WriteLine("\n--- Warnings ---");
                    foreach (var warning in result.Warnings)
                    {
                        Console.WriteLine($"  ⚠ {warning}");
                    }
                }
                else
                {
                    Console.WriteLine("\n✓ No warnings - ship is fully functional!");
                }
                
                // Performance analysis based on goal
                Console.WriteLine("\n--- Goal Achievement Analysis ---");
                AnalyzeGoalAchievement(result);
                
            }
            catch (Exception ex)
            {
                Console.WriteLine($"✗ Error generating {goal}: {ex.Message}");
                _logger.Error("AIShipGenerationExample", $"Failed to generate {goal}", ex);
            }
            
            Console.WriteLine("\n" + new string('-', 80));
        }
    }
    
    /// <summary>
    /// Analyze how well the ship meets its design goal
    /// </summary>
    private void AnalyzeGoalAchievement(AIGeneratedShip ship)
    {
        var aggregate = ship.Aggregate;
        var goal = ship.Parameters.Goal;
        
        switch (goal)
        {
            case ShipDesignGoal.CargoHauler:
                Console.WriteLine($"  Cargo capacity: {aggregate.TotalCargoCapacity:F0} m³");
                Console.WriteLine($"  Cargo efficiency: {aggregate.CargoEfficiencyRating:F0}/100");
                if (aggregate.CargoEfficiencyRating > 60)
                    Console.WriteLine("  ✓ Excellent cargo capacity for hauling");
                else if (aggregate.CargoEfficiencyRating > 40)
                    Console.WriteLine("  ◐ Good cargo capacity");
                else
                    Console.WriteLine("  ✗ Insufficient cargo capacity for hauling");
                break;
                
            case ShipDesignGoal.Battleship:
                Console.WriteLine($"  Weapon mounts: {aggregate.WeaponMountCount}");
                Console.WriteLine($"  Combat effectiveness: {aggregate.CombatEffectivenessRating:F0}/100");
                Console.WriteLine($"  Shield capacity: {aggregate.TotalShieldCapacity:F0}");
                Console.WriteLine($"  Armor points: {aggregate.TotalArmorPoints:F0}");
                if (aggregate.CombatEffectivenessRating > 60 && aggregate.WeaponMountCount >= 6)
                    Console.WriteLine("  ✓ Formidable battleship");
                else if (aggregate.CombatEffectivenessRating > 40)
                    Console.WriteLine("  ◐ Capable combat vessel");
                else
                    Console.WriteLine("  ✗ Weak for a battleship");
                break;
                
            case ShipDesignGoal.Scout:
                Console.WriteLine($"  Max speed: {aggregate.MaxSpeed:F1} m/s");
                Console.WriteLine($"  Maneuverability: {aggregate.ManeuverabilityRating:F0}/100");
                Console.WriteLine($"  Has hyperdrive: {aggregate.HasHyperdrive}");
                if (aggregate.ManeuverabilityRating > 60 && aggregate.MaxSpeed > 50)
                    Console.WriteLine("  ✓ Fast and agile scout");
                else if (aggregate.ManeuverabilityRating > 40)
                    Console.WriteLine("  ◐ Decent scout capabilities");
                else
                    Console.WriteLine("  ✗ Too slow for scouting");
                break;
                
            default:
                Console.WriteLine($"  Balanced design:");
                Console.WriteLine($"  - Maneuverability: {aggregate.ManeuverabilityRating:F0}/100");
                Console.WriteLine($"  - Combat: {aggregate.CombatEffectivenessRating:F0}/100");
                Console.WriteLine($"  - Cargo: {aggregate.CargoEfficiencyRating:F0}/100");
                break;
        }
    }
    
    /// <summary>
    /// Run a comprehensive test of all systems
    /// </summary>
    public static void RunComprehensiveTest()
    {
        var example = new AIShipGenerationExample();
        example.Run();
    }
}
