namespace AvorionLike.Core.Faction;

/// <summary>
/// Represents an individual population unit (pop)
/// </summary>
public class Pop
{
    public string Id { get; set; } = "";
    public string PlanetId { get; set; } = ""; // Which planet/station this pop lives on
    public string Name { get; set; } = "";
    
    // Ethics and faction alignment
    public FactionEthics PrimaryEthic { get; set; }
    public string? AlignedFactionId { get; set; } // Which faction this pop supports
    
    // Happiness and productivity
    public float Happiness { get; set; } = 50f; // 0-100
    public float Productivity { get; set; } = 1.0f; // Multiplier based on happiness
    
    // Living conditions
    public float LivingStandard { get; set; } = 50f; // 0-100
    public bool HasBasicNeeds { get; set; } = true;
    public bool HasLuxuryGoods { get; set; } = false;
    
    // Work assignment
    public string? JobType { get; set; } // e.g., "Worker", "Specialist", "Ruler"
    public float JobSatisfaction { get; set; } = 50f;
    
    // State
    public bool IsAgitating { get; set; } = false; // Actively causing unrest
    public float UnrestContribution { get; set; } = 0f;
    
    public Pop(string id, string planetId, FactionEthics primaryEthic)
    {
        Id = id;
        PlanetId = planetId;
        PrimaryEthic = primaryEthic;
    }
    
    /// <summary>
    /// Update pop happiness based on living conditions and faction alignment
    /// </summary>
    public void UpdateHappiness(Faction? alignedFaction)
    {
        float happiness = 0f;
        
        // Base happiness from living standards
        happiness += LivingStandard * 0.3f;
        
        // Happiness from basic needs
        happiness += HasBasicNeeds ? 20f : -20f;
        
        // Bonus from luxury goods
        if (HasLuxuryGoods)
            happiness += 10f;
        
        // Happiness from job satisfaction
        happiness += JobSatisfaction * 0.3f;
        
        // Happiness from faction approval (if aligned)
        if (alignedFaction != null)
        {
            happiness += alignedFaction.Approval * 0.2f;
        }
        
        Happiness = Math.Clamp(happiness, 0f, 100f);
        
        // Update productivity based on happiness
        Productivity = 0.5f + (Happiness / 100f) * 1.0f; // Range: 0.5x to 1.5x
        
        // Update unrest contribution
        if (Happiness < 30f)
        {
            IsAgitating = true;
            UnrestContribution = (30f - Happiness) / 10f;
        }
        else
        {
            IsAgitating = false;
            UnrestContribution = 0f;
        }
    }
    
    /// <summary>
    /// Align pop with a faction based on ethics match
    /// </summary>
    public void AlignWithFaction(List<Faction> factions)
    {
        // Find faction that best matches pop's ethics
        var bestMatch = factions
            .Where(f => !f.IsSuppressed)
            .OrderByDescending(f => 
            {
                int score = 0;
                if (f.PrimaryEthic == PrimaryEthic) score += 10;
                if (f.SecondaryEthic == PrimaryEthic) score += 5;
                return score + f.Approval; // Prefer factions with higher approval
            })
            .FirstOrDefault();
        
        if (bestMatch != null)
        {
            AlignedFactionId = bestMatch.Id;
        }
    }
}

/// <summary>
/// Represents a planet or station with pops
/// </summary>
public class Planet
{
    public string Id { get; set; } = "";
    public string Name { get; set; } = "";
    public List<Pop> Pops { get; set; } = new();
    public float Stability { get; set; } = 100f; // 0-100
    public float ProductionEfficiency { get; set; } = 1.0f;
    
    public Planet(string id, string name)
    {
        Id = id;
        Name = name;
    }
    
    /// <summary>
    /// Update planet stability based on pop happiness
    /// </summary>
    public void UpdateStability()
    {
        if (Pops.Count == 0)
        {
            Stability = 100f;
            ProductionEfficiency = 1.0f;
            return;
        }
        
        // Calculate average happiness
        float avgHappiness = Pops.Average(p => p.Happiness);
        
        // Calculate total unrest
        float totalUnrest = Pops.Sum(p => p.UnrestContribution);
        
        // Stability based on happiness and unrest
        Stability = Math.Clamp(avgHappiness - totalUnrest, 0f, 100f);
        
        // Production efficiency based on stability
        ProductionEfficiency = 0.5f + (Stability / 100f) * 0.5f; // Range: 0.5x to 1.0x
    }
    
    /// <summary>
    /// Get faction distribution on this planet
    /// </summary>
    public Dictionary<string, int> GetFactionDistribution()
    {
        return Pops
            .Where(p => p.AlignedFactionId != null)
            .GroupBy(p => p.AlignedFactionId!)
            .ToDictionary(g => g.Key, g => g.Count());
    }
}
