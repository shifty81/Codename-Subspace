using System.Numerics;
using AvorionLike.Core.Procedural;
using AvorionLike.Core.ECS;
using AvorionLike.Core.SolarSystem;

namespace AvorionLike.Examples;

/// <summary>
/// Example demonstrating the complete procedural generation system
/// Shows how to generate galaxy, systems, asteroids, and stargates
/// </summary>
public class ProceduralGenerationExample
{
    private readonly int _galaxySeed = 42; // Fixed seed for consistent universe
    private GalaxyNetwork? _galaxyNetwork;
    private EntityManager? _entityManager;
    private HyperspaceJump? _hyperspaceJump;
    private Dictionary<string, AsteroidField> _asteroidFields = new();
    
    /// <summary>
    /// Initialize the procedural generation system
    /// </summary>
    public void Initialize()
    {
        Console.WriteLine("=== Procedural Generation System Demo ===\n");
        
        // Step 1: Initialize galaxy network
        Console.WriteLine("1. Initializing Galaxy Network...");
        _galaxyNetwork = new GalaxyNetwork(_galaxySeed);
        Console.WriteLine($"   Galaxy seed: {_galaxySeed}");
        
        // Step 2: Initialize entity manager for game objects
        Console.WriteLine("\n2. Initializing Entity Manager...");
        _entityManager = new EntityManager();
        
        // Step 3: Initialize hyperspace jump system
        Console.WriteLine("\n3. Initializing Hyperspace Jump System...");
        _hyperspaceJump = new HyperspaceJump();
        _hyperspaceJump.SetGalaxyNetwork(_galaxyNetwork);
        
        Console.WriteLine("\n✓ Initialization complete!\n");
    }
    
    /// <summary>
    /// Example 1: Generate a solar system
    /// </summary>
    public void ExampleGenerateSolarSystem()
    {
        Console.WriteLine("=== Example 1: Generate Solar System ===\n");
        
        // Generate system at coordinates (0, 0, 0) - center of galaxy
        var coords = new Vector3Int(0, 0, 0);
        var system = _galaxyNetwork!.GetOrGenerateSystem(coords);
        
        Console.WriteLine($"System ID: {system.SystemId}");
        Console.WriteLine($"System Name: {system.Name}");
        Console.WriteLine($"System Type: {system.Type}");
        Console.WriteLine($"Danger Level: {system.DangerLevel}/10");
        Console.WriteLine($"Radius: {system.Radius:F0} units");
        
        // Display central star
        if (system.CentralStar != null)
        {
            Console.WriteLine($"\nCentral Star:");
            Console.WriteLine($"  Type: {system.CentralStar.Type}");
            Console.WriteLine($"  Size: {system.CentralStar.Size:F0} units");
        }
        
        // Display planets
        Console.WriteLine($"\nPlanets ({system.Planets.Count}):");
        foreach (var planet in system.Planets)
        {
            Console.WriteLine($"  - {planet.Name}: {planet.Type}, Orbit: {planet.OrbitRadius:F0} units");
        }
        
        // Display asteroid belts
        Console.WriteLine($"\nAsteroid Belts ({system.AsteroidBelts.Count}):");
        foreach (var belt in system.AsteroidBelts)
        {
            Console.WriteLine($"  - {belt.Name}: {belt.PrimaryResource}, Density: {belt.Density:F6}");
            Console.WriteLine($"    Radius: {belt.InnerRadius:F0} - {belt.OuterRadius:F0} units");
        }
        
        // Display space stations
        Console.WriteLine($"\nSpace Stations ({system.Stations.Count}):");
        foreach (var station in system.Stations)
        {
            Console.WriteLine($"  - {station.Name} ({station.StationType})");
        }
        
        // Display stargates
        Console.WriteLine($"\nStargates ({system.Stargates.Count}):");
        foreach (var gate in system.Stargates)
        {
            Console.WriteLine($"  - {gate.Name} → {gate.DestinationSystemId}");
            Console.WriteLine($"    Type: {gate.Type}, Active: {gate.IsActive}");
        }
        
        Console.WriteLine();
    }
    
    /// <summary>
    /// Example 2: Generate multiple systems and explore galaxy network
    /// </summary>
    public void ExampleExploreGalaxyNetwork()
    {
        Console.WriteLine("=== Example 2: Explore Galaxy Network ===\n");
        
        // Generate several nearby systems
        var startCoords = new Vector3Int(0, 0, 0);
        var startSystem = _galaxyNetwork!.GetOrGenerateSystem(startCoords);
        
        Console.WriteLine($"Starting System: {startSystem.Name} ({startSystem.SystemId})");
        Console.WriteLine($"Connected to {startSystem.Stargates.Count} systems\n");
        
        // Generate connected systems
        foreach (var gate in startSystem.Stargates.Take(3))
        {
            var destCoords = ParseSystemCoordinates(gate.DestinationSystemId);
            var destSystem = _galaxyNetwork.GetOrGenerateSystem(destCoords);
            
            Console.WriteLine($"→ {destSystem.Name} ({destSystem.SystemId})");
            Console.WriteLine($"  Type: {destSystem.Type}, Danger: {destSystem.DangerLevel}/10");
        }
        
        // Show network statistics
        Console.WriteLine($"\nGalaxy Network Statistics:");
        var stats = _galaxyNetwork.GetStats();
        Console.WriteLine($"  Total Systems Generated: {stats.TotalSystems}");
        Console.WriteLine($"  Total Connections: {stats.TotalConnections}");
        Console.WriteLine($"  Avg Connections/System: {stats.AverageConnectionsPerSystem:F2}");
        
        Console.WriteLine();
    }
    
    /// <summary>
    /// Example 3: Demonstrate pathfinding between systems
    /// </summary>
    public void ExamplePathfinding()
    {
        Console.WriteLine("=== Example 3: Galaxy Pathfinding ===\n");
        
        // Generate start and end systems
        var startCoords = new Vector3Int(0, 0, 0);
        var endCoords = new Vector3Int(2, 1, 0);
        
        var startSystem = _galaxyNetwork!.GetOrGenerateSystem(startCoords);
        var endSystem = _galaxyNetwork.GetOrGenerateSystem(endCoords);
        
        Console.WriteLine($"Finding path from {startSystem.Name} to {endSystem.Name}...\n");
        
        // Find path
        var path = _galaxyNetwork.FindPath(startSystem.SystemId, endSystem.SystemId);
        
        if (path != null)
        {
            Console.WriteLine($"Path found! ({path.Count - 1} jumps)\n");
            
            for (int i = 0; i < path.Count; i++)
            {
                var coords = ParseSystemCoordinates(path[i]);
                var system = _galaxyNetwork.GetOrGenerateSystem(coords);
                
                if (i == 0)
                {
                    Console.WriteLine($"START: {system.Name}");
                }
                else if (i == path.Count - 1)
                {
                    Console.WriteLine($"  ↓ (Jump #{i})");
                    Console.WriteLine($"END:   {system.Name}");
                }
                else
                {
                    Console.WriteLine($"  ↓ (Jump #{i})");
                    Console.WriteLine($"       {system.Name}");
                }
            }
        }
        else
        {
            Console.WriteLine("No path found!");
        }
        
        Console.WriteLine();
    }
    
    /// <summary>
    /// Example 4: Generate asteroid field with LOD
    /// </summary>
    public void ExampleGenerateAsteroidField()
    {
        Console.WriteLine("=== Example 4: Asteroid Field Generation ===\n");
        
        // Get a system with asteroid belts
        var coords = new Vector3Int(0, 0, 0);
        var system = _galaxyNetwork!.GetOrGenerateSystem(coords);
        
        if (system.AsteroidBelts.Count == 0)
        {
            Console.WriteLine("No asteroid belts in this system.");
            return;
        }
        
        var belt = system.AsteroidBelts[0];
        Console.WriteLine($"Generating asteroids for: {belt.Name}");
        Console.WriteLine($"Primary Resource: {belt.PrimaryResource}");
        Console.WriteLine($"Belt Density: {belt.Density:F6} asteroids/unit³\n");
        
        // Create asteroid field
        var field = new AsteroidField($"{system.SystemId}-Belt-0", belt, _galaxySeed);
        _asteroidFields[field.FieldId] = field;
        
        // Simulate player at center of belt
        var playerPosition = belt.Center;
        var viewRadius = 10000f;
        
        Console.WriteLine($"Player position: {playerPosition}");
        Console.WriteLine($"Loading asteroids within {viewRadius:F0} units...\n");
        
        // Get asteroids in view
        var asteroids = field.GetAsteroidsInRegion(playerPosition, viewRadius);
        Console.WriteLine($"Total asteroids in range: {asteroids.Count}");
        
        // Update LOD levels
        field.UpdateLOD(playerPosition, asteroids);
        
        // Show LOD distribution
        var lodCounts = asteroids.GroupBy(a => a.LODLevel)
                                .ToDictionary(g => g.Key, g => g.Count());
        
        Console.WriteLine("\nLOD Distribution:");
        foreach (var lod in Enum.GetValues<LODLevel>())
        {
            var count = lodCounts.GetValueOrDefault(lod, 0);
            Console.WriteLine($"  {lod}: {count} asteroids");
        }
        
        // Show some sample asteroids
        Console.WriteLine("\nSample Asteroids:");
        foreach (var asteroid in asteroids.Take(5))
        {
            var distance = Vector3.Distance(playerPosition, asteroid.Position);
            Console.WriteLine($"  - Size: {asteroid.Size:F1}, Distance: {distance:F0}, LOD: {asteroid.LODLevel}");
        }
        
        // Show field statistics
        Console.WriteLine("\nField Statistics:");
        var fieldStats = field.GetStats();
        Console.WriteLine($"  Generated Cells: {fieldStats.TotalGeneratedCells}");
        Console.WriteLine($"  Total Asteroids: {fieldStats.TotalAsteroids}");
        Console.WriteLine($"  Visible Asteroids: {fieldStats.VisibleAsteroids}");
        Console.WriteLine($"  Field Volume: {fieldStats.FieldVolume:E2} units³");
        
        Console.WriteLine();
    }
    
    /// <summary>
    /// Example 5: Demonstrate stargate jump
    /// </summary>
    public void ExampleStargateJump()
    {
        Console.WriteLine("=== Example 5: Stargate Jump ===\n");
        
        // Setup current system
        var currentCoords = new Vector3Int(0, 0, 0);
        var currentSystem = _galaxyNetwork!.GetOrGenerateSystem(currentCoords);
        
        Console.WriteLine($"Current System: {currentSystem.Name}");
        _hyperspaceJump!.SetCurrentSystem(currentSystem.SystemId);
        
        if (currentSystem.Stargates.Count == 0)
        {
            Console.WriteLine("No stargates in this system.");
            return;
        }
        
        // Pick first gate
        var gate = currentSystem.Stargates[0];
        Console.WriteLine($"Approaching gate: {gate.Name}");
        Console.WriteLine($"Destination: {gate.DestinationSystemId}");
        Console.WriteLine($"Gate Type: {gate.Type}\n");
        
        // Initiate jump (simulate)
        Console.WriteLine("Initiating hyperspace jump...");
        
        bool jumpInitiated = _hyperspaceJump.InitiateGateJump(
            gate.DestinationSystemId,
            gate.DestinationGateId,
            LoadDestinationSystem
        );
        
        if (jumpInitiated)
        {
            Console.WriteLine("✓ Jump initiated successfully!");
            Console.WriteLine("  Loading destination system...");
            Console.WriteLine("  No fuel cost for gate jumps!");
            
            if (_hyperspaceJump.ExitGatePosition.HasValue)
            {
                Console.WriteLine($"  Exit gate position: {_hyperspaceJump.ExitGatePosition.Value}");
            }
        }
        else
        {
            Console.WriteLine("✗ Jump failed!");
        }
        
        Console.WriteLine();
    }
    
    /// <summary>
    /// Example 6: Demonstrate floating origin coordinates
    /// </summary>
    public void ExampleFloatingOriginCoordinates()
    {
        Console.WriteLine("=== Example 6: Floating Origin Coordinates ===\n");
        
        // Create a position very far from origin
        var farPosition = new Vector3(500000f, 300000f, 200000f);
        Console.WriteLine($"World Position: {farPosition}");
        
        // Convert to floating origin coordinates
        var coords = FloatingOriginCoordinates.FromWorldPosition(farPosition);
        Console.WriteLine($"Sector: ({coords.Sector.X}, {coords.Sector.Y}, {coords.Sector.Z})");
        Console.WriteLine($"Local Position: {coords.LocalPosition}");
        Console.WriteLine($"Sector Size: {FloatingOriginCoordinates.SectorSize} units\n");
        
        // Calculate distance to another far position
        var otherPosition = new Vector3(700000f, 400000f, 300000f);
        var otherCoords = FloatingOriginCoordinates.FromWorldPosition(otherPosition);
        
        Console.WriteLine($"Other World Position: {otherPosition}");
        Console.WriteLine($"Other Sector: ({otherCoords.Sector.X}, {otherCoords.Sector.Y}, {otherCoords.Sector.Z})");
        Console.WriteLine($"Other Local Position: {otherCoords.LocalPosition}\n");
        
        // Calculate distance (safe for large distances)
        var distance = coords.DistanceTo(otherCoords);
        Console.WriteLine($"Distance between positions: {distance:F2} units");
        
        // Direct calculation would have precision issues at this scale
        var directDistance = Vector3.Distance(farPosition, otherPosition);
        Console.WriteLine($"Direct calculation: {directDistance:F2} units");
        Console.WriteLine($"Difference: {Math.Abs(distance - directDistance):F6} units (floating point error)");
        
        Console.WriteLine();
    }
    
    /// <summary>
    /// Run all examples
    /// </summary>
    public void RunAllExamples()
    {
        Initialize();
        
        ExampleGenerateSolarSystem();
        ExampleExploreGalaxyNetwork();
        ExamplePathfinding();
        ExampleGenerateAsteroidField();
        ExampleStargateJump();
        ExampleFloatingOriginCoordinates();
        
        Console.WriteLine("=== All Examples Complete ===");
    }
    
    // Helper methods
    
    private Vector3Int ParseSystemCoordinates(string systemId)
    {
        var parts = systemId.Split('-');
        if (parts.Length >= 4 &&
            int.TryParse(parts[1], out int x) &&
            int.TryParse(parts[2], out int y) &&
            int.TryParse(parts[3], out int z))
        {
            return new Vector3Int(x, y, z);
        }
        return Vector3Int.Zero;
    }
    
    private void LoadDestinationSystem(string systemId)
    {
        // This would be called to actually load the destination system
        // In a real game, this would unload current system and load new one
        Console.WriteLine($"    [Callback] Loading system: {systemId}");
    }
}

/// <summary>
/// Main entry point for the example
/// </summary>
public static class ProceduralGenerationExampleRunner
{
    public static void RunExample()
    {
        var example = new ProceduralGenerationExample();
        example.RunAllExamples();
    }
}
