using AvorionLike.Core.ECS;
using AvorionLike.Core.Events;
using AvorionLike.Core.Logging;

namespace AvorionLike.Core.Faction;

/// <summary>
/// Main faction management system - handles all faction-related mechanics
/// </summary>
public class FactionSystem : SystemBase
{
    private readonly EntityManager _entityManager;
    private readonly Logger _logger;
    private readonly PolicyManager _policyManager;
    
    // Empire state
    private GovernmentType _governmentType = GovernmentType.Democracy;
    private float _empireInfluence = 100f;
    private List<FactionEthics> _governmentEthics = new();
    
    // Factions and pops
    private Dictionary<string, Faction> _factions = new();
    private Dictionary<string, Pop> _pops = new();
    private Dictionary<string, Planet> _planets = new();
    
    // Configuration
    private float _factionFormationThreshold = 100; // Min pops to form new faction
    private float _rebellionThreshold = 80f; // Unrest level that triggers rebellion
    private bool _allowFactionSuppression = false; // Government can suppress factions
    
    // Timing
    private float _timeSinceLastRealignment = 0f;
    private const float REALIGNMENT_INTERVAL = 10f; // Seconds between realignments
    
    public IReadOnlyDictionary<string, Faction> Factions => _factions;
    public IReadOnlyDictionary<string, Pop> Pops => _pops;
    public IReadOnlyDictionary<string, Planet> Planets => _planets;
    public PolicyManager PolicyManager => _policyManager;
    public float Influence => _empireInfluence;
    public GovernmentType GovernmentType => _governmentType;
    
    public FactionSystem(EntityManager entityManager) : base("FactionSystem")
    {
        _entityManager = entityManager;
        _logger = Logger.Instance;
        _policyManager = new PolicyManager();
        
        InitializeDefaultFactions();
        _logger.Info("FactionSystem", "Faction system initialized");
    }
    
    /// <summary>
    /// Initialize default factions
    /// </summary>
    private void InitializeDefaultFactions()
    {
        // Create starting factions based on common ethics
        CreateFaction("militarists", "The Protectors", FactionEthics.Militarist, "Believe in military strength and defense");
        CreateFaction("pacifists", "Peace Coalition", FactionEthics.Pacifist, "Advocate for diplomacy and peaceful solutions");
        CreateFaction("materialists", "Tech Progressives", FactionEthics.Materialist, "Support scientific advancement and technology");
        CreateFaction("xenophiles", "Universal Brotherhood", FactionEthics.Xenophile, "Promote cooperation with other species");
        CreateFaction("industrialists", "Industrial Union", FactionEthics.Industrialist, "Focus on production and economic growth");
        
        _logger.Info("FactionSystem", $"Created {_factions.Count} default factions");
    }
    
    /// <summary>
    /// Set government type (affects faction behavior)
    /// </summary>
    public void SetGovernmentType(GovernmentType type)
    {
        _governmentType = type;
        _allowFactionSuppression = type == GovernmentType.Autocracy || type == GovernmentType.Militaristic;
        _logger.Info("FactionSystem", $"Government type set to: {type}");
    }
    
    /// <summary>
    /// Set government ethics (ruling faction alignment)
    /// </summary>
    public void SetGovernmentEthics(params FactionEthics[] ethics)
    {
        _governmentEthics = ethics.ToList();
        
        // Mark aligned factions as ruling
        foreach (var faction in _factions.Values)
        {
            faction.IsRulingFaction = _governmentEthics.Contains(faction.PrimaryEthic) || 
                                      (faction.SecondaryEthic.HasValue && _governmentEthics.Contains(faction.SecondaryEthic.Value));
        }
        
        _logger.Info("FactionSystem", $"Government ethics set: {string.Join(", ", ethics)}");
    }
    
    /// <summary>
    /// Create a new faction
    /// </summary>
    public Faction CreateFaction(string id, string name, FactionEthics primaryEthic, string description, FactionEthics? secondaryEthic = null)
    {
        var faction = new Faction(id, name, primaryEthic)
        {
            Description = description,
            SecondaryEthic = secondaryEthic
        };
        
        // Add default demands based on ethics
        AddDefaultDemands(faction);
        
        _factions[id] = faction;
        _logger.Info("FactionSystem", $"Created faction: {name}");
        
        // Publish event
        EventSystem.Instance.Publish("FactionCreated", new GameEvent { EventType = "FactionCreated" });
        
        return faction;
    }
    
    /// <summary>
    /// Add default demands based on faction ethics
    /// </summary>
    private void AddDefaultDemands(Faction faction)
    {
        switch (faction.PrimaryEthic)
        {
            case FactionEthics.Militarist:
                faction.AddDemand(new FactionDemand(DemandType.MilitaryExpansion, "Maintain strong military", 0.8f));
                faction.AddDemand(new FactionDemand(DemandType.DefensiveFocus, "Protect our borders", 0.6f));
                break;
                
            case FactionEthics.Pacifist:
                faction.AddDemand(new FactionDemand(DemandType.PeaceTreaties, "Maintain peace", 0.9f));
                faction.AddDemand(new FactionDemand(DemandType.Disarmament, "Reduce military spending", 0.7f));
                break;
                
            case FactionEthics.Materialist:
                faction.AddDemand(new FactionDemand(DemandType.ResearchFunding, "Fund research", 0.9f));
                faction.AddDemand(new FactionDemand(DemandType.RoboticWorkforce, "Allow automation", 0.7f));
                break;
                
            case FactionEthics.Spiritualist:
                faction.AddDemand(new FactionDemand(DemandType.CulturalPreservation, "Preserve traditions", 0.8f));
                faction.AddDemand(new FactionDemand(DemandType.TraditionalMethods, "Limit automation", 0.6f));
                break;
                
            case FactionEthics.Xenophile:
                faction.AddDemand(new FactionDemand(DemandType.OpenBorders, "Open borders", 0.9f));
                faction.AddDemand(new FactionDemand(DemandType.IncreaseTrade, "Increase trade", 0.7f));
                break;
                
            case FactionEthics.Xenophobe:
                faction.AddDemand(new FactionDemand(DemandType.ClosedBorders, "Close borders", 0.9f));
                faction.AddDemand(new FactionDemand(DemandType.SelfSufficiency, "Self-sufficiency", 0.8f));
                break;
                
            case FactionEthics.Industrialist:
                faction.AddDemand(new FactionDemand(DemandType.IndustrialExpansion, "Expand industry", 0.9f));
                faction.AddDemand(new FactionDemand(DemandType.RoboticWorkforce, "Automate production", 0.7f));
                break;
                
            case FactionEthics.Traditionalist:
                faction.AddDemand(new FactionDemand(DemandType.CulturalPreservation, "Preserve culture", 0.8f));
                faction.AddDemand(new FactionDemand(DemandType.TraditionalMethods, "Traditional methods", 0.7f));
                break;
        }
    }
    
    /// <summary>
    /// Create a new pop
    /// </summary>
    public Pop CreatePop(string id, string planetId, FactionEthics primaryEthic)
    {
        var pop = new Pop(id, planetId, primaryEthic);
        
        // Align with faction
        pop.AlignWithFaction(_factions.Values.ToList());
        
        _pops[id] = pop;
        
        // Update faction support counts
        UpdateFactionSupport();
        
        return pop;
    }
    
    /// <summary>
    /// Create a planet
    /// </summary>
    public Planet CreatePlanet(string id, string name)
    {
        var planet = new Planet(id, name);
        _planets[id] = planet;
        return planet;
    }
    
    /// <summary>
    /// Add pop to planet
    /// </summary>
    public void AddPopToPlanet(string popId, string planetId)
    {
        if (!_pops.TryGetValue(popId, out var pop)) return;
        if (!_planets.TryGetValue(planetId, out var planet)) return;
        
        planet.Pops.Add(pop);
        pop.PlanetId = planetId;
    }
    
    /// <summary>
    /// Update faction support based on pop alignment
    /// </summary>
    private void UpdateFactionSupport()
    {
        int totalPops = _pops.Count;
        if (totalPops == 0) return;
        
        // Reset support counts
        foreach (var faction in _factions.Values)
        {
            faction.PopSupport = 0;
        }
        
        // Count pops per faction
        foreach (var pop in _pops.Values)
        {
            if (pop.AlignedFactionId != null && _factions.TryGetValue(pop.AlignedFactionId, out var faction))
            {
                faction.PopSupport++;
            }
        }
        
        // Calculate percentages
        foreach (var faction in _factions.Values)
        {
            faction.SupportPercentage = (faction.PopSupport / (float)totalPops) * 100f;
        }
    }
    
    /// <summary>
    /// Enact a policy
    /// </summary>
    public bool EnactPolicy(string policyId)
    {
        if (!_policyManager.EnactPolicy(policyId, ref _empireInfluence))
            return false;
        
        _logger.Info("FactionSystem", $"Enacted policy: {policyId}");
        
        // Update faction demands based on policy
        UpdateFactionDemandsForPolicy(policyId);
        
        // Apply faction approval modifiers
        ApplyPolicyApprovalModifiers(policyId);
        
        EventSystem.Instance.Publish("PolicyEnacted", new GameEvent { EventType = "PolicyEnacted" });
        return true;
    }
    
    /// <summary>
    /// Update faction demands when a policy is enacted
    /// </summary>
    private void UpdateFactionDemandsForPolicy(string policyId)
    {
        // Match policy to demand types and mark as met
        foreach (var faction in _factions.Values)
        {
            foreach (var demand in faction.Demands)
            {
                // Check if policy satisfies this demand
                bool isMet = policyId switch
                {
                    "closed_borders" => demand.Type == DemandType.ClosedBorders,
                    "open_borders" => demand.Type == DemandType.OpenBorders,
                    "military_expansion" => demand.Type == DemandType.MilitaryExpansion,
                    "defensive_doctrine" => demand.Type == DemandType.DefensiveFocus,
                    "robot_workers" => demand.Type == DemandType.RoboticWorkforce,
                    "research_grants" => demand.Type == DemandType.ResearchFunding,
                    _ => false
                };
                
                if (isMet)
                {
                    demand.IsMet = true;
                }
            }
        }
    }
    
    /// <summary>
    /// Apply faction approval modifiers from policy
    /// </summary>
    private void ApplyPolicyApprovalModifiers(string policyId)
    {
        foreach (var faction in _factions.Values)
        {
            float modifier = _policyManager.GetFactionApprovalModifier(policyId, faction.PrimaryEthic);
            faction.Approval = Math.Clamp(faction.Approval + modifier, 0f, 100f);
            
            if (modifier != 0)
            {
                _logger.Debug("FactionSystem", $"Faction {faction.Name} approval changed by {modifier:F1} due to policy {policyId}");
            }
        }
    }
    
    /// <summary>
    /// Suppress a faction (only in certain government types)
    /// </summary>
    public bool SuppressFaction(string factionId)
    {
        if (!_allowFactionSuppression)
        {
            _logger.Warning("FactionSystem", "Current government type does not allow faction suppression");
            return false;
        }
        
        if (!_factions.TryGetValue(factionId, out var faction))
            return false;
        
        faction.IsSuppressed = true;
        faction.Approval = Math.Max(faction.Approval - 30f, 0f);
        
        _logger.Info("FactionSystem", $"Suppressed faction: {faction.Name}");
        EventSystem.Instance.Publish("FactionSuppressed", new GameEvent { EventType = "FactionSuppressed" });
        
        return true;
    }
    
    /// <summary>
    /// Lift suppression from a faction
    /// </summary>
    public bool LiftSuppression(string factionId)
    {
        if (!_factions.TryGetValue(factionId, out var faction))
            return false;
        
        faction.IsSuppressed = false;
        _logger.Info("FactionSystem", $"Lifted suppression from faction: {faction.Name}");
        
        return true;
    }
    
    /// <summary>
    /// Main update loop - called each game tick
    /// </summary>
    public override void Update(float deltaTime)
    {
        // Update all factions
        foreach (var faction in _factions.Values)
        {
            faction.UpdateApproval(deltaTime);
            
            // Generate influence
            float influenceGain = faction.CalculateInfluenceGeneration() * deltaTime;
            _empireInfluence += influenceGain;
            faction.Influence += influenceGain;
            
            // Check for rebellion risk
            if (faction.IsRebellionRisk())
            {
                _logger.Warning("FactionSystem", $"Faction {faction.Name} is at risk of rebellion!");
                EventSystem.Instance.Publish("FactionRebellionRisk", new GameEvent { EventType = "FactionRebellionRisk" });
            }
        }
        
        // Update all pops
        foreach (var pop in _pops.Values)
        {
            var alignedFaction = pop.AlignedFactionId != null ? _factions.GetValueOrDefault(pop.AlignedFactionId) : null;
            pop.UpdateHappiness(alignedFaction);
        }
        
        // Update all planets
        foreach (var planet in _planets.Values)
        {
            planet.UpdateStability();
            
            if (planet.Stability < 30f)
            {
                _logger.Warning("FactionSystem", $"Planet {planet.Name} has low stability: {planet.Stability:F1}%");
            }
        }
        
        // Periodically realign pops with factions (every 10 seconds)
        _timeSinceLastRealignment += deltaTime;
        if (_timeSinceLastRealignment >= REALIGNMENT_INTERVAL)
        {
            foreach (var pop in _pops.Values)
            {
                pop.AlignWithFaction(_factions.Values.ToList());
            }
            UpdateFactionSupport();
            _timeSinceLastRealignment = 0f;
        }
    }
    
    /// <summary>
    /// Get dominant factions (top 3 by support)
    /// </summary>
    public List<Faction> GetDominantFactions()
    {
        return _factions.Values
            .OrderByDescending(f => f.SupportPercentage)
            .Take(3)
            .ToList();
    }
    
    /// <summary>
    /// Get faction summary report
    /// </summary>
    public string GetFactionReport()
    {
        var report = new System.Text.StringBuilder();
        report.AppendLine("=== FACTION REPORT ===");
        report.AppendLine($"Government: {_governmentType}");
        report.AppendLine($"Empire Influence: {_empireInfluence:F1}");
        report.AppendLine($"Total Pops: {_pops.Count}");
        report.AppendLine($"Total Planets: {_planets.Count}");
        report.AppendLine();
        
        report.AppendLine("=== DOMINANT FACTIONS ===");
        var dominant = GetDominantFactions();
        foreach (var faction in dominant)
        {
            report.AppendLine(faction.GetSummary());
            report.AppendLine();
        }
        
        return report.ToString();
    }
}
