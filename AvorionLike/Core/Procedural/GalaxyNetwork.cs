using System.Numerics;

namespace AvorionLike.Core.Procedural;

/// <summary>
/// Manages the galaxy-wide network of star systems and their jump gate connections
/// Models the galaxy as a graph where systems are nodes and gates are edges
/// </summary>
public class GalaxyNetwork
{
    private readonly Dictionary<string, SolarSystemData> _systems = new();
    private readonly Dictionary<string, List<string>> _connections = new();
    private readonly int _galaxySeed;
    private readonly StarSystemGenerator _systemGenerator;
    
    public IReadOnlyDictionary<string, SolarSystemData> Systems => _systems;
    public IReadOnlyDictionary<string, List<string>> Connections => _connections;
    
    public GalaxyNetwork(int galaxySeed)
    {
        _galaxySeed = galaxySeed;
        _systemGenerator = new StarSystemGenerator(galaxySeed);
    }
    
    /// <summary>
    /// Generate or retrieve a solar system at given coordinates
    /// </summary>
    public SolarSystemData GetOrGenerateSystem(Vector3Int coordinates)
    {
        string systemId = $"System-{coordinates.X}-{coordinates.Y}-{coordinates.Z}";
        
        if (_systems.TryGetValue(systemId, out var existing))
            return existing;
        
        // Generate new system
        var system = _systemGenerator.GenerateSystem(coordinates);
        _systems[systemId] = system;
        
        // Generate connections for this system
        GenerateSystemConnections(system);
        
        return system;
    }
    
    /// <summary>
    /// Generate connections between this system and nearby systems
    /// Uses deterministic algorithm to ensure consistency
    /// </summary>
    private void GenerateSystemConnections(SolarSystemData system)
    {
        if (_connections.ContainsKey(system.SystemId))
            return;
        
        var random = new Random(system.Seed);
        var connections = new List<string>();
        
        // Determine number of connections based on system type
        int connectionCount = system.Type switch
        {
            SystemType.Core => random.Next(4, 7),
            SystemType.Civilized => random.Next(3, 5),
            SystemType.Frontier => random.Next(2, 4),
            SystemType.Contested => random.Next(2, 3),
            SystemType.Empty => random.Next(1, 2),
            SystemType.AsteroidRich => random.Next(2, 4),
            SystemType.Nebula => random.Next(2, 3),
            _ => 2
        };
        
        // Generate connections to nearby systems
        var nearbyCoordinates = GetNearbySystemCoordinates(system.Coordinates, connectionCount * 2, random);
        
        // Select subset for actual connections
        for (int i = 0; i < Math.Min(connectionCount, nearbyCoordinates.Count); i++)
        {
            var destCoords = nearbyCoordinates[i];
            var destSystemId = $"System-{destCoords.X}-{destCoords.Y}-{destCoords.Z}";
            
            // Don't connect to self
            if (destSystemId == system.SystemId)
                continue;
            
            connections.Add(destSystemId);
            
            // Add bidirectional connection
            if (!_connections.ContainsKey(destSystemId))
                _connections[destSystemId] = new List<string>();
            
            if (!_connections[destSystemId].Contains(system.SystemId))
                _connections[destSystemId].Add(system.SystemId);
        }
        
        _connections[system.SystemId] = connections;
        
        // Add stargates to the system
        _systemGenerator.AddStargatesToSystem(system, connections);
    }
    
    /// <summary>
    /// Get list of nearby system coordinates
    /// </summary>
    private List<Vector3Int> GetNearbySystemCoordinates(Vector3Int origin, int count, Random random)
    {
        var coordinates = new List<Vector3Int>();
        var offsets = new List<Vector3Int>();
        
        // Generate potential neighbor offsets (within 1-3 units)
        for (int x = -3; x <= 3; x++)
        {
            for (int y = -3; y <= 3; y++)
            {
                for (int z = -3; z <= 3; z++)
                {
                    if (x == 0 && y == 0 && z == 0)
                        continue;
                    
                    int distSq = x * x + y * y + z * z;
                    if (distSq <= 9) // Within 3 units
                    {
                        offsets.Add(new Vector3Int(x, y, z));
                    }
                }
            }
        }
        
        // Shuffle and select
        offsets = offsets.OrderBy(_ => random.Next()).ToList();
        
        for (int i = 0; i < Math.Min(count, offsets.Count); i++)
        {
            var offset = offsets[i];
            coordinates.Add(new Vector3Int(
                origin.X + offset.X,
                origin.Y + offset.Y,
                origin.Z + offset.Z
            ));
        }
        
        return coordinates;
    }
    
    /// <summary>
    /// Find path between two systems (for route planning)
    /// Uses breadth-first search
    /// </summary>
    public List<string>? FindPath(string startSystemId, string endSystemId)
    {
        if (startSystemId == endSystemId)
            return new List<string> { startSystemId };
        
        var queue = new Queue<(string systemId, List<string> path)>();
        var visited = new HashSet<string>();
        
        queue.Enqueue((startSystemId, new List<string> { startSystemId }));
        visited.Add(startSystemId);
        
        while (queue.Count > 0)
        {
            var (currentId, currentPath) = queue.Dequeue();
            
            if (!_connections.TryGetValue(currentId, out var connections))
                continue;
            
            foreach (var neighborId in connections)
            {
                if (visited.Contains(neighborId))
                    continue;
                
                var newPath = new List<string>(currentPath) { neighborId };
                
                if (neighborId == endSystemId)
                    return newPath;
                
                queue.Enqueue((neighborId, newPath));
                visited.Add(neighborId);
            }
        }
        
        return null; // No path found
    }
    
    /// <summary>
    /// Get distance between two systems (in jumps)
    /// </summary>
    public int GetJumpDistance(string startSystemId, string endSystemId)
    {
        var path = FindPath(startSystemId, endSystemId);
        return path != null ? path.Count - 1 : -1;
    }
    
    /// <summary>
    /// Get all systems within N jumps of a system
    /// </summary>
    public List<string> GetSystemsInRange(string systemId, int maxJumps)
    {
        var result = new List<string>();
        var queue = new Queue<(string id, int depth)>();
        var visited = new HashSet<string>();
        
        queue.Enqueue((systemId, 0));
        visited.Add(systemId);
        
        while (queue.Count > 0)
        {
            var (currentId, depth) = queue.Dequeue();
            
            if (depth > 0) // Don't include starting system
                result.Add(currentId);
            
            if (depth >= maxJumps)
                continue;
            
            if (!_connections.TryGetValue(currentId, out var connections))
                continue;
            
            foreach (var neighborId in connections)
            {
                if (!visited.Contains(neighborId))
                {
                    visited.Add(neighborId);
                    queue.Enqueue((neighborId, depth + 1));
                }
            }
        }
        
        return result;
    }
    
    /// <summary>
    /// Get the stargate that leads to a specific destination system
    /// </summary>
    public StargateData? GetGateToDestination(string currentSystemId, string destinationSystemId)
    {
        if (!_systems.TryGetValue(currentSystemId, out var system))
            return null;
        
        return system.Stargates.FirstOrDefault(g => g.DestinationSystemId == destinationSystemId);
    }
    
    /// <summary>
    /// Generate network statistics
    /// </summary>
    public GalaxyNetworkStats GetStats()
    {
        int totalConnections = _connections.Values.Sum(list => list.Count);
        double avgConnections = _systems.Count > 0 ? (double)totalConnections / _systems.Count : 0;
        
        return new GalaxyNetworkStats
        {
            TotalSystems = _systems.Count,
            TotalConnections = totalConnections / 2, // Bidirectional, so divide by 2
            AverageConnectionsPerSystem = avgConnections,
            GeneratedFromSeed = _galaxySeed
        };
    }
    
    /// <summary>
    /// Clear all generated data (for reset)
    /// </summary>
    public void Clear()
    {
        _systems.Clear();
        _connections.Clear();
    }
}

/// <summary>
/// Statistics about the galaxy network
/// </summary>
public class GalaxyNetworkStats
{
    public int TotalSystems { get; set; }
    public int TotalConnections { get; set; }
    public double AverageConnectionsPerSystem { get; set; }
    public int GeneratedFromSeed { get; set; }
}
