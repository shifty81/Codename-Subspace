using System.Numerics;
using AvorionLike.Core.ECS;
using AvorionLike.Core.Voxel;

namespace AvorionLike.Core.Combat;

/// <summary>
/// Types of turrets/weapons
/// </summary>
public enum WeaponType
{
    Chaingun,
    Laser,
    Cannon,
    RocketLauncher,
    Railgun,
    PlasmaGun
}

/// <summary>
/// Represents a turret/weapon
/// </summary>
public class Turret
{
    public string Name { get; set; } = "Basic Turret";
    public WeaponType Type { get; set; } = WeaponType.Chaingun;
    public float Damage { get; set; } = 10f;
    public float FireRate { get; set; } = 1f; // Shots per second
    public float Range { get; set; } = 1000f;
    public float ProjectileSpeed { get; set; } = 500f;
    public float EnergyCost { get; set; } = 5f;
    public bool IsAutoTargeting { get; set; } = false;
    public float TimeSinceLastShot { get; set; } = 0f;
    public Vector3 MountPosition { get; set; } = Vector3.Zero;
}

/// <summary>
/// Component for combat capabilities
/// </summary>
public class CombatComponent : IComponent
{
    public Guid EntityId { get; set; }
    public List<Turret> Turrets { get; set; } = new();
    public float CurrentShields { get; set; } = 0f;
    public float MaxShields { get; set; } = 0f;
    public float ShieldRegenRate { get; set; } = 10f; // Per second
    public float CurrentEnergy { get; set; } = 100f;
    public float MaxEnergy { get; set; } = 100f;
    public Guid? CurrentTarget { get; set; } = null;
    
    /// <summary>
    /// Add a turret to this ship
    /// </summary>
    public void AddTurret(Turret turret)
    {
        Turrets.Add(turret);
    }
    
    /// <summary>
    /// Check if we can fire a turret
    /// </summary>
    public bool CanFire(Turret turret)
    {
        return CurrentEnergy >= turret.EnergyCost && turret.TimeSinceLastShot >= 1f / turret.FireRate;
    }
}

/// <summary>
/// Represents a projectile in flight
/// </summary>
public class Projectile
{
    public Guid SourceEntityId { get; set; }
    public Vector3 Position { get; set; }
    public Vector3 Velocity { get; set; }
    public float Damage { get; set; }
    public float TimeToLive { get; set; }
    public WeaponType Type { get; set; }
}

/// <summary>
/// System for managing combat
/// </summary>
public class CombatSystem : SystemBase
{
    private readonly EntityManager _entityManager;
    private readonly List<Projectile> _activeProjectiles = new();
    
    /// <summary>
    /// Default energy regeneration rate (energy per second)
    /// </summary>
    private const float DefaultEnergyRegenRate = 20f;

    public CombatSystem(EntityManager entityManager) : base("CombatSystem")
    {
        _entityManager = entityManager;
    }

    public override void Update(float deltaTime)
    {
        var combatComponents = _entityManager.GetAllComponents<CombatComponent>();

        foreach (var combat in combatComponents)
        {
            // Update turret cooldowns
            foreach (var turret in combat.Turrets)
            {
                turret.TimeSinceLastShot += deltaTime;
            }
            
            // Regenerate shields
            if (combat.CurrentShields < combat.MaxShields)
            {
                combat.CurrentShields = Math.Min(combat.MaxShields, 
                    combat.CurrentShields + combat.ShieldRegenRate * deltaTime);
            }
            
            // Regenerate energy
            if (combat.CurrentEnergy < combat.MaxEnergy)
            {
                combat.CurrentEnergy = Math.Min(combat.MaxEnergy,
                    combat.CurrentEnergy + DefaultEnergyRegenRate * deltaTime);
            }
            
            // Auto-targeting turrets
            if (combat.CurrentTarget.HasValue)
            {
                UpdateAutoTargeting(combat, deltaTime);
            }
        }
        
        // Update projectiles
        UpdateProjectiles(deltaTime);
    }
    
    /// <summary>
    /// Fire a turret at a target
    /// </summary>
    public bool FireTurret(CombatComponent combat, Turret turret, Vector3 targetPosition, Vector3 shooterPosition)
    {
        if (!combat.CanFire(turret))
        {
            return false;
        }
        
        // Calculate lead for moving targets
        Vector3 direction = Vector3.Normalize(targetPosition - shooterPosition);
        
        // Create projectile
        var projectile = new Projectile
        {
            SourceEntityId = combat.EntityId,
            Position = shooterPosition + turret.MountPosition,
            Velocity = direction * turret.ProjectileSpeed,
            Damage = turret.Damage,
            TimeToLive = turret.Range / turret.ProjectileSpeed,
            Type = turret.Type
        };
        
        _activeProjectiles.Add(projectile);
        
        // Consume energy
        combat.CurrentEnergy -= turret.EnergyCost;
        turret.TimeSinceLastShot = 0f;
        
        return true;
    }
    
    /// <summary>
    /// Update auto-targeting turrets
    /// </summary>
    private void UpdateAutoTargeting(CombatComponent combat, float deltaTime)
    {
        if (!combat.CurrentTarget.HasValue) return;
        
        var targetEntity = _entityManager.GetEntity(combat.CurrentTarget.Value);
        if (targetEntity == null) return;
        
        // Get positions (simplified - would need physics component in real implementation)
        foreach (var turret in combat.Turrets.Where(t => t.IsAutoTargeting))
        {
            // Would fire at target if in range and can fire
            // Simplified for now
        }
    }
    
    /// <summary>
    /// Update all projectiles
    /// </summary>
    private void UpdateProjectiles(float deltaTime)
    {
        var projectilesToRemove = new List<Projectile>();
        
        foreach (var projectile in _activeProjectiles)
        {
            // Update position
            projectile.Position += projectile.Velocity * deltaTime;
            projectile.TimeToLive -= deltaTime;
            
            // Check if expired
            if (projectile.TimeToLive <= 0)
            {
                projectilesToRemove.Add(projectile);
                continue;
            }
            
            // Check for hits
            CheckProjectileHits(projectile, projectilesToRemove);
        }
        
        // Remove expired projectiles
        foreach (var projectile in projectilesToRemove)
        {
            _activeProjectiles.Remove(projectile);
        }
    }
    
    /// <summary>
    /// Check if projectile hits any ships
    /// </summary>
    private void CheckProjectileHits(Projectile projectile, List<Projectile> projectilesToRemove)
    {
        var combatComponents = _entityManager.GetAllComponents<CombatComponent>();
        
        foreach (var combat in combatComponents)
        {
            // Don't hit own ship
            if (combat.EntityId == projectile.SourceEntityId) continue;
            
            // Would need to get position from physics component
            // Simplified collision detection for now
        }
    }
    
    /// <summary>
    /// Apply damage to a ship
    /// </summary>
    public void ApplyDamage(CombatComponent combat, VoxelStructureComponent structure, Vector3 hitPosition, float damage)
    {
        // Shields absorb damage first
        if (combat.CurrentShields > 0)
        {
            float shieldDamage = Math.Min(combat.CurrentShields, damage);
            combat.CurrentShields -= shieldDamage;
            damage -= shieldDamage;
        }
        
        // Remaining damage goes to hull
        if (damage > 0)
        {
            // Damage blocks at hit position
            structure.DamageAtPosition(hitPosition, 5f, damage);
        }
    }
}
