using System.Numerics;
using AvorionLike.Core.Procedural;
using AvorionLike.Core.ECS;
using AvorionLike.Core.Station;

namespace AvorionLike.Examples;

/// <summary>
/// Example demonstrating the new enhanced generation systems
/// </summary>
public class EnhancedGenerationExample
{
    private readonly EntityManager _entityManager;
    private readonly GalaxyGenerator _galaxyGenerator;
    private readonly ProceduralShipGenerator _shipGenerator;
    
    public EnhancedGenerationExample(EntityManager entityManager, int seed = 42)
    {
        _entityManager = entityManager;
        _galaxyGenerator = new GalaxyGenerator(seed);
        _shipGenerator = new ProceduralShipGenerator(seed);
    }
    
    /// <summary>
    /// Demonstrate enhanced ship generation with 1.5x more blocks
    /// </summary>
    public void DemonstrateEnhancedShipGeneration()
    {
        Console.WriteLine("\n=== ENHANCED SHIP GENERATION DEMO ===\n");
        
        // Generate ships of various sizes
        var shipSizes = new[] { ShipSize.Fighter, ShipSize.Frigate, ShipSize.Destroyer };
        
        foreach (var size in shipSizes)
        {
            var config = new ShipGenerationConfig
            {
                Size = size,
                Role = ShipRole.Combat,
                Material = "Titanium",
                Style = FactionShipStyle.GetDefaultStyle("Military"),
                Seed = Environment.TickCount
            };
            
            var ship = _shipGenerator.GenerateShip(config);
            
            Console.WriteLine($"{size} Ship:");
            Console.WriteLine($"  Blocks: {ship.Structure.Blocks.Count} (1.5x enhanced)");
            Console.WriteLine($"  Mass: {ship.TotalMass:F0} kg");
            Console.WriteLine($"  Varied block sizes used for aesthetic diversity");
            Console.WriteLine($"  Triangular/angular elements included");
            Console.WriteLine();
        }
    }
    
    /// <summary>
    /// Demonstrate station generation (1000+ blocks)
    /// </summary>
    public void DemonstrateMassiveStations()
    {
        Console.WriteLine("\n=== MASSIVE STATION GENERATION DEMO ===\n");
        
        var stationGenerator = new ProceduralStationGenerator(42);
        
        // Generate different station types
        var stationTypes = new[] { "Trading", "Military", "Refinery", "Shipyard" };
        
        foreach (var type in stationTypes)
        {
            var config = new StationGenerationConfig
            {
                Size = StationSize.Medium,
                StationType = type,
                Material = "Titanium",
                Architecture = StationArchitecture.Modular,
                Seed = Environment.TickCount + type.GetHashCode(),
                IncludeDockingBays = true,
                MinDockingBays = 6
            };
            
            var station = stationGenerator.GenerateStation(config);
            
            Console.WriteLine($"{type} Station:");
            Console.WriteLine($"  Blocks: {station.BlockCount} (Minimum 2000 enforced)");
            Console.WriteLine($"  Architecture: {config.Architecture}");
            Console.WriteLine($"  Docking Bays: {station.DockingPoints.Count}");
            Console.WriteLine($"  Facilities: {string.Join(", ", station.Facilities)}");
            Console.WriteLine();
        }
    }
    
    /// <summary>
    /// Demonstrate captain hiring system
    /// </summary>
    public void DemonstrateCaptainHiring()
    {
        Console.WriteLine("\n=== CAPTAIN HIRING SYSTEM DEMO ===\n");
        
        var random = new Random(42);
        var captainRoster = new StationCaptainRosterComponent();
        
        // Refresh roster for different station types
        var stationTypes = new[] { "Military", "Trading", "Refinery" };
        
        foreach (var stationType in stationTypes)
        {
            captainRoster.RefreshRoster(stationType, random);
            
            Console.WriteLine($"{stationType} Station Captains:");
            
            foreach (var captain in captainRoster.AvailableCaptains.Take(3))
            {
                Console.WriteLine($"  {captain.Name}");
                Console.WriteLine($"    Specialization: {captain.Specialization}");
                Console.WriteLine($"    Personality: {captain.Personality}");
                Console.WriteLine($"    Overall Rating: {captain.GetOverallRating()}/100");
                Console.WriteLine($"    Hire Cost: {captain.HireCost} credits");
                Console.WriteLine($"    Daily Salary: {captain.DailySalary} credits/day");
            }
            Console.WriteLine();
        }
    }
    
    /// <summary>
    /// Demonstrate refinery ore processing system
    /// </summary>
    public void DemonstrateRefinerySystem()
    {
        Console.WriteLine("\n=== REFINERY PROCESSING SYSTEM DEMO ===\n");
        
        var refinery = new RefineryComponent
        {
            MaxConcurrentOrders = 10,
            ProcessingSpeedMultiplier = 1.0f,
            EfficiencyBonus = 0.1f,
            MaxStoragePerType = 10000
        };
        
        var random = new Random(42);
        
        // Place some orders
        Console.WriteLine("Placing refinery orders:");
        
        var oreTypes = new[] { "Iron", "Titanium", "Naonite" };
        foreach (var oreType in oreTypes)
        {
            int oreAmount = 100 + random.Next(200);
            
            if (refinery.PlaceOrder("Player1", oreType, oreAmount, random, out string error))
            {
                var order = refinery.ActiveOrders.Last();
                Console.WriteLine($"  {oreType} Ore: {oreAmount} units");
                Console.WriteLine($"    Processing Time: {order.ProcessingTimeMinutes:F1} minutes");
                Console.WriteLine($"    Expected Ingots: {order.IngotAmount} units (70-85% efficiency)");
                Console.WriteLine($"    Processing Cost: {order.ProcessingCost:F0} credits");
                Console.WriteLine($"    Status: {order.Status}");
            }
            else
            {
                Console.WriteLine($"  Failed to place {oreType} order: {error}");
            }
        }
        
        Console.WriteLine();
        Console.WriteLine("Players can drop off ore and pick up processed ingots after waiting.");
    }
    
    /// <summary>
    /// Demonstrate massive claimable asteroid generation
    /// </summary>
    public void DemonstrateMassiveAsteroids()
    {
        Console.WriteLine("\n=== MASSIVE CLAIMABLE ASTEROID DEMO ===\n");
        
        var asteroidGenerator = new MassiveAsteroidGenerator(42);
        
        Console.WriteLine("5% spawn chance when warping to new systems\n");
        
        var asteroidTypes = Enum.GetValues<MassiveAsteroidType>();
        
        foreach (var type in asteroidTypes)
        {
            var config = new MassiveAsteroidConfig
            {
                Type = type,
                Seed = Environment.TickCount + ((int)type * 1000),
                MinSize = 2000f,
                MaxSize = 5000f
            };
            
            var asteroid = asteroidGenerator.GenerateAsteroid(config);
            
            Console.WriteLine($"{type} Asteroid:");
            Console.WriteLine($"  Blocks: {asteroid.BlockCount} (NO extractable resources)");
            Console.WriteLine($"  Mass: {asteroid.TotalMass:F0} kg");
            Console.WriteLine($"  Landing Zone: {asteroid.LandingZone}");
            Console.WriteLine($"  Docking Points: {asteroid.DockingPoints.Count}");
            Console.WriteLine($"  Status: {(asteroid.IsClaimed ? "Claimed" : "Claimable")}");
            Console.WriteLine($"  Purpose: Player hub, renamable, can build structures");
            Console.WriteLine();
        }
        
        Console.WriteLine("Players can claim asteroids and establish hyperspace network between hubs.");
    }
    
    /// <summary>
    /// Demonstrate integrated galaxy generation
    /// </summary>
    public void DemonstrateIntegratedGalaxy()
    {
        Console.WriteLine("\n=== INTEGRATED GALAXY GENERATION DEMO ===\n");
        
        // Generate a few sectors
        for (int i = 0; i < 5; i++)
        {
            var sector = _galaxyGenerator.GenerateSector(i, 0, 0, _entityManager);
            
            Console.WriteLine($"Sector [{sector.X}, {sector.Y}, {sector.Z}]:");
            Console.WriteLine($"  Normal Asteroids: {sector.Asteroids.Count}");
            
            if (sector.MassiveAsteroid != null)
            {
                Console.WriteLine($"  ⭐ MASSIVE CLAIMABLE ASTEROID FOUND!");
                Console.WriteLine($"     Type: {sector.MassiveAsteroid.Type}");
                Console.WriteLine($"     Blocks: {sector.MassiveAsteroid.BlockCount}");
                Console.WriteLine($"     Landing Zone: {sector.MassiveAsteroid.LandingZone}");
            }
            
            if (sector.Station != null)
            {
                Console.WriteLine($"  Station: {sector.Station.Name}");
                Console.WriteLine($"    Type: {sector.Station.StationType}");
                Console.WriteLine($"    Blocks: {sector.Station.BlockCount}");
                Console.WriteLine($"    Docking Bays: {sector.Station.DockingPoints.Count}");
                Console.WriteLine($"    Facilities: {string.Join(", ", sector.Station.Facilities)}");
            }
            
            Console.WriteLine();
        }
    }
    
    /// <summary>
    /// Run all demonstrations
    /// </summary>
    public void RunAllDemos()
    {
        Console.WriteLine("╔════════════════════════════════════════════════════════════════╗");
        Console.WriteLine("║  CODENAME:SUBSPACE - ENHANCED GENERATION SYSTEMS DEMO          ║");
        Console.WriteLine("╚════════════════════════════════════════════════════════════════╝");
        
        DemonstrateEnhancedShipGeneration();
        DemonstrateMassiveStations();
        DemonstrateCaptainHiring();
        DemonstrateRefinerySystem();
        DemonstrateMassiveAsteroids();
        DemonstrateIntegratedGalaxy();
        
        Console.WriteLine("\n=== ALL DEMOS COMPLETE ===\n");
        Console.WriteLine("Summary of Enhancements:");
        Console.WriteLine("  ✓ Ship blocks increased by 1.5x with varied sizes");
        Console.WriteLine("  ✓ Stations with 1000+ blocks (reduced for faster loading)");
        Console.WriteLine("  ✓ Hireable captains at all stations");
        Console.WriteLine("  ✓ Refinery ore processing system");
        Console.WriteLine("  ✓ Massive claimable asteroids (5% spawn rate)");
        Console.WriteLine("  ✓ Player hub system with hyperspace network");
    }
}
