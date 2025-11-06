namespace AvorionLike.Core.Faction;

/// <summary>
/// Policy categories
/// </summary>
public enum PolicyCategory
{
    Diplomatic,
    Economic,
    Military,
    Social,
    Technology,
    Expansion
}

/// <summary>
/// Represents a game policy that can be enacted
/// </summary>
public class Policy
{
    public string Id { get; set; } = "";
    public string Name { get; set; } = "";
    public string Description { get; set; } = "";
    public PolicyCategory Category { get; set; }
    
    // Policy effects
    public Dictionary<string, float> Effects { get; set; } = new();
    
    // Faction reactions
    public Dictionary<string, float> FactionApprovalModifiers { get; set; } = new();
    
    // Requirements
    public float InfluenceCost { get; set; } = 10f;
    public List<string> Prerequisites { get; set; } = new();
    
    // State
    public bool IsActive { get; set; } = false;
    public DateTime EnactedAt { get; set; }
    
    public Policy(string id, string name, PolicyCategory category)
    {
        Id = id;
        Name = name;
        Category = category;
    }
    
    /// <summary>
    /// Check if policy can be enacted
    /// </summary>
    public bool CanEnact(float availableInfluence, List<string> activePolicies)
    {
        if (IsActive) return false;
        if (availableInfluence < InfluenceCost) return false;
        
        // Check prerequisites
        return Prerequisites.All(prereq => activePolicies.Contains(prereq));
    }
}

/// <summary>
/// Manages empire policies
/// </summary>
public class PolicyManager
{
    private Dictionary<string, Policy> _availablePolicies = new();
    private List<string> _activePolicies = new();
    
    public IReadOnlyDictionary<string, Policy> AvailablePolicies => _availablePolicies;
    public IReadOnlyList<string> ActivePolicies => _activePolicies;
    
    public PolicyManager()
    {
        InitializeDefaultPolicies();
    }
    
    /// <summary>
    /// Initialize default policies
    /// </summary>
    private void InitializeDefaultPolicies()
    {
        // Diplomatic policies
        AddPolicy(new Policy("closed_borders", "Closed Borders", PolicyCategory.Diplomatic)
        {
            Description = "Restrict immigration and trade",
            InfluenceCost = 20f,
            FactionApprovalModifiers = new Dictionary<string, float>
            {
                { "Xenophobe", 15f },
                { "Xenophile", -15f }
            }
        });
        
        AddPolicy(new Policy("open_borders", "Open Borders", PolicyCategory.Diplomatic)
        {
            Description = "Allow free movement and trade",
            InfluenceCost = 20f,
            FactionApprovalModifiers = new Dictionary<string, float>
            {
                { "Xenophile", 15f },
                { "Xenophobe", -15f }
            }
        });
        
        // Economic policies
        AddPolicy(new Policy("free_market", "Free Market Economy", PolicyCategory.Economic)
        {
            Description = "Minimal regulation, maximum trade",
            InfluenceCost = 15f,
            FactionApprovalModifiers = new Dictionary<string, float>
            {
                { "Materialist", 10f },
                { "Traditionalist", -10f }
            }
        });
        
        AddPolicy(new Policy("planned_economy", "Planned Economy", PolicyCategory.Economic)
        {
            Description = "Central planning and resource distribution",
            InfluenceCost = 15f,
            FactionApprovalModifiers = new Dictionary<string, float>
            {
                { "Authoritarian", 10f },
                { "Egalitarian", -10f }
            }
        });
        
        // Military policies
        AddPolicy(new Policy("military_expansion", "Military Expansion", PolicyCategory.Military)
        {
            Description = "Increase military funding and readiness",
            InfluenceCost = 25f,
            FactionApprovalModifiers = new Dictionary<string, float>
            {
                { "Militarist", 20f },
                { "Pacifist", -20f }
            }
        });
        
        AddPolicy(new Policy("defensive_doctrine", "Defensive Doctrine", PolicyCategory.Military)
        {
            Description = "Focus on defense and peacekeeping",
            InfluenceCost = 15f,
            FactionApprovalModifiers = new Dictionary<string, float>
            {
                { "Pacifist", 15f },
                { "Militarist", -10f }
            }
        });
        
        // Technology policies
        AddPolicy(new Policy("robot_workers", "Robotic Workforce", PolicyCategory.Technology)
        {
            Description = "Allow automation and robot workers",
            InfluenceCost = 30f,
            FactionApprovalModifiers = new Dictionary<string, float>
            {
                { "Materialist", 25f },
                { "Spiritualist", -15f },
                { "Industrialist", 20f },
                { "Traditionalist", -20f }
            }
        });
        
        AddPolicy(new Policy("research_grants", "Research Grants", PolicyCategory.Technology)
        {
            Description = "Increase funding for research",
            InfluenceCost = 20f,
            FactionApprovalModifiers = new Dictionary<string, float>
            {
                { "Materialist", 15f }
            }
        });
        
        // Social policies
        AddPolicy(new Policy("equal_rights", "Equal Rights", PolicyCategory.Social)
        {
            Description = "Ensure equality for all pops",
            InfluenceCost = 15f,
            FactionApprovalModifiers = new Dictionary<string, float>
            {
                { "Egalitarian", 20f },
                { "Authoritarian", -10f }
            }
        });
        
        AddPolicy(new Policy("meritocracy", "Meritocracy", PolicyCategory.Social)
        {
            Description = "Advancement based on merit",
            InfluenceCost = 10f,
            FactionApprovalModifiers = new Dictionary<string, float>
            {
                { "Egalitarian", 10f },
                { "Materialist", 10f }
            }
        });
        
        // Expansion policies
        AddPolicy(new Policy("aggressive_expansion", "Aggressive Expansion", PolicyCategory.Expansion)
        {
            Description = "Rapid territorial expansion",
            InfluenceCost = 30f,
            FactionApprovalModifiers = new Dictionary<string, float>
            {
                { "Militarist", 15f },
                { "Pacifist", -20f }
            }
        });
    }
    
    /// <summary>
    /// Add a policy to available policies
    /// </summary>
    public void AddPolicy(Policy policy)
    {
        _availablePolicies[policy.Id] = policy;
    }
    
    /// <summary>
    /// Enact a policy
    /// </summary>
    public bool EnactPolicy(string policyId, ref float influence)
    {
        if (!_availablePolicies.TryGetValue(policyId, out var policy))
            return false;
        
        if (!policy.CanEnact(influence, _activePolicies))
            return false;
        
        // Deduct influence cost
        influence -= policy.InfluenceCost;
        
        // Activate policy
        policy.IsActive = true;
        policy.EnactedAt = DateTime.UtcNow;
        _activePolicies.Add(policyId);
        
        return true;
    }
    
    /// <summary>
    /// Revoke a policy
    /// </summary>
    public bool RevokePolicy(string policyId)
    {
        if (!_availablePolicies.TryGetValue(policyId, out var policy))
            return false;
        
        if (!policy.IsActive)
            return false;
        
        policy.IsActive = false;
        _activePolicies.Remove(policyId);
        
        return true;
    }
    
    /// <summary>
    /// Get faction approval modifier for a policy
    /// </summary>
    public float GetFactionApprovalModifier(string policyId, FactionEthics ethics)
    {
        if (!_availablePolicies.TryGetValue(policyId, out var policy))
            return 0f;
        
        var ethicsKey = ethics.ToString();
        return policy.FactionApprovalModifiers.TryGetValue(ethicsKey, out var modifier) ? modifier : 0f;
    }
}
