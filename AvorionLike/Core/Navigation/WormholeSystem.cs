using System.Numerics;
using AvorionLike.Core.ECS;
using AvorionLike.Core.Logging;

namespace AvorionLike.Core.Navigation;

/// <summary>
/// System managing wormhole lifecycle, spawning, and traversal
/// Implements EVE Online-inspired dynamic wormhole topology
/// </summary>
public class WormholeSystem : SystemBase
{
    private readonly EntityManager _entityManager;
    private readonly Random _random;
    private readonly Dictionary<Vector3, List<WormholeComponent>> _sectorWormholes = new();
    
    // Configuration
    private const float WormholeSpawnCheckInterval = 600f; // Check every 10 minutes
    private const float WormholeSpawnChance = 0.15f; // 15% chance per check
    private float _timeSinceLastSpawnCheck = 0f;
    
    public WormholeSystem(EntityManager entityManager, int seed = 0) : base("WormholeSystem")
    {
        _entityManager = entityManager;
        _random = seed == 0 ? new Random() : new Random(seed);
    }
    
    public override void Update(float deltaTime)
    {
        // Update existing wormholes
        var wormholes = _entityManager.GetAllComponents<WormholeComponent>();
        foreach (var wormhole in wormholes)
        {
            UpdateWormhole(wormhole, deltaTime);
        }
        
        // Periodically check for new wormhole spawns
        _timeSinceLastSpawnCheck += deltaTime;
        if (_timeSinceLastSpawnCheck >= WormholeSpawnCheckInterval)
        {
            _timeSinceLastSpawnCheck = 0f;
            CheckForNewWormholeSpawns();
        }
    }
    
    /// <summary>
    /// Update a single wormhole's state
    /// </summary>
    private void UpdateWormhole(WormholeComponent wormhole, float deltaTime)
    {
        if (!wormhole.IsActive)
            return;
            
        // Age the wormhole
        wormhole.Age(deltaTime);
        
        // If collapsed, remove it
        if (wormhole.Stability == WormholeStability.Collapsed)
        {
            CollapseWormhole(wormhole);
        }
    }
    
    /// <summary>
    /// Collapse and remove a wormhole
    /// </summary>
    private void CollapseWormhole(WormholeComponent wormhole)
    {
        Logger.Instance.Info("WormholeSystem", $"Wormhole {wormhole.Designation} has collapsed");
        
        // Remove from sector tracking
        if (_sectorWormholes.TryGetValue(wormhole.SourceSector, out var wormholes))
        {
            wormholes.Remove(wormhole);
        }
        
        // Destroy the entity
        _entityManager.DestroyEntity(wormhole.EntityId);
    }
    
    /// <summary>
    /// Check if new wandering wormholes should spawn
    /// </summary>
    private void CheckForNewWormholeSpawns()
    {
        // Get all active sectors (simplified - in real game, track loaded sectors)
        // For now, just ensure we have some wormholes in the universe
        int currentWormholeCount = _entityManager.GetAllComponents<WormholeComponent>().Count();
        
        // Maintain a minimum number of active wormholes
        int targetWormholes = 50; // Adjust based on galaxy size
        
        while (currentWormholeCount < targetWormholes && _random.NextDouble() < WormholeSpawnChance)
        {
            SpawnWanderingWormhole();
            currentWormholeCount++;
        }
    }
    
    /// <summary>
    /// Spawn a wandering wormhole at a random location
    /// </summary>
    public Guid SpawnWanderingWormhole()
    {
        // Random source sector
        Vector3 sourceSector = new Vector3(
            _random.Next(-500, 500),
            _random.Next(-500, 500),
            _random.Next(-500, 500)
        );
        
        // Random destination sector
        Vector3 destSector = new Vector3(
            _random.Next(-500, 500),
            _random.Next(-500, 500),
            _random.Next(-500, 500)
        );
        
        // Random class
        WormholeClass whClass = (WormholeClass)_random.Next(1, 7);
        
        return CreateWormhole(sourceSector, destSector, whClass, WormholeType.Wandering);
    }
    
    /// <summary>
    /// Create a static wormhole that always leads to a specific security type
    /// </summary>
    public Guid CreateStaticWormhole(Vector3 sourceSector, SecurityLevel destinationType, WormholeClass whClass = WormholeClass.Class3)
    {
        // Destination will be determined when first traversed
        Vector3 destSector = Vector3.Zero;
        
        var entity = _entityManager.CreateEntity($"Static Wormhole to {destinationType}");
        
        var wormhole = new WormholeComponent
        {
            EntityId = entity.Id,
            Class = whClass,
            Type = WormholeType.Static,
            SourceSector = sourceSector,
            DestinationSector = destSector,
            StaticDestinationType = destinationType,
            Position = GetRandomSectorPosition(),
            Designation = GenerateWormholeDesignation(),
            RemainingLifetime = float.MaxValue, // Static wormholes don't decay naturally
            RemainingMass = GetMaxMassForClass(whClass),
            MaxTotalMass = GetMaxMassForClass(whClass),
            MaxShipMass = GetMaxShipMassForClass(whClass)
        };
        
        _entityManager.AddComponent(entity.Id, wormhole);
        
        // Track in sector
        if (!_sectorWormholes.ContainsKey(sourceSector))
            _sectorWormholes[sourceSector] = new List<WormholeComponent>();
        _sectorWormholes[sourceSector].Add(wormhole);
        
        Logger.Instance.Info("WormholeSystem", $"Created static wormhole {wormhole.Designation} to {destinationType}");
        
        return entity.Id;
    }
    
    /// <summary>
    /// Create a wormhole connection between two sectors
    /// </summary>
    public Guid CreateWormhole(Vector3 sourceSector, Vector3 destSector, WormholeClass whClass, WormholeType type)
    {
        var entity = _entityManager.CreateEntity($"Wormhole {whClass}");
        
        var wormhole = new WormholeComponent
        {
            EntityId = entity.Id,
            Class = whClass,
            Type = type,
            SourceSector = sourceSector,
            DestinationSector = destSector,
            Position = GetRandomSectorPosition(),
            Designation = GenerateWormholeDesignation(),
            RemainingLifetime = GetLifetimeForClass(whClass),
            MaxLifetime = GetLifetimeForClass(whClass),
            RemainingMass = GetMaxMassForClass(whClass),
            MaxTotalMass = GetMaxMassForClass(whClass),
            MaxShipMass = GetMaxShipMassForClass(whClass)
        };
        
        _entityManager.AddComponent(entity.Id, wormhole);
        
        // Track in sector
        if (!_sectorWormholes.ContainsKey(sourceSector))
            _sectorWormholes[sourceSector] = new List<WormholeComponent>();
        _sectorWormholes[sourceSector].Add(wormhole);
        
        Logger.Instance.Info("WormholeSystem", $"Created {type} wormhole {wormhole.Designation} ({whClass})");
        
        return entity.Id;
    }
    
    /// <summary>
    /// Get all wormholes in a specific sector
    /// </summary>
    public IEnumerable<WormholeComponent> GetWormholesInSector(Vector3 sector)
    {
        if (_sectorWormholes.TryGetValue(sector, out var wormholes))
            return wormholes.Where(w => w.IsActive);
        return Enumerable.Empty<WormholeComponent>();
    }
    
    /// <summary>
    /// Jump through a wormhole
    /// </summary>
    public bool JumpThroughWormhole(Guid wormholeId, Guid shipEntityId, float shipMass)
    {
        var wormhole = _entityManager.GetComponent<WormholeComponent>(wormholeId);
        if (wormhole == null)
            return false;
            
        if (!wormhole.CanShipJump(shipMass))
        {
            Logger.Instance.Warning("WormholeSystem", $"Ship too massive for wormhole {wormhole.Designation}");
            return false;
        }
        
        // Process the jump
        wormhole.ProcessJump(shipMass);
        
        Logger.Instance.Info("WormholeSystem", $"Ship jumped through {wormhole.Designation}");
        
        // Move ship to destination (would integrate with navigation system)
        // This is a simplified version
        return true;
    }
    
    /// <summary>
    /// Generate a random wormhole designation (like EVE's K162, etc.)
    /// </summary>
    private string GenerateWormholeDesignation()
    {
        char letter = (char)('A' + _random.Next(26));
        int number = _random.Next(100, 999);
        return $"{letter}{number}";
    }
    
    /// <summary>
    /// Get random position within a sector
    /// </summary>
    private Vector3 GetRandomSectorPosition()
    {
        return new Vector3(
            (float)(_random.NextDouble() * 10000 - 5000),
            (float)(_random.NextDouble() * 10000 - 5000),
            (float)(_random.NextDouble() * 10000 - 5000)
        );
    }
    
    /// <summary>
    /// Get lifetime based on wormhole class
    /// </summary>
    private float GetLifetimeForClass(WormholeClass whClass)
    {
        return whClass switch
        {
            WormholeClass.Class1 => 86400f,   // 24 hours
            WormholeClass.Class2 => 129600f,  // 36 hours
            WormholeClass.Class3 => 172800f,  // 48 hours
            WormholeClass.Class4 => 172800f,  // 48 hours
            WormholeClass.Class5 => 86400f,   // 24 hours (high traffic)
            WormholeClass.Class6 => 64800f,   // 18 hours (very high traffic)
            _ => 172800f
        };
    }
    
    /// <summary>
    /// Get maximum total mass for wormhole class
    /// </summary>
    private float GetMaxMassForClass(WormholeClass whClass)
    {
        return whClass switch
        {
            WormholeClass.Class1 => 5000000000f,    // 5 billion kg
            WormholeClass.Class2 => 3000000000f,    // 3 billion kg
            WormholeClass.Class3 => 2000000000f,    // 2 billion kg
            WormholeClass.Class4 => 1500000000f,    // 1.5 billion kg
            WormholeClass.Class5 => 1000000000f,    // 1 billion kg
            WormholeClass.Class6 => 750000000f,     // 750 million kg
            _ => 2000000000f
        };
    }
    
    /// <summary>
    /// Get maximum single ship mass for wormhole class
    /// </summary>
    private float GetMaxShipMassForClass(WormholeClass whClass)
    {
        return whClass switch
        {
            WormholeClass.Class1 => 200000000f,   // 200M kg (Battleships)
            WormholeClass.Class2 => 300000000f,   // 300M kg (Battleships+)
            WormholeClass.Class3 => 300000000f,   // 300M kg (Battleships+)
            WormholeClass.Class4 => 180000000f,   // 180M kg (Limited capitals)
            WormholeClass.Class5 => 180000000f,   // 180M kg (Limited capitals)
            WormholeClass.Class6 => 135000000f,   // 135M kg (Very limited capitals)
            _ => 300000000f
        };
    }
}
