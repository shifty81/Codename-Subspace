using System;
using System.Linq;
using System.Numerics;
using AvorionLike.Core.ECS;
using AvorionLike.Core.Procedural;
using AvorionLike.Core.Voxel;
using AvorionLike.Core.Resources;
using AvorionLike.Core.Logging;

namespace AvorionLike.Examples;

/// <summary>
/// Test and demonstrate the enhanced visual features for ships, stations, and asteroids
/// </summary>
public class VisualEnhancementsTest
{
    private readonly EntityManager _entityManager;
    private readonly Logger _logger = Logger.Instance;
    
    public VisualEnhancementsTest(EntityManager entityManager)
    {
        _entityManager = entityManager;
    }
    
    public void RunDemo()
    {
        Console.WriteLine("\n=== VISUAL ENHANCEMENTS DEMO ===\n");
        
        // Test enhanced ships
        TestEnhancedShips();
        
        // Test enhanced stations
        TestEnhancedStations();
        
        // Test enhanced asteroids
        TestEnhancedAsteroids();
        
        Console.WriteLine("\n=== DEMO COMPLETE ===\n");
    }
    
    private void TestEnhancedShips()
    {
        Console.WriteLine("1. ENHANCED SHIP GENERATION TEST");
        Console.WriteLine("=================================");
        
        var generator = new ProceduralShipGenerator(seed: 42);
        
        // Test different faction styles
        var factionNames = new[] { "Military", "Trading", "Pirate", "Science", "Industrial" };
        
        foreach (var factionName in factionNames)
        {
            var config = new ShipGenerationConfig
            {
                Size = ShipSize.Frigate,
                Role = ShipRole.Combat,
                Material = "Titanium",
                Style = FactionShipStyle.GetDefaultStyle(factionName),
                Seed = factionName.GetHashCode()
            };
            
            var ship = generator.GenerateShip(config);
            
            Console.WriteLine($"\n{factionName} Frigate:");
            Console.WriteLine($"  Total Blocks: {ship.Structure.Blocks.Count}");
            Console.WriteLine($"  Hull Blocks: {ship.Structure.Blocks.Count(b => b.BlockType == BlockType.Hull)}");
            Console.WriteLine($"  Armor Blocks: {ship.Structure.Blocks.Count(b => b.BlockType == BlockType.Armor)}");
            Console.WriteLine($"  Engine Blocks: {ship.Structure.Blocks.Count(b => b.BlockType == BlockType.Engine)}");
            Console.WriteLine($"  Thruster Blocks: {ship.Structure.Blocks.Count(b => b.BlockType == BlockType.Thruster)}");
            Console.WriteLine($"  Turret Mounts: {ship.Structure.Blocks.Count(b => b.BlockType == BlockType.TurretMount)}");
            
            // Check for visual enhancements
            var uniqueColors = ship.Structure.Blocks
                .Select(b => b.ColorRGB)
                .Distinct()
                .Count();
            Console.WriteLine($"  Unique Colors: {uniqueColors} (more = better variety)");
            
            // Check for detail blocks
            var detailBlocks = ship.Structure.Blocks
                .Count(b => b.Size.X < 1.5f || b.Size.Y < 1.5f || b.Size.Z < 1.5f);
            Console.WriteLine($"  Detail Blocks: {detailBlocks} (small blocks = greebles/details)");
            
            // Check for elongated blocks (antennas)
            var antennaBlocks = ship.Structure.Blocks
                .Count(b => b.Size.Y > 3 || b.Size.X > 3 || b.Size.Z > 3);
            Console.WriteLine($"  Antenna/Extended Blocks: {antennaBlocks}");
        }
    }
    
    private void TestEnhancedStations()
    {
        Console.WriteLine("\n2. ENHANCED STATION GENERATION TEST");
        Console.WriteLine("===================================");
        
        var generator = new ProceduralStationGenerator(seed: 123);
        
        var stationTypes = new[] { "Trading", "Military", "Industrial", "Research" };
        
        foreach (var stationType in stationTypes)
        {
            var config = new StationGenerationConfig
            {
                Size = StationSize.Small,
                StationType = stationType,
                Material = "Titanium",
                Architecture = StationArchitecture.Modular,
                Seed = stationType.GetHashCode()
            };
            
            var station = generator.GenerateStation(config);
            
            Console.WriteLine($"\n{stationType} Station:");
            Console.WriteLine($"  Total Blocks: {station.BlockCount}");
            Console.WriteLine($"  Docking Points: {station.DockingPoints.Count}");
            
            // Check for antennas (elongated blocks)
            var antennas = station.Structure.Blocks
                .Count(b => b.Size.X > 8 || b.Size.Y > 8 || b.Size.Z > 8);
            Console.WriteLine($"  Antenna Arrays: {antennas}");
            
            // Check for turret mounts (communication dishes/sensors)
            var turretMounts = station.Structure.Blocks
                .Count(b => b.BlockType == BlockType.TurretMount);
            Console.WriteLine($"  Communication/Sensor Arrays: {turretMounts}");
            
            // Check for color variety
            var uniqueColors = station.Structure.Blocks
                .Select(b => b.ColorRGB)
                .Distinct()
                .Count();
            Console.WriteLine($"  Unique Colors: {uniqueColors}");
        }
    }
    
    private void TestEnhancedAsteroids()
    {
        Console.WriteLine("\n3. ENHANCED ASTEROID GENERATION TEST");
        Console.WriteLine("====================================");
        
        var generator = new AsteroidVoxelGenerator(seed: 456);
        
        var resourceTypes = new[] { "Iron", "Titanium", "Naonite", "Avorion" };
        
        foreach (var resourceType in resourceTypes)
        {
            var asteroidData = new AsteroidData
            {
                Position = Vector3.Zero,
                Size = 50f,
                ResourceType = resourceType
            };
            
            // Generate enhanced asteroid
            var blocks = generator.GenerateEnhancedAsteroid(asteroidData, voxelResolution: 8);
            
            Console.WriteLine($"\n{resourceType} Asteroid:");
            Console.WriteLine($"  Total Blocks: {blocks.Count}");
            
            // Check for resource variety (different materials)
            var materials = blocks
                .Select(b => b.MaterialType)
                .Distinct()
                .ToList();
            Console.WriteLine($"  Material Types: {materials.Count} - {string.Join(", ", materials.Take(5))}");
            
            // Check for crystal protrusions (blocks extending above surface)
            var surfaceAvg = blocks.Average(b => b.Position.Length());
            var protrusions = blocks.Count(b => b.Position.Length() > surfaceAvg * 1.1);
            Console.WriteLine($"  Surface Details/Crystals: {protrusions}");
            
            // Check for size variation (craters = fewer blocks in certain areas)
            var totalVolume = blocks.Sum(b => b.Size.X * b.Size.Y * b.Size.Z);
            var avgBlockSize = totalVolume / blocks.Count;
            Console.WriteLine($"  Average Block Size: {avgBlockSize:F2} (variety in sizes)");
        }
    }
}
