using System.Numerics;
using AvorionLike.Core.ECS;

namespace AvorionLike.Core.Navigation;

/// <summary>
/// Component tracking security status and aggression flags
/// Inspired by EVE Online's CONCORD system
/// </summary>
public class SecurityStatusComponent : IComponent
{
    public Guid EntityId { get; set; }
    
    /// <summary>
    /// Current security status (-10.0 to 10.0)
    /// Negative = criminal, Positive = lawful
    /// </summary>
    public float SecurityStatus { get; set; } = 0f;
    
    /// <summary>
    /// Whether the entity has an active aggression flag
    /// </summary>
    public bool HasAggressionFlag { get; set; } = false;
    
    /// <summary>
    /// Time remaining on aggression flag (seconds)
    /// </summary>
    public float AggressionFlagTimer { get; set; } = 0f;
    
    /// <summary>
    /// Whether the entity has a criminal flag
    /// </summary>
    public bool IsCriminal { get; set; } = false;
    
    /// <summary>
    /// Time remaining on criminal flag (seconds)
    /// </summary>
    public float CriminalFlagTimer { get; set; } = 0f;
    
    /// <summary>
    /// Whether the entity is being targeted by CONCORD
    /// </summary>
    public bool IsCONCORDTarget { get; set; } = false;
    
    /// <summary>
    /// Time until CONCORD response arrives (seconds)
    /// </summary>
    public float CONCORDResponseTimer { get; set; } = 0f;
    
    /// <summary>
    /// List of entities that have been illegally attacked
    /// </summary>
    public HashSet<Guid> IllegalAttackVictims { get; set; } = new();
    
    /// <summary>
    /// Number of kills against lawful targets
    /// </summary>
    public int UnlawfulKills { get; set; } = 0;
    
    /// <summary>
    /// Current sector security level
    /// </summary>
    public SecurityLevel CurrentSectorSecurity { get; set; } = SecurityLevel.HighSec;
}

/// <summary>
/// Configuration for sector security levels
/// </summary>
public class SectorSecurityData
{
    public Vector3 SectorCoordinates { get; set; }
    public SecurityLevel SecurityLevel { get; set; }
    public float SecurityRating { get; set; } // 0.0 to 1.0
    
    /// <summary>
    /// Get security level from rating
    /// </summary>
    public static SecurityLevel GetSecurityLevelFromRating(float rating)
    {
        if (rating >= 0.5f)
            return SecurityLevel.HighSec;
        else if (rating > 0.0f)
            return SecurityLevel.LowSec;
        else if (rating == 0.0f)
            return SecurityLevel.NullSec;
        else
            return SecurityLevel.WormholeSpace;
    }
    
    /// <summary>
    /// Calculate CONCORD response time based on security rating
    /// </summary>
    public float GetCONCORDResponseTime()
    {
        if (SecurityLevel == SecurityLevel.HighSec)
        {
            // 1.0 sec = immediate, 0.5 sec = 6 seconds
            return MathF.Max(1f, 13f - (SecurityRating * 12f));
        }
        else if (SecurityLevel == SecurityLevel.LowSec)
        {
            // Slower response in low-sec
            return 30f + (1f - SecurityRating) * 30f;
        }
        
        // No CONCORD in null-sec or wormhole space
        return float.MaxValue;
    }
}
