using AvorionLike.Core.ECS;
using AvorionLike.Core.Voxel;
using AvorionLike.Core.Combat;
using AvorionLike.Core.Physics;
using AvorionLike.Core.Events;
using AvorionLike.Core.Logging;

namespace AvorionLike.Core.Power;

/// <summary>
/// System for managing ship power generation, distribution, and consumption
/// Handles power priorities and automatic system shutdown when power is insufficient
/// </summary>
public class PowerSystem : SystemBase
{
    private readonly EntityManager _entityManager;
    private readonly EventSystem _eventSystem;
    private readonly Logger _logger;
    
    // Power consumption rates (per unit)
    private const float ENGINE_POWER_CONSUMPTION = 5f;
    private const float THRUSTER_POWER_CONSUMPTION = 3f;
    private const float SHIELD_BASE_CONSUMPTION = 10f;
    private const float WEAPON_BASE_CONSUMPTION = 8f;
    private const float GYRO_POWER_CONSUMPTION = 2f;
    
    // Power storage rates
    private const float STORAGE_CHARGE_RATE = 10f; // Power per second to charge storage
    private const float STORAGE_CAPACITY_PER_GENERATOR = 50f;
    
    public PowerSystem(EntityManager entityManager, EventSystem eventSystem, Logger logger) 
        : base("PowerSystem")
    {
        _entityManager = entityManager;
        _eventSystem = eventSystem;
        _logger = logger;
    }
    
    public override void Update(float deltaTime)
    {
        var entities = _entityManager.GetAllEntities();
        
        foreach (var entity in entities)
        {
            var powerComponent = _entityManager.GetComponent<PowerComponent>(entity.Id);
            if (powerComponent == null) continue;
            
            var voxelComponent = _entityManager.GetComponent<VoxelStructureComponent>(entity.Id);
            if (voxelComponent == null) continue;
            
            // Calculate power generation from generators
            CalculatePowerGeneration(powerComponent, voxelComponent);
            
            // Calculate power consumption from all systems
            CalculatePowerConsumption(powerComponent, voxelComponent, entity.Id);
            
            // Update total consumption
            powerComponent.UpdateTotalConsumption();
            
            // Handle power distribution and priorities
            DistributePower(powerComponent, entity.Id, deltaTime);
            
            // Charge power storage if excess power available
            ChargePowerStorage(powerComponent, deltaTime);
            
            // Apply power effects to systems
            ApplyPowerEffects(powerComponent, entity.Id);
        }
    }
    
    /// <summary>
    /// Calculate total power generation from all generator blocks
    /// </summary>
    private void CalculatePowerGeneration(PowerComponent power, VoxelStructureComponent voxels)
    {
        power.MaxPowerGeneration = voxels.PowerGeneration;
        power.CurrentPowerGeneration = voxels.PowerGeneration;
        
        // Calculate storage capacity based on number of generators
        var generators = voxels.GetBlocksByType(BlockType.Generator).Count();
        power.MaxStoredPower = generators * STORAGE_CAPACITY_PER_GENERATOR;
    }
    
    /// <summary>
    /// Calculate power consumption from all active systems
    /// </summary>
    private void CalculatePowerConsumption(PowerComponent power, VoxelStructureComponent voxels, Guid entityId)
    {
        // Engines and thrusters
        var engines = voxels.GetBlocksByType(BlockType.Engine).Count();
        var thrusters = voxels.GetBlocksByType(BlockType.Thruster).Count();
        var gyros = voxels.GetBlocksByType(BlockType.GyroArray).Count();
        power.EnginesPowerConsumption = (engines * ENGINE_POWER_CONSUMPTION) + 
                                        (thrusters * THRUSTER_POWER_CONSUMPTION) +
                                        (gyros * GYRO_POWER_CONSUMPTION);
        
        // Shields
        var shieldGenerators = voxels.GetBlocksByType(BlockType.ShieldGenerator).Count();
        power.ShieldsPowerConsumption = shieldGenerators * SHIELD_BASE_CONSUMPTION;
        
        // Weapons
        var combatComponent = _entityManager.GetComponent<CombatComponent>(entityId);
        if (combatComponent != null)
        {
            // Base consumption + additional per weapon
            power.WeaponsPowerConsumption = WEAPON_BASE_CONSUMPTION * combatComponent.Turrets.Count;
        }
        else
        {
            power.WeaponsPowerConsumption = 0;
        }
        
        // Systems (life support, sensors, etc.) - fixed small amount
        power.SystemsPowerConsumption = 5f;
    }
    
    /// <summary>
    /// Distribute power based on priorities when there's insufficient power
    /// </summary>
    private void DistributePower(PowerComponent power, Guid entityId, float deltaTime)
    {
        float powerDeficit = power.GetPowerDeficit();
        
        if (powerDeficit <= 0) return; // Sufficient power
        
        // If we have stored power, use it first
        if (power.CurrentStoredPower > 0)
        {
            float powerNeeded = Math.Min(powerDeficit, power.CurrentStoredPower);
            power.CurrentStoredPower -= powerNeeded;
            _logger.Log(LogLevel.Debug, "PowerSystem", $"Entity {entityId}: Using {powerNeeded:F1}W from storage");
            return;
        }
        
        // Not enough power - disable systems by priority (lowest priority first)
        var systemsByPriority = new List<(PowerSystemType system, int priority)>
        {
            (PowerSystemType.Weapons, power.WeaponsPriority),
            (PowerSystemType.Shields, power.ShieldsPriority),
            (PowerSystemType.Engines, power.EnginesPriority),
            (PowerSystemType.Systems, power.SystemsPriority)
        }
        .OrderByDescending(x => x.priority) // Higher number = lower priority = disabled first
        .ToList();
        
        foreach (var (system, priority) in systemsByPriority)
        {
            if (power.GetPowerDeficit() <= 0) break; // Sufficient power now
            
            bool wasEnabled = system switch
            {
                PowerSystemType.Weapons => power.WeaponsEnabled,
                PowerSystemType.Shields => power.ShieldsEnabled,
                PowerSystemType.Engines => power.EnginesEnabled,
                PowerSystemType.Systems => power.SystemsEnabled,
                _ => false
            };
            
            if (wasEnabled)
            {
                power.ToggleSystem(system);
                _logger.Log(LogLevel.Warning, "PowerSystem", $"Entity {entityId}: {system} disabled due to insufficient power");
                
                // Publish power shortage event
                _eventSystem.Publish("PowerShortage", new PowerShortageEvent
                {
                    EntityId = entityId,
                    DisabledSystem = system,
                    PowerDeficit = power.GetPowerDeficit()
                });
            }
        }
    }
    
    /// <summary>
    /// Charge power storage when excess power is available
    /// </summary>
    private void ChargePowerStorage(PowerComponent power, float deltaTime)
    {
        if (power.CurrentStoredPower >= power.MaxStoredPower) return;
        
        float excessPower = power.GetAvailablePower();
        if (excessPower <= 0) return;
        
        float chargeAmount = Math.Min(
            STORAGE_CHARGE_RATE * deltaTime,
            Math.Min(excessPower, power.MaxStoredPower - power.CurrentStoredPower)
        );
        
        power.CurrentStoredPower += chargeAmount;
    }
    
    /// <summary>
    /// Apply power effects to various ship systems
    /// </summary>
    private void ApplyPowerEffects(PowerComponent power, Guid entityId)
    {
        // Reduce shield effectiveness if shields are disabled
        var combatComponent = _entityManager.GetComponent<CombatComponent>(entityId);
        if (combatComponent != null && !power.ShieldsEnabled)
        {
            // Shields slowly drain when disabled
            combatComponent.CurrentShields = Math.Max(0, combatComponent.CurrentShields - 1f);
        }
        
        // Reduce engine thrust if engines are disabled
        var physicsComponent = _entityManager.GetComponent<PhysicsComponent>(entityId);
        if (physicsComponent != null && !power.EnginesEnabled)
        {
            // Can't apply thrust without power
            physicsComponent.MaxThrust *= 0.1f; // 10% emergency thrust
        }
    }
}

/// <summary>
/// Event fired when a ship experiences power shortage
/// </summary>
public class PowerShortageEvent : GameEvent
{
    public Guid EntityId { get; set; }
    public PowerSystemType DisabledSystem { get; set; }
    public float PowerDeficit { get; set; }
}
