using System.Numerics;
using AvorionLike.Core.Voxel;
using AvorionLike.Core.Procedural;
using AvorionLike.Core.ECS;

namespace AvorionLike.Examples;

/// <summary>
/// Simple integration tests for the cohesive ship generation systems
/// </summary>
public class CohesiveShipGenerationTests
{
    private readonly EntityManager _entityManager;

    public CohesiveShipGenerationTests()
    {
        _entityManager = new EntityManager();
    }

    /// <summary>
    /// Run all tests and report results
    /// </summary>
    public void RunAllTests()
    {
        Console.WriteLine("╔════════════════════════════════════════════════════════════════╗");
        Console.WriteLine("║        COHESIVE SHIP GENERATION - INTEGRATION TESTS           ║");
        Console.WriteLine("╚════════════════════════════════════════════════════════════════╝\n");

        int passed = 0;
        int failed = 0;

        // Test 1: Structural Integrity
        if (TestStructuralIntegrity())
        {
            Console.WriteLine("✓ Test 1: Structural Integrity System - PASSED\n");
            passed++;
        }
        else
        {
            Console.WriteLine("✗ Test 1: Structural Integrity System - FAILED\n");
            failed++;
        }

        // Test 2: Functional Requirements
        if (TestFunctionalRequirements())
        {
            Console.WriteLine("✓ Test 2: Functional Requirements System - PASSED\n");
            passed++;
        }
        else
        {
            Console.WriteLine("✗ Test 2: Functional Requirements System - FAILED\n");
            failed++;
        }

        // Test 3: Aesthetic Guidelines
        if (TestAestheticGuidelines())
        {
            Console.WriteLine("✓ Test 3: Aesthetic Guidelines System - PASSED\n");
            passed++;
        }
        else
        {
            Console.WriteLine("✗ Test 3: Aesthetic Guidelines System - FAILED\n");
            failed++;
        }

        // Test 4: Complete Ship Generation
        if (TestCompleteShipGeneration())
        {
            Console.WriteLine("✓ Test 4: Complete Ship Generation - PASSED\n");
            passed++;
        }
        else
        {
            Console.WriteLine("✗ Test 4: Complete Ship Generation - FAILED\n");
            failed++;
        }

        // Test 5: Multiple Faction Ships
        if (TestMultipleFactionShips())
        {
            Console.WriteLine("✓ Test 5: Multiple Faction Ships - PASSED\n");
            passed++;
        }
        else
        {
            Console.WriteLine("✗ Test 5: Multiple Faction Ships - FAILED\n");
            failed++;
        }

        // Summary
        Console.WriteLine(new string('═', 64));
        Console.WriteLine($"Test Results: {passed} passed, {failed} failed");
        Console.WriteLine(new string('═', 64) + "\n");
    }

    /// <summary>
    /// Test structural integrity validation
    /// </summary>
    private bool TestStructuralIntegrity()
    {
        try
        {
            Console.WriteLine("Testing Structural Integrity System...");

            var structure = new VoxelStructureComponent();
            
            // Create a simple connected structure
            var coreBlock = new VoxelBlock(Vector3.Zero, new Vector3(3, 3, 3), "Iron", BlockType.HyperdriveCore);
            structure.AddBlock(coreBlock);

            // Add connected blocks
            for (int i = 1; i <= 5; i++)
            {
                var block = new VoxelBlock(new Vector3(i * 3, 0, 0), new Vector3(2, 2, 2), "Iron", BlockType.Hull);
                structure.AddBlock(block);
            }

            // Add engine at the end
            var engine = new VoxelBlock(new Vector3(18, 0, 0), new Vector3(3, 3, 3), "Iron", BlockType.Engine);
            structure.AddBlock(engine);

            // Validate
            var integritySystem = new StructuralIntegritySystem();
            var result = integritySystem.ValidateStructure(structure);

            Console.WriteLine($"  Blocks: {structure.Blocks.Count}");
            Console.WriteLine($"  Valid: {result.IsValid}");
            Console.WriteLine($"  Connected: {result.ConnectedBlocks.Count}");
            Console.WriteLine($"  Disconnected: {result.DisconnectedBlocks.Count}");

            // Should be valid - all blocks connected
            return result.IsValid && result.DisconnectedBlocks.Count == 0;
        }
        catch (Exception ex)
        {
            Console.WriteLine($"  Error: {ex.Message}");
            return false;
        }
    }

    /// <summary>
    /// Test functional requirements validation
    /// </summary>
    private bool TestFunctionalRequirements()
    {
        try
        {
            Console.WriteLine("Testing Functional Requirements System...");

            var structure = new VoxelStructureComponent();

            // Add core
            structure.AddBlock(new VoxelBlock(Vector3.Zero, new Vector3(3, 3, 3), "Iron", BlockType.HyperdriveCore));

            // Add engines
            structure.AddBlock(new VoxelBlock(new Vector3(0, 0, -10), new Vector3(3, 3, 3), "Iron", BlockType.Engine));
            structure.AddBlock(new VoxelBlock(new Vector3(3, 0, -10), new Vector3(3, 3, 3), "Iron", BlockType.Engine));

            // Add generators
            structure.AddBlock(new VoxelBlock(new Vector3(0, 0, 5), new Vector3(3, 3, 3), "Iron", BlockType.Generator));
            structure.AddBlock(new VoxelBlock(new Vector3(0, 3, 5), new Vector3(3, 3, 3), "Iron", BlockType.Generator));

            // Add thrusters
            structure.AddBlock(new VoxelBlock(new Vector3(0, 5, 0), new Vector3(2, 2, 2), "Iron", BlockType.Thruster));
            structure.AddBlock(new VoxelBlock(new Vector3(0, -5, 0), new Vector3(2, 2, 2), "Iron", BlockType.Thruster));
            structure.AddBlock(new VoxelBlock(new Vector3(5, 0, 0), new Vector3(2, 2, 2), "Iron", BlockType.Thruster));
            structure.AddBlock(new VoxelBlock(new Vector3(-5, 0, 0), new Vector3(2, 2, 2), "Iron", BlockType.Thruster));

            // Validate
            var requirementsSystem = new FunctionalRequirementsSystem();
            var result = requirementsSystem.ValidateRequirements(structure);

            Console.WriteLine($"  Valid: {result.IsValid}");
            Console.WriteLine($"  Engines: {result.EngineCount}");
            Console.WriteLine($"  Generators: {result.GeneratorCount}");
            Console.WriteLine($"  Thrusters: {result.ThrusterCount}");
            Console.WriteLine($"  Power Adequate: {result.HasAdequatePower}");

            // Should have minimum requirements
            return result.EngineCount >= 1 && result.GeneratorCount >= 1;
        }
        catch (Exception ex)
        {
            Console.WriteLine($"  Error: {ex.Message}");
            return false;
        }
    }

    /// <summary>
    /// Test aesthetic guidelines validation
    /// </summary>
    private bool TestAestheticGuidelines()
    {
        try
        {
            Console.WriteLine("Testing Aesthetic Guidelines System...");

            var structure = new VoxelStructureComponent();

            // Create a symmetric structure
            for (int x = -5; x <= 5; x += 2)
            {
                for (int y = -2; y <= 2; y += 2)
                {
                    for (int z = -10; z <= 10; z += 5)
                    {
                        var block = new VoxelBlock(
                            new Vector3(x, y, z),
                            new Vector3(2, 2, 2),
                            "Iron",
                            BlockType.Hull
                        );
                        structure.AddBlock(block);
                    }
                }
            }

            // Validate
            var aestheticsSystem = new AestheticGuidelinesSystem();
            var result = aestheticsSystem.ValidateAesthetics(structure);

            Console.WriteLine($"  Symmetry: {result.DetectedSymmetry} ({result.SymmetryScore:F2})");
            Console.WriteLine($"  Balance: {result.BalanceScore:F2}");
            Console.WriteLine($"  Proportions: {(result.HasReasonableProportions ? "Good" : "Poor")}");

            // Should detect symmetry
            return result.SymmetryScore > 0.5f;
        }
        catch (Exception ex)
        {
            Console.WriteLine($"  Error: {ex.Message}");
            return false;
        }
    }

    /// <summary>
    /// Test complete ship generation with all validation
    /// </summary>
    private bool TestCompleteShipGeneration()
    {
        try
        {
            Console.WriteLine("Testing Complete Ship Generation...");

            var generator = new ProceduralShipGenerator(42);
            var config = new ShipGenerationConfig
            {
                Size = ShipSize.Frigate,
                Role = ShipRole.Combat,
                Material = "Titanium",
                Style = FactionShipStyle.GetDefaultStyle("Military"),
                Seed = 42
            };

            var ship = generator.GenerateShip(config);

            Console.WriteLine($"  Blocks: {ship.Structure.Blocks.Count}");
            Console.WriteLine($"  Warnings: {ship.Warnings.Count}");
            Console.WriteLine($"  Mass: {ship.TotalMass:F0} kg");
            Console.WriteLine($"  Thrust: {ship.TotalThrust:F0} N");

            // Should generate a valid ship
            return ship.Structure.Blocks.Count > 0 && 
                   ship.TotalThrust > 0 && 
                   ship.TotalPowerGeneration > 0;
        }
        catch (Exception ex)
        {
            Console.WriteLine($"  Error: {ex.Message}");
            return false;
        }
    }

    /// <summary>
    /// Test generation of ships from multiple factions
    /// </summary>
    private bool TestMultipleFactionShips()
    {
        try
        {
            Console.WriteLine("Testing Multiple Faction Ships...");

            var generator = new ProceduralShipGenerator(123);
            var factions = new[] { "Military", "Traders", "Pirates" };
            int successCount = 0;

            foreach (var faction in factions)
            {
                var config = new ShipGenerationConfig
                {
                    Size = ShipSize.Corvette,
                    Role = ShipRole.Multipurpose,
                    Material = "Iron",
                    Style = FactionShipStyle.GetDefaultStyle(faction),
                    Seed = faction.GetHashCode()
                };

                var ship = generator.GenerateShip(config);

                Console.WriteLine($"  {faction}: {ship.Structure.Blocks.Count} blocks, {ship.Warnings.Count} warnings");

                if (ship.Structure.Blocks.Count > 0)
                    successCount++;
            }

            // All factions should generate valid ships
            return successCount == factions.Length;
        }
        catch (Exception ex)
        {
            Console.WriteLine($"  Error: {ex.Message}");
            return false;
        }
    }
}
