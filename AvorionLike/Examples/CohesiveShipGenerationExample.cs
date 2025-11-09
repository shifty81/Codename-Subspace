using System.Numerics;
using AvorionLike.Core.Procedural;
using AvorionLike.Core.Voxel;
using AvorionLike.Core.ECS;

namespace AvorionLike.Examples;

/// <summary>
/// Example demonstrating cohesive voxel ship generation with structural integrity,
/// functional requirements, and aesthetic guidelines.
/// </summary>
public class CohesiveShipGenerationExample
{
    private readonly EntityManager _entityManager;
    private readonly ProceduralShipGenerator _shipGenerator;
    private readonly StructuralIntegritySystem _integritySystem;
    private readonly FunctionalRequirementsSystem _requirementsSystem;
    private readonly AestheticGuidelinesSystem _aestheticsSystem;

    public CohesiveShipGenerationExample(EntityManager entityManager, int seed = 42)
    {
        _entityManager = entityManager;
        _shipGenerator = new ProceduralShipGenerator(seed);
        _integritySystem = new StructuralIntegritySystem();
        _requirementsSystem = new FunctionalRequirementsSystem();
        _aestheticsSystem = new AestheticGuidelinesSystem();
    }

    /// <summary>
    /// Run comprehensive demonstration of cohesive ship generation
    /// </summary>
    public void RunDemo()
    {
        Console.WriteLine("╔════════════════════════════════════════════════════════════════╗");
        Console.WriteLine("║     COHESIVE VOXEL SHIP GENERATION DEMONSTRATION              ║");
        Console.WriteLine("╚════════════════════════════════════════════════════════════════╝\n");

        // Demo 1: Generate and validate a combat frigate
        Console.WriteLine("═══ Demo 1: Combat Frigate with Full Validation ═══\n");
        DemonstrateCombatFrigate();

        Console.WriteLine("\n" + new string('─', 64) + "\n");

        // Demo 2: Show structural integrity validation
        Console.WriteLine("═══ Demo 2: Structural Integrity Analysis ═══\n");
        DemonstrateStructuralIntegrity();

        Console.WriteLine("\n" + new string('─', 64) + "\n");

        // Demo 3: Show functional requirements validation
        Console.WriteLine("═══ Demo 3: Functional Requirements Validation ═══\n");
        DemonstrateFunctionalRequirements();

        Console.WriteLine("\n" + new string('─', 64) + "\n");

        // Demo 4: Show aesthetic guidelines validation
        Console.WriteLine("═══ Demo 4: Aesthetic Guidelines Analysis ═══\n");
        DemonstrateAestheticGuidelines();

        Console.WriteLine("\n" + new string('─', 64) + "\n");

        // Demo 5: Compare different faction styles
        Console.WriteLine("═══ Demo 5: Faction Style Comparison ═══\n");
        CompareFactionStyles();

        Console.WriteLine("\n╔════════════════════════════════════════════════════════════════╗");
        Console.WriteLine("║                    DEMONSTRATION COMPLETE                      ║");
        Console.WriteLine("╚════════════════════════════════════════════════════════════════╝");
    }

    /// <summary>
    /// Demonstrate combat frigate generation with all validation systems
    /// </summary>
    private void DemonstrateCombatFrigate()
    {
        // Configure ship generation
        var config = new ShipGenerationConfig
        {
            Size = ShipSize.Frigate,
            Role = ShipRole.Combat,
            Material = "Titanium",
            Style = FactionShipStyle.GetDefaultStyle("Military"),
            Seed = 12345,
            RequireHyperdrive = true,
            RequireCargo = true,
            MinimumWeaponMounts = 4
        };

        Console.WriteLine($"Generating {config.Size} {config.Role} ship...");
        Console.WriteLine($"Faction: {config.Style.FactionName}");
        Console.WriteLine($"Material: {config.Material}\n");

        // Generate ship
        var ship = _shipGenerator.GenerateShip(config);

        // Print comprehensive stats
        PrintShipStatistics(ship);
        PrintValidationResults(ship);
    }

    /// <summary>
    /// Demonstrate structural integrity validation
    /// </summary>
    private void DemonstrateStructuralIntegrity()
    {
        // Generate a test ship
        var config = new ShipGenerationConfig
        {
            Size = ShipSize.Corvette,
            Role = ShipRole.Multipurpose,
            Material = "Iron",
            Style = FactionShipStyle.GetDefaultStyle("Default")
        };

        var ship = _shipGenerator.GenerateShip(config);

        Console.WriteLine("Analyzing structural integrity...\n");

        // Validate structure
        var result = _integritySystem.ValidateStructure(ship.Structure);

        Console.WriteLine($"Structure Valid: {(result.IsValid ? "✓ YES" : "✗ NO")}");
        Console.WriteLine($"Total Blocks: {ship.Structure.Blocks.Count}");
        Console.WriteLine($"Connected Blocks: {result.ConnectedBlocks.Count}");
        Console.WriteLine($"Disconnected Blocks: {result.DisconnectedBlocks.Count}");
        Console.WriteLine($"Core Block: {result.CoreBlockId?.ToString().Substring(0, 8) ?? "None"}");
        Console.WriteLine($"Max Distance from Core: {result.MaxDistanceFromCore}");

        if (result.BlockDistancesFromCore.Count > 0)
        {
            var avgDistance = result.BlockDistancesFromCore.Values.Average();
            var maxDistance = result.BlockDistancesFromCore.Values.Max();
            Console.WriteLine($"Average Block Distance: {avgDistance:F1}");
            Console.WriteLine($"Farthest Block Distance: {maxDistance}");
        }

        Console.WriteLine($"\nIntegrity Percentage: {_integritySystem.CalculateStructuralIntegrityPercentage(ship.Structure, result):F1}%");

        // Show errors and warnings
        if (result.Errors.Count > 0)
        {
            Console.WriteLine("\nStructural Errors:");
            foreach (var error in result.Errors)
            {
                Console.WriteLine($"  ✗ {error}");
            }
        }

        if (result.Warnings.Count > 0)
        {
            Console.WriteLine("\nStructural Warnings:");
            foreach (var warning in result.Warnings)
            {
                Console.WriteLine($"  ⚠ {warning}");
            }
        }

        // Suggest connecting blocks if needed
        if (result.DisconnectedBlocks.Count > 0)
        {
            Console.WriteLine("\nAttempting to repair disconnected blocks...");
            var suggestions = _integritySystem.SuggestConnectingBlocks(ship.Structure, result);
            Console.WriteLine($"Suggested {suggestions.Count} connecting blocks");
        }
    }

    /// <summary>
    /// Demonstrate functional requirements validation
    /// </summary>
    private void DemonstrateFunctionalRequirements()
    {
        // Generate a mining ship
        var config = new ShipGenerationConfig
        {
            Size = ShipSize.Frigate,
            Role = ShipRole.Mining,
            Material = "Naonite",
            Style = FactionShipStyle.GetDefaultStyle("Miners")
        };

        var ship = _shipGenerator.GenerateShip(config);

        Console.WriteLine($"Analyzing functional requirements for {config.Role} ship...\n");

        // Validate requirements
        var result = _requirementsSystem.ValidateRequirements(ship.Structure);

        Console.WriteLine($"Requirements Met: {(result.IsValid ? "✓ YES" : "✗ NO")}\n");

        Console.WriteLine("Component Inventory:");
        Console.WriteLine($"  Engines: {result.EngineCount}");
        Console.WriteLine($"  Generators: {result.GeneratorCount}");
        Console.WriteLine($"  Thrusters: {result.ThrusterCount}");
        Console.WriteLine($"  Shield Generators: {result.ShieldGeneratorCount}");
        Console.WriteLine($"  Gyro Arrays: {result.GyroCount}");
        Console.WriteLine($"  Core Systems: {result.CoreSystemCount}");

        Console.WriteLine("\nConnectivity Status:");
        Console.WriteLine($"  Engines → Power: {(result.EnginesConnectedToPower ? "✓" : "✗")}");
        Console.WriteLine($"  Thrusters → Power: {(result.ThrustersConnectedToPower ? "✓" : "✗")}");
        Console.WriteLine($"  Shields → Power: {(result.ShieldsConnectedToPower ? "✓" : "✗")}");

        Console.WriteLine("\nPositioning Validation:");
        Console.WriteLine($"  Engines at Rear: {(result.EnginesAtRear ? "✓" : "✗")}");
        Console.WriteLine($"  Thrusters Distributed: {(result.ThrustersDistributed ? "✓" : "✗")}");
        Console.WriteLine($"  Generators Internal: {(result.GeneratorsInternal ? "✓" : "✗")}");

        Console.WriteLine("\nPower Analysis:");
        Console.WriteLine($"  Power Generation: {result.TotalPowerGeneration:F0} W");
        Console.WriteLine($"  Power Consumption: {result.TotalPowerConsumption:F0} W");
        Console.WriteLine($"  Power Margin: {(result.TotalPowerConsumption > 0 ? (result.TotalPowerGeneration / result.TotalPowerConsumption) : 0):F2}x");
        Console.WriteLine($"  Adequate Power: {(result.HasAdequatePower ? "✓" : "✗")}");

        // Get suggestions
        var suggestions = _requirementsSystem.GetComponentSuggestions(result);
        if (suggestions.Count > 0)
        {
            Console.WriteLine("\nImprovement Suggestions:");
            foreach (var suggestion in suggestions)
            {
                Console.WriteLine($"  → {suggestion}");
            }
        }
    }

    /// <summary>
    /// Demonstrate aesthetic guidelines validation
    /// </summary>
    private void DemonstrateAestheticGuidelines()
    {
        // Generate an exploration ship with high symmetry style
        var config = new ShipGenerationConfig
        {
            Size = ShipSize.Destroyer,
            Role = ShipRole.Exploration,
            Material = "Trinium",
            Style = FactionShipStyle.GetDefaultStyle("Explorers")
        };

        var ship = _shipGenerator.GenerateShip(config);

        Console.WriteLine($"Analyzing aesthetic guidelines for {config.Style.FactionName} ship...\n");

        // Validate aesthetics
        var result = _aestheticsSystem.ValidateAesthetics(ship.Structure, config.Style);

        Console.WriteLine($"Meets Guidelines: {(result.MeetsGuidelines ? "✓ YES" : "✗ NO")}\n");

        Console.WriteLine("Symmetry Analysis:");
        Console.WriteLine($"  Type: {result.DetectedSymmetry}");
        Console.WriteLine($"  Score: {result.SymmetryScore:F2} / 1.00");
        Console.WriteLine($"  Target (Faction): {config.Style.SymmetryLevel:F2}");

        Console.WriteLine("\nBalance Analysis:");
        Console.WriteLine($"  Center of Mass: ({result.CenterOfMass.X:F1}, {result.CenterOfMass.Y:F1}, {result.CenterOfMass.Z:F1})");
        Console.WriteLine($"  Geometric Center: ({result.GeometricCenter.X:F1}, {result.GeometricCenter.Y:F1}, {result.GeometricCenter.Z:F1})");
        Console.WriteLine($"  Balance Score: {result.BalanceScore:F2} / 1.00");

        Console.WriteLine("\nProportions:");
        Console.WriteLine($"  Dimensions: {result.Dimensions.X:F1} × {result.Dimensions.Y:F1} × {result.Dimensions.Z:F1}");
        Console.WriteLine($"  Width/Height Ratio: {result.AspectRatioXY:F2}");
        Console.WriteLine($"  Height/Length Ratio: {result.AspectRatioYZ:F2}");
        Console.WriteLine($"  Width/Length Ratio: {result.AspectRatioXZ:F2}");
        Console.WriteLine($"  Reasonable Proportions: {(result.HasReasonableProportions ? "✓" : "✗")}");

        Console.WriteLine("\nDesign Language:");
        Console.WriteLine($"  Consistent: {(result.HasConsistentDesignLanguage ? "✓" : "✗")}");
        Console.WriteLine($"  Functional Color Variety: {result.FunctionalColorVariety}");
        Console.WriteLine($"  Colors by Block Type:");
        foreach (var kvp in result.ColorsByType.Take(5))
        {
            Console.WriteLine($"    {kvp.Key}: #{kvp.Value:X6}");
        }

        // Show suggestions
        if (result.Suggestions.Count > 0)
        {
            Console.WriteLine("\nAesthetic Suggestions:");
            foreach (var suggestion in result.Suggestions)
            {
                Console.WriteLine($"  → {suggestion}");
            }
        }
    }

    /// <summary>
    /// Compare different faction styles
    /// </summary>
    private void CompareFactionStyles()
    {
        var factions = new[] { "Military", "Traders", "Pirates", "Explorers", "Miners" };

        Console.WriteLine("Generating and comparing ships from different factions...\n");

        foreach (var faction in factions)
        {
            var config = new ShipGenerationConfig
            {
                Size = ShipSize.Frigate,
                Role = ShipRole.Multipurpose,
                Material = "Titanium",
                Style = FactionShipStyle.GetDefaultStyle(faction),
                Seed = faction.GetHashCode()
            };

            var ship = _shipGenerator.GenerateShip(config);

            // Quick validation
            var integrity = _integritySystem.ValidateStructure(ship.Structure);
            var requirements = _requirementsSystem.ValidateRequirements(ship.Structure);
            var aesthetics = _aestheticsSystem.ValidateAesthetics(ship.Structure, config.Style);

            Console.WriteLine($"{faction} Faction:");
            Console.WriteLine($"  Hull Shape: {config.Style.PreferredHullShape}");
            Console.WriteLine($"  Blocks: {ship.Structure.Blocks.Count}");
            Console.WriteLine($"  Structural Integrity: {(integrity.IsValid ? "✓" : "✗")} ({_integritySystem.CalculateStructuralIntegrityPercentage(ship.Structure, integrity):F0}%)");
            Console.WriteLine($"  Functional Requirements: {(requirements.IsValid ? "✓" : "✗")}");
            Console.WriteLine($"  Symmetry: {aesthetics.DetectedSymmetry} ({aesthetics.SymmetryScore:F2})");
            Console.WriteLine($"  Balance: {aesthetics.BalanceScore:F2}");
            Console.WriteLine($"  Power Margin: {(requirements.TotalPowerConsumption > 0 ? requirements.TotalPowerGeneration / requirements.TotalPowerConsumption : 0):F2}x");
            Console.WriteLine($"  Weapons: {ship.WeaponMountCount}");
            Console.WriteLine();
        }
    }

    /// <summary>
    /// Print comprehensive ship statistics
    /// </summary>
    private void PrintShipStatistics(GeneratedShip ship)
    {
        Console.WriteLine("Ship Statistics:");
        Console.WriteLine($"  Total Blocks: {ship.Structure.Blocks.Count}");
        Console.WriteLine($"  Mass: {ship.TotalMass:F0} kg");
        Console.WriteLine($"  Thrust: {ship.TotalThrust:F0} N");
        Console.WriteLine($"  Power Generation: {ship.TotalPowerGeneration:F0} W");
        Console.WriteLine($"  Shield Capacity: {ship.TotalShieldCapacity:F0}");
        Console.WriteLine($"  Weapon Mounts: {ship.WeaponMountCount}");
        Console.WriteLine($"  Cargo Blocks: {ship.CargoBlockCount}");
        Console.WriteLine($"  Thrust/Mass Ratio: {(ship.TotalMass > 0 ? ship.TotalThrust / ship.TotalMass : 0):F2}");
        Console.WriteLine();
    }

    /// <summary>
    /// Print validation results from ship stats
    /// </summary>
    private void PrintValidationResults(GeneratedShip ship)
    {
        Console.WriteLine("Validation Results:");

        if (ship.Stats.ContainsKey("StructuralIntegrity"))
        {
            Console.WriteLine($"  Structural Integrity: {ship.Stats["StructuralIntegrity"]:F1}%");
        }

        if (ship.Stats.ContainsKey("PowerMargin"))
        {
            Console.WriteLine($"  Power Margin: {ship.Stats["PowerMargin"]:F2}x");
        }

        if (ship.Stats.ContainsKey("Symmetry"))
        {
            Console.WriteLine($"  Symmetry Score: {ship.Stats["Symmetry"]:F2}");
        }

        if (ship.Stats.ContainsKey("Balance"))
        {
            Console.WriteLine($"  Balance Score: {ship.Stats["Balance"]:F2}");
        }

        if (ship.Stats.ContainsKey("DesignLanguage"))
        {
            Console.WriteLine($"  Design Language: {(ship.Stats["DesignLanguage"] > 0.5f ? "Consistent" : "Inconsistent")}");
        }

        // Show warnings
        if (ship.Warnings.Count > 0)
        {
            Console.WriteLine("\nWarnings & Suggestions:");
            foreach (var warning in ship.Warnings.Take(10))
            {
                Console.WriteLine($"  ⚠ {warning}");
            }

            if (ship.Warnings.Count > 10)
            {
                Console.WriteLine($"  ... and {ship.Warnings.Count - 10} more");
            }
        }
    }
}
