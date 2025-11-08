using System.Numerics;

namespace AvorionLike.Core.Procedural;

/// <summary>
/// Generates complete solar systems deterministically
/// Handles macro-level system generation with planets, belts, stations, and gates
/// </summary>
public class StarSystemGenerator
{
    private readonly int _galaxySeed;
    
    public StarSystemGenerator(int galaxySeed)
    {
        _galaxySeed = galaxySeed;
    }
    
    /// <summary>
    /// Generate a complete solar system at given coordinates
    /// </summary>
    public SolarSystemData GenerateSystem(Vector3Int coordinates)
    {
        // Create deterministic seed from coordinates and galaxy seed
        int systemSeed = HashCoordinates(coordinates.X, coordinates.Y, coordinates.Z, _galaxySeed);
        var random = new Random(systemSeed);
        
        var system = new SolarSystemData(
            $"System-{coordinates.X}-{coordinates.Y}-{coordinates.Z}",
            coordinates,
            systemSeed
        );
        
        // Generate system name
        system.Name = GenerateSystemName(random);
        
        // Determine system type based on distance from center
        var distanceFromCenter = Math.Sqrt(coordinates.X * coordinates.X + 
                                          coordinates.Y * coordinates.Y + 
                                          coordinates.Z * coordinates.Z);
        system.Type = DetermineSystemType(distanceFromCenter, random);
        
        // Set danger level
        system.DangerLevel = CalculateDangerLevel(system.Type, distanceFromCenter, random);
        
        // Generate central star
        system.CentralStar = GenerateStar(random);
        
        // Generate planets
        int planetCount = random.Next(2, 8);
        for (int i = 0; i < planetCount; i++)
        {
            system.Planets.Add(GeneratePlanet(i, random));
        }
        
        // Generate asteroid belts
        int beltCount = system.Type == SystemType.AsteroidRich ? random.Next(2, 4) : random.Next(0, 2);
        for (int i = 0; i < beltCount; i++)
        {
            system.AsteroidBelts.Add(GenerateAsteroidBelt(i, planetCount, random));
        }
        
        // Generate space stations
        int stationCount = system.Type == SystemType.Civilized ? random.Next(3, 6) : random.Next(0, 3);
        for (int i = 0; i < stationCount; i++)
        {
            system.Stations.Add(GenerateStation(i, system.Planets, system.AsteroidBelts, random));
        }
        
        // System radius encompasses all objects
        system.Radius = CalculateSystemRadius(system);
        
        return system;
    }
    
    /// <summary>
    /// Add stargates to a system (called after galaxy network is created)
    /// </summary>
    public void AddStargatesToSystem(SolarSystemData system, List<string> connectedSystemIds)
    {
        var random = new Random(system.Seed);
        
        foreach (var destSystemId in connectedSystemIds)
        {
            var gate = new StargateData
            {
                GateId = $"{system.SystemId}-Gate-{destSystemId}",
                Name = $"Gate to {destSystemId}",
                Position = GenerateGatePosition(system, random),
                Rotation = new Vector3(0, (float)(random.NextDouble() * Math.PI * 2), 0),
                DestinationSystemId = destSystemId,
                Type = random.NextDouble() < 0.9 ? GateType.Standard : GateType.Ancient,
                IsActive = true
            };
            
            system.Stargates.Add(gate);
        }
    }
    
    private StarData GenerateStar(Random random)
    {
        var starTypes = Enum.GetValues<StarType>();
        var weights = new[] { 40, 35, 10, 8, 5, 2 }; // Distribution weights
        
        var starType = WeightedRandom(starTypes, weights, random);
        
        var size = starType switch
        {
            StarType.RedDwarf => 5000f + (float)random.NextDouble() * 3000f,
            StarType.YellowStar => 8000f + (float)random.NextDouble() * 4000f,
            StarType.BlueGiant => 15000f + (float)random.NextDouble() * 10000f,
            StarType.WhiteDwarf => 3000f + (float)random.NextDouble() * 2000f,
            StarType.Neutron => 2000f + (float)random.NextDouble() * 1000f,
            StarType.BlackHole => 1000f + (float)random.NextDouble() * 500f,
            _ => 8000f
        };
        
        return new StarData
        {
            Position = Vector3.Zero,
            Size = size,
            Type = starType,
            Name = $"{starType} Star"
        };
    }
    
    private PlanetData GeneratePlanet(int index, Random random)
    {
        var planetTypes = Enum.GetValues<PlanetType>();
        var planetType = planetTypes[random.Next(planetTypes.Length)];
        
        // Orbit radius increases with index
        float orbitRadius = 15000f + index * 10000f + (float)random.NextDouble() * 5000f;
        
        // Random position on orbit
        float angle = (float)(random.NextDouble() * Math.PI * 2);
        var position = new Vector3(
            (float)Math.Cos(angle) * orbitRadius,
            (float)(random.NextDouble() - 0.5f) * 2000f, // Slight vertical offset
            (float)Math.Sin(angle) * orbitRadius
        );
        
        float size = planetType switch
        {
            PlanetType.Gas => 4000f + (float)random.NextDouble() * 6000f,
            PlanetType.Rocky => 1000f + (float)random.NextDouble() * 2000f,
            PlanetType.Ice => 800f + (float)random.NextDouble() * 1500f,
            PlanetType.Lava => 1200f + (float)random.NextDouble() * 1800f,
            PlanetType.Desert => 1000f + (float)random.NextDouble() * 2000f,
            PlanetType.Ocean => 1500f + (float)random.NextDouble() * 2500f,
            PlanetType.Habitable => 1800f + (float)random.NextDouble() * 2200f,
            _ => 1500f
        };
        
        return new PlanetData
        {
            Name = $"Planet {(char)('A' + index)}",
            Position = position,
            Size = size,
            OrbitRadius = orbitRadius,
            Type = planetType
        };
    }
    
    private AsteroidBeltData GenerateAsteroidBelt(int index, int planetCount, Random random)
    {
        // Place belts between planet orbits
        float baseRadius = 15000f + (index + planetCount / 2) * 10000f;
        float width = 3000f + (float)random.NextDouble() * 7000f;
        
        var resources = new[] { "Iron", "Titanium", "Naonite", "Trinium", "Xanion" };
        
        return new AsteroidBeltData
        {
            Name = $"Belt {(char)('A' + index)}",
            Center = Vector3.Zero,
            InnerRadius = baseRadius - width / 2,
            OuterRadius = baseRadius + width / 2,
            Height = 1000f + (float)random.NextDouble() * 2000f,
            Density = 0.0001f + (float)random.NextDouble() * 0.0003f,
            PrimaryResource = resources[random.Next(resources.Length)]
        };
    }
    
    private StationData GenerateStation(int index, List<PlanetData> planets, 
        List<AsteroidBeltData> belts, Random random)
    {
        Vector3 position;
        string stationType;
        
        // Decide station location strategy
        float locationRoll = (float)random.NextDouble();
        
        if (locationRoll < 0.4f && planets.Count > 0)
        {
            // Near a planet
            var planet = planets[random.Next(planets.Count)];
            float offset = planet.Size * 2 + 1000f + (float)random.NextDouble() * 2000f;
            float angle = (float)(random.NextDouble() * Math.PI * 2);
            position = planet.Position + new Vector3(
                (float)Math.Cos(angle) * offset,
                (float)(random.NextDouble() - 0.5f) * 500f,
                (float)Math.Sin(angle) * offset
            );
            stationType = "Orbital";
        }
        else if (locationRoll < 0.7f && belts.Count > 0)
        {
            // In asteroid belt
            var belt = belts[random.Next(belts.Count)];
            float radius = belt.InnerRadius + (float)random.NextDouble() * (belt.OuterRadius - belt.InnerRadius);
            float angle = (float)(random.NextDouble() * Math.PI * 2);
            position = belt.Center + new Vector3(
                (float)Math.Cos(angle) * radius,
                (float)(random.NextDouble() - 0.5f) * belt.Height,
                (float)Math.Sin(angle) * radius
            );
            stationType = "Mining";
        }
        else
        {
            // Free floating
            float distance = 20000f + (float)random.NextDouble() * 40000f;
            float angle = (float)(random.NextDouble() * Math.PI * 2);
            float elevation = (float)((random.NextDouble() - 0.5f) * Math.PI / 4);
            position = new Vector3(
                (float)(Math.Cos(angle) * Math.Cos(elevation)) * distance,
                (float)Math.Sin(elevation) * distance,
                (float)(Math.Sin(angle) * Math.Cos(elevation)) * distance
            );
            stationType = random.NextDouble() < 0.5 ? "Trading" : "Military";
        }
        
        return new StationData
        {
            Position = position,
            StationType = stationType,
            Name = GenerateStationName(stationType, index, random)
        };
    }
    
    private Vector3 GenerateGatePosition(SolarSystemData system, Random random)
    {
        // Place gates at the edge of the system
        float distance = system.Radius * 0.9f;
        float angle = (float)(random.NextDouble() * Math.PI * 2);
        float elevation = (float)((random.NextDouble() - 0.5f) * Math.PI / 6); // ±30 degrees
        
        return new Vector3(
            (float)(Math.Cos(angle) * Math.Cos(elevation)) * distance,
            (float)Math.Sin(elevation) * distance,
            (float)(Math.Sin(angle) * Math.Cos(elevation)) * distance
        );
    }
    
    private SystemType DetermineSystemType(double distanceFromCenter, Random random)
    {
        if (distanceFromCenter < 5)
            return SystemType.Core;
        
        if (distanceFromCenter < 15)
        {
            return random.NextDouble() switch
            {
                < 0.4 => SystemType.Civilized,
                < 0.7 => SystemType.AsteroidRich,
                _ => SystemType.Frontier
            };
        }
        
        if (distanceFromCenter < 30)
        {
            return random.NextDouble() switch
            {
                < 0.3 => SystemType.Frontier,
                < 0.5 => SystemType.Contested,
                < 0.7 => SystemType.AsteroidRich,
                _ => SystemType.Empty
            };
        }
        
        // Far from center
        return random.NextDouble() < 0.7 ? SystemType.Empty : SystemType.Frontier;
    }
    
    private int CalculateDangerLevel(SystemType type, double distance, Random random)
    {
        int baseLevel = type switch
        {
            SystemType.Core => 8,
            SystemType.Civilized => 2,
            SystemType.Frontier => 4,
            SystemType.Contested => 7,
            SystemType.Empty => 1,
            SystemType.AsteroidRich => 3,
            SystemType.Nebula => 5,
            _ => 3
        };
        
        // Add distance modifier
        int distanceBonus = (int)(distance / 10);
        
        return Math.Clamp(baseLevel + distanceBonus + random.Next(-1, 2), 0, 10);
    }
    
    private float CalculateSystemRadius(SolarSystemData system)
    {
        float maxRadius = 50000f; // Minimum radius
        
        foreach (var planet in system.Planets)
        {
            maxRadius = Math.Max(maxRadius, planet.OrbitRadius + 5000f);
        }
        
        foreach (var belt in system.AsteroidBelts)
        {
            maxRadius = Math.Max(maxRadius, belt.OuterRadius + 5000f);
        }
        
        return maxRadius;
    }
    
    private string GenerateSystemName(Random random)
    {
        string[] prefixes = { "Alpha", "Beta", "Gamma", "Delta", "Epsilon", "Zeta", "Eta", "Theta" };
        string[] suffixes = { "Prime", "Secundus", "Tertius", "Minor", "Major", "Nova" };
        string[] names = { "Centauri", "Draconis", "Orionis", "Cygni", "Lyrae", "Aquilae" };
        
        return random.NextDouble() switch
        {
            < 0.5 => $"{prefixes[random.Next(prefixes.Length)]} {names[random.Next(names.Length)]}",
            < 0.8 => $"{names[random.Next(names.Length)]} {suffixes[random.Next(suffixes.Length)]}",
            _ => $"{prefixes[random.Next(prefixes.Length)]}-{random.Next(100, 999)}"
        };
    }
    
    private string GenerateStationName(string type, int index, Random random)
    {
        string[] prefixes = { "Station", "Outpost", "Base", "Hub", "Terminal", "Platform" };
        string[] designations = { "Alpha", "Beta", "Gamma", "Delta" };
        
        return $"{type} {prefixes[random.Next(prefixes.Length)]} {designations[Math.Min(index, 3)]}";
    }
    
    private int HashCoordinates(int x, int y, int z, int seed)
    {
        unchecked
        {
            int hash = seed;
            hash = hash * 397 ^ x;
            hash = hash * 397 ^ y;
            hash = hash * 397 ^ z;
            return hash;
        }
    }
    
    private T WeightedRandom<T>(T[] items, int[] weights, Random random)
    {
        int totalWeight = weights.Sum();
        int randomValue = random.Next(totalWeight);
        int cumulativeWeight = 0;
        
        for (int i = 0; i < items.Length; i++)
        {
            cumulativeWeight += weights[i];
            if (randomValue < cumulativeWeight)
                return items[i];
        }
        
        return items[^1];
    }
}
