using System.Numerics;
using AvorionLike.Core.Logging;

namespace AvorionLike.Core.Procedural;

/// <summary>
/// Enhanced solar system generator that creates large-scale, Avorion-style clustered sectors
/// Each system is a complete playable environment with all game features
/// </summary>
public class EnhancedSolarSystemGenerator
{
    private readonly StarSystemGenerator _baseGenerator;
    private readonly Logger _logger = Logger.Instance;
    
    // Avorion-style sector dimensions (very large for proper scale)
    private const float SECTOR_SIZE = 200000f;  // 200km per sector
    private const float CLUSTER_DENSITY = 0.0002f;  // Asteroids per cubic unit
    
    public EnhancedSolarSystemGenerator(int galaxySeed)
    {
        _baseGenerator = new StarSystemGenerator(galaxySeed);
    }
    
    /// <summary>
    /// Generate a complete playable solar system with Avorion-style scale and clustering
    /// </summary>
    public PlayableSolarSystem GeneratePlayableSystem(Vector3Int coordinates)
    {
        _logger.Info("EnhancedSystemGen", $"Generating playable system at {coordinates}");
        
        // Use base generator for initial system structure
        var baseSystem = _baseGenerator.GenerateSystem(coordinates);
        var random = new Random(baseSystem.Seed);
        
        var playableSystem = new PlayableSolarSystem
        {
            BaseSystem = baseSystem,
            SystemId = baseSystem.SystemId,
            Coordinates = coordinates,
            Seed = baseSystem.Seed,
            Scale = SECTOR_SIZE
        };
        
        // Add spawn zone (safe starting area)
        playableSystem.SpawnZone = GenerateSpawnZone(baseSystem, random);
        
        // Add dense asteroid clusters (Avorion-style resource zones)
        playableSystem.AsteroidClusters = GenerateAsteroidClusters(baseSystem, random);
        
        // Add resource extraction zones
        playableSystem.ResourceZones = GenerateResourceZones(baseSystem, random);
        
        // Add ship building facilities
        playableSystem.BuildingFacilities = GenerateShipyards(baseSystem, random);
        
        // Add trade routes and economic zones
        playableSystem.TradeRoutes = GenerateTradeRoutes(baseSystem, random);
        
        // Add combat zones and patrol routes
        playableSystem.CombatZones = GenerateCombatZones(baseSystem, random);
        
        // Add wreckage fields for salvage
        playableSystem.WreckageFields = GenerateWreckageFields(baseSystem, random);
        
        // Add exploration points of interest
        playableSystem.PointsOfInterest = GeneratePointsOfInterest(baseSystem, random);
        
        _logger.Info("EnhancedSystemGen", 
            $"Generated system with {playableSystem.AsteroidClusters.Count} clusters, " +
            $"{playableSystem.BuildingFacilities.Count} shipyards, " +
            $"{playableSystem.ResourceZones.Count} resource zones");
        
        return playableSystem;
    }
    
    /// <summary>
    /// Generate a safe spawn zone where players start
    /// </summary>
    private SpawnZoneData GenerateSpawnZone(SolarSystemData system, Random random)
    {
        // Find a safe location near a civilized station or planet
        Vector3 spawnPosition = Vector3.Zero;
        
        if (system.Stations.Count > 0)
        {
            // Near the first major station
            var station = system.Stations.First(s => s.StationType == "Trading" || s.StationType == "Orbital");
            if (station != null)
            {
                spawnPosition = station.Position + new Vector3(2000, 0, 2000);
            }
        }
        else if (system.Planets.Count > 0)
        {
            // Near the innermost habitable planet
            var planet = system.Planets.FirstOrDefault(p => p.Type == PlanetType.Habitable) 
                         ?? system.Planets[0];
            float offset = planet.Size * 3;
            spawnPosition = planet.Position + new Vector3(offset, 0, 0);
        }
        else
        {
            // Middle of system
            spawnPosition = new Vector3(10000, 0, 10000);
        }
        
        return new SpawnZoneData
        {
            Position = spawnPosition,
            Radius = 5000f,
            Description = "Safe Zone - Protected Area",
            HasStartingStation = true,
            StartingStationPosition = spawnPosition + new Vector3(1000, 500, 0)
        };
    }
    
    /// <summary>
    /// Generate dense asteroid clusters (Avorion-style concentrated resource areas)
    /// </summary>
    private List<AsteroidClusterData> GenerateAsteroidClusters(SolarSystemData system, Random random)
    {
        var clusters = new List<AsteroidClusterData>();
        
        // Create clusters in asteroid belts
        foreach (var belt in system.AsteroidBelts)
        {
            int clustersInBelt = 5 + random.Next(10); // 5-14 clusters per belt
            
            for (int i = 0; i < clustersInBelt; i++)
            {
                float radius = belt.InnerRadius + (float)random.NextDouble() * (belt.OuterRadius - belt.InnerRadius);
                float angle = (float)(random.NextDouble() * Math.PI * 2);
                
                var clusterCenter = new Vector3(
                    (float)Math.Cos(angle) * radius,
                    (float)(random.NextDouble() - 0.5f) * belt.Height * 0.5f,
                    (float)Math.Sin(angle) * radius
                );
                
                var cluster = new AsteroidClusterData
                {
                    Center = clusterCenter,
                    Radius = 2000f + (float)random.NextDouble() * 3000f,  // 2-5km clusters
                    Density = CLUSTER_DENSITY * (1.5f + (float)random.NextDouble()),  // Varied density
                    PrimaryResource = belt.PrimaryResource,
                    SecondaryResources = GetSecondaryResources(belt.PrimaryResource, random),
                    AsteroidCount = 100 + random.Next(200),  // 100-300 asteroids per cluster
                    ClusterShape = (ClusterShape)random.Next(Enum.GetValues<ClusterShape>().Length)
                };
                
                clusters.Add(cluster);
            }
        }
        
        // Add free-floating clusters (rogue asteroids)
        int freeFloatingClusters = 3 + random.Next(5);  // 3-7 free clusters
        for (int i = 0; i < freeFloatingClusters; i++)
        {
            float distance = 30000f + (float)random.NextDouble() * 60000f;
            float angle = (float)(random.NextDouble() * Math.PI * 2);
            float elevation = (float)((random.NextDouble() - 0.5f) * Math.PI / 4);
            
            var clusterCenter = new Vector3(
                (float)(Math.Cos(angle) * Math.Cos(elevation)) * distance,
                (float)Math.Sin(elevation) * distance,
                (float)(Math.Sin(angle) * Math.Cos(elevation)) * distance
            );
            
            var resources = new[] { "Iron", "Titanium", "Crystal", "Platinum" };
            
            clusters.Add(new AsteroidClusterData
            {
                Center = clusterCenter,
                Radius = 1500f + (float)random.NextDouble() * 2500f,
                Density = CLUSTER_DENSITY * 0.8f,
                PrimaryResource = resources[random.Next(resources.Length)],
                SecondaryResources = new List<string>(),
                AsteroidCount = 50 + random.Next(100),
                ClusterShape = ClusterShape.Scattered
            });
        }
        
        return clusters;
    }
    
    /// <summary>
    /// Generate resource extraction zones (mining hotspots)
    /// </summary>
    private List<ResourceZoneData> GenerateResourceZones(SolarSystemData system, Random random)
    {
        var zones = new List<ResourceZoneData>();
        
        var resourceTiers = new[]
        {
            ("Iron", 1),
            ("Titanium", 2),
            ("Naonite", 3),
            ("Trinium", 4),
            ("Xanion", 5),
            ("Ogonite", 6),
            ("Avorion", 7)
        };
        
        // Determine which resources are available based on system danger level
        var availableResources = resourceTiers
            .Where(r => r.Item2 <= system.DangerLevel + 2)
            .ToList();
        
        // Create 2-4 specialized resource zones
        int zoneCount = 2 + random.Next(3);
        for (int i = 0; i < zoneCount && availableResources.Count > 0; i++)
        {
            var resource = availableResources[random.Next(availableResources.Count)];
            
            float distance = 25000f + (float)random.NextDouble() * 40000f;
            float angle = (float)(random.NextDouble() * Math.PI * 2);
            
            zones.Add(new ResourceZoneData
            {
                Name = $"{resource.Item1} Rich Zone",
                Center = new Vector3(
                    (float)Math.Cos(angle) * distance,
                    (float)(random.NextDouble() - 0.5f) * 5000f,
                    (float)Math.Sin(angle) * distance
                ),
                Radius = 8000f + (float)random.NextDouble() * 7000f,
                PrimaryResource = resource.Item1,
                ResourceTier = resource.Item2,
                ExtractionRate = 1.5f + (float)random.NextDouble(),  // Bonus extraction
                DangerLevel = Math.Max(1, resource.Item2 - 1)
            });
        }
        
        return zones;
    }
    
    /// <summary>
    /// Generate shipyard and construction facilities
    /// </summary>
    private List<ShipBuildingFacility> GenerateShipyards(SolarSystemData system, Random random)
    {
        var facilities = new List<ShipBuildingFacility>();
        
        // Add shipyard at each station
        foreach (var station in system.Stations)
        {
            var facilityType = station.StationType == "Trading" 
                ? ShipyardType.Civilian 
                : station.StationType == "Military" 
                    ? ShipyardType.Military 
                    : ShipyardType.Industrial;
            
            facilities.Add(new ShipBuildingFacility
            {
                Name = $"{station.Name} Shipyard",
                Position = station.Position,
                Type = facilityType,
                MaxShipSize = facilityType switch
                {
                    ShipyardType.Military => ShipSize.Battleship,
                    ShipyardType.Industrial => ShipSize.Carrier,
                    _ => ShipSize.Cruiser
                },
                ModulesAvailable = GetAvailableModules(facilityType, system.DangerLevel),
                HasTurretShop = true,
                HasModuleSwapBay = true
            });
        }
        
        // Add at least one shipyard in spawn zone if none exist
        if (facilities.Count == 0)
        {
            facilities.Add(new ShipBuildingFacility
            {
                Name = "Starter Shipyard",
                Position = new Vector3(5000, 0, 5000),
                Type = ShipyardType.Civilian,
                MaxShipSize = ShipSize.Frigate,
                ModulesAvailable = new List<ModuleType> 
                { 
                    ModuleType.Cockpit, ModuleType.CoreHull, ModuleType.EngineNacelle,
                    ModuleType.WeaponMount, ModuleType.CargoContainer
                },
                HasTurretShop = true,
                HasModuleSwapBay = true
            });
        }
        
        return facilities;
    }
    
    /// <summary>
    /// Generate trade routes between stations
    /// </summary>
    private List<TradeRouteData> GenerateTradeRoutes(SolarSystemData system, Random random)
    {
        var routes = new List<TradeRouteData>();
        
        if (system.Stations.Count < 2)
            return routes;
        
        // Create routes between stations
        for (int i = 0; i < system.Stations.Count - 1; i++)
        {
            for (int j = i + 1; j < system.Stations.Count; j++)
            {
                if (random.NextDouble() < 0.6)  // 60% chance of connection
                {
                    routes.Add(new TradeRouteData
                    {
                        StartStation = system.Stations[i].Name,
                        EndStation = system.Stations[j].Name,
                        StartPosition = system.Stations[i].Position,
                        EndPosition = system.Stations[j].Position,
                        TrafficDensity = (float)random.NextDouble(),
                        ConvoyFrequency = 60f + (float)random.NextDouble() * 180f  // 1-4 minutes
                    });
                }
            }
        }
        
        return routes;
    }
    
    /// <summary>
    /// Generate combat zones and hostile territories
    /// </summary>
    private List<CombatZoneData> GenerateCombatZones(SolarSystemData system, Random random)
    {
        var zones = new List<CombatZoneData>();
        
        // More combat zones in contested/dangerous systems
        int zoneCount = system.Type switch
        {
            SystemType.Contested => 4 + random.Next(3),
            SystemType.Core => 3 + random.Next(3),
            SystemType.Frontier => 1 + random.Next(2),
            _ => 0
        };
        
        for (int i = 0; i < zoneCount; i++)
        {
            float distance = 40000f + (float)random.NextDouble() * 50000f;
            float angle = (float)(random.NextDouble() * Math.PI * 2);
            
            zones.Add(new CombatZoneData
            {
                Name = $"Hostile Zone {(char)('A' + i)}",
                Center = new Vector3(
                    (float)Math.Cos(angle) * distance,
                    (float)(random.NextDouble() - 0.5f) * 10000f,
                    (float)Math.Sin(angle) * distance
                ),
                Radius = 10000f + (float)random.NextDouble() * 15000f,
                DangerLevel = system.DangerLevel + random.Next(-1, 2),
                HostileFaction = GetRandomFaction(random),
                PatrolShipCount = 2 + random.Next(5)
            });
        }
        
        return zones;
    }
    
    /// <summary>
    /// Generate wreckage fields for salvage operations
    /// </summary>
    private List<WreckageFieldData> GenerateWreckageFields(SolarSystemData system, Random random)
    {
        var fields = new List<WreckageFieldData>();
        
        int fieldCount = 1 + random.Next(3);  // 1-3 wreckage fields
        
        for (int i = 0; i < fieldCount; i++)
        {
            float distance = 20000f + (float)random.NextDouble() * 60000f;
            float angle = (float)(random.NextDouble() * Math.PI * 2);
            
            fields.Add(new WreckageFieldData
            {
                Name = $"Wreckage Field {(char)('A' + i)}",
                Center = new Vector3(
                    (float)Math.Cos(angle) * distance,
                    (float)(random.NextDouble() - 0.5f) * 8000f,
                    (float)Math.Sin(angle) * distance
                ),
                Radius = 5000f + (float)random.NextDouble() * 5000f,
                WreckCount = 10 + random.Next(20),
                SalvageValue = (float)random.NextDouble() * system.DangerLevel
            });
        }
        
        return fields;
    }
    
    /// <summary>
    /// Generate points of interest for exploration
    /// </summary>
    private List<PointOfInterestData> GeneratePointsOfInterest(SolarSystemData system, Random random)
    {
        var pois = new List<PointOfInterestData>();
        
        var poiTypes = new[]
        {
            ("Ancient Artifact", "Mysterious alien structure"),
            ("Research Station", "Abandoned science facility"),
            ("Derelict Freighter", "Large abandoned cargo ship"),
            ("Anomaly", "Spatial phenomenon"),
            ("Hidden Cache", "Secret supply stash")
        };
        
        int poiCount = 2 + random.Next(4);  // 2-5 POIs
        
        for (int i = 0; i < poiCount; i++)
        {
            var poiType = poiTypes[random.Next(poiTypes.Length)];
            float distance = 30000f + (float)random.NextDouble() * 70000f;
            float angle = (float)(random.NextDouble() * Math.PI * 2);
            
            pois.Add(new PointOfInterestData
            {
                Name = poiType.Item1,
                Description = poiType.Item2,
                Position = new Vector3(
                    (float)Math.Cos(angle) * distance,
                    (float)(random.NextDouble() - 0.5f) * 15000f,
                    (float)Math.Sin(angle) * distance
                ),
                RequiresExploration = true,
                RewardTier = random.Next(1, system.DangerLevel + 1)
            });
        }
        
        return pois;
    }
    
    // Helper methods
    
    private List<string> GetSecondaryResources(string primary, Random random)
    {
        var allResources = new[] { "Iron", "Titanium", "Naonite", "Trinium", "Xanion", "Crystal", "Platinum" };
        return allResources.Where(r => r != primary).OrderBy(x => random.Next()).Take(2).ToList();
    }
    
    private List<ModuleType> GetAvailableModules(ShipyardType type, int dangerLevel)
    {
        var basicModules = new List<ModuleType>
        {
            ModuleType.Cockpit, ModuleType.CoreHull, ModuleType.EngineNacelle,
            ModuleType.CargoContainer, ModuleType.WeaponMount, ModuleType.Wing
        };
        
        if (type == ShipyardType.Military || dangerLevel >= 5)
        {
            basicModules.AddRange(new[]
            {
                ModuleType.ShieldEmitter, ModuleType.PowerCore, ModuleType.ThrusterArray
            });
        }
        
        if (type == ShipyardType.Industrial)
        {
            basicModules.Add(ModuleType.CargoContainer);
        }
        
        return basicModules;
    }
    
    private string GetRandomFaction(Random random)
    {
        var factions = new[] { "Pirates", "Xsotan", "Raiders", "Smugglers", "Mercenaries" };
        return factions[random.Next(factions.Length)];
    }
}

/// <summary>
/// Complete playable solar system with all gameplay features
/// </summary>
public class PlayableSolarSystem
{
    public SolarSystemData BaseSystem { get; set; } = null!;
    public string SystemId { get; set; } = "";
    public Vector3Int Coordinates { get; set; }
    public int Seed { get; set; }
    public float Scale { get; set; }
    
    // Gameplay zones
    public SpawnZoneData SpawnZone { get; set; } = null!;
    public List<AsteroidClusterData> AsteroidClusters { get; set; } = new();
    public List<ResourceZoneData> ResourceZones { get; set; } = new();
    public List<ShipBuildingFacility> BuildingFacilities { get; set; } = new();
    public List<TradeRouteData> TradeRoutes { get; set; } = new();
    public List<CombatZoneData> CombatZones { get; set; } = new();
    public List<WreckageFieldData> WreckageFields { get; set; } = new();
    public List<PointOfInterestData> PointsOfInterest { get; set; } = new();
}

public class SpawnZoneData
{
    public Vector3 Position { get; set; }
    public float Radius { get; set; }
    public string Description { get; set; } = "";
    public bool HasStartingStation { get; set; }
    public Vector3 StartingStationPosition { get; set; }
}

public class AsteroidClusterData
{
    public Vector3 Center { get; set; }
    public float Radius { get; set; }
    public float Density { get; set; }
    public string PrimaryResource { get; set; } = "";
    public List<string> SecondaryResources { get; set; } = new();
    public int AsteroidCount { get; set; }
    public ClusterShape ClusterShape { get; set; }
}

public enum ClusterShape
{
    Spherical,      // Roughly spherical cluster
    Disc,           // Flat disc of asteroids
    Ring,           // Ring shape
    Scattered,      // Loosely scattered
    Dense           // Tightly packed
}

public class ResourceZoneData
{
    public string Name { get; set; } = "";
    public Vector3 Center { get; set; }
    public float Radius { get; set; }
    public string PrimaryResource { get; set; } = "";
    public int ResourceTier { get; set; }
    public float ExtractionRate { get; set; }
    public int DangerLevel { get; set; }
}

public class ShipBuildingFacility
{
    public string Name { get; set; } = "";
    public Vector3 Position { get; set; }
    public ShipyardType Type { get; set; }
    public ShipSize MaxShipSize { get; set; }
    public List<ModuleType> ModulesAvailable { get; set; } = new();
    public bool HasTurretShop { get; set; }
    public bool HasModuleSwapBay { get; set; }
}

public enum ShipyardType
{
    Civilian,       // Basic modules, small ships
    Military,       // Combat modules, military ships
    Industrial      // Cargo and mining modules, large ships
}

public class TradeRouteData
{
    public string StartStation { get; set; } = "";
    public string EndStation { get; set; } = "";
    public Vector3 StartPosition { get; set; }
    public Vector3 EndPosition { get; set; }
    public float TrafficDensity { get; set; }
    public float ConvoyFrequency { get; set; }
}

public class CombatZoneData
{
    public string Name { get; set; } = "";
    public Vector3 Center { get; set; }
    public float Radius { get; set; }
    public int DangerLevel { get; set; }
    public string HostileFaction { get; set; } = "";
    public int PatrolShipCount { get; set; }
}

public class WreckageFieldData
{
    public string Name { get; set; } = "";
    public Vector3 Center { get; set; }
    public float Radius { get; set; }
    public int WreckCount { get; set; }
    public float SalvageValue { get; set; }
}

public class PointOfInterestData
{
    public string Name { get; set; } = "";
    public string Description { get; set; } = "";
    public Vector3 Position { get; set; }
    public bool RequiresExploration { get; set; }
    public int RewardTier { get; set; }
}
