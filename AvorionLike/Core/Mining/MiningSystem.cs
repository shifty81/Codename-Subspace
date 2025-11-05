using System.Numerics;
using AvorionLike.Core.ECS;
using AvorionLike.Core.Resources;
using AvorionLike.Core.Procedural;

namespace AvorionLike.Core.Mining;

/// <summary>
/// Component for mining capabilities
/// </summary>
public class MiningComponent : IComponent
{
    public Guid EntityId { get; set; }
    public float MiningPower { get; set; } = 10f; // Resources per second
    public float MiningRange { get; set; } = 50f;
    public bool IsMining { get; set; } = false;
    public Guid? TargetAsteroidId { get; set; } = null;
}

/// <summary>
/// Component for salvaging capabilities
/// </summary>
public class SalvagingComponent : IComponent
{
    public Guid EntityId { get; set; }
    public float SalvagePower { get; set; } = 8f; // Resources per second
    public float SalvageRange { get; set; } = 50f;
    public bool IsSalvaging { get; set; } = false;
    public Guid? TargetWreckageId { get; set; } = null;
}

/// <summary>
/// Represents a mineable asteroid
/// </summary>
public class Asteroid
{
    public Guid Id { get; set; } = Guid.NewGuid();
    public Vector3 Position { get; set; }
    public float Size { get; set; }
    public ResourceType ResourceType { get; set; }
    public float RemainingResources { get; set; }
    
    public Asteroid(AsteroidData data)
    {
        Position = data.Position;
        Size = data.Size;
        // Try to parse resource type, default to Iron if invalid
        if (!Enum.TryParse<ResourceType>(data.ResourceType, out var resourceType))
        {
            resourceType = ResourceType.Iron;
        }
        ResourceType = resourceType;
        RemainingResources = Size * 10f; // Size determines total resources
    }
}

/// <summary>
/// Represents salvageable wreckage
/// </summary>
public class Wreckage
{
    public Guid Id { get; set; } = Guid.NewGuid();
    public Vector3 Position { get; set; }
    public Dictionary<ResourceType, int> Resources { get; set; } = new();
    public bool IsFullySalvaged => Resources.All(kvp => kvp.Value <= 0);
}

/// <summary>
/// System for mining and salvaging
/// </summary>
public class MiningSystem : SystemBase
{
    private readonly EntityManager _entityManager;
    private readonly Dictionary<Guid, Asteroid> _asteroids = new();
    private readonly Dictionary<Guid, Wreckage> _wreckage = new();

    public MiningSystem(EntityManager entityManager) : base("MiningSystem")
    {
        _entityManager = entityManager;
    }

    public override void Update(float deltaTime)
    {
        UpdateMining(deltaTime);
        UpdateSalvaging(deltaTime);
    }
    
    /// <summary>
    /// Add an asteroid to the sector
    /// </summary>
    public void AddAsteroid(Asteroid asteroid)
    {
        _asteroids[asteroid.Id] = asteroid;
    }
    
    /// <summary>
    /// Add wreckage to the sector
    /// </summary>
    public void AddWreckage(Wreckage wreckage)
    {
        _wreckage[wreckage.Id] = wreckage;
    }
    
    /// <summary>
    /// Start mining an asteroid
    /// </summary>
    public bool StartMining(MiningComponent miner, Guid asteroidId, Vector3 minerPosition)
    {
        if (!_asteroids.TryGetValue(asteroidId, out var asteroid))
        {
            return false;
        }
        
        // Check range
        float distance = Vector3.Distance(minerPosition, asteroid.Position);
        if (distance > miner.MiningRange)
        {
            return false;
        }
        
        miner.IsMining = true;
        miner.TargetAsteroidId = asteroidId;
        return true;
    }
    
    /// <summary>
    /// Start salvaging wreckage
    /// </summary>
    public bool StartSalvaging(SalvagingComponent salvager, Guid wreckageId, Vector3 salvagerPosition)
    {
        if (!_wreckage.TryGetValue(wreckageId, out var wreck))
        {
            return false;
        }
        
        // Check range
        float distance = Vector3.Distance(salvagerPosition, wreck.Position);
        if (distance > salvager.SalvageRange)
        {
            return false;
        }
        
        salvager.IsSalvaging = true;
        salvager.TargetWreckageId = wreckageId;
        return true;
    }
    
    /// <summary>
    /// Update all mining operations
    /// </summary>
    private void UpdateMining(float deltaTime)
    {
        var miners = _entityManager.GetAllComponents<MiningComponent>();
        
        foreach (var miner in miners)
        {
            if (!miner.IsMining || !miner.TargetAsteroidId.HasValue)
            {
                continue;
            }
            
            if (!_asteroids.TryGetValue(miner.TargetAsteroidId.Value, out var asteroid))
            {
                miner.IsMining = false;
                continue;
            }
            
            // Extract resources
            float extracted = Math.Min(miner.MiningPower * deltaTime, asteroid.RemainingResources);
            asteroid.RemainingResources -= extracted;
            
            // Add to inventory
            var inventory = _entityManager.GetComponent<InventoryComponent>(miner.EntityId);
            if (inventory != null)
            {
                inventory.Inventory.AddResource(asteroid.ResourceType, (int)extracted);
            }
            
            // Remove asteroid if depleted
            if (asteroid.RemainingResources <= 0)
            {
                _asteroids.Remove(miner.TargetAsteroidId.Value);
                miner.IsMining = false;
                miner.TargetAsteroidId = null;
            }
        }
    }
    
    /// <summary>
    /// Update all salvaging operations
    /// </summary>
    private void UpdateSalvaging(float deltaTime)
    {
        var salvagers = _entityManager.GetAllComponents<SalvagingComponent>();
        
        foreach (var salvager in salvagers)
        {
            if (!salvager.IsSalvaging || !salvager.TargetWreckageId.HasValue)
            {
                continue;
            }
            
            if (!_wreckage.TryGetValue(salvager.TargetWreckageId.Value, out var wreck))
            {
                salvager.IsSalvaging = false;
                continue;
            }
            
            // Salvage resources
            var inventory = _entityManager.GetComponent<InventoryComponent>(salvager.EntityId);
            if (inventory != null)
            {
                float salvageAmount = salvager.SalvagePower * deltaTime;
                
                foreach (var resource in wreck.Resources.Keys.ToList())
                {
                    if (wreck.Resources[resource] <= 0) continue;
                    
                    int toSalvage = Math.Min((int)salvageAmount, wreck.Resources[resource]);
                    if (inventory.Inventory.AddResource(resource, toSalvage))
                    {
                        wreck.Resources[resource] -= toSalvage;
                    }
                }
            }
            
            // Remove wreckage if fully salvaged
            if (wreck.IsFullySalvaged)
            {
                _wreckage.Remove(salvager.TargetWreckageId.Value);
                salvager.IsSalvaging = false;
                salvager.TargetWreckageId = null;
            }
        }
    }
    
    /// <summary>
    /// Get all asteroids in the sector
    /// </summary>
    public IEnumerable<Asteroid> GetAsteroids()
    {
        return _asteroids.Values;
    }
    
    /// <summary>
    /// Get all wreckage in the sector
    /// </summary>
    public IEnumerable<Wreckage> GetWreckage()
    {
        return _wreckage.Values;
    }
}
