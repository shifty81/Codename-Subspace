namespace AvorionLike.Core.Faction;

/// <summary>
/// Represents a faction's ethics and ideological stance
/// </summary>
public enum FactionEthics
{
    // Government Structure
    Authoritarian,      // Centralized control, order, hierarchy
    Egalitarian,        // Equality, freedom, democracy
    
    // Economic System
    Materialist,        // Science, technology, progress
    Spiritualist,       // Tradition, culture, faith
    
    // Expansion Policy
    Militarist,         // Military strength, conquest
    Pacifist,           // Peace, diplomacy, trade
    
    // Foreign Policy
    Xenophile,          // Open borders, cooperation
    Xenophobe,          // Isolationism, self-reliance
    
    // Labor Policy
    Industrialist,      // Production, efficiency, automation
    Traditionalist,     // Manual labor, preservation, heritage
    
    // Special
    Neutral             // No strong stance
}

/// <summary>
/// Government types that affect faction behavior
/// </summary>
public enum GovernmentType
{
    Democracy,          // Factions are political parties
    Oligarchy,          // Elite factions compete for power
    Autocracy,          // Single ruler, factions vie for favor
    Technocracy,        // Science-focused factions dominate
    Militaristic,       // Military factions control policy
    Corporate,          // Economic factions drive decisions
    Theocracy           // Religious/spiritual factions lead
}

/// <summary>
/// Types of demands factions can make
/// </summary>
public enum DemandType
{
    // Diplomatic
    ClosedBorders,
    OpenBorders,
    PeaceTreaties,
    MilitaryAlliances,
    
    // Economic
    IncreaseTrade,
    SelfSufficiency,
    IndustrialExpansion,
    ResourceConservation,
    
    // Technology
    ResearchFunding,
    TechnologySharing,
    RoboticWorkforce,
    TraditionalMethods,
    
    // Military
    MilitaryExpansion,
    DefensiveFocus,
    Disarmament,
    
    // Social
    PopulationGrowth,
    ImmigrationControl,
    CulturalPreservation,
    Diversification,
    
    // Expansion
    TerritorialExpansion,
    Consolidation,
    Colonization
}

/// <summary>
/// Faction approval states
/// </summary>
public enum ApprovalLevel
{
    Rebellious,         // < 20% - May cause rebellion
    Angry,              // 20-40% - Severely unhappy
    Displeased,         // 40-60% - Somewhat unhappy
    Content,            // 60-75% - Satisfied
    Happy,              // 75-90% - Quite happy
    Ecstatic            // > 90% - Extremely happy
}
