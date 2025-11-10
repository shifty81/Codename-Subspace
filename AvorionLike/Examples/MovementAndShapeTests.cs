using System.Numerics;
using AvorionLike.Core.Voxel;
using AvorionLike.Core.Procedural;
using AvorionLike.Core.ECS;
using AvorionLike.Core.Physics;

namespace AvorionLike.Examples;

/// <summary>
/// Tests for physics interpolation and improved ship generation
/// </summary>
public class MovementAndShapeTests
{
    private readonly EntityManager _entityManager;
    private readonly PhysicsSystem _physicsSystem;

    public MovementAndShapeTests()
    {
        _entityManager = new EntityManager();
        _physicsSystem = new PhysicsSystem(_entityManager);
    }

    /// <summary>
    /// Run all tests and report results
    /// </summary>
    public void RunAllTests()
    {
        Console.WriteLine("╔════════════════════════════════════════════════════════════════╗");
        Console.WriteLine("║        MOVEMENT & SHIP SHAPE - INTEGRATION TESTS              ║");
        Console.WriteLine("╚════════════════════════════════════════════════════════════════╝\n");

        int passed = 0;
        int failed = 0;

        // Test 1: Physics Interpolation
        if (TestPhysicsInterpolation())
        {
            Console.WriteLine("✓ Test 1: Physics Interpolation - PASSED\n");
            passed++;
        }
        else
        {
            Console.WriteLine("✗ Test 1: Physics Interpolation - FAILED\n");
            failed++;
        }

        // Test 2: Ship Shape Variety
        if (TestShipShapeVariety())
        {
            Console.WriteLine("✓ Test 2: Ship Shape Variety - PASSED\n");
            passed++;
        }
        else
        {
            Console.WriteLine("✗ Test 2: Ship Shape Variety - FAILED\n");
            failed++;
        }

        // Test 3: Ship Not Plain Squares
        if (TestShipNotPlainSquares())
        {
            Console.WriteLine("✓ Test 3: Ships Not Plain Squares - PASSED\n");
            passed++;
        }
        else
        {
            Console.WriteLine("✗ Test 3: Ships Not Plain Squares - FAILED\n");
            failed++;
        }

        // Test 4: Smooth Movement
        if (TestSmoothMovement())
        {
            Console.WriteLine("✓ Test 4: Smooth Movement - PASSED\n");
            passed++;
        }
        else
        {
            Console.WriteLine("✗ Test 4: Smooth Movement - FAILED\n");
            failed++;
        }

        // Summary
        Console.WriteLine(new string('═', 64));
        Console.WriteLine($"Test Results: {passed} passed, {failed} failed");
        Console.WriteLine(new string('═', 64) + "\n");
    }

    /// <summary>
    /// Test that physics interpolation produces smooth intermediate values
    /// </summary>
    private bool TestPhysicsInterpolation()
    {
        try
        {
            Console.WriteLine("Testing physics interpolation...");

            // Create entity with physics
            var entity = _entityManager.CreateEntity("Test Ship");
            var physics = new PhysicsComponent
            {
                Position = new Vector3(0, 0, 0),
                PreviousPosition = new Vector3(0, 0, 0),
                Velocity = new Vector3(10, 0, 0),
                Mass = 1000f
            };
            _entityManager.AddComponent(entity.Id, physics);

            // Simulate one physics step
            _physicsSystem.Update(0.1f);

            // Check that position changed
            if (physics.Position.X <= 0.5f)
            {
                Console.WriteLine($"  ✗ Position didn't change enough: {physics.Position.X}");
                return false;
            }

            // Test interpolation at 50%
            _physicsSystem.InterpolatePhysics(0.5f);

            // Interpolated position should be between previous and current
            float expectedX = (physics.PreviousPosition.X + physics.Position.X) * 0.5f;
            float tolerance = 0.1f;

            if (Math.Abs(physics.InterpolatedPosition.X - expectedX) > tolerance)
            {
                Console.WriteLine($"  ✗ Interpolation incorrect. Expected ~{expectedX}, got {physics.InterpolatedPosition.X}");
                return false;
            }

            Console.WriteLine($"  ✓ Interpolation working: prev={physics.PreviousPosition.X:F2}, " +
                            $"current={physics.Position.X:F2}, interpolated={physics.InterpolatedPosition.X:F2}");
            return true;
        }
        catch (Exception ex)
        {
            Console.WriteLine($"  ✗ Exception: {ex.Message}");
            return false;
        }
    }

    /// <summary>
    /// Test that different ship types generate varied shapes
    /// Verify block counts are in viable range (50-150 blocks)
    /// </summary>
    private bool TestShipShapeVariety()
    {
        try
        {
            Console.WriteLine("Testing ship shape variety...");

            var generator = new ProceduralShipGenerator(12345);

            // Generate different hull types
            var blockyConfig = new ShipGenerationConfig
            {
                Size = ShipSize.Frigate,
                Style = FactionShipStyle.GetDefaultStyle("Default")
            };
            blockyConfig.Style.PreferredHullShape = ShipHullShape.Blocky;

            var angularConfig = new ShipGenerationConfig
            {
                Size = ShipSize.Frigate,
                Style = FactionShipStyle.GetDefaultStyle("Default")
            };
            angularConfig.Style.PreferredHullShape = ShipHullShape.Angular;

            var sleekConfig = new ShipGenerationConfig
            {
                Size = ShipSize.Frigate,
                Style = FactionShipStyle.GetDefaultStyle("Default")
            };
            sleekConfig.Style.PreferredHullShape = ShipHullShape.Sleek;

            var blockyShip = generator.GenerateShip(blockyConfig);
            var angularShip = generator.GenerateShip(angularConfig);
            var sleekShip = generator.GenerateShip(sleekConfig);

            // All ships should have blocks
            if (blockyShip.Structure.Blocks.Count == 0 || 
                angularShip.Structure.Blocks.Count == 0 || 
                sleekShip.Structure.Blocks.Count == 0)
            {
                Console.WriteLine("  ✗ Some ships have no blocks");
                return false;
            }

            // Verify block counts are in viable range (50-150)
            const int minBlocks = 50;
            const int maxBlocks = 150;
            
            if (blockyShip.Structure.Blocks.Count < minBlocks || blockyShip.Structure.Blocks.Count > maxBlocks)
            {
                Console.WriteLine($"  ✗ Blocky ship block count out of range: {blockyShip.Structure.Blocks.Count} (expected {minBlocks}-{maxBlocks})");
                return false;
            }
            
            if (angularShip.Structure.Blocks.Count < minBlocks || angularShip.Structure.Blocks.Count > maxBlocks)
            {
                Console.WriteLine($"  ✗ Angular ship block count out of range: {angularShip.Structure.Blocks.Count} (expected {minBlocks}-{maxBlocks})");
                return false;
            }
            
            if (sleekShip.Structure.Blocks.Count < minBlocks || sleekShip.Structure.Blocks.Count > maxBlocks)
            {
                Console.WriteLine($"  ✗ Sleek ship block count out of range: {sleekShip.Structure.Blocks.Count} (expected {minBlocks}-{maxBlocks})");
                return false;
            }

            Console.WriteLine($"  ✓ Blocky ship: {blockyShip.Structure.Blocks.Count} blocks (industrial/utilitarian)");
            Console.WriteLine($"  ✓ Angular ship: {angularShip.Structure.Blocks.Count} blocks (military fighter with wings)");
            Console.WriteLine($"  ✓ Sleek ship: {sleekShip.Structure.Blocks.Count} blocks (streamlined needle design)");
            Console.WriteLine($"  ✓ All ships in viable range ({minBlocks}-{maxBlocks} blocks)");

            return true;
        }
        catch (Exception ex)
        {
            Console.WriteLine($"  ✗ Exception: {ex.Message}");
            return false;
        }
    }

    /// <summary>
    /// Test that ships are not just plain rectangular boxes
    /// </summary>
    private bool TestShipNotPlainSquares()
    {
        try
        {
            Console.WriteLine("Testing ships are not plain squares...");

            var generator = new ProceduralShipGenerator(12345);
            var config = new ShipGenerationConfig
            {
                Size = ShipSize.Frigate,
                Style = FactionShipStyle.GetDefaultStyle("Default")
            };
            config.Style.PreferredHullShape = ShipHullShape.Angular;

            var ship = generator.GenerateShip(config);

            if (ship.Structure.Blocks.Count == 0)
            {
                Console.WriteLine("  ✗ Ship has no blocks");
                return false;
            }

            // Check Z-axis distribution (length)
            var zPositions = ship.Structure.Blocks.Select(b => b.Position.Z).ToList();
            float zRange = zPositions.Max() - zPositions.Min();

            // Check X-axis distribution (width)
            var xPositions = ship.Structure.Blocks.Select(b => b.Position.X).ToList();
            float xRange = xPositions.Max() - xPositions.Min();

            // Check Y-axis distribution (height)
            var yPositions = ship.Structure.Blocks.Select(b => b.Position.Y).ToList();
            float yRange = yPositions.Max() - yPositions.Min();

            // Ship should generally be longer or have varied proportions (not a perfect cube)
            // Allow for either elongated OR varied cross-sections
            bool isElongated = zRange > xRange * 0.8f; // Length is at least 80% of width or more
            bool hasVariedProportions = Math.Abs(zRange - xRange) < xRange * 0.5f && Math.Abs(zRange - yRange) > yRange * 0.3f;
            
            if (!isElongated && !hasVariedProportions)
            {
                Console.WriteLine($"  ✗ Ship appears to be a simple box (Z:{zRange:F1} X:{xRange:F1} Y:{yRange:F1})");
                return false;
            }

            // Check for varied block distribution (not a perfect rectangle)
            // Count blocks at different Z positions - should vary
            var blocksPerZ = zPositions.GroupBy(z => Math.Round(z / 2) * 2)
                                      .Select(g => g.Count())
                                      .ToList();

            float variance = CalculateVariance(blocksPerZ);
            if (variance < 1.0f)
            {
                Console.WriteLine($"  ✗ Ship appears to be a uniform rectangle (variance: {variance:F2})");
                return false;
            }

            Console.WriteLine($"  ✓ Ship dimensions: Length={zRange:F1}, Width={xRange:F1}, Height={yRange:F1}");
            Console.WriteLine($"  ✓ Ship has varied cross-sections (variance: {variance:F2})");

            return true;
        }
        catch (Exception ex)
        {
            Console.WriteLine($"  ✗ Exception: {ex.Message}");
            return false;
        }
    }

    /// <summary>
    /// Test that movement appears smooth with interpolation
    /// </summary>
    private bool TestSmoothMovement()
    {
        try
        {
            Console.WriteLine("Testing smooth movement simulation...");

            // Create moving entity
            var entity = _entityManager.CreateEntity("Moving Ship");
            var physics = new PhysicsComponent
            {
                Position = new Vector3(0, 0, 0),
                Velocity = new Vector3(100, 0, 0),
                Mass = 1000f,
                Drag = 0.1f
            };
            _entityManager.AddComponent(entity.Id, physics);

            // Simulate several frames
            List<Vector3> interpolatedPositions = new List<Vector3>();
            float dt = 0.016f; // ~60 FPS

            for (int frame = 0; frame < 10; frame++)
            {
                _physicsSystem.Update(dt);
                _physicsSystem.InterpolatePhysics(0.5f);
                interpolatedPositions.Add(physics.InterpolatedPosition);
            }

            // Check that movement is progressive (no sudden jumps)
            for (int i = 1; i < interpolatedPositions.Count; i++)
            {
                float distance = Vector3.Distance(interpolatedPositions[i], interpolatedPositions[i - 1]);
                
                // Movement should be consistent (no big jumps or stops)
                if (distance < 0.5f || distance > 10f)
                {
                    Console.WriteLine($"  ✗ Inconsistent movement at frame {i}: distance={distance:F2}");
                    return false;
                }
            }

            float totalDistance = Vector3.Distance(interpolatedPositions[0], interpolatedPositions[^1]);
            Console.WriteLine($"  ✓ Smooth movement over {interpolatedPositions.Count} frames, total distance: {totalDistance:F2}");

            return true;
        }
        catch (Exception ex)
        {
            Console.WriteLine($"  ✗ Exception: {ex.Message}");
            return false;
        }
    }

    /// <summary>
    /// Calculate variance of a list of values
    /// </summary>
    private float CalculateVariance(List<int> values)
    {
        if (values.Count == 0) return 0;

        float mean = (float)values.Average();
        float sumSquaredDiffs = values.Sum(v => (v - mean) * (v - mean));
        return sumSquaredDiffs / values.Count;
    }
}
