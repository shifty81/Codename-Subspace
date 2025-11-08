using System.Numerics;

namespace AvorionLike.Core.Procedural;

/// <summary>
/// Represents a complete solar system with all its celestial objects
/// Generated deterministically from system coordinates and seed
/// </summary>
public class SolarSystemData
{
    public string SystemId { get; set; }
    public Vector3Int Coordinates { get; set; }
    public string Name { get; set; }
    public int Seed { get; set; }
    
    // Celestial bodies
    public StarData? CentralStar { get; set; }
    public List<PlanetData> Planets { get; set; } = new();
    public List<AsteroidBeltData> AsteroidBelts { get; set; } = new();
    
    // Points of Interest
    public List<StationData> Stations { get; set; } = new();
    public List<StargateData> Stargates { get; set; } = new();
    
    // System properties
    public float Radius { get; set; } // Total system radius
    public SystemType Type { get; set; }
    public int DangerLevel { get; set; } // 0-10, affects spawn rates
    
    public SolarSystemData(string systemId, Vector3Int coordinates, int seed)
    {
        SystemId = systemId;
        Coordinates = coordinates;
        Seed = seed;
        Name = $"System-{coordinates.X}-{coordinates.Y}-{coordinates.Z}";
    }
}

/// <summary>
/// Star data for the central star of a system
/// </summary>
public class StarData
{
    public Vector3 Position { get; set; }
    public float Size { get; set; }
    public StarType Type { get; set; }
    public string Name { get; set; } = "Unknown Star";
}

/// <summary>
/// Planet data
/// </summary>
public class PlanetData
{
    public string Name { get; set; } = "Unknown Planet";
    public Vector3 Position { get; set; }
    public float Size { get; set; }
    public float OrbitRadius { get; set; }
    public PlanetType Type { get; set; }
}

/// <summary>
/// Asteroid belt data - defines boundaries and density
/// </summary>
public class AsteroidBeltData
{
    public string Name { get; set; } = "Asteroid Belt";
    public Vector3 Center { get; set; }
    public float InnerRadius { get; set; }
    public float OuterRadius { get; set; }
    public float Height { get; set; } // Thickness of the belt
    public float Density { get; set; } // Asteroids per cubic unit
    public string PrimaryResource { get; set; } = "Iron";
    
    /// <summary>
    /// Check if a position is within this belt
    /// </summary>
    public bool Contains(Vector3 position)
    {
        var offset = position - Center;
        var horizontalDistance = Math.Sqrt(offset.X * offset.X + offset.Z * offset.Z);
        var verticalDistance = Math.Abs(offset.Y);
        
        return horizontalDistance >= InnerRadius && 
               horizontalDistance <= OuterRadius && 
               verticalDistance <= Height / 2;
    }
}

/// <summary>
/// Stargate/Jump gate data
/// </summary>
public class StargateData
{
    public string GateId { get; set; } = Guid.NewGuid().ToString();
    public string Name { get; set; } = "Stargate";
    public Vector3 Position { get; set; }
    public Vector3 Rotation { get; set; }
    public string DestinationSystemId { get; set; } = "";
    public string? DestinationGateId { get; set; }
    public GateType Type { get; set; }
    public bool IsActive { get; set; } = true;
}

/// <summary>
/// System type enum
/// </summary>
public enum SystemType
{
    Core,           // Center of galaxy, high resources, high danger
    Civilized,      // Many stations, low danger
    Frontier,       // Few stations, medium danger
    Contested,      // Faction warfare, high danger
    Empty,          // Few objects, low resources
    AsteroidRich,   // High asteroid density
    Nebula          // Special effects, medium resources
}

/// <summary>
/// Star type enum
/// </summary>
public enum StarType
{
    RedDwarf,
    YellowStar,
    BlueGiant,
    WhiteDwarf,
    Neutron,
    BlackHole
}

/// <summary>
/// Planet type enum
/// </summary>
public enum PlanetType
{
    Rocky,
    Gas,
    Ice,
    Lava,
    Desert,
    Ocean,
    Habitable
}

/// <summary>
/// Gate type enum
/// </summary>
public enum GateType
{
    Standard,   // Normal jump gate
    Ancient,    // Pre-built, faster travel
    Unstable,   // Risky but useful
    Military    // Faction-controlled
}
