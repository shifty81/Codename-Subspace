using System.Numerics;
using AvorionLike.Core.ECS;
using AvorionLike.Core.Physics;
using AvorionLike.Core.Logging;

namespace AvorionLike.Core.Modular;

/// <summary>
/// System that synchronizes ModularShipComponent stats with PhysicsComponent
/// Ensures physics properties (mass, collision radius) stay up-to-date with ship configuration
/// </summary>
public class ModularShipSyncSystem : SystemBase
{
    private readonly EntityManager _entityManager;
    private readonly Logger _logger = Logger.Instance;
    
    // Synchronization thresholds
    private const float MassSyncThreshold = 0.1f;
    private const float RadiusSyncThreshold = 0.1f;
    
    // Physics constants
    // Note: Using sphere approximation for moment of inertia. Real ships are complex structures,
    // but 0.4 (solid sphere) provides a reasonable approximation. For hollow spheres it would be
    // ~0.67, but ships have distributed mass that falls somewhere in between.
    private const float SphereInertiaConstant = 0.4f; // I = 0.4 * m * r^2
    private const float DefaultMinimumRadius = 1.0f;
    
    public ModularShipSyncSystem(EntityManager entityManager) : base("ModularShipSyncSystem")
    {
        _entityManager = entityManager;
    }
    
    public override void Update(float deltaTime)
    {
        // Get all modular ships
        var modularShips = _entityManager.GetAllComponents<ModularShipComponent>();
        
        foreach (var ship in modularShips)
        {
            SyncShipWithPhysics(ship);
        }
    }
    
    /// <summary>
    /// Synchronize a ship's physics component with its modular configuration
    /// </summary>
    private void SyncShipWithPhysics(ModularShipComponent ship)
    {
        var physics = _entityManager.GetComponent<PhysicsComponent>(ship.EntityId);
        
        if (physics == null)
        {
            // Create physics component if it doesn't exist
            physics = new PhysicsComponent
            {
                EntityId = ship.EntityId,
                Position = Vector3.Zero,
                Velocity = Vector3.Zero,
                Mass = ship.TotalMass,
                CollisionRadius = CalculateCollisionRadius(ship),
                Drag = 0.1f,
                AngularDrag = 0.1f
            };
            
            _entityManager.AddComponent(ship.EntityId, physics);
            _logger.Debug("ModularShipSync", $"Created physics component for ship {ship.Name}");
        }
        else
        {
            // Update physics properties from ship stats
            UpdatePhysicsFromShip(physics, ship);
        }
    }
    
    /// <summary>
    /// Update physics component properties based on ship configuration
    /// </summary>
    private void UpdatePhysicsFromShip(PhysicsComponent physics, ModularShipComponent ship)
    {
        // Early exit if ship is destroyed - no need to update physics properties
        if (ship.IsDestroyed && !physics.IsStatic)
        {
            physics.IsStatic = true;
            physics.Velocity = Vector3.Zero;
            physics.AngularVelocity = Vector3.Zero;
            _logger.Info("ModularShipSync", $"Ship {ship.Name} destroyed - physics set to static");
            return;
        }
        
        // Calculate collision radius once and reuse (performance optimization)
        float newRadius = CalculateCollisionRadius(ship);
        
        // Update mass and moment of inertia (only if significantly changed)
        if (Math.Abs(physics.Mass - ship.TotalMass) > MassSyncThreshold)
        {
            physics.Mass = ship.TotalMass;
            physics.MomentOfInertia = SphereInertiaConstant * physics.Mass * newRadius * newRadius;
        }
        
        // Update collision radius (only if significantly changed)
        if (Math.Abs(physics.CollisionRadius - newRadius) > RadiusSyncThreshold)
        {
            physics.CollisionRadius = newRadius;
        }
    }
    
    /// <summary>
    /// Calculate collision radius from ship bounding box
    /// </summary>
    private float CalculateCollisionRadius(ModularShipComponent ship)
    {
        if (ship.Modules.Count == 0)
            return DefaultMinimumRadius;
        
        // Calculate radius from bounding box diagonal
        // Note: This is slightly conservative for elongated ships, but ensures
        // collision detection works in all orientations. Alternative would be
        // using max(width, height, depth) but that could miss collisions for
        // diagonal approaches.
        var bounds = ship.Bounds;
        var size = bounds.Max - bounds.Min;
        float radius = size.Length() * 0.5f;
        
        // Minimum radius to prevent issues
        return Math.Max(radius, DefaultMinimumRadius);
    }
}
