namespace AvorionLike.Core.Navigation;

/// <summary>
/// Classification of wormholes determining difficulty and ship restrictions
/// Inspired by EVE Online's wormhole space
/// </summary>
public enum WormholeClass
{
    /// <summary>
    /// Class 1 - Easiest, allows all ship sizes
    /// </summary>
    Class1 = 1,
    
    /// <summary>
    /// Class 2 - Low difficulty, allows all ship sizes
    /// </summary>
    Class2 = 2,
    
    /// <summary>
    /// Class 3 - Medium difficulty, allows all ship sizes
    /// </summary>
    Class3 = 3,
    
    /// <summary>
    /// Class 4 - High difficulty, capital ship restrictions
    /// </summary>
    Class4 = 4,
    
    /// <summary>
    /// Class 5 - Very high difficulty, strict capital limits
    /// </summary>
    Class5 = 5,
    
    /// <summary>
    /// Class 6 - Highest difficulty, very strict mass limits
    /// </summary>
    Class6 = 6,
    
    /// <summary>
    /// High-security space connection
    /// </summary>
    HighSec,
    
    /// <summary>
    /// Low-security space connection
    /// </summary>
    LowSec,
    
    /// <summary>
    /// Null-security (lawless) space connection
    /// </summary>
    NullSec
}

/// <summary>
/// Type of wormhole connection behavior
/// </summary>
public enum WormholeType
{
    /// <summary>
    /// Always leads to the same type of destination
    /// </summary>
    Static,
    
    /// <summary>
    /// Random destination, changes each spawn
    /// </summary>
    Wandering
}

/// <summary>
/// Current stability state of the wormhole
/// </summary>
public enum WormholeStability
{
    /// <summary>
    /// Freshly spawned, fully stable
    /// </summary>
    Stable,
    
    /// <summary>
    /// Beginning to destabilize
    /// </summary>
    Destabilizing,
    
    /// <summary>
    /// Critical stability, near collapse
    /// </summary>
    Critical,
    
    /// <summary>
    /// Collapsed, no longer traversable
    /// </summary>
    Collapsed
}

/// <summary>
/// Security level of space
/// </summary>
public enum SecurityLevel
{
    /// <summary>
    /// High security - CONCORD protection (1.0 - 0.5)
    /// </summary>
    HighSec,
    
    /// <summary>
    /// Low security - Limited CONCORD (0.4 - 0.1)
    /// </summary>
    LowSec,
    
    /// <summary>
    /// Null security - No protection (0.0)
    /// </summary>
    NullSec,
    
    /// <summary>
    /// Wormhole space - Unknown systems
    /// </summary>
    WormholeSpace
}
