using System.Numerics;
using AvorionLike.Core.ECS;
using AvorionLike.Core.Voxel;
using AvorionLike.Core.Events;
using AvorionLike.Core.Logging;

namespace AvorionLike.Core.Physics;

/// <summary>
/// Enhanced collision detection system with AABB and spatial partitioning
/// Provides accurate collision detection and response for voxel-based ships
/// </summary>
public class CollisionSystem : SystemBase
{
    private readonly EntityManager _entityManager;
    private readonly SpatialGrid _spatialGrid;
    private const float CollisionRestitution = 0.5f; // Bounciness factor (0-1)

    public CollisionSystem(EntityManager entityManager) : base("CollisionSystem")
    {
        _entityManager = entityManager;
        _spatialGrid = new SpatialGrid(cellSize: 100f); // 100 units per cell
    }

    public override void Update(float deltaTime)
    {
        // Update spatial grid with all physics entities
        _spatialGrid.Clear();
        var physicsComponents = _entityManager.GetAllComponents<PhysicsComponent>();
        
        foreach (var physics in physicsComponents)
        {
            _spatialGrid.Insert(physics.EntityId, GetBoundingBox(physics));
        }

        // Detect and handle collisions
        foreach (var physics1 in physicsComponents)
        {
            if (physics1.IsStatic) continue;

            var box1 = GetBoundingBox(physics1);
            var potentialCollisions = _spatialGrid.QueryNearby(box1);

            foreach (var entityId2 in potentialCollisions)
            {
                if (entityId2 == physics1.EntityId) continue;

                var physics2 = _entityManager.GetComponent<PhysicsComponent>(entityId2);
                if (physics2 == null) continue;
                if (physics1.IsStatic && physics2.IsStatic) continue;

                var box2 = GetBoundingBox(physics2);
                
                if (CheckAABBCollision(box1, box2, out var collisionData))
                {
                    HandleCollision(physics1, physics2, collisionData);
                    
                    // Publish collision event
                    EventSystem.Instance.Publish(GameEvents.EntityCollision, new EntityCollisionEvent
                    {
                        Entity1Id = physics1.EntityId,
                        Entity2Id = physics2.EntityId,
                        CollisionPoint = collisionData.ContactPoint,
                        CollisionNormal = collisionData.Normal,
                        CollisionImpulse = collisionData.Impulse
                    });
                }
            }
        }
    }

    /// <summary>
    /// Get the axis-aligned bounding box for an entity
    /// </summary>
    private AABB GetBoundingBox(PhysicsComponent physics)
    {
        // Try to get actual bounds from voxel structure
        var voxelComponent = _entityManager.GetComponent<VoxelStructureComponent>(physics.EntityId);
        
        if (voxelComponent != null && voxelComponent.Blocks.Any())
        {
            Vector3 min = new Vector3(float.MaxValue);
            Vector3 max = new Vector3(float.MinValue);

            foreach (var block in voxelComponent.Blocks)
            {
                var blockMin = physics.Position + block.Position - block.Size * 0.5f;
                var blockMax = physics.Position + block.Position + block.Size * 0.5f;

                min = Vector3.Min(min, blockMin);
                max = Vector3.Max(max, blockMax);
            }

            return new AABB(min, max);
        }
        else
        {
            // Fallback to sphere-based bounding box
            var radius = physics.CollisionRadius;
            return new AABB(
                physics.Position - new Vector3(radius),
                physics.Position + new Vector3(radius)
            );
        }
    }

    /// <summary>
    /// Check if two AABBs are colliding
    /// </summary>
    private bool CheckAABBCollision(AABB box1, AABB box2, out CollisionData collisionData)
    {
        collisionData = new CollisionData();

        // Check for overlap on all axes
        if (box1.Max.X < box2.Min.X || box1.Min.X > box2.Max.X) return false;
        if (box1.Max.Y < box2.Min.Y || box1.Min.Y > box2.Max.Y) return false;
        if (box1.Max.Z < box2.Min.Z || box1.Min.Z > box2.Max.Z) return false;

        // Calculate penetration depth on each axis
        float overlapX = Math.Min(box1.Max.X - box2.Min.X, box2.Max.X - box1.Min.X);
        float overlapY = Math.Min(box1.Max.Y - box2.Min.Y, box2.Max.Y - box1.Min.Y);
        float overlapZ = Math.Min(box1.Max.Z - box2.Min.Z, box2.Max.Z - box1.Min.Z);

        // Find the axis with minimum penetration (collision normal)
        if (overlapX < overlapY && overlapX < overlapZ)
        {
            collisionData.Normal = box1.Center.X < box2.Center.X ? new Vector3(-1, 0, 0) : new Vector3(1, 0, 0);
            collisionData.PenetrationDepth = overlapX;
        }
        else if (overlapY < overlapZ)
        {
            collisionData.Normal = box1.Center.Y < box2.Center.Y ? new Vector3(0, -1, 0) : new Vector3(0, 1, 0);
            collisionData.PenetrationDepth = overlapY;
        }
        else
        {
            collisionData.Normal = box1.Center.Z < box2.Center.Z ? new Vector3(0, 0, -1) : new Vector3(0, 0, 1);
            collisionData.PenetrationDepth = overlapZ;
        }

        // Calculate contact point (center of overlap volume)
        collisionData.ContactPoint = new Vector3(
            (Math.Max(box1.Min.X, box2.Min.X) + Math.Min(box1.Max.X, box2.Max.X)) * 0.5f,
            (Math.Max(box1.Min.Y, box2.Min.Y) + Math.Min(box1.Max.Y, box2.Max.Y)) * 0.5f,
            (Math.Max(box1.Min.Z, box2.Min.Z) + Math.Min(box1.Max.Z, box2.Max.Z)) * 0.5f
        );

        return true;
    }

    /// <summary>
    /// Handle collision response between two physics objects
    /// </summary>
    private void HandleCollision(PhysicsComponent obj1, PhysicsComponent obj2, CollisionData data)
    {
        // Separate objects to prevent overlap
        if (!obj1.IsStatic && !obj2.IsStatic)
        {
            float totalMass = obj1.Mass + obj2.Mass;
            obj1.Position -= data.Normal * data.PenetrationDepth * (obj2.Mass / totalMass);
            obj2.Position += data.Normal * data.PenetrationDepth * (obj1.Mass / totalMass);
        }
        else if (!obj1.IsStatic)
        {
            obj1.Position -= data.Normal * data.PenetrationDepth;
        }
        else if (!obj2.IsStatic)
        {
            obj2.Position += data.Normal * data.PenetrationDepth;
        }

        // Apply impulse-based collision response
        Vector3 relativeVelocity = obj1.Velocity - obj2.Velocity;
        float velocityAlongNormal = Vector3.Dot(relativeVelocity, data.Normal);

        // Objects moving apart? No response needed
        if (velocityAlongNormal > 0) return;

        // Calculate impulse magnitude
        float invMass1 = obj1.IsStatic ? 0 : 1f / obj1.Mass;
        float invMass2 = obj2.IsStatic ? 0 : 1f / obj2.Mass;
        float j = -(1 + CollisionRestitution) * velocityAlongNormal;
        j /= invMass1 + invMass2;

        // Apply impulse
        Vector3 impulse = j * data.Normal;
        data.Impulse = impulse.Length();

        if (!obj1.IsStatic)
        {
            obj1.Velocity += impulse * invMass1;
        }
        if (!obj2.IsStatic)
        {
            obj2.Velocity -= impulse * invMass2;
        }

        Logger.Instance.Debug("CollisionSystem", 
            $"Collision: Entity {obj1.EntityId} vs {obj2.EntityId}, Impulse: {data.Impulse:F2}");
    }
}

/// <summary>
/// Axis-Aligned Bounding Box
/// </summary>
public struct AABB
{
    public Vector3 Min { get; set; }
    public Vector3 Max { get; set; }
    public Vector3 Center => (Min + Max) * 0.5f;
    public Vector3 Size => Max - Min;

    public AABB(Vector3 min, Vector3 max)
    {
        Min = min;
        Max = max;
    }

    public bool Intersects(AABB other)
    {
        return !(Max.X < other.Min.X || Min.X > other.Max.X ||
                 Max.Y < other.Min.Y || Min.Y > other.Max.Y ||
                 Max.Z < other.Min.Z || Min.Z > other.Max.Z);
    }
}

/// <summary>
/// Collision data containing information about a collision
/// </summary>
public struct CollisionData
{
    public Vector3 Normal;
    public Vector3 ContactPoint;
    public float PenetrationDepth;
    public float Impulse;
}

/// <summary>
/// Spatial grid for efficient broad-phase collision detection
/// Divides space into cells and only checks collisions within nearby cells
/// </summary>
public class SpatialGrid
{
    private readonly float _cellSize;
    private readonly Dictionary<Vector3Int, HashSet<Guid>> _grid;

    public SpatialGrid(float cellSize)
    {
        _cellSize = cellSize;
        _grid = new Dictionary<Vector3Int, HashSet<Guid>>();
    }

    public void Clear()
    {
        _grid.Clear();
    }

    /// <summary>
    /// Insert an entity's bounding box into the spatial grid
    /// </summary>
    public void Insert(Guid entityId, AABB bounds)
    {
        var minCell = WorldToCell(bounds.Min);
        var maxCell = WorldToCell(bounds.Max);

        for (int x = minCell.X; x <= maxCell.X; x++)
        {
            for (int y = minCell.Y; y <= maxCell.Y; y++)
            {
                for (int z = minCell.Z; z <= maxCell.Z; z++)
                {
                    var cellKey = new Vector3Int(x, y, z);
                    if (!_grid.ContainsKey(cellKey))
                    {
                        _grid[cellKey] = new HashSet<Guid>();
                    }
                    _grid[cellKey].Add(entityId);
                }
            }
        }
    }

    /// <summary>
    /// Query entities near a bounding box
    /// </summary>
    public HashSet<Guid> QueryNearby(AABB bounds)
    {
        var result = new HashSet<Guid>();
        var minCell = WorldToCell(bounds.Min);
        var maxCell = WorldToCell(bounds.Max);

        for (int x = minCell.X; x <= maxCell.X; x++)
        {
            for (int y = minCell.Y; y <= maxCell.Y; y++)
            {
                for (int z = minCell.Z; z <= maxCell.Z; z++)
                {
                    var cellKey = new Vector3Int(x, y, z);
                    if (_grid.TryGetValue(cellKey, out var entities))
                    {
                        result.UnionWith(entities);
                    }
                }
            }
        }

        return result;
    }

    private Vector3Int WorldToCell(Vector3 position)
    {
        return new Vector3Int(
            (int)Math.Floor(position.X / _cellSize),
            (int)Math.Floor(position.Y / _cellSize),
            (int)Math.Floor(position.Z / _cellSize)
        );
    }
}

/// <summary>
/// Integer vector for grid cell coordinates
/// </summary>
public struct Vector3Int : IEquatable<Vector3Int>
{
    public int X { get; set; }
    public int Y { get; set; }
    public int Z { get; set; }

    public Vector3Int(int x, int y, int z)
    {
        X = x;
        Y = y;
        Z = z;
    }

    public bool Equals(Vector3Int other)
    {
        return X == other.X && Y == other.Y && Z == other.Z;
    }

    public override bool Equals(object? obj)
    {
        return obj is Vector3Int other && Equals(other);
    }

    public override int GetHashCode()
    {
        return HashCode.Combine(X, Y, Z);
    }

    public static bool operator ==(Vector3Int left, Vector3Int right)
    {
        return left.Equals(right);
    }

    public static bool operator !=(Vector3Int left, Vector3Int right)
    {
        return !left.Equals(right);
    }
}

/// <summary>
/// Event data for entity collisions
/// </summary>
public class EntityCollisionEvent : GameEvent
{
    public Guid Entity1Id { get; set; }
    public Guid Entity2Id { get; set; }
    public Vector3 CollisionPoint { get; set; }
    public Vector3 CollisionNormal { get; set; }
    public float CollisionImpulse { get; set; }
}
