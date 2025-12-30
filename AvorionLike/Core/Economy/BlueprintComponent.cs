using AvorionLike.Core.ECS;
using AvorionLike.Core.Resources;
using AvorionLike.Core.Voxel;

namespace AvorionLike.Core.Economy;

/// <summary>
/// Blueprint for manufacturing items and ships
/// Inspired by EVE Online's blueprint system
/// </summary>
public class BlueprintComponent : IComponent
{
    public Guid EntityId { get; set; }
    
    /// <summary>
    /// Name of the blueprint
    /// </summary>
    public string Name { get; set; } = "Unknown Blueprint";
    
    /// <summary>
    /// Type of item this blueprint produces
    /// </summary>
    public BlueprintType Type { get; set; }
    
    /// <summary>
    /// Material efficiency level (0-10)
    /// Reduces material requirements
    /// </summary>
    public int MaterialEfficiency { get; set; } = 0;
    
    /// <summary>
    /// Time efficiency level (0-10)
    /// Reduces production time
    /// </summary>
    public int TimeEfficiency { get; set; } = 0;
    
    /// <summary>
    /// Number of runs remaining (-1 for infinite/BPO)
    /// </summary>
    public int RunsRemaining { get; set; } = -1;
    
    /// <summary>
    /// Whether this is an original (BPO) or copy (BPC)
    /// </summary>
    public bool IsOriginal { get; set; } = true;
    
    /// <summary>
    /// Base materials required for production
    /// </summary>
    public Dictionary<ResourceType, int> MaterialRequirements { get; set; } = new();
    
    /// <summary>
    /// Base production time in seconds
    /// </summary>
    public float BaseProductionTime { get; set; } = 3600f; // 1 hour
    
    /// <summary>
    /// Research points invested in material efficiency
    /// </summary>
    public int MaterialResearchPoints { get; set; } = 0;
    
    /// <summary>
    /// Research points invested in time efficiency
    /// </summary>
    public int TimeResearchPoints { get; set; } = 0;
    
    /// <summary>
    /// Get actual material requirements after efficiency
    /// </summary>
    public Dictionary<ResourceType, int> GetActualMaterialRequirements()
    {
        var requirements = new Dictionary<ResourceType, int>();
        float efficiency = 1.0f - (MaterialEfficiency * 0.01f); // 1% per level
        
        foreach (var req in MaterialRequirements)
        {
            requirements[req.Key] = (int)(req.Value * efficiency);
        }
        
        return requirements;
    }
    
    /// <summary>
    /// Get actual production time after efficiency
    /// </summary>
    public float GetActualProductionTime()
    {
        float efficiency = 1.0f - (TimeEfficiency * 0.02f); // 2% per level
        return BaseProductionTime * efficiency;
    }
}

/// <summary>
/// Type of blueprint
/// </summary>
public enum BlueprintType
{
    ShipComponent,
    Ship,
    Module,
    Ammunition,
    Structure,
    Consumable
}

/// <summary>
/// Active manufacturing job
/// </summary>
public class ManufacturingJob
{
    public Guid JobId { get; set; } = Guid.NewGuid();
    public Guid BlueprintId { get; set; }
    public Guid FacilityId { get; set; }
    public Guid OwnerId { get; set; }
    public int Runs { get; set; } = 1;
    public float TimeRemaining { get; set; }
    public float TotalTime { get; set; }
    public bool IsComplete { get; set; } = false;
    public DateTime StartTime { get; set; }
}

/// <summary>
/// Component for manufacturing facilities
/// </summary>
public class ManufacturingFacilityComponent : IComponent
{
    public Guid EntityId { get; set; }
    
    /// <summary>
    /// Maximum number of concurrent jobs
    /// </summary>
    public int MaxJobs { get; set; } = 10;
    
    /// <summary>
    /// Currently running jobs
    /// </summary>
    public List<ManufacturingJob> ActiveJobs { get; set; } = new();
    
    /// <summary>
    /// Material efficiency bonus (%)
    /// </summary>
    public float MaterialBonus { get; set; } = 0f;
    
    /// <summary>
    /// Time efficiency bonus (%)
    /// </summary>
    public float TimeBonus { get; set; } = 0f;
    
    /// <summary>
    /// Cost multiplier (affects job costs)
    /// </summary>
    public float CostMultiplier { get; set; } = 1.0f;
}
