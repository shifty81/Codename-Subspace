namespace AvorionLike.Core.AI;

/// <summary>
/// AI behavioral states
/// </summary>
public enum AIState
{
    /// <summary>
    /// Default state - idle or patrolling
    /// </summary>
    Idle,
    
    /// <summary>
    /// Moving to a waypoint or patrol location
    /// </summary>
    Patrol,
    
    /// <summary>
    /// Mining asteroids for resources
    /// </summary>
    Mining,
    
    /// <summary>
    /// Salvaging wreckage
    /// </summary>
    Salvaging,
    
    /// <summary>
    /// Trading at stations
    /// </summary>
    Trading,
    
    /// <summary>
    /// Engaging in combat
    /// </summary>
    Combat,
    
    /// <summary>
    /// Fleeing from danger
    /// </summary>
    Fleeing,
    
    /// <summary>
    /// Evading attacks
    /// </summary>
    Evasion,
    
    /// <summary>
    /// Returning to base
    /// </summary>
    ReturningToBase,
    
    /// <summary>
    /// Repairing at station
    /// </summary>
    Repairing,
    
    /// <summary>
    /// Scanning for wormholes and anomalies
    /// </summary>
    Scanning,
    
    /// <summary>
    /// Exploring wormhole space
    /// </summary>
    Exploring
}

/// <summary>
/// AI personality types that affect behavior priorities
/// </summary>
public enum AIPersonality
{
    /// <summary>
    /// Balanced behavior across all activities
    /// </summary>
    Balanced,
    
    /// <summary>
    /// Aggressive, combat-focused
    /// </summary>
    Aggressive,
    
    /// <summary>
    /// Cautious, avoids combat
    /// </summary>
    Defensive,
    
    /// <summary>
    /// Focuses on resource gathering
    /// </summary>
    Miner,
    
    /// <summary>
    /// Focuses on trading
    /// </summary>
    Trader,
    
    /// <summary>
    /// Focuses on salvaging
    /// </summary>
    Salvager,
    
    /// <summary>
    /// Focuses on exploration and scanning
    /// </summary>
    Explorer,
    
    /// <summary>
    /// Cowardly, flees easily
    /// </summary>
    Coward
}

/// <summary>
/// Combat tactics used by AI
/// </summary>
public enum CombatTactic
{
    /// <summary>
    /// Aggressive frontal assault
    /// </summary>
    Aggressive,
    
    /// <summary>
    /// Keep distance and use ranged weapons
    /// </summary>
    Kiting,
    
    /// <summary>
    /// Circle strafe around target
    /// </summary>
    Strafing,
    
    /// <summary>
    /// Broadside for maximum turret coverage
    /// </summary>
    Broadsiding,
    
    /// <summary>
    /// Defensive posture
    /// </summary>
    Defensive
}

/// <summary>
/// Target priority for AI decision making
/// </summary>
public enum TargetPriority
{
    None = 0,
    Low = 1,
    Medium = 2,
    High = 3,
    Critical = 4
}
