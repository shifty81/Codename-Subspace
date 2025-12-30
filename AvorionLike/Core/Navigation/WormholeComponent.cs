using System.Numerics;
using AvorionLike.Core.ECS;

namespace AvorionLike.Core.Navigation;

/// <summary>
/// Component representing a wormhole connection between sectors
/// Inspired by EVE Online's wormhole mechanics
/// </summary>
public class WormholeComponent : IComponent
{
    public Guid EntityId { get; set; }
    
    /// <summary>
    /// Classification of this wormhole
    /// </summary>
    public WormholeClass Class { get; set; } = WormholeClass.Class1;
    
    /// <summary>
    /// Type of wormhole (Static or Wandering)
    /// </summary>
    public WormholeType Type { get; set; } = WormholeType.Wandering;
    
    /// <summary>
    /// Current stability state
    /// </summary>
    public WormholeStability Stability { get; set; } = WormholeStability.Stable;
    
    /// <summary>
    /// Source sector coordinates
    /// </summary>
    public Vector3 SourceSector { get; set; }
    
    /// <summary>
    /// Destination sector coordinates
    /// </summary>
    public Vector3 DestinationSector { get; set; }
    
    /// <summary>
    /// Destination security level (for static wormholes)
    /// </summary>
    public SecurityLevel? StaticDestinationType { get; set; }
    
    /// <summary>
    /// Maximum lifetime in seconds before natural collapse
    /// </summary>
    public float MaxLifetime { get; set; } = 172800f; // 48 hours default
    
    /// <summary>
    /// Current remaining lifetime in seconds
    /// </summary>
    public float RemainingLifetime { get; set; }
    
    /// <summary>
    /// Maximum total mass that can pass through (kg)
    /// </summary>
    public float MaxTotalMass { get; set; } = 2000000000f; // 2 billion kg
    
    /// <summary>
    /// Remaining mass capacity (kg)
    /// </summary>
    public float RemainingMass { get; set; }
    
    /// <summary>
    /// Maximum single-jump mass (prevents oversized ships)
    /// </summary>
    public float MaxShipMass { get; set; } = 300000000f; // 300 million kg
    
    /// <summary>
    /// Whether this wormhole is currently active
    /// </summary>
    public bool IsActive { get; set; } = true;
    
    /// <summary>
    /// Signature strength for scanning (0.0 - 1.0)
    /// </summary>
    public float SignatureStrength { get; set; } = 1.0f;
    
    /// <summary>
    /// Position in current sector
    /// </summary>
    public Vector3 Position { get; set; }
    
    /// <summary>
    /// Visual size/radius for rendering
    /// </summary>
    public float Radius { get; set; } = 500f;
    
    /// <summary>
    /// Unique identifier for this wormhole
    /// </summary>
    public string Designation { get; set; } = "Unknown";
    
    /// <summary>
    /// Check if a ship with given mass can jump through
    /// </summary>
    public bool CanShipJump(float shipMass)
    {
        if (!IsActive || Stability == WormholeStability.Collapsed)
            return false;
            
        if (shipMass > MaxShipMass)
            return false;
            
        if (shipMass > RemainingMass)
            return false;
            
        return true;
    }
    
    /// <summary>
    /// Process a ship jumping through
    /// </summary>
    public void ProcessJump(float shipMass)
    {
        RemainingMass -= shipMass;
        
        // Jumping destabilizes the wormhole
        float destabilizationFactor = shipMass / MaxTotalMass;
        RemainingLifetime -= destabilizationFactor * 3600f; // Lose up to 1 hour per jump
        
        // Update stability based on remaining resources
        UpdateStability();
    }
    
    /// <summary>
    /// Update stability state based on remaining lifetime and mass
    /// </summary>
    private void UpdateStability()
    {
        float lifetimePercent = RemainingLifetime / MaxLifetime;
        float massPercent = RemainingMass / MaxTotalMass;
        float overallHealth = (lifetimePercent + massPercent) / 2f;
        
        if (overallHealth <= 0f)
        {
            Stability = WormholeStability.Collapsed;
            IsActive = false;
        }
        else if (overallHealth < 0.25f)
        {
            Stability = WormholeStability.Critical;
        }
        else if (overallHealth < 0.5f)
        {
            Stability = WormholeStability.Destabilizing;
        }
        else
        {
            Stability = WormholeStability.Stable;
        }
    }
    
    /// <summary>
    /// Age the wormhole by delta time
    /// </summary>
    public void Age(float deltaTime)
    {
        RemainingLifetime -= deltaTime;
        
        if (RemainingLifetime <= 0)
        {
            Stability = WormholeStability.Collapsed;
            IsActive = false;
        }
        else
        {
            UpdateStability();
        }
    }
    
    /// <summary>
    /// Get the mass class restrictions for this wormhole
    /// </summary>
    public string GetMassRestriction()
    {
        return Class switch
        {
            WormholeClass.Class1 => "All ships",
            WormholeClass.Class2 => "All ships",
            WormholeClass.Class3 => "All ships",
            WormholeClass.Class4 => "Capitals restricted",
            WormholeClass.Class5 => "Heavy capitals restricted",
            WormholeClass.Class6 => "Super capitals restricted",
            WormholeClass.HighSec => "All ships",
            WormholeClass.LowSec => "All ships",
            WormholeClass.NullSec => "All ships",
            _ => "Unknown"
        };
    }
}
