using System.Numerics;
using AvorionLike.Core.Voxel;
using AvorionLike.Core.ECS;
using AvorionLike.Core.Navigation;

namespace AvorionLike.Core.Procedural;

/// <summary>
/// Procedural galaxy generator - Enhanced with massive stations and claimable asteroids
/// </summary>
public class GalaxyGenerator
{
    private readonly Random _random;
    private readonly int _seed;
    private readonly ProceduralStationGenerator _stationGenerator;
    private readonly MassiveAsteroidGenerator _massiveAsteroidGenerator;

    public GalaxyGenerator(int seed = 0)
    {
        _seed = seed == 0 ? Environment.TickCount : seed;
        _random = new Random(_seed);
        _stationGenerator = new ProceduralStationGenerator(_seed);
        _massiveAsteroidGenerator = new MassiveAsteroidGenerator(_seed);
    }

    /// <summary>
    /// Generate a galaxy sector with massive stations and rare claimable asteroids
    /// </summary>
    public GalaxySector GenerateSector(int x, int y, int z, EntityManager? entityManager = null)
    {
        // Use sector coordinates as seed for consistent generation
        var sectorSeed = HashCoordinates(x, y, z, _seed);
        var sectorRandom = new Random(sectorSeed);

        var sector = new GalaxySector(x, y, z);
        
        // Generate asteroids
        int asteroidCount = sectorRandom.Next(5, 20);
        for (int i = 0; i < asteroidCount; i++)
        {
            var position = new Vector3(
                (float)sectorRandom.NextDouble() * 10000 - 5000,
                (float)sectorRandom.NextDouble() * 10000 - 5000,
                (float)sectorRandom.NextDouble() * 10000 - 5000
            );
            
            sector.Asteroids.Add(new AsteroidData
            {
                Position = position,
                Size = (float)sectorRandom.NextDouble() * 50 + 10,
                ResourceType = GetRandomResourceType(sectorRandom)
            });
        }
        
        // 5% chance for a massive claimable asteroid (only if system hasn't been generated before)
        if (MassiveAsteroidGenerator.ShouldSpawnMassiveAsteroid(sectorRandom))
        {
            var massiveAsteroidConfig = new MassiveAsteroidConfig
            {
                Type = (MassiveAsteroidType)sectorRandom.Next(Enum.GetValues<MassiveAsteroidType>().Length),
                Seed = sectorSeed + 1000,
                MinSize = 2000f,
                MaxSize = 5000f
            };
            
            var massiveAsteroid = _massiveAsteroidGenerator.GenerateAsteroid(massiveAsteroidConfig);
            
            // Position far from center for dramatic effect
            Vector3 asteroidPosition = new Vector3(
                (float)(sectorRandom.NextDouble() * 8000 - 4000),
                (float)(sectorRandom.NextDouble() * 8000 - 4000),
                (float)(sectorRandom.NextDouble() * 8000 - 4000)
            );
            
            sector.MassiveAsteroid = new MassiveAsteroidData
            {
                Position = asteroidPosition,
                Type = massiveAsteroidConfig.Type,
                BlockCount = massiveAsteroid.BlockCount,
                LandingZone = massiveAsteroid.LandingZone,
                DockingPoints = massiveAsteroid.DockingPoints
            };
            
            // If EntityManager is provided, create the actual entity
            if (entityManager != null)
            {
                var asteroidEntity = entityManager.CreateEntity($"Massive {massiveAsteroidConfig.Type} Asteroid");
                
                // Add voxel structure
                entityManager.AddComponent(asteroidEntity.Id, massiveAsteroid.Structure);
                
                // Add hub component for claiming
                var hubComponent = new AsteroidHubComponent
                {
                    EntityId = asteroidEntity.Id,
                    HubName = $"Unclaimed {massiveAsteroidConfig.Type}"
                };
                entityManager.AddComponent(asteroidEntity.Id, hubComponent);
                
                sector.MassiveAsteroid.EntityId = asteroidEntity.Id;
            }
        }

        // Potentially generate a massive station (20% chance)
        if (sectorRandom.NextDouble() < 0.2)
        {
            string stationType = GetRandomStationType(sectorRandom);
            
            // Generate station using new massive station generator
            var stationConfig = new StationGenerationConfig
            {
                Size = (StationSize)sectorRandom.Next(Enum.GetValues<StationSize>().Length),
                StationType = stationType,
                Material = GetStationMaterial(sectorRandom),
                Architecture = (StationArchitecture)sectorRandom.Next(Enum.GetValues<StationArchitecture>().Length),
                Seed = sectorSeed + 500,
                IncludeDockingBays = true,
                MinDockingBays = 4 + sectorRandom.Next(8)
            };
            
            var generatedStation = _stationGenerator.GenerateStation(stationConfig);
            
            sector.Station = new StationData
            {
                Position = new Vector3(0, 0, 0),
                StationType = stationType,
                Name = GenerateStationName(sectorRandom),
                BlockCount = generatedStation.BlockCount,
                DockingPoints = generatedStation.DockingPoints,
                Facilities = generatedStation.Facilities
            };
            
            // If EntityManager is provided, create the actual entity
            if (entityManager != null)
            {
                var stationEntity = entityManager.CreateEntity(sector.Station.Name);
                
                // Add voxel structure
                entityManager.AddComponent(stationEntity.Id, generatedStation.Structure);
                
                // Add station-specific components based on type
                if (stationType.ToLower() == "refinery")
                {
                    var refineryComponent = new Station.RefineryComponent
                    {
                        EntityId = stationEntity.Id,
                        MaxConcurrentOrders = 10 + sectorRandom.Next(10),
                        ProcessingSpeedMultiplier = 0.8f + (float)sectorRandom.NextDouble() * 0.4f,  // 0.8-1.2x
                        MaxStoragePerType = 5000 + sectorRandom.Next(5000)
                    };
                    entityManager.AddComponent(stationEntity.Id, refineryComponent);
                }
                
                // Add captain roster for all stations
                var captainRoster = new Station.StationCaptainRosterComponent
                {
                    EntityId = stationEntity.Id
                };
                captainRoster.RefreshRoster(stationType, sectorRandom);
                entityManager.AddComponent(stationEntity.Id, captainRoster);
                
                sector.Station.EntityId = stationEntity.Id;
            }
        }
        
        // Generate wormholes (5% chance for wandering, static wormholes in specific sectors)
        if (sectorRandom.NextDouble() < 0.05)
        {
            // Wandering wormhole
            var wormholeClass = (WormholeClass)sectorRandom.Next(1, 7);
            var destX = sectorRandom.Next(-500, 500);
            var destY = sectorRandom.Next(-500, 500);
            var destZ = sectorRandom.Next(-500, 500);
            
            var wormholeData = new WormholeData
            {
                Position = new Vector3(
                    (float)sectorRandom.NextDouble() * 10000 - 5000,
                    (float)sectorRandom.NextDouble() * 10000 - 5000,
                    (float)sectorRandom.NextDouble() * 10000 - 5000
                ),
                Designation = GenerateWormholeDesignation(sectorRandom),
                WormholeClass = wormholeClass.ToString(),
                Type = "Wandering",
                DestinationSector = new Vector3(destX, destY, destZ)
            };
            
            sector.Wormholes.Add(wormholeData);
            
            // If EntityManager is provided, create the actual wormhole entity
            if (entityManager != null)
            {
                var wormholeEntity = entityManager.CreateEntity($"Wormhole {wormholeData.Designation}");
                
                var wormhole = new WormholeComponent
                {
                    EntityId = wormholeEntity.Id,
                    Class = wormholeClass,
                    Type = WormholeType.Wandering,
                    SourceSector = new Vector3(x, y, z),
                    DestinationSector = wormholeData.DestinationSector,
                    Position = wormholeData.Position,
                    Designation = wormholeData.Designation,
                    RemainingLifetime = GetWormholeLifetime(wormholeClass),
                    MaxLifetime = GetWormholeLifetime(wormholeClass),
                    RemainingMass = GetWormholeMass(wormholeClass),
                    MaxTotalMass = GetWormholeMass(wormholeClass),
                    MaxShipMass = GetWormholeMaxShipMass(wormholeClass)
                };
                
                entityManager.AddComponent(wormholeEntity.Id, wormhole);
                wormholeData.EntityId = wormholeEntity.Id;
            }
        }

        return sector;
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

    private string GetRandomResourceType(Random random)
    {
        string[] types = { "Iron", "Titanium", "Naonite", "Trinium", "Xanion", "Ogonite", "Avorion" };
        return types[random.Next(types.Length)];
    }

    private string GetRandomStationType(Random random)
    {
        string[] types = { "Trading", "Military", "Mining", "Shipyard", "Research", "Refinery" };
        return types[random.Next(types.Length)];
    }
    
    private string GetStationMaterial(Random random)
    {
        string[] materials = { "Titanium", "Naonite", "Trinium" };
        return materials[random.Next(materials.Length)];
    }

    private string GenerateStationName(Random random)
    {
        string[] prefixes = { "Alpha", "Beta", "Gamma", "Delta", "Epsilon", "Zeta", "Sigma", "Omega", "Nova", "Stellar" };
        string[] suffixes = { "Outpost", "Station", "Base", "Hub", "Terminal", "Complex", "Nexus", "Citadel" };
        
        return $"{prefixes[random.Next(prefixes.Length)]} {suffixes[random.Next(suffixes.Length)]}";
    }
    
    private string GenerateWormholeDesignation(Random random)
    {
        char letter = (char)('A' + random.Next(26));
        int number = random.Next(100, 999);
        return $"{letter}{number}";
    }
    
    private float GetWormholeLifetime(WormholeClass whClass)
    {
        return whClass switch
        {
            WormholeClass.Class1 => 86400f,   // 24 hours
            WormholeClass.Class2 => 129600f,  // 36 hours
            WormholeClass.Class3 => 172800f,  // 48 hours
            WormholeClass.Class4 => 172800f,  // 48 hours
            WormholeClass.Class5 => 86400f,   // 24 hours
            WormholeClass.Class6 => 64800f,   // 18 hours
            _ => 172800f
        };
    }
    
    private float GetWormholeMass(WormholeClass whClass)
    {
        return whClass switch
        {
            WormholeClass.Class1 => 5000000000f,
            WormholeClass.Class2 => 3000000000f,
            WormholeClass.Class3 => 2000000000f,
            WormholeClass.Class4 => 1500000000f,
            WormholeClass.Class5 => 1000000000f,
            WormholeClass.Class6 => 750000000f,
            _ => 2000000000f
        };
    }
    
    private float GetWormholeMaxShipMass(WormholeClass whClass)
    {
        return whClass switch
        {
            WormholeClass.Class1 => 200000000f,
            WormholeClass.Class2 => 300000000f,
            WormholeClass.Class3 => 300000000f,
            WormholeClass.Class4 => 180000000f,
            WormholeClass.Class5 => 180000000f,
            WormholeClass.Class6 => 135000000f,
            _ => 300000000f
        };
    }
}

/// <summary>
/// Represents a sector in the galaxy
/// </summary>
public class GalaxySector
{
    public int X { get; set; }
    public int Y { get; set; }
    public int Z { get; set; }
    public List<AsteroidData> Asteroids { get; set; } = new();
    public StationData? Station { get; set; }
    public MassiveAsteroidData? MassiveAsteroid { get; set; }  // NEW: Rare claimable asteroid
    public List<ShipData> Ships { get; set; } = new();
    public List<WormholeData> Wormholes { get; set; } = new();  // NEW: Wormhole connections

    public GalaxySector(int x, int y, int z)
    {
        X = x;
        Y = y;
        Z = z;
    }
}

public class AsteroidData
{
    public Vector3 Position { get; set; }
    public float Size { get; set; }
    public string ResourceType { get; set; } = "Iron";
}

public class StationData
{
    public Vector3 Position { get; set; }
    public string StationType { get; set; } = "Trading";
    public string Name { get; set; } = "Unknown Station";
    public int BlockCount { get; set; }  // NEW: Track block count
    public List<Vector3> DockingPoints { get; set; } = new();  // NEW: Docking locations
    public List<string> Facilities { get; set; } = new();  // NEW: Available facilities
    public Guid EntityId { get; set; }  // NEW: Link to actual entity
}

/// <summary>
/// Data for massive claimable asteroids
/// </summary>
public class MassiveAsteroidData
{
    public Vector3 Position { get; set; }
    public MassiveAsteroidType Type { get; set; }
    public int BlockCount { get; set; }
    public Vector3 LandingZone { get; set; }
    public List<Vector3> DockingPoints { get; set; } = new();
    public Guid EntityId { get; set; }
}

public class ShipData
{
    public Vector3 Position { get; set; }
    public string ShipType { get; set; } = "Fighter";
    public string Faction { get; set; } = "Neutral";
}

/// <summary>
/// Data for wormhole connections in a sector
/// </summary>
public class WormholeData
{
    public Vector3 Position { get; set; }
    public string Designation { get; set; } = "Unknown";
    public string WormholeClass { get; set; } = "Class1";
    public string Type { get; set; } = "Wandering";
    public Vector3 DestinationSector { get; set; }
    public Guid EntityId { get; set; }
}
