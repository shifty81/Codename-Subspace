using System.Numerics;
using AvorionLike.Core.ECS;
using AvorionLike.Core.Voxel;
using AvorionLike.Core.Physics;
using AvorionLike.Core.Events;
using AvorionLike.Core.Logging;
using AvorionLike.Core.Resources;

namespace AvorionLike.Core.Combat;

/// <summary>
/// Types of damage that can be applied
/// </summary>
public enum DamageType
{
    Kinetic,      // Physical projectiles, collisions
    Energy,       // Lasers, plasma
    Explosive,    // Rockets, mines
    Thermal,      // Fire, heat
    EMP           // Disables systems
}

/// <summary>
/// Data about damage being applied
/// </summary>
public class DamageInfo
{
    public Guid SourceEntityId { get; set; }
    public Guid TargetEntityId { get; set; }
    public Vector3 HitPosition { get; set; }
    public float Damage { get; set; }
    public float Radius { get; set; } = 5f;
    public DamageType DamageType { get; set; } = DamageType.Kinetic;
    public Vector3 ImpactVelocity { get; set; }
}

/// <summary>
/// Event data for entity damage
/// </summary>
public class EntityDamageEvent : GameEvent
{
    public Guid EntityId { get; set; }
    public float DamageAmount { get; set; }
    public DamageType DamageType { get; set; }
    public Vector3 HitPosition { get; set; }
    public int BlocksDestroyed { get; set; }
}

/// <summary>
/// Event data for entity destruction
/// </summary>
public class EntityDestroyedEvent : GameEvent
{
    public Guid EntityId { get; set; }
    public string EntityName { get; set; } = "";
    public Vector3 Position { get; set; }
    public Guid? KillerId { get; set; }
}

/// <summary>
/// Enhanced damage system that handles block destruction, debris, and ship death
/// </summary>
public class DamageSystem : SystemBase
{
    private readonly EntityManager _entityManager;
    private readonly Random _random = new();

    public DamageSystem(EntityManager entityManager) : base("DamageSystem")
    {
        _entityManager = entityManager;
        
        // Subscribe to collision events to apply collision damage
        EventSystem.Instance.Subscribe(GameEvents.EntityCollision, OnEntityCollision);
    }

    public override void Update(float deltaTime)
    {
        // Check for destroyed entities and clean them up
        var entities = _entityManager.GetAllEntities();
        var entitiesToDestroy = new List<Guid>();

        foreach (var entity in entities)
        {
            var voxelComponent = _entityManager.GetComponent<VoxelStructureComponent>(entity.Id);
            if (voxelComponent != null)
            {
                // Check if ship has no more blocks or integrity is zero
                if (voxelComponent.Blocks.Count == 0 || voxelComponent.StructuralIntegrity <= 0)
                {
                    entitiesToDestroy.Add(entity.Id);
                }
            }
        }

        // Destroy entities that have been completely destroyed
        foreach (var entityId in entitiesToDestroy)
        {
            DestroyEntity(entityId);
        }
    }

    /// <summary>
    /// Apply damage to an entity at a specific position
    /// </summary>
    public void ApplyDamage(DamageInfo damageInfo)
    {
        var voxelComponent = _entityManager.GetComponent<VoxelStructureComponent>(damageInfo.TargetEntityId);
        var combatComponent = _entityManager.GetComponent<CombatComponent>(damageInfo.TargetEntityId);
        
        if (voxelComponent == null)
        {
            Logger.Instance.Warning("DamageSystem", $"Cannot apply damage - entity {damageInfo.TargetEntityId} has no voxel structure");
            return;
        }

        float remainingDamage = damageInfo.Damage;

        // Shields absorb damage first
        if (combatComponent != null && combatComponent.CurrentShields > 0)
        {
            float shieldDamage = Math.Min(combatComponent.CurrentShields, remainingDamage);
            combatComponent.CurrentShields -= shieldDamage;
            remainingDamage -= shieldDamage;

            Logger.Instance.Debug("DamageSystem", 
                $"Shields absorbed {shieldDamage:F1} damage. Remaining shields: {combatComponent.CurrentShields:F1}");
        }

        // Apply remaining damage to blocks
        if (remainingDamage > 0)
        {
            var destroyedBlocks = ApplyBlockDamage(voxelComponent, damageInfo, remainingDamage);

            // Spawn debris from destroyed blocks
            SpawnDebris(damageInfo.TargetEntityId, destroyedBlocks);

            // Publish damage event
            EventSystem.Instance.Publish(GameEvents.EntityDamaged, new EntityDamageEvent
            {
                EntityId = damageInfo.TargetEntityId,
                DamageAmount = damageInfo.Damage,
                DamageType = damageInfo.DamageType,
                HitPosition = damageInfo.HitPosition,
                BlocksDestroyed = destroyedBlocks.Count
            });

            Logger.Instance.Info("DamageSystem",
                $"Entity {damageInfo.TargetEntityId} took {remainingDamage:F1} damage. " +
                $"{destroyedBlocks.Count} blocks destroyed. Integrity: {voxelComponent.StructuralIntegrity:F1}%");
        }
    }

    /// <summary>
    /// Apply damage to voxel blocks with damage falloff
    /// </summary>
    private List<VoxelBlock> ApplyBlockDamage(VoxelStructureComponent structure, DamageInfo damageInfo, float damage)
    {
        var destroyedBlocks = new List<VoxelBlock>();

        // Apply damage type modifiers
        float damageMultiplier = GetDamageMultiplier(damageInfo.DamageType);
        float effectiveDamage = damage * damageMultiplier;

        // Find blocks within damage radius
        foreach (var block in structure.Blocks.ToList())
        {
            float distance = Vector3.Distance(block.Position, damageInfo.HitPosition);
            
            if (distance <= damageInfo.Radius)
            {
                // Calculate damage with falloff (linear falloff from center)
                float falloff = 1f - (distance / damageInfo.Radius);
                float blockDamage = effectiveDamage * falloff;

                // Apply damage
                block.TakeDamage(blockDamage);

                if (block.IsDestroyed)
                {
                    destroyedBlocks.Add(block);
                }
            }
        }

        // Remove destroyed blocks using the component's RemoveBlock method
        // This ensures RecalculateProperties() is properly called
        foreach (var block in destroyedBlocks)
        {
            structure.RemoveBlock(block);
        }

        return destroyedBlocks;
    }

    /// <summary>
    /// Get damage multiplier based on damage type
    /// </summary>
    private float GetDamageMultiplier(DamageType damageType)
    {
        return damageType switch
        {
            DamageType.Kinetic => 1.0f,
            DamageType.Energy => 1.2f,      // More effective against shields
            DamageType.Explosive => 1.5f,   // More effective in radius
            DamageType.Thermal => 0.8f,     // Less effective overall but causes DOT
            DamageType.EMP => 0.5f,         // Low damage but disables systems
            _ => 1.0f
        };
    }

    /// <summary>
    /// Spawn debris and resources from destroyed blocks
    /// </summary>
    private void SpawnDebris(Guid entityId, List<VoxelBlock> destroyedBlocks)
    {
        if (destroyedBlocks.Count == 0) return;

        var physicsComponent = _entityManager.GetComponent<PhysicsComponent>(entityId);
        if (physicsComponent == null) return;

        foreach (var block in destroyedBlocks)
        {
            // Calculate material yield (50% of block volume)
            float volume = block.Size.X * block.Size.Y * block.Size.Z;
            int materialYield = (int)(volume * 10 * 0.5f); // 50% recovery

            // Spawn debris entity (simplified - in full implementation would create actual debris entities)
            Logger.Instance.Debug("DamageSystem",
                $"Spawned debris: {materialYield} units of {block.MaterialType} at {physicsComponent.Position + block.Position}");
            
            // Could publish event for debris creation
            EventSystem.Instance.Publish(GameEvents.ResourceCollected, new Events.ResourceEvent
            {
                EntityId = entityId,
                ResourceType = block.MaterialType,
                Amount = materialYield
            });
        }
    }

    /// <summary>
    /// Destroy an entity completely
    /// </summary>
    private void DestroyEntity(Guid entityId)
    {
        var entity = _entityManager.GetEntity(entityId);
        if (entity == null) return;

        var physicsComponent = _entityManager.GetComponent<PhysicsComponent>(entityId);
        var position = physicsComponent?.Position ?? Vector3.Zero;

        Logger.Instance.Info("DamageSystem", $"Entity {entity.Name} ({entityId}) has been destroyed");

        // Publish destruction event
        EventSystem.Instance.Publish(GameEvents.ShipDestroyed, new EntityDestroyedEvent
        {
            EntityId = entityId,
            EntityName = entity.Name,
            Position = position
        });

        // Remove entity from manager
        _entityManager.DestroyEntity(entityId);
    }

    /// <summary>
    /// Handle collision damage
    /// </summary>
    private void OnEntityCollision(GameEvent gameEvent)
    {
        if (gameEvent is not EntityCollisionEvent collisionEvent) return;

        var physics1 = _entityManager.GetComponent<PhysicsComponent>(collisionEvent.Entity1Id);
        var physics2 = _entityManager.GetComponent<PhysicsComponent>(collisionEvent.Entity2Id);

        if (physics1 == null || physics2 == null) return;

        // Calculate collision damage based on impulse
        // Damage is proportional to impact force and velocity
        float baseDamage = collisionEvent.CollisionImpulse * 0.01f; // Scale factor

        // Apply damage to both entities if the collision was hard enough
        if (baseDamage > 1f) // Minimum damage threshold
        {
            // Apply damage to entity 1
            ApplyDamage(new DamageInfo
            {
                SourceEntityId = collisionEvent.Entity2Id,
                TargetEntityId = collisionEvent.Entity1Id,
                HitPosition = collisionEvent.CollisionPoint,
                Damage = baseDamage,
                Radius = 3f,
                DamageType = DamageType.Kinetic
            });

            // Apply damage to entity 2 (if not static)
            if (!physics2.IsStatic)
            {
                ApplyDamage(new DamageInfo
                {
                    SourceEntityId = collisionEvent.Entity1Id,
                    TargetEntityId = collisionEvent.Entity2Id,
                    HitPosition = collisionEvent.CollisionPoint,
                    Damage = baseDamage,
                    Radius = 3f,
                    DamageType = DamageType.Kinetic
                });
            }

            Logger.Instance.Debug("DamageSystem",
                $"Collision damage: {baseDamage:F1} damage from impact at {collisionEvent.CollisionPoint}");
        }
    }

    /// <summary>
    /// Apply explosion damage in a radius
    /// </summary>
    public void ApplyExplosionDamage(Vector3 position, float radius, float damage, Guid? sourceEntityId = null)
    {
        var entities = _entityManager.GetAllEntities();

        foreach (var entity in entities)
        {
            var physicsComponent = _entityManager.GetComponent<PhysicsComponent>(entity.Id);
            if (physicsComponent == null) continue;

            float distance = Vector3.Distance(physicsComponent.Position, position);
            if (distance <= radius)
            {
                float falloff = 1f - (distance / radius);
                float actualDamage = damage * falloff;

                ApplyDamage(new DamageInfo
                {
                    SourceEntityId = sourceEntityId ?? Guid.Empty,
                    TargetEntityId = entity.Id,
                    HitPosition = position,
                    Damage = actualDamage,
                    Radius = 10f, // Block damage radius
                    DamageType = DamageType.Explosive
                });

                // Apply impulse force from explosion
                Vector3 direction = Vector3.Normalize(physicsComponent.Position - position);
                float force = 10000f * falloff; // Explosion force
                physicsComponent.AddForce(direction * force);
            }
        }

        Logger.Instance.Info("DamageSystem",
            $"Explosion at {position} with radius {radius:F1} and damage {damage:F1}");
    }

    /// <summary>
    /// Repair a block
    /// </summary>
    public void RepairBlock(Guid entityId, VoxelBlock block, float amount)
    {
        if (block.IsDestroyed) return;

        block.Durability = Math.Min(block.MaxDurability, block.Durability + amount);
        
        Logger.Instance.Debug("DamageSystem",
            $"Repaired block in entity {entityId}: {block.Durability:F1}/{block.MaxDurability:F1}");
    }

    /// <summary>
    /// Repair entire ship
    /// </summary>
    public void RepairShip(Guid entityId, float amount)
    {
        var voxelComponent = _entityManager.GetComponent<VoxelStructureComponent>(entityId);
        if (voxelComponent == null) return;

        foreach (var block in voxelComponent.Blocks)
        {
            if (!block.IsDestroyed)
            {
                block.Durability = Math.Min(block.MaxDurability, block.Durability + amount);
            }
        }

        EventSystem.Instance.Publish(GameEvents.ShipRepaired, new EntityDamageEvent
        {
            EntityId = entityId,
            DamageAmount = -amount // Negative for healing
        });

        Logger.Instance.Info("DamageSystem", $"Repaired entity {entityId} by {amount:F1}");
    }
}
