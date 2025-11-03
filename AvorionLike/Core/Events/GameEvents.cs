namespace AvorionLike.Core.Events;

/// <summary>
/// Common game event types
/// </summary>
public static class GameEvents
{
    // Entity events
    public const string EntityCreated = "entity.created";
    public const string EntityDestroyed = "entity.destroyed";
    public const string EntityDamaged = "entity.damaged";
    
    // Component events
    public const string ComponentAdded = "component.added";
    public const string ComponentRemoved = "component.removed";
    
    // Resource events
    public const string ResourceCollected = "resource.collected";
    public const string ResourceSpent = "resource.spent";
    public const string InventoryFull = "inventory.full";
    
    // Progression events
    public const string PlayerLevelUp = "player.levelup";
    public const string ExperienceGained = "player.experience";
    public const string SkillPointsEarned = "player.skillpoints";
    
    // Ship events
    public const string ShipDamaged = "ship.damaged";
    public const string ShipDestroyed = "ship.destroyed";
    public const string ShipRepaired = "ship.repaired";
    public const string VoxelBlockAdded = "ship.block.added";
    public const string VoxelBlockRemoved = "ship.block.removed";
    
    // Physics events
    public const string CollisionDetected = "physics.collision";
    public const string VelocityChanged = "physics.velocity";
    
    // Combat events
    public const string WeaponFired = "combat.weapon.fired";
    public const string ProjectileHit = "combat.projectile.hit";
    public const string ShieldHit = "combat.shield.hit";
    
    // Trading events
    public const string TradeCompleted = "trade.completed";
    public const string ItemPurchased = "trade.purchased";
    public const string ItemSold = "trade.sold";
    
    // Faction events
    public const string ReputationChanged = "faction.reputation";
    public const string FactionStatusChanged = "faction.status";
    
    // Network events
    public const string ClientConnected = "network.client.connected";
    public const string ClientDisconnected = "network.client.disconnected";
    public const string ServerStarted = "network.server.started";
    public const string ServerStopped = "network.server.stopped";
    
    // System events
    public const string GameStarted = "game.started";
    public const string GamePaused = "game.paused";
    public const string GameResumed = "game.resumed";
    public const string GameSaved = "game.saved";
    public const string GameLoaded = "game.loaded";
    
    // Sector events
    public const string SectorEntered = "sector.entered";
    public const string SectorExited = "sector.exited";
    public const string SectorGenerated = "sector.generated";
}

/// <summary>
/// Entity event data
/// </summary>
public class EntityEvent : GameEvent
{
    public Guid EntityId { get; set; }
    public string EntityName { get; set; } = "";
}

/// <summary>
/// Resource event data
/// </summary>
public class ResourceEvent : GameEvent
{
    public Guid EntityId { get; set; }
    public string ResourceType { get; set; } = "";
    public int Amount { get; set; }
}

/// <summary>
/// Collision event data
/// </summary>
public class CollisionEvent : GameEvent
{
    public Guid Entity1Id { get; set; }
    public Guid Entity2Id { get; set; }
    public float ImpactForce { get; set; }
}

/// <summary>
/// Network event data
/// </summary>
public class NetworkEvent : GameEvent
{
    public string ClientId { get; set; } = "";
    public string? AdditionalInfo { get; set; }
}

/// <summary>
/// Progression event data
/// </summary>
public class ProgressionEvent : GameEvent
{
    public Guid EntityId { get; set; }
    public int Level { get; set; }
    public int Experience { get; set; }
    public int SkillPoints { get; set; }
}
