using System.Numerics;
using AvorionLike.Core.Procedural;
using AvorionLike.Core.Voxel;

namespace AvorionLike.Examples;

/// <summary>
/// Tests for the Industrial Mining Ship Generator
/// </summary>
public class IndustrialMiningShipTests
{
    /// <summary>
    /// Run all tests
    /// </summary>
    public static void Run()
    {
        Console.WriteLine("\n╔════════════════════════════════════════════════════════════════╗");
        Console.WriteLine("║        INDUSTRIAL MINING SHIP GENERATOR TESTS                  ║");
        Console.WriteLine("╚════════════════════════════════════════════════════════════════╝\n");

        int passed = 0;
        int failed = 0;

        // Test 1: Basic generation
        if (TestBasicGeneration())
        {
            Console.WriteLine("✓ Test 1: Basic Generation - PASSED");
            passed++;
        }
        else
        {
            Console.WriteLine("✗ Test 1: Basic Generation - FAILED");
            failed++;
        }

        // Test 2: All ship sizes
        if (TestAllShipSizes())
        {
            Console.WriteLine("✓ Test 2: All Ship Sizes - PASSED");
            passed++;
        }
        else
        {
            Console.WriteLine("✗ Test 2: All Ship Sizes - FAILED");
            failed++;
        }

        // Test 3: Mining equipment
        if (TestMiningEquipment())
        {
            Console.WriteLine("✓ Test 3: Mining Equipment - PASSED");
            passed++;
        }
        else
        {
            Console.WriteLine("✗ Test 3: Mining Equipment - FAILED");
            failed++;
        }

        // Test 4: Cargo capacity
        if (TestCargoCapacity())
        {
            Console.WriteLine("✓ Test 4: Cargo Capacity - PASSED");
            passed++;
        }
        else
        {
            Console.WriteLine("✗ Test 4: Cargo Capacity - FAILED");
            failed++;
        }

        // Test 5: No division by zero (single engine)
        if (TestSingleEngineShip())
        {
            Console.WriteLine("✓ Test 5: Single Engine Ship - PASSED");
            passed++;
        }
        else
        {
            Console.WriteLine("✗ Test 5: Single Engine Ship - FAILED");
            failed++;
        }

        // Summary
        Console.WriteLine();
        Console.WriteLine("═══════════════════════════════════════════════════════════════");
        Console.WriteLine($"  Total: {passed + failed} tests | Passed: {passed} | Failed: {failed}");
        Console.WriteLine("═══════════════════════════════════════════════════════════════");
    }

    private static bool TestBasicGeneration()
    {
        try
        {
            var generator = new IndustrialMiningShipGenerator(12345);
            var config = new IndustrialMiningShipConfig
            {
                Size = ShipSize.Frigate,
                Seed = 12345
            };

            var ship = generator.GenerateMiningShip(config);

            // Verify ship has blocks
            if (ship.Structure.Blocks.Count == 0)
            {
                Console.WriteLine("    ERROR: Ship has no blocks!");
                return false;
            }

            // Verify ship has thrust
            if (ship.TotalThrust <= 0)
            {
                Console.WriteLine("    ERROR: Ship has no thrust!");
                return false;
            }

            // Verify ship has power
            if (ship.TotalPowerGeneration <= 0)
            {
                Console.WriteLine("    ERROR: Ship has no power generation!");
                return false;
            }

            Console.WriteLine($"    Generated ship with {ship.Structure.Blocks.Count} blocks");
            return true;
        }
        catch (Exception ex)
        {
            Console.WriteLine($"    EXCEPTION: {ex.Message}");
            return false;
        }
    }

    private static bool TestAllShipSizes()
    {
        try
        {
            var generator = new IndustrialMiningShipGenerator(12345);
            var sizes = new[] { ShipSize.Fighter, ShipSize.Corvette, ShipSize.Frigate, 
                                ShipSize.Destroyer, ShipSize.Cruiser, ShipSize.Battleship, ShipSize.Carrier };

            foreach (var size in sizes)
            {
                var config = new IndustrialMiningShipConfig
                {
                    Size = size,
                    Seed = 12345
                };

                var ship = generator.GenerateMiningShip(config);

                if (ship.Structure.Blocks.Count == 0)
                {
                    Console.WriteLine($"    ERROR: {size} ship has no blocks!");
                    return false;
                }
            }

            Console.WriteLine($"    All {sizes.Length} ship sizes generated successfully");
            return true;
        }
        catch (Exception ex)
        {
            Console.WriteLine($"    EXCEPTION: {ex.Message}");
            return false;
        }
    }

    private static bool TestMiningEquipment()
    {
        try
        {
            var generator = new IndustrialMiningShipGenerator(12345);
            var config = new IndustrialMiningShipConfig
            {
                Size = ShipSize.Frigate,
                MiningLaserCount = 6,
                Seed = 12345
            };

            var ship = generator.GenerateMiningShip(config);

            if (ship.MiningLaserCount <= 0)
            {
                Console.WriteLine("    ERROR: Ship has no mining lasers!");
                return false;
            }

            // Count turret mounts (mining lasers)
            var turretCount = ship.Structure.Blocks.Count(b => b.BlockType == BlockType.TurretMount);
            Console.WriteLine($"    Mining lasers: {ship.MiningLaserCount}, Turret mounts: {turretCount}");

            return ship.MiningLaserCount > 0;
        }
        catch (Exception ex)
        {
            Console.WriteLine($"    EXCEPTION: {ex.Message}");
            return false;
        }
    }

    private static bool TestCargoCapacity()
    {
        try
        {
            var generator = new IndustrialMiningShipGenerator(12345);
            var config = new IndustrialMiningShipConfig
            {
                Size = ShipSize.Frigate,
                CargoModuleCount = 8,
                Seed = 12345
            };

            var ship = generator.GenerateMiningShip(config);

            if (ship.CargoCapacity <= 0)
            {
                Console.WriteLine("    ERROR: Ship has no cargo capacity!");
                return false;
            }

            // Count cargo blocks
            var cargoCount = ship.Structure.Blocks.Count(b => b.BlockType == BlockType.Cargo);
            Console.WriteLine($"    Cargo capacity: {ship.CargoCapacity}, Cargo blocks: {cargoCount}");

            return ship.CargoCapacity > 0;
        }
        catch (Exception ex)
        {
            Console.WriteLine($"    EXCEPTION: {ex.Message}");
            return false;
        }
    }

    private static bool TestSingleEngineShip()
    {
        try
        {
            var generator = new IndustrialMiningShipGenerator(12345);
            var config = new IndustrialMiningShipConfig
            {
                Size = ShipSize.Fighter, // Fighter has single engine
                Seed = 12345
            };

            // This should not throw a division by zero exception
            var ship = generator.GenerateMiningShip(config);

            if (ship.Structure.Blocks.Count == 0)
            {
                Console.WriteLine("    ERROR: Single-engine ship has no blocks!");
                return false;
            }

            // Count engine blocks
            var engineCount = ship.Structure.Blocks.Count(b => b.BlockType == BlockType.Engine);
            Console.WriteLine($"    Single-engine ship generated with {engineCount} engine(s)");

            return true;
        }
        catch (DivideByZeroException)
        {
            Console.WriteLine("    ERROR: Division by zero when generating single-engine ship!");
            return false;
        }
        catch (Exception ex)
        {
            Console.WriteLine($"    EXCEPTION: {ex.Message}");
            return false;
        }
    }
}
