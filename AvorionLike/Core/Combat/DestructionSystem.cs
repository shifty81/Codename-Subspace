using System.Numerics;
using AvorionLike.Core.ECS;
using AvorionLike.Core.Voxel;
using AvorionLike.Core.Events;
using AvorionLike.Core.Physics;

namespace AvorionLike.Core.Combat;

/// <summary>
/// System for dynamic voxel-based destruction
/// </summary>
public class DestructionSystem : SystemBase
{
    private readonly EntityManager _entityManager;
    private readonly EventSystem _eventSystem;
    private readonly List<DestructionEvent> _pendingDestructions = new();
    
    public DestructionSystem(EntityManager entityManager, EventSystem eventSystem) 
        : base("DestructionSystem")
    {
        _entityManager = entityManager;
        _eventSystem = eventSystem;
    }
    
    /// <summary>
    /// Apply damage to a specific voxel block
    /// </summary>
    public void DamageBlock(Guid entityId, VoxelBlock block, float damage)
    {
        block.TakeDamage(damage);
        
        if (block.IsDestroyed)
        {
            _pendingDestructions.Add(new DestructionEvent
            {
                EntityId = entityId,
                Block = block,
                Type = DestructionType.BlockDestroyed
            });
        }
    }
    
    /// <summary>
    /// Apply damage to voxels in a radius (explosion)
    /// </summary>
    public void DamageRadius(Vector3 position, float radius, float damage)
    {
        var entities = _entityManager.GetAllEntities();
        
        foreach (var entity in entities)
        {
            var voxelComponent = _entityManager.GetComponent<VoxelStructureComponent>(entity.Id);
            if (voxelComponent == null)
                continue;
            
            // Find blocks within radius
            var blocksInRadius = voxelComponent.Blocks
                .Where(b => Vector3.Distance(b.Position, position) <= radius)
                .ToList();
            
            foreach (var block in blocksInRadius)
            {
                // Damage falls off with distance
                float distance = Vector3.Distance(block.Position, position);
                float distanceFactor = 1.0f - (distance / radius);
                float actualDamage = damage * distanceFactor;
                
                DamageBlock(entity.Id, block, actualDamage);
            }
        }
    }
    
    /// <summary>
    /// Apply damage along a ray (laser/projectile)
    /// </summary>
    public void DamageRay(Vector3 start, Vector3 direction, float length, float damage)
    {
        var entities = _entityManager.GetAllEntities();
        Vector3 end = start + direction * length;
        
        foreach (var entity in entities)
        {
            var voxelComponent = _entityManager.GetComponent<VoxelStructureComponent>(entity.Id);
            if (voxelComponent == null)
                continue;
            
            // Find blocks intersecting with ray
            var hitBlocks = voxelComponent.Blocks
                .Where(b => RayIntersectsBlock(start, direction, length, b))
                .OrderBy(b => Vector3.Distance(start, b.Position))
                .Take(3) // Only damage first 3 blocks hit
                .ToList();
            
            foreach (var block in hitBlocks)
            {
                DamageBlock(entity.Id, block, damage);
                
                // If block is destroyed, continue ray through it
                if (!block.IsDestroyed)
                    break; // Stop at first non-destroyed block
            }
        }
    }
    
    /// <summary>
    /// Process pending destructions and update structures
    /// </summary>
    public override void Update(float deltaTime)
    {
        if (_pendingDestructions.Count == 0)
            return;
        
        // Group destructions by entity
        var destructionsByEntity = _pendingDestructions
            .GroupBy(d => d.EntityId)
            .ToList();
        
        foreach (var group in destructionsByEntity)
        {
            var entityId = group.Key;
            var voxelComponent = _entityManager.GetComponent<VoxelStructureComponent>(entityId);
            
            if (voxelComponent == null)
                continue;
            
            // Remove destroyed blocks
            foreach (var destruction in group)
            {
                voxelComponent.RemoveBlock(destruction.Block);
            }
            
            // Update entity properties
            UpdateEntityAfterDestruction(entityId, voxelComponent);
            
            // Publish destruction event
            _eventSystem.Publish("BlocksDestroyed", new EntityEvent
            {
                EntityId = entityId,
                EventType = "BlocksDestroyed"
            });
        }
        
        _pendingDestructions.Clear();
    }
    
    /// <summary>
    /// Update entity properties after destruction
    /// </summary>
    private void UpdateEntityAfterDestruction(Guid entityId, VoxelStructureComponent voxelComponent)
    {
        // Update physics based on new mass
        var physicsComponent = _entityManager.GetComponent<PhysicsComponent>(entityId);
        if (physicsComponent != null)
        {
            physicsComponent.Mass = voxelComponent.TotalMass;
        }
        
        // Check if entity should be destroyed (no blocks left)
        if (voxelComponent.Blocks.Count == 0)
        {
            _eventSystem.Publish("EntityDestroyed", new EntityEvent
            {
                EntityId = entityId,
                EventType = "EntityDestroyed"
            });
        }
    }
    
    /// <summary>
    /// Check if ray intersects with voxel block
    /// </summary>
    private bool RayIntersectsBlock(Vector3 rayStart, Vector3 rayDir, float rayLength, VoxelBlock block)
    {
        // Simple AABB ray intersection test
        Vector3 min = block.Position - block.Size / 2;
        Vector3 max = block.Position + block.Size / 2;
        
        float tMin = 0.0f;
        float tMax = rayLength;
        
        for (int i = 0; i < 3; i++)
        {
            float rayDirComponent = i == 0 ? rayDir.X : (i == 1 ? rayDir.Y : rayDir.Z);
            float rayStartComponent = i == 0 ? rayStart.X : (i == 1 ? rayStart.Y : rayStart.Z);
            float minComponent = i == 0 ? min.X : (i == 1 ? min.Y : min.Z);
            float maxComponent = i == 0 ? max.X : (i == 1 ? max.Y : max.Z);
            
            if (Math.Abs(rayDirComponent) < 0.0001f)
            {
                if (rayStartComponent < minComponent || rayStartComponent > maxComponent)
                    return false;
            }
            else
            {
                float t1 = (minComponent - rayStartComponent) / rayDirComponent;
                float t2 = (maxComponent - rayStartComponent) / rayDirComponent;
                
                if (t1 > t2)
                    (t1, t2) = (t2, t1);
                
                tMin = Math.Max(tMin, t1);
                tMax = Math.Min(tMax, t2);
                
                if (tMin > tMax)
                    return false;
            }
        }
        
        return true;
    }
    
    /// <summary>
    /// Create debris from destroyed blocks
    /// </summary>
    public void CreateDebris(Guid entityId, VoxelBlock block, Vector3 velocity)
    {
        // Create small debris entity
        var debris = _entityManager.CreateEntity($"Debris-{Guid.NewGuid()}");
        
        // Add voxel structure with single block
        var voxelComponent = new VoxelStructureComponent();
        var debrisBlock = new VoxelBlock(
            block.Position,
            block.Size * 0.5f, // Smaller debris
            block.MaterialType,
            block.BlockType
        );
        voxelComponent.AddBlock(debrisBlock);
        _entityManager.AddComponent(debris.Id, voxelComponent);
        
        // Add physics with random velocity
        var random = new Random();
        var physicsComponent = new PhysicsComponent
        {
            Position = block.Position,
            Velocity = velocity + new Vector3(
                (float)(random.NextDouble() - 0.5) * 10,
                (float)(random.NextDouble() - 0.5) * 10,
                (float)(random.NextDouble() - 0.5) * 10
            ),
            AngularVelocity = new Vector3(
                (float)(random.NextDouble() - 0.5) * 5,
                (float)(random.NextDouble() - 0.5) * 5,
                (float)(random.NextDouble() - 0.5) * 5
            ),
            Mass = voxelComponent.TotalMass
        };
        _entityManager.AddComponent(debris.Id, physicsComponent);
    }
}

/// <summary>
/// Type of destruction event
/// </summary>
public enum DestructionType
{
    BlockDestroyed,
    BlockDamaged,
    EntityDestroyed
}

/// <summary>
/// Destruction event data
/// </summary>
public class DestructionEvent
{
    public Guid EntityId { get; set; }
    public VoxelBlock Block { get; set; } = null!;
    public DestructionType Type { get; set; }
}
