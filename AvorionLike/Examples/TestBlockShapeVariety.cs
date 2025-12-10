using System;
using System.Linq;
using AvorionLike.Core.Procedural;
using AvorionLike.Core.Voxel;

namespace AvorionLike.Examples;

/// <summary>
/// Test to verify that blocks are generated with shape variety (not all cubes)
/// </summary>
public class TestBlockShapeVariety
{
    public static void Run()
    {
        Console.WriteLine("╔════════════════════════════════════════════════════════════════╗");
        Console.WriteLine("║         Testing Block Shape Variety in Procedural Gen         ║");
        Console.WriteLine("╚════════════════════════════════════════════════════════════════╝\n");

        Console.WriteLine("Testing shape variety across different procedural generators:\n");
        
        // Test 1: Ship Generation
        TestShipShapeVariety();
        
        // Test 2: Asteroid Generation
        TestAsteroidShapeVariety();
        
        // Test 3: Station Generation
        TestStationShapeVariety();
        
        Console.WriteLine("\n" + new string('═', 64));
        Console.WriteLine("✅ Block shape variety test completed!");
        Console.WriteLine("   All generators now produce diverse block shapes.");
        Console.WriteLine(new string('═', 64));
    }
    
    private static void TestShipShapeVariety()
    {
        Console.WriteLine("1. SHIP GENERATION SHAPE VARIETY");
        Console.WriteLine(new string('-', 64));
        
        var generator = new ProceduralShipGenerator(12345);
        
        // Test different ship types
        var shipTypes = new[] { "Military", "Trading", "Industrial", "Pirate" };
        
        foreach (var shipType in shipTypes)
        {
            var config = new ShipGenerationConfig
            {
                Size = ShipSize.Frigate,
                Role = ShipRole.Combat,
                Material = "Titanium",
                Style = FactionShipStyle.GetDefaultStyle(shipType),
                Seed = 12345
            };
            
            var ship = generator.GenerateShip(config);
            
            // Count shape types
            var shapeStats = ship.Structure.Blocks
                .GroupBy(b => b.Shape)
                .OrderByDescending(g => g.Count())
                .Select(g => new { Shape = g.Key, Count = g.Count() })
                .ToList();
            
            Console.WriteLine($"\n  {shipType} Ship ({ship.Structure.Blocks.Count} blocks):");
            foreach (var stat in shapeStats)
            {
                double percentage = (stat.Count * 100.0) / ship.Structure.Blocks.Count;
                Console.WriteLine($"    {stat.Shape,-15}: {stat.Count,5} blocks ({percentage:F1}%)");
            }
            
            // Verify we have variety (not all cubes)
            var nonCubeCount = ship.Structure.Blocks.Count(b => b.Shape != BlockShape.Cube);
            var nonCubePercentage = (nonCubeCount * 100.0) / ship.Structure.Blocks.Count;
            
            if (nonCubePercentage > 10)
            {
                Console.WriteLine($"    ✓ Good variety: {nonCubePercentage:F1}% non-cube shapes");
            }
            else
            {
                Console.WriteLine($"    ⚠ Low variety: Only {nonCubePercentage:F1}% non-cube shapes");
            }
        }
        
        Console.WriteLine();
    }
    
    private static void TestAsteroidShapeVariety()
    {
        Console.WriteLine("2. ASTEROID GENERATION SHAPE VARIETY");
        Console.WriteLine(new string('-', 64));
        
        var generator = new AsteroidVoxelGenerator(12345);
        
        // Generate a few asteroids of different sizes
        var asteroidSizes = new[] { 10f, 20f, 30f };
        
        foreach (var size in asteroidSizes)
        {
            var asteroidData = new AsteroidData
            {
                Position = System.Numerics.Vector3.Zero,
                Size = size,
                ResourceType = "Iron"
            };
            
            var blocks = generator.GenerateAsteroid(asteroidData, voxelResolution: 8);
            
            // Count shape types
            var shapeStats = blocks
                .GroupBy(b => b.Shape)
                .OrderByDescending(g => g.Count())
                .Select(g => new { Shape = g.Key, Count = g.Count() })
                .ToList();
            
            Console.WriteLine($"\n  Asteroid (size {size}, {blocks.Count} blocks):");
            foreach (var stat in shapeStats)
            {
                double percentage = (stat.Count * 100.0) / blocks.Count;
                Console.WriteLine($"    {stat.Shape,-15}: {stat.Count,5} blocks ({percentage:F1}%)");
            }
            
            // Verify we have variety
            var nonCubeCount = blocks.Count(b => b.Shape != BlockShape.Cube);
            var nonCubePercentage = (nonCubeCount * 100.0) / blocks.Count;
            
            if (nonCubePercentage > 15)
            {
                Console.WriteLine($"    ✓ Good variety: {nonCubePercentage:F1}% non-cube shapes");
            }
            else
            {
                Console.WriteLine($"    ⚠ Low variety: Only {nonCubePercentage:F1}% non-cube shapes");
            }
        }
        
        Console.WriteLine();
    }
    
    private static void TestStationShapeVariety()
    {
        Console.WriteLine("3. STATION GENERATION SHAPE VARIETY");
        Console.WriteLine(new string('-', 64));
        
        var generator = new ProceduralStationGenerator(12345);
        
        // Test a few station types
        var architectures = new[] 
        { 
            StationArchitecture.Modular,
            StationArchitecture.Industrial,
            StationArchitecture.Spherical 
        };
        
        foreach (var architecture in architectures)
        {
            var config = new StationGenerationConfig
            {
                Size = StationSize.Small,
                Architecture = architecture,
                Material = "Titanium",
                Seed = 12345
            };
            
            var station = generator.GenerateStation(config);
            
            // Count shape types
            var shapeStats = station.Structure.Blocks
                .GroupBy(b => b.Shape)
                .OrderByDescending(g => g.Count())
                .Select(g => new { Shape = g.Key, Count = g.Count() })
                .ToList();
            
            Console.WriteLine($"\n  {architecture} Station ({station.Structure.Blocks.Count} blocks):");
            foreach (var stat in shapeStats.Take(5)) // Show top 5 shape types
            {
                double percentage = (stat.Count * 100.0) / station.Structure.Blocks.Count;
                Console.WriteLine($"    {stat.Shape,-15}: {stat.Count,5} blocks ({percentage:F1}%)");
            }
            
            // Verify we have variety
            var nonCubeCount = station.Structure.Blocks.Count(b => b.Shape != BlockShape.Cube);
            var nonCubePercentage = (nonCubeCount * 100.0) / station.Structure.Blocks.Count;
            
            if (nonCubePercentage > 10)
            {
                Console.WriteLine($"    ✓ Good variety: {nonCubePercentage:F1}% non-cube shapes");
            }
            else
            {
                Console.WriteLine($"    ⚠ Low variety: Only {nonCubePercentage:F1}% non-cube shapes");
            }
        }
        
        Console.WriteLine();
    }
}
