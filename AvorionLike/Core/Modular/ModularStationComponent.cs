using System.Numerics;
using AvorionLike.Core.ECS;

namespace AvorionLike.Core.Modular;

/// <summary>
/// Component for stations built from modular parts
/// Similar to ModularShipComponent but for stationary structures
/// </summary>
public class ModularStationComponent : IComponent
{
    public Guid EntityId { get; set; }
    
    /// <summary>
    /// All modules that make up this station
    /// </summary>
    public List<StationModulePart> Modules { get; set; } = new();
    
    /// <summary>
    /// Station's name
    /// </summary>
    public string Name { get; set; } = "Unnamed Station";
    
    /// <summary>
    /// Station type (TradingPost, Shipyard, Factory, etc.)
    /// </summary>
    public StationType Type { get; set; } = StationType.TradingPost;
    
    /// <summary>
    /// Center of mass calculated from all modules
    /// </summary>
    public Vector3 CenterOfMass { get; private set; }
    
    /// <summary>
    /// Total mass of all modules
    /// </summary>
    public float TotalMass { get; private set; }
    
    /// <summary>
    /// Total health of all modules
    /// </summary>
    public float TotalHealth { get; private set; }
    
    /// <summary>
    /// Maximum total health
    /// </summary>
    public float MaxTotalHealth { get; private set; }
    
    /// <summary>
    /// Bounding box of the station
    /// </summary>
    public BoundingBox Bounds { get; private set; }
    
    /// <summary>
    /// Aggregated functional stats from all modules
    /// </summary>
    public StationFunctionalStats AggregatedStats { get; private set; } = new();
    
    /// <summary>
    /// Module that serves as the "core" or "command center" - critical for station survival
    /// </summary>
    public Guid? CoreModuleId { get; set; }
    
    /// <summary>
    /// Is station destroyed (core destroyed or no modules left)
    /// </summary>
    public bool IsDestroyed => Modules.Count == 0 || 
                               (CoreModuleId.HasValue && GetModule(CoreModuleId.Value)?.IsDestroyed == true);
    
    /// <summary>
    /// Faction that owns this station
    /// </summary>
    public string OwningFaction { get; set; } = "Independent";
    
    /// <summary>
    /// Add a module to the station
    /// </summary>
    public void AddModule(StationModulePart module)
    {
        if (module == null) return;
        Modules.Add(module);
        RecalculateStats();
    }
    
    /// <summary>
    /// Remove a module from the station
    /// </summary>
    public bool RemoveModule(Guid moduleId)
    {
        var module = GetModule(moduleId);
        if (module == null) return false;
        
        // Remove attachments
        foreach (var attachedId in module.AttachedModules)
        {
            var attached = GetModule(attachedId);
            if (attached != null)
            {
                attached.AttachedToModules.Remove(moduleId);
            }
        }
        
        foreach (var attachedToId in module.AttachedToModules)
        {
            var attachedTo = GetModule(attachedToId);
            if (attachedTo != null)
            {
                attachedTo.AttachedModules.Remove(moduleId);
            }
        }
        
        Modules.Remove(module);
        RecalculateStats();
        return true;
    }
    
    /// <summary>
    /// Get a module by ID
    /// </summary>
    public StationModulePart? GetModule(Guid moduleId)
    {
        return Modules.FirstOrDefault(m => m.Id == moduleId);
    }
    
    /// <summary>
    /// Recalculate all derived stats
    /// </summary>
    public void RecalculateStats()
    {
        if (Modules.Count == 0)
        {
            CenterOfMass = Vector3.Zero;
            TotalMass = 0;
            TotalHealth = 0;
            MaxTotalHealth = 0;
            Bounds = new BoundingBox();
            AggregatedStats = new StationFunctionalStats();
            return;
        }
        
        // Calculate center of mass
        Vector3 weightedSum = Vector3.Zero;
        float totalMass = 0;
        
        foreach (var module in Modules)
        {
            weightedSum += module.Position * module.Mass;
            totalMass += module.Mass;
        }
        
        CenterOfMass = weightedSum / totalMass;
        TotalMass = totalMass;
        
        // Calculate health
        TotalHealth = Modules.Sum(m => m.Health);
        MaxTotalHealth = Modules.Sum(m => m.MaxHealth);
        
        // Calculate bounding box
        if (Modules.Any())
        {
            var positions = Modules.Select(m => m.Position);
            var min = new Vector3(
                positions.Min(p => p.X),
                positions.Min(p => p.Y),
                positions.Min(p => p.Z)
            );
            var max = new Vector3(
                positions.Max(p => p.X),
                positions.Max(p => p.Y),
                positions.Max(p => p.Z)
            );
            Bounds = new BoundingBox { Min = min, Max = max };
        }
        
        // Aggregate functional stats
        AggregatedStats = new StationFunctionalStats();
        foreach (var module in Modules)
        {
            AggregatedStats.Add(module.FunctionalStats);
        }
    }
    
    /// <summary>
    /// Damage a specific module
    /// </summary>
    public void DamageModule(Guid moduleId, float damage)
    {
        var module = GetModule(moduleId);
        if (module == null) return;
        
        module.Health = Math.Max(0, module.Health - damage);
        TotalHealth = Modules.Sum(m => m.Health);
    }
    
    /// <summary>
    /// Repair a specific module
    /// </summary>
    public void RepairModule(Guid moduleId, float amount)
    {
        var module = GetModule(moduleId);
        if (module == null) return;
        
        module.Health = Math.Min(module.MaxHealth, module.Health + amount);
        TotalHealth = Modules.Sum(m => m.Health);
    }
    
    /// <summary>
    /// Get all modules of a specific category
    /// </summary>
    public List<StationModulePart> GetModulesByCategory(StationModuleCategory category)
    {
        return Modules.Where(m => m.Category == category).ToList();
    }
    
    /// <summary>
    /// Check if station can perform a function (trading, production, etc.)
    /// </summary>
    public bool CanPerformFunction(StationFunction function)
    {
        return function switch
        {
            StationFunction.Trading => AggregatedStats.TradingCapacity > 0,
            StationFunction.Production => AggregatedStats.ProductionCapacity > 0,
            StationFunction.Repair => AggregatedStats.RepairCapacity > 0,
            StationFunction.Refueling => AggregatedStats.RefuelCapacity > 0,
            StationFunction.Docking => AggregatedStats.DockingBays > 0,
            StationFunction.Research => AggregatedStats.ResearchPoints > 0,
            _ => false
        };
    }
}

/// <summary>
/// Station types
/// </summary>
public enum StationType
{
    TradingPost,
    Shipyard,
    Factory,
    MiningStation,
    ResearchStation,
    DefensePlatform,
    RefuelingDepot,
    CommandCenter,
    Habitat
}

/// <summary>
/// Station functions
/// </summary>
public enum StationFunction
{
    Trading,
    Production,
    Repair,
    Refueling,
    Docking,
    Research
}

/// <summary>
/// Station-specific functional stats
/// </summary>
public class StationFunctionalStats
{
    public int DockingBays { get; set; } = 0;
    public float TradingCapacity { get; set; } = 0;
    public float ProductionCapacity { get; set; } = 0;
    public float StorageCapacity { get; set; } = 0;
    public float RepairCapacity { get; set; } = 0;
    public float RefuelCapacity { get; set; } = 0;
    public float PowerGeneration { get; set; } = 0;
    public float PowerConsumption { get; set; } = 0;
    public float DefenseRating { get; set; } = 0;
    public int ResearchPoints { get; set; } = 0;
    public int CrewCapacity { get; set; } = 0;
    public float ShieldCapacity { get; set; } = 0;
    public float ShieldRechargeRate { get; set; } = 0;
    
    public void Add(StationFunctionalStats other)
    {
        DockingBays += other.DockingBays;
        TradingCapacity += other.TradingCapacity;
        ProductionCapacity += other.ProductionCapacity;
        StorageCapacity += other.StorageCapacity;
        RepairCapacity += other.RepairCapacity;
        RefuelCapacity += other.RefuelCapacity;
        PowerGeneration += other.PowerGeneration;
        PowerConsumption += other.PowerConsumption;
        DefenseRating += other.DefenseRating;
        ResearchPoints += other.ResearchPoints;
        CrewCapacity += other.CrewCapacity;
        ShieldCapacity += other.ShieldCapacity;
        ShieldRechargeRate += other.ShieldRechargeRate;
    }
}
