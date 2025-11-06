namespace AvorionLike.Core.Faction;

/// <summary>
/// Represents a faction demand with its current state
/// </summary>
public class FactionDemand
{
    public DemandType Type { get; set; }
    public string Description { get; set; } = "";
    public float Priority { get; set; } = 0.5f; // 0-1, how important this is
    public bool IsMet { get; set; } = false;
    public float ApprovalBonus { get; set; } = 10f; // Approval gained when met
    public float ApprovalPenalty { get; set; } = 5f; // Approval lost when not met
    
    public FactionDemand(DemandType type, string description, float priority = 0.5f)
    {
        Type = type;
        Description = description;
        Priority = priority;
    }
}

/// <summary>
/// Represents a political faction within the empire
/// </summary>
public class Faction
{
    public string Id { get; set; } = "";
    public string Name { get; set; } = "";
    public string Description { get; set; } = "";
    public FactionEthics PrimaryEthic { get; set; }
    public FactionEthics? SecondaryEthic { get; set; }
    
    // Core metrics
    public float Approval { get; set; } = 50f; // 0-100
    public float Influence { get; set; } = 0f; // Generated based on approval
    public int PopSupport { get; set; } = 0; // Number of pops supporting this faction
    public float SupportPercentage { get; set; } = 0f; // % of total population
    
    // Demands and policies
    public List<FactionDemand> Demands { get; set; } = new();
    public List<string> PreferredPolicies { get; set; } = new();
    public List<string> OpposedPolicies { get; set; } = new();
    
    // State
    public bool IsRulingFaction { get; set; } = false;
    public bool IsSuppressed { get; set; } = false;
    public float UnrestLevel { get; set; } = 0f; // 0-100, can trigger rebellion
    
    // History tracking
    public float ApprovalTrend { get; set; } = 0f; // Change per time unit
    public DateTime LastUpdated { get; set; } = DateTime.UtcNow;
    
    public Faction(string id, string name, FactionEthics primaryEthic)
    {
        Id = id;
        Name = name;
        PrimaryEthic = primaryEthic;
    }
    
    /// <summary>
    /// Get current approval level
    /// </summary>
    public ApprovalLevel GetApprovalLevel()
    {
        return Approval switch
        {
            < 20 => ApprovalLevel.Rebellious,
            < 40 => ApprovalLevel.Angry,
            < 60 => ApprovalLevel.Displeased,
            < 75 => ApprovalLevel.Content,
            < 90 => ApprovalLevel.Happy,
            _ => ApprovalLevel.Ecstatic
        };
    }
    
    /// <summary>
    /// Calculate influence generation based on approval
    /// </summary>
    public float CalculateInfluenceGeneration()
    {
        if (IsSuppressed) return 0f;
        
        // Base influence from approval
        float baseInfluence = (Approval / 100f) * SupportPercentage;
        
        // Bonus for high approval
        if (Approval > 75)
        {
            baseInfluence *= 1.5f;
        }
        
        // Bonus if ruling faction
        if (IsRulingFaction)
        {
            baseInfluence *= 2f;
        }
        
        return baseInfluence;
    }
    
    /// <summary>
    /// Update approval based on met/unmet demands
    /// </summary>
    public void UpdateApproval(float deltaTime)
    {
        float approvalChange = 0f;
        
        foreach (var demand in Demands)
        {
            if (demand.IsMet)
            {
                approvalChange += demand.ApprovalBonus * demand.Priority * deltaTime;
            }
            else
            {
                approvalChange -= demand.ApprovalPenalty * demand.Priority * deltaTime;
            }
        }
        
        // Clamp approval change
        approvalChange = Math.Clamp(approvalChange, -10f, 10f);
        
        // Apply change
        var oldApproval = Approval;
        Approval = Math.Clamp(Approval + approvalChange, 0f, 100f);
        
        // Track trend
        ApprovalTrend = Approval - oldApproval;
        
        // Update unrest based on low approval
        if (Approval < 30)
        {
            UnrestLevel = Math.Min(UnrestLevel + (30 - Approval) * 0.1f * deltaTime, 100f);
        }
        else if (UnrestLevel > 0)
        {
            UnrestLevel = Math.Max(UnrestLevel - 5f * deltaTime, 0f);
        }
        
        LastUpdated = DateTime.UtcNow;
    }
    
    /// <summary>
    /// Check if faction is at risk of rebellion
    /// </summary>
    public bool IsRebellionRisk()
    {
        return UnrestLevel > 70f && Approval < 25f && !IsSuppressed;
    }
    
    /// <summary>
    /// Add a new demand
    /// </summary>
    public void AddDemand(FactionDemand demand)
    {
        Demands.Add(demand);
    }
    
    /// <summary>
    /// Remove a demand
    /// </summary>
    public void RemoveDemand(DemandType type)
    {
        Demands.RemoveAll(d => d.Type == type);
    }
    
    /// <summary>
    /// Get summary of faction state
    /// </summary>
    public string GetSummary()
    {
        return $"{Name} ({PrimaryEthic})\n" +
               $"  Approval: {Approval:F1}% ({GetApprovalLevel()})\n" +
               $"  Support: {SupportPercentage:F1}% ({PopSupport} pops)\n" +
               $"  Influence: +{CalculateInfluenceGeneration():F2}/turn\n" +
               $"  Unrest: {UnrestLevel:F1}%\n" +
               $"  Demands: {Demands.Count(d => d.IsMet)}/{Demands.Count} met";
    }
}
