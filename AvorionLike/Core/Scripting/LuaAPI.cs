using AvorionLike.Core.ECS;
using AvorionLike.Core.Voxel;
using AvorionLike.Core.Physics;
using AvorionLike.Core.Resources;
using AvorionLike.Core.Events;
using AvorionLike.Core.Logging;
using System.Numerics;

namespace AvorionLike.Core.Scripting;

/// <summary>
/// Lua API wrapper that provides safe, documented access to game systems
/// This class acts as a bridge between Lua scripts and the game engine
/// </summary>
public class LuaAPI
{
    private readonly GameEngine _engine;
    private readonly Logger _logger;

    public LuaAPI(GameEngine engine)
    {
        _engine = engine;
        _logger = Logger.Instance;
    }

    #region Entity Management

    /// <summary>
    /// Create a new entity with the given name
    /// </summary>
    public string CreateEntity(string name)
    {
        _logger.Debug("LuaAPI", $"Lua script creating entity: {name}");
        var entity = _engine.EntityManager.CreateEntity(name);
        return entity.Id.ToString();
    }

    /// <summary>
    /// Destroy an entity
    /// </summary>
    public bool DestroyEntity(string entityId)
    {
        if (!Guid.TryParse(entityId, out var guid)) return false;
        
        _logger.Debug("LuaAPI", $"Lua script destroying entity: {entityId}");
        _engine.EntityManager.DestroyEntity(guid);
        return true;
    }

    /// <summary>
    /// Get count of all entities
    /// </summary>
    public int GetEntityCount()
    {
        return _engine.EntityManager.GetAllEntities().Count();
    }

    #endregion

    #region Voxel System

    /// <summary>
    /// Add a voxel structure component to an entity
    /// </summary>
    public bool AddVoxelStructure(string entityId)
    {
        if (!Guid.TryParse(entityId, out var guid)) return false;
        
        var voxelComponent = new VoxelStructureComponent();
        _engine.EntityManager.AddComponent(guid, voxelComponent);
        return true;
    }

    /// <summary>
    /// Add a voxel block to an entity's structure
    /// </summary>
    public bool AddVoxelBlock(string entityId, float x, float y, float z, float sizeX, float sizeY, float sizeZ, string material)
    {
        if (!Guid.TryParse(entityId, out var guid)) return false;
        
        var voxelComponent = _engine.EntityManager.GetComponent<VoxelStructureComponent>(guid);
        if (voxelComponent == null) return false;

        var block = new VoxelBlock(
            new Vector3(x, y, z),
            new Vector3(sizeX, sizeY, sizeZ),
            material
        );
        voxelComponent.AddBlock(block);
        
        _logger.Debug("LuaAPI", $"Added voxel block to entity {entityId} at ({x},{y},{z})");
        return true;
    }

    /// <summary>
    /// Get the total mass of an entity's voxel structure
    /// </summary>
    public float GetVoxelMass(string entityId)
    {
        if (!Guid.TryParse(entityId, out var guid)) return 0f;
        
        var voxelComponent = _engine.EntityManager.GetComponent<VoxelStructureComponent>(guid);
        return voxelComponent?.TotalMass ?? 0f;
    }

    #endregion

    #region Physics System

    /// <summary>
    /// Add physics component to an entity
    /// </summary>
    public bool AddPhysics(string entityId, float x, float y, float z, float mass)
    {
        if (!Guid.TryParse(entityId, out var guid)) return false;

        var physicsComponent = new PhysicsComponent
        {
            Position = new Vector3(x, y, z),
            Mass = mass
        };
        _engine.EntityManager.AddComponent(guid, physicsComponent);
        
        _logger.Debug("LuaAPI", $"Added physics to entity {entityId}");
        return true;
    }

    /// <summary>
    /// Apply force to an entity
    /// </summary>
    public bool ApplyForce(string entityId, float x, float y, float z)
    {
        if (!Guid.TryParse(entityId, out var guid)) return false;
        
        var physicsComponent = _engine.EntityManager.GetComponent<PhysicsComponent>(guid);
        if (physicsComponent == null) return false;

        physicsComponent.AddForce(new Vector3(x, y, z));
        return true;
    }

    /// <summary>
    /// Set velocity of an entity
    /// </summary>
    public bool SetVelocity(string entityId, float x, float y, float z)
    {
        if (!Guid.TryParse(entityId, out var guid)) return false;
        
        var physicsComponent = _engine.EntityManager.GetComponent<PhysicsComponent>(guid);
        if (physicsComponent == null) return false;

        physicsComponent.Velocity = new Vector3(x, y, z);
        return true;
    }

    /// <summary>
    /// Get position of an entity
    /// </summary>
    public Dictionary<string, float> GetPosition(string entityId)
    {
        if (!Guid.TryParse(entityId, out var guid))
            return new Dictionary<string, float> { { "x", 0 }, { "y", 0 }, { "z", 0 } };
        
        var physicsComponent = _engine.EntityManager.GetComponent<PhysicsComponent>(guid);
        if (physicsComponent == null)
            return new Dictionary<string, float> { { "x", 0 }, { "y", 0 }, { "z", 0 } };

        return new Dictionary<string, float>
        {
            { "x", physicsComponent.Position.X },
            { "y", physicsComponent.Position.Y },
            { "z", physicsComponent.Position.Z }
        };
    }

    #endregion

    #region Resource Management

    /// <summary>
    /// Add inventory component to an entity
    /// </summary>
    public bool AddInventory(string entityId, int capacity)
    {
        if (!Guid.TryParse(entityId, out var guid)) return false;

        var inventory = new Inventory();
        var inventoryComponent = new InventoryComponent { Inventory = inventory };
        _engine.EntityManager.AddComponent(guid, inventoryComponent);
        
        _logger.Debug("LuaAPI", $"Added inventory to entity {entityId}");
        return true;
    }

    /// <summary>
    /// Add resource to entity's inventory
    /// </summary>
    public bool AddResource(string entityId, string resourceType, int amount)
    {
        if (!Guid.TryParse(entityId, out var guid)) return false;
        
        var inventoryComponent = _engine.EntityManager.GetComponent<InventoryComponent>(guid);
        if (inventoryComponent?.Inventory == null) return false;

        if (Enum.TryParse<ResourceType>(resourceType, true, out var type))
        {
            return inventoryComponent.Inventory.AddResource(type, amount);
        }
        return false;
    }

    /// <summary>
    /// Get resource amount from entity's inventory
    /// </summary>
    public int GetResourceAmount(string entityId, string resourceType)
    {
        if (!Guid.TryParse(entityId, out var guid)) return 0;
        
        var inventoryComponent = _engine.EntityManager.GetComponent<InventoryComponent>(guid);
        if (inventoryComponent?.Inventory == null) return 0;

        if (Enum.TryParse<ResourceType>(resourceType, true, out var type))
        {
            return inventoryComponent.Inventory.GetResourceAmount(type);
        }
        return 0;
    }

    #endregion

    #region Event System

    /// <summary>
    /// Subscribe to a game event
    /// </summary>
    public void SubscribeToEvent(string eventName, Action<GameEvent> callback)
    {
        _logger.Debug("LuaAPI", $"Lua script subscribing to event: {eventName}");
        EventSystem.Instance.Subscribe(eventName, callback);
    }

    /// <summary>
    /// Publish a game event
    /// </summary>
    public void PublishEvent(string eventName, string message)
    {
        _logger.Debug("LuaAPI", $"Lua script publishing event: {eventName}: {message}");
        EventSystem.Instance.Publish(eventName, new GameEvent { EventType = eventName });
    }

    #endregion

    #region Logging

    /// <summary>
    /// Log a message from Lua
    /// </summary>
    public void Log(string message)
    {
        _logger.Info("LuaScript", message);
    }

    /// <summary>
    /// Log a warning from Lua
    /// </summary>
    public void LogWarning(string message)
    {
        _logger.Warning("LuaScript", message);
    }

    /// <summary>
    /// Log an error from Lua
    /// </summary>
    public void LogError(string message)
    {
        _logger.Error("LuaScript", message);
    }

    #endregion

    #region Galaxy & Procedural Generation

    /// <summary>
    /// Generate a galaxy sector
    /// </summary>
    public object GenerateSector(int x, int y, int z)
    {
        var sector = _engine.GalaxyGenerator.GenerateSector(x, y, z);
        _logger.Debug("LuaAPI", $"Generated sector at ({x},{y},{z})");
        
        return new
        {
            X = sector.X,
            Y = sector.Y,
            Z = sector.Z,
            AsteroidCount = sector.Asteroids.Count,
            HasStation = sector.Station != null
        };
    }

    #endregion

    #region Utility Functions

    /// <summary>
    /// Get engine statistics
    /// </summary>
    public Dictionary<string, object> GetStatistics()
    {
        var stats = _engine.GetStatistics();
        return new Dictionary<string, object>
        {
            { "TotalEntities", stats.TotalEntities }
        };
    }

    /// <summary>
    /// Get game time
    /// </summary>
    public double GetGameTime()
    {
        return DateTime.UtcNow.Ticks / (double)TimeSpan.TicksPerSecond;
    }

    #endregion
}
