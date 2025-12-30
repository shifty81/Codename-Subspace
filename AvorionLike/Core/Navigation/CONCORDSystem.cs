using System.Numerics;
using AvorionLike.Core.ECS;
using AvorionLike.Core.Logging;
using AvorionLike.Core.Physics;
using AvorionLike.Core.Combat;

namespace AvorionLike.Core.Navigation;

/// <summary>
/// CONCORD enforcement system - automated law enforcement
/// Inspired by EVE Online's CONCORD mechanics
/// </summary>
public class CONCORDSystem : SystemBase
{
    private readonly EntityManager _entityManager;
    private readonly Dictionary<Vector3, SectorSecurityData> _sectorSecurity = new();
    private readonly Random _random = new();
    
    // CONCORD settings
    private const float AggressionFlagDuration = 60f; // 1 minute
    private const float CriminalFlagDuration = 900f; // 15 minutes
    private const float SecurityStatusLossPerKill = 0.5f;
    private const float CONCORDDamagePerSecond = 1000000f; // Massive damage
    
    public CONCORDSystem(EntityManager entityManager) : base("CONCORDSystem")
    {
        _entityManager = entityManager;
        InitializeSectorSecurity();
    }
    
    public override void Update(float deltaTime)
    {
        var securityStatuses = _entityManager.GetAllComponents<SecurityStatusComponent>();
        
        foreach (var status in securityStatuses)
        {
            UpdateSecurityStatus(status, deltaTime);
            
            // Process CONCORD response
            if (status.IsCONCORDTarget)
            {
                ProcessCONCORDResponse(status, deltaTime);
            }
        }
    }
    
    /// <summary>
    /// Initialize sector security levels based on distance from center
    /// </summary>
    private void InitializeSectorSecurity()
    {
        // High-sec near center, null-sec at edges (simplified)
        // In a full implementation, this would be more sophisticated
    }
    
    /// <summary>
    /// Update timers and flags for a security status
    /// </summary>
    private void UpdateSecurityStatus(SecurityStatusComponent status, float deltaTime)
    {
        // Update aggression flag
        if (status.HasAggressionFlag)
        {
            status.AggressionFlagTimer -= deltaTime;
            if (status.AggressionFlagTimer <= 0)
            {
                status.HasAggressionFlag = false;
                status.AggressionFlagTimer = 0;
                Logger.Instance.Info("CONCORDSystem", "Aggression flag expired");
            }
        }
        
        // Update criminal flag
        if (status.IsCriminal)
        {
            status.CriminalFlagTimer -= deltaTime;
            if (status.CriminalFlagTimer <= 0)
            {
                status.IsCriminal = false;
                status.CriminalFlagTimer = 0;
                Logger.Instance.Info("CONCORDSystem", "Criminal flag expired");
            }
        }
        
        // Update CONCORD response timer
        if (status.CONCORDResponseTimer > 0)
        {
            status.CONCORDResponseTimer -= deltaTime;
            if (status.CONCORDResponseTimer <= 0 && status.IsCONCORDTarget)
            {
                // CONCORD arrives
                Logger.Instance.Warning("CONCORDSystem", "CONCORD has arrived!");
            }
        }
    }
    
    /// <summary>
    /// Process CONCORD attacking a criminal
    /// </summary>
    private void ProcessCONCORDResponse(SecurityStatusComponent status, float deltaTime)
    {
        if (status.CONCORDResponseTimer > 0)
            return; // Still on the way
            
        // CONCORD is here - deal massive damage
        var combat = _entityManager.GetComponent<CombatComponent>(status.EntityId);
        if (combat != null)
        {
            float damage = CONCORDDamagePerSecond * deltaTime;
            combat.CurrentShields = MathF.Max(0, combat.CurrentShields - damage);
            
            if (combat.CurrentShields <= 0)
            {
                // Shields down, destroy ship
                Logger.Instance.Warning("CONCORDSystem", $"CONCORD destroyed entity {status.EntityId}");
                _entityManager.DestroyEntity(status.EntityId);
            }
        }
    }
    
    /// <summary>
    /// Register an illegal attack
    /// </summary>
    public void RegisterIllegalAttack(Guid attackerId, Guid victimId, Vector3 sectorCoordinates)
    {
        var attackerStatus = _entityManager.GetComponent<SecurityStatusComponent>(attackerId);
        if (attackerStatus == null)
        {
            // Create security status component
            attackerStatus = new SecurityStatusComponent
            {
                EntityId = attackerId
            };
            _entityManager.AddComponent(attackerId, attackerStatus);
        }
        
        // Get sector security
        var sectorSecurity = GetSectorSecurity(sectorCoordinates);
        attackerStatus.CurrentSectorSecurity = sectorSecurity.SecurityLevel;
        
        // Apply aggression flag
        attackerStatus.HasAggressionFlag = true;
        attackerStatus.AggressionFlagTimer = AggressionFlagDuration;
        
        // Check if this is an illegal attack
        var victimStatus = _entityManager.GetComponent<SecurityStatusComponent>(victimId);
        bool victimIsLawful = victimStatus == null || victimStatus.SecurityStatus >= 0;
        
        if (victimIsLawful)
        {
            // Illegal attack on lawful target
            attackerStatus.IsCriminal = true;
            attackerStatus.CriminalFlagTimer = CriminalFlagDuration;
            attackerStatus.IllegalAttackVictims.Add(victimId);
            
            // Security status loss
            attackerStatus.SecurityStatus -= SecurityStatusLossPerKill;
            
            // Trigger CONCORD in high-sec or low-sec
            if (sectorSecurity.SecurityLevel == SecurityLevel.HighSec || 
                sectorSecurity.SecurityLevel == SecurityLevel.LowSec)
            {
                TriggerCONCORDResponse(attackerStatus, sectorSecurity);
            }
            
            Logger.Instance.Warning("CONCORDSystem", 
                $"Illegal attack by {attackerId} on {victimId} in {sectorSecurity.SecurityLevel}");
        }
        else
        {
            // Legal attack on criminal
            Logger.Instance.Info("CONCORDSystem", "Legal attack on criminal target");
        }
    }
    
    /// <summary>
    /// Trigger CONCORD response to criminal activity
    /// </summary>
    private void TriggerCONCORDResponse(SecurityStatusComponent criminal, SectorSecurityData sector)
    {
        criminal.IsCONCORDTarget = true;
        criminal.CONCORDResponseTimer = sector.GetCONCORDResponseTime();
        
        Logger.Instance.Warning("CONCORDSystem", 
            $"CONCORD response initiated - arriving in {criminal.CONCORDResponseTimer:F1} seconds");
    }
    
    /// <summary>
    /// Register a kill
    /// </summary>
    public void RegisterKill(Guid killerId, Guid victimId)
    {
        var killerStatus = _entityManager.GetComponent<SecurityStatusComponent>(killerId);
        if (killerStatus == null)
            return;
            
        var victimStatus = _entityManager.GetComponent<SecurityStatusComponent>(victimId);
        bool victimWasLawful = victimStatus == null || victimStatus.SecurityStatus >= 0;
        
        if (victimWasLawful)
        {
            killerStatus.UnlawfulKills++;
            killerStatus.SecurityStatus -= SecurityStatusLossPerKill * 2; // Double penalty for kill
            
            Logger.Instance.Warning("CONCORDSystem", 
                $"Unlawful kill by {killerId} - Security status now {killerStatus.SecurityStatus:F1}");
        }
    }
    
    /// <summary>
    /// Get or create sector security data
    /// </summary>
    public SectorSecurityData GetSectorSecurity(Vector3 sectorCoordinates)
    {
        if (_sectorSecurity.TryGetValue(sectorCoordinates, out var data))
            return data;
            
        // Generate security based on distance from center
        float distanceFromCenter = sectorCoordinates.Length();
        float securityRating = CalculateSecurityRating(distanceFromCenter);
        
        data = new SectorSecurityData
        {
            SectorCoordinates = sectorCoordinates,
            SecurityRating = securityRating,
            SecurityLevel = SectorSecurityData.GetSecurityLevelFromRating(securityRating)
        };
        
        _sectorSecurity[sectorCoordinates] = data;
        return data;
    }
    
    /// <summary>
    /// Calculate security rating based on distance from galactic center
    /// </summary>
    private float CalculateSecurityRating(float distanceFromCenter)
    {
        // High-sec in inner regions, null-sec at edges
        if (distanceFromCenter < 100f)
            return 1.0f; // Core regions
        else if (distanceFromCenter < 200f)
            return 0.8f; // High-sec
        else if (distanceFromCenter < 300f)
            return 0.5f; // Mid high-sec
        else if (distanceFromCenter < 400f)
            return 0.3f; // Low-sec
        else if (distanceFromCenter < 500f)
            return 0.1f; // Low-sec edge
        else
            return 0.0f; // Null-sec
    }
    
    /// <summary>
    /// Check if an attack would be legal
    /// </summary>
    public bool IsAttackLegal(Guid attackerId, Guid victimId)
    {
        var victimStatus = _entityManager.GetComponent<SecurityStatusComponent>(victimId);
        
        // Attack is legal if victim is criminal
        if (victimStatus != null && (victimStatus.IsCriminal || victimStatus.SecurityStatus < -2.0f))
            return true;
            
        return false;
    }
    
    /// <summary>
    /// Can an entity dock at a station?
    /// </summary>
    public bool CanDock(Guid entityId, SecurityLevel stationSecurity)
    {
        var status = _entityManager.GetComponent<SecurityStatusComponent>(entityId);
        if (status == null)
            return true;
            
        // Criminals cannot dock in high-sec
        if (status.IsCriminal && stationSecurity == SecurityLevel.HighSec)
            return false;
            
        // Very low security status cannot dock in high-sec
        if (status.SecurityStatus < -5.0f && stationSecurity == SecurityLevel.HighSec)
            return false;
            
        return true;
    }
}
