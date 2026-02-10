using AvorionLike.Core.ECS;
using AvorionLike.Core.Physics;
using AvorionLike.Core.Voxel;
using AvorionLike.Core.Logging;

namespace AvorionLike.Core.Voxel;

/// <summary>
/// System that compiles ship stats from VoxelStructureComponent blocks and
/// synchronises the results into PhysicsComponent.
///
/// This is the authoritative bridge between the block grid and the physics
/// simulation. Gameplay systems should read from CompiledShipStats (via
/// <see cref="GetStats"/>) rather than iterating over blocks.
///
/// Design goals (from architecture review):
///   • One rigid body per ship — no block-level physics.
///   • Mass, thrust, torque, moment-of-inertia derived from blocks.
///   • Stats are recompiled only when the block grid is marked dirty,
///     not every frame.
/// </summary>
public class ShipStatsSyncSystem : SystemBase
{
    private readonly EntityManager _entityManager;

    // Threshold to avoid syncing tiny floating-point changes
    private const float MassSyncThreshold = 0.1f;
    private const float PropulsionSyncThreshold = 0.1f;

    // Minimum moment of inertia to avoid division-by-zero edge cases
    private const float MinMomentOfInertia = 1.0f;

    /// <summary>
    /// Cache of last-compiled stats keyed by entity id.
    /// Read by other systems via <see cref="GetStats"/>.
    /// </summary>
    private readonly Dictionary<Guid, CompiledShipStats> _statsCache = new();

    public ShipStatsSyncSystem(EntityManager entityManager) : base("ShipStatsSyncSystem")
    {
        _entityManager = entityManager;
    }

    public override void Update(float deltaTime)
    {
        var voxelComponents = _entityManager.GetAllComponents<VoxelStructureComponent>();

        foreach (var voxel in voxelComponents)
        {
            // Compile stats from blocks
            var stats = ShipStatsCompiler.Compile(voxel);

            // Cache for other systems to read
            _statsCache[voxel.EntityId] = stats;

            // Sync to PhysicsComponent if one exists
            var physics = _entityManager.GetComponent<PhysicsComponent>(voxel.EntityId);
            if (physics != null)
            {
                SyncPhysics(physics, stats);
            }
        }
    }

    /// <summary>
    /// Get the latest compiled stats for an entity.
    /// Returns default (invalid) stats if the entity hasn't been compiled yet.
    /// </summary>
    public CompiledShipStats GetStats(Guid entityId)
    {
        return _statsCache.TryGetValue(entityId, out var stats) ? stats : default;
    }

    /// <summary>
    /// Force a recompile for a specific entity (e.g. after block damage).
    /// </summary>
    public CompiledShipStats Recompile(Guid entityId)
    {
        var voxel = _entityManager.GetComponent<VoxelStructureComponent>(entityId);
        if (voxel == null) return default;

        var stats = ShipStatsCompiler.Compile(voxel);
        _statsCache[entityId] = stats;

        var physics = _entityManager.GetComponent<PhysicsComponent>(entityId);
        if (physics != null)
        {
            SyncPhysics(physics, stats);
        }

        return stats;
    }

    /// <summary>
    /// Push compiled stats into the PhysicsComponent so the physics
    /// simulation uses ship-level values (not block-level).
    /// </summary>
    private static void SyncPhysics(PhysicsComponent physics, CompiledShipStats stats)
    {
        if (Math.Abs(physics.Mass - stats.Mass) > MassSyncThreshold)
        {
            physics.Mass = stats.Mass > 0 ? stats.Mass : 1f; // avoid zero mass
        }

        physics.MomentOfInertia = Math.Max(stats.MomentOfInertia, MinMomentOfInertia);

        if (Math.Abs(physics.MaxThrust - stats.EffectiveThrust) > PropulsionSyncThreshold)
        {
            physics.MaxThrust = stats.EffectiveThrust;
        }

        if (Math.Abs(physics.MaxTorque - stats.EffectiveTorque) > PropulsionSyncThreshold)
        {
            physics.MaxTorque = stats.EffectiveTorque;
        }
    }
}
