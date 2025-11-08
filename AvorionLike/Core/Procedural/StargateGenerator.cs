using System.Numerics;
using AvorionLike.Core.ECS;

namespace AvorionLike.Core.Procedural;

/// <summary>
/// Generates stargate/jump gate entities in solar systems
/// Manages gate placement, trigger zones, and destination linking
/// </summary>
public class StargateGenerator
{
    private readonly GalaxyNetwork _galaxyNetwork;
    private readonly EntityManager _entityManager;
    
    public StargateGenerator(GalaxyNetwork galaxyNetwork, EntityManager entityManager)
    {
        _galaxyNetwork = galaxyNetwork;
        _entityManager = entityManager;
    }
    
    /// <summary>
    /// Generate all stargates for a solar system
    /// </summary>
    public List<Entity> GenerateStargatesForSystem(SolarSystemData system)
    {
        var stargateEntities = new List<Entity>();
        
        foreach (var gateData in system.Stargates)
        {
            var entity = CreateStargateEntity(gateData, system);
            stargateEntities.Add(entity);
        }
        
        return stargateEntities;
    }
    
    /// <summary>
    /// Create a single stargate entity
    /// </summary>
    private Entity CreateStargateEntity(StargateData gateData, SolarSystemData system)
    {
        var entity = _entityManager.CreateEntity($"Stargate-{gateData.GateId}");
        
        // Add stargate component
        var stargateComponent = new StargateComponent
        {
            GateId = gateData.GateId,
            Name = gateData.Name,
            DestinationSystemId = gateData.DestinationSystemId,
            DestinationGateId = gateData.DestinationGateId,
            GateType = gateData.Type,
            IsActive = gateData.IsActive,
            CurrentSystemId = system.SystemId
        };
        _entityManager.AddComponent(entity.Id, stargateComponent);
        
        // Add transform component
        var transformComponent = new TransformComponent
        {
            Position = gateData.Position,
            Rotation = gateData.Rotation,
            Scale = GetGateScale(gateData.Type)
        };
        _entityManager.AddComponent(entity.Id, transformComponent);
        
        // Add trigger zone component for detection
        var triggerComponent = new TriggerZoneComponent
        {
            Shape = TriggerShape.Cylinder,
            Radius = GetGateTriggerRadius(gateData.Type),
            Height = 500f, // Gate passage height
            OnEnter = (enteringEntityId) => OnGateEntered(enteringEntityId, entity.Id.ToString())
        };
        _entityManager.AddComponent(entity.Id, triggerComponent);
        
        return entity;
    }
    
    /// <summary>
    /// Get the appropriate scale for gate type
    /// </summary>
    private Vector3 GetGateScale(GateType type)
    {
        return type switch
        {
            GateType.Standard => new Vector3(100f, 100f, 50f),
            GateType.Ancient => new Vector3(150f, 150f, 75f),
            GateType.Unstable => new Vector3(80f, 80f, 40f),
            GateType.Military => new Vector3(120f, 120f, 60f),
            _ => new Vector3(100f, 100f, 50f)
        };
    }
    
    /// <summary>
    /// Get trigger radius for gate type
    /// </summary>
    private float GetGateTriggerRadius(GateType type)
    {
        return type switch
        {
            GateType.Standard => 200f,
            GateType.Ancient => 300f,
            GateType.Unstable => 150f,
            GateType.Military => 250f,
            _ => 200f
        };
    }
    
    /// <summary>
    /// Called when an entity enters a stargate trigger zone
    /// </summary>
    private void OnGateEntered(string enteringEntityId, string gateEntityId)
    {
        // Check if entering entity is the player's ship
        if (!IsPlayerShip(enteringEntityId))
            return;
        
        // Get gate component - convert string to Guid
        if (!Guid.TryParse(gateEntityId, out var gateGuid))
            return;
            
        if (!_entityManager.HasComponent<StargateComponent>(gateGuid))
            return;
        
        var gate = _entityManager.GetComponent<StargateComponent>(gateGuid);
        
        if (gate != null && !gate.IsActive)
            return;
        
        if (gate != null)
        {
            // Trigger hyperspace jump
            TriggerGateJump(enteringEntityId, gate);
        }
    }
    
    /// <summary>
    /// Check if entity is the player's ship
    /// </summary>
    private bool IsPlayerShip(string entityId)
    {
        // Check for player component or tag
        if (!Guid.TryParse(entityId, out var guid))
            return entityId.Contains("Player");
            
        return _entityManager.HasComponent<PlayerComponent>(guid) ||
               entityId.Contains("Player");
    }
    
    /// <summary>
    /// Trigger a hyperspace jump through the gate
    /// </summary>
    private void TriggerGateJump(string playerEntityId, StargateComponent gate)
    {
        // This would be called on the actual game's hyperspace jump system
        // For now, we'll just mark it as triggered
        gate.LastUsedTime = DateTime.UtcNow;
        
        // In a real implementation, this would call:
        // _hyperspaceJump.InitiateJump(gate.DestinationSystemId, LoadDestinationSystem);
    }
    
    /// <summary>
    /// Get the exit gate position in the destination system
    /// </summary>
    public Vector3? GetExitGatePosition(string destinationSystemId, string? destinationGateId)
    {
        var destinationSystem = _galaxyNetwork.GetOrGenerateSystem(
            ParseSystemCoordinates(destinationSystemId)
        );
        
        if (destinationGateId != null)
        {
            var exitGate = destinationSystem.Stargates
                .FirstOrDefault(g => g.GateId == destinationGateId);
            return exitGate?.Position;
        }
        
        // If no specific gate, return first gate position
        return destinationSystem.Stargates.FirstOrDefault()?.Position;
    }
    
    /// <summary>
    /// Parse system ID to get coordinates
    /// </summary>
    private Vector3Int ParseSystemCoordinates(string systemId)
    {
        // Expected format: "System-X-Y-Z"
        var parts = systemId.Split('-');
        if (parts.Length >= 4)
        {
            return new Vector3Int(
                int.Parse(parts[1]),
                int.Parse(parts[2]),
                int.Parse(parts[3])
            );
        }
        return Vector3Int.Zero;
    }
}

/// <summary>
/// Component for stargate entities
/// </summary>
public class StargateComponent : IComponent
{
    public Guid EntityId { get; set; }
    public string GateId { get; set; } = "";
    public string Name { get; set; } = "Stargate";
    public string DestinationSystemId { get; set; } = "";
    public string? DestinationGateId { get; set; }
    public string CurrentSystemId { get; set; } = "";
    public GateType GateType { get; set; } = GateType.Standard;
    public bool IsActive { get; set; } = true;
    public DateTime LastUsedTime { get; set; } = DateTime.MinValue;
}

/// <summary>
/// Transform component for entity positioning
/// </summary>
public class TransformComponent : IComponent
{
    public Guid EntityId { get; set; }
    public Vector3 Position { get; set; }
    public Vector3 Rotation { get; set; }
    public Vector3 Scale { get; set; } = Vector3.One;
}

/// <summary>
/// Trigger zone component for detecting entities
/// </summary>
public class TriggerZoneComponent : IComponent
{
    public Guid EntityId { get; set; }
    public TriggerShape Shape { get; set; } = TriggerShape.Sphere;
    public float Radius { get; set; } = 100f;
    public float Height { get; set; } = 100f; // For cylinder shape
    public Action<string>? OnEnter { get; set; }
    public Action<string>? OnExit { get; set; }
}

/// <summary>
/// Player marker component
/// </summary>
public class PlayerComponent : IComponent
{
    public Guid EntityId { get; set; }
    public string PlayerId { get; set; } = "";
    public string PlayerName { get; set; } = "Player";
}

/// <summary>
/// Trigger shape types
/// </summary>
public enum TriggerShape
{
    Sphere,
    Box,
    Cylinder
}
