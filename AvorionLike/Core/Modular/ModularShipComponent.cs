using System.Numerics;
using AvorionLike.Core.ECS;

namespace AvorionLike.Core.Modular;

/// <summary>
/// Component for ships built from modular parts instead of voxels
/// This replaces VoxelStructureComponent for ship construction
/// </summary>
public class ModularShipComponent : IComponent
{
    public Guid EntityId { get; set; }
    
    /// <summary>
    /// All modules that make up this ship
    /// </summary>
    public List<ShipModulePart> Modules { get; set; } = new();
    
    /// <summary>
    /// Ship's name
    /// </summary>
    public string Name { get; set; } = "Unnamed Ship";
    
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
    /// Bounding box of the ship
    /// </summary>
    public BoundingBox Bounds { get; private set; }
    
    /// <summary>
    /// Aggregated functional stats from all modules
    /// </summary>
    public ModuleFunctionalStats AggregatedStats { get; private set; } = new();
    
    /// <summary>
    /// Module that serves as the "core" or "cockpit" - critical for ship survival
    /// </summary>
    public Guid? CoreModuleId { get; set; }
    
    /// <summary>
    /// Is ship destroyed (core destroyed or no modules left)
    /// </summary>
    public bool IsDestroyed => Modules.Count == 0 || 
                               (CoreModuleId.HasValue && GetModule(CoreModuleId.Value)?.IsDestroyed == true);
    
    /// <summary>
    /// Add a module to the ship
    /// </summary>
    public void AddModule(ShipModulePart module)
    {
        if (module == null) return;
        Modules.Add(module);
        RecalculateStats();
    }
    
    /// <summary>
    /// Remove a module from the ship
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
    public ShipModulePart? GetModule(Guid moduleId)
    {
        return Modules.FirstOrDefault(m => m.Id == moduleId);
    }
    
    /// <summary>
    /// Get all modules of a specific category
    /// </summary>
    public List<ShipModulePart> GetModulesByCategory(ModuleCategory category, ModuleLibrary library)
    {
        var result = new List<ShipModulePart>();
        foreach (var module in Modules)
        {
            var definition = library.GetDefinition(module.ModuleDefinitionId);
            if (definition != null && definition.Category == category)
            {
                result.Add(module);
            }
        }
        return result;
    }
    
    /// <summary>
    /// Recalculate all ship statistics from modules
    /// </summary>
    public void RecalculateStats()
    {
        if (Modules.Count == 0)
        {
            TotalMass = 0;
            TotalHealth = 0;
            MaxTotalHealth = 0;
            CenterOfMass = Vector3.Zero;
            Bounds = new BoundingBox();
            AggregatedStats = new ModuleFunctionalStats();
            return;
        }
        
        // Calculate total mass and center of mass
        TotalMass = 0;
        Vector3 weightedPosition = Vector3.Zero;
        
        foreach (var module in Modules)
        {
            TotalMass += module.Mass;
            weightedPosition += module.Position * module.Mass;
        }
        
        CenterOfMass = TotalMass > 0 ? weightedPosition / TotalMass : Vector3.Zero;
        
        // Calculate total health
        TotalHealth = Modules.Sum(m => m.Health);
        MaxTotalHealth = Modules.Sum(m => m.MaxHealth);
        
        // Calculate bounding box
        if (Modules.Count > 0)
        {
            var firstModule = Modules[0];
            var min = firstModule.Position;
            var max = firstModule.Position;
            
            foreach (var module in Modules)
            {
                min = Vector3.Min(min, module.Position);
                max = Vector3.Max(max, module.Position);
            }
            
            Bounds = new BoundingBox { Min = min, Max = max };
        }
        
        // Aggregate functional stats
        AggregatedStats = new ModuleFunctionalStats();
        foreach (var module in Modules)
        {
            var stats = module.FunctionalStats;
            AggregatedStats.ThrustPower += stats.ThrustPower;
            AggregatedStats.MaxSpeed = Math.Max(AggregatedStats.MaxSpeed, stats.MaxSpeed);
            AggregatedStats.PowerGeneration += stats.PowerGeneration;
            AggregatedStats.PowerConsumption += stats.PowerConsumption;
            AggregatedStats.PowerStorage += stats.PowerStorage;
            AggregatedStats.ShieldCapacity += stats.ShieldCapacity;
            AggregatedStats.ShieldRechargeRate += stats.ShieldRechargeRate;
            AggregatedStats.WeaponDamage += stats.WeaponDamage;
            AggregatedStats.WeaponRange = Math.Max(AggregatedStats.WeaponRange, stats.WeaponRange);
            AggregatedStats.WeaponMountPoints += stats.WeaponMountPoints;
            AggregatedStats.CargoCapacity += stats.CargoCapacity;
            AggregatedStats.CrewCapacity += stats.CrewCapacity;
            AggregatedStats.CrewRequired += stats.CrewRequired;
            AggregatedStats.HasHyperdrive = AggregatedStats.HasHyperdrive || stats.HasHyperdrive;
            AggregatedStats.HyperdriveRange = Math.Max(AggregatedStats.HyperdriveRange, stats.HyperdriveRange);
            AggregatedStats.MiningPower += stats.MiningPower;
            AggregatedStats.SensorRange = Math.Max(AggregatedStats.SensorRange, stats.SensorRange);
        }
    }
    
    /// <summary>
    /// Check if two modules can be attached based on attachment points
    /// </summary>
    public bool CanAttach(ShipModulePart module1, ShipModulePart module2, 
                          string attachmentPoint1, string attachmentPoint2,
                          ModuleLibrary library)
    {
        var def1 = library.GetDefinition(module1.ModuleDefinitionId);
        var def2 = library.GetDefinition(module2.ModuleDefinitionId);
        
        if (def1 == null || def2 == null) return false;
        
        if (!def1.AttachmentPoints.ContainsKey(attachmentPoint1)) return false;
        if (!def2.AttachmentPoints.ContainsKey(attachmentPoint2)) return false;
        
        var point1 = def1.AttachmentPoints[attachmentPoint1];
        var point2 = def2.AttachmentPoints[attachmentPoint2];
        
        // Check category restrictions
        if (point1.AllowedCategories.Count > 0 && !point1.AllowedCategories.Contains(def2.Category))
            return false;
        
        if (point2.AllowedCategories.Count > 0 && !point2.AllowedCategories.Contains(def1.Category))
            return false;
        
        // Check tag requirements
        if (point1.RequiredTags.Count > 0 && !point1.RequiredTags.All(tag => def2.Tags.Contains(tag)))
            return false;
        
        if (point2.RequiredTags.Count > 0 && !point2.RequiredTags.All(tag => def1.Tags.Contains(tag)))
            return false;
        
        return true;
    }
    
    /// <summary>
    /// Attach two modules together
    /// </summary>
    public bool AttachModules(Guid moduleId1, Guid moduleId2, 
                             string attachmentPoint1, string attachmentPoint2,
                             ModuleLibrary library)
    {
        var module1 = GetModule(moduleId1);
        var module2 = GetModule(moduleId2);
        
        if (module1 == null || module2 == null) return false;
        
        if (!CanAttach(module1, module2, attachmentPoint1, attachmentPoint2, library))
            return false;
        
        module1.AttachedModules.Add(moduleId2);
        module2.AttachedToModules.Add(moduleId1);
        module2.AttachmentPointUsed = attachmentPoint2;
        
        return true;
    }
    
    /// <summary>
    /// Get all modules attached to a specific module
    /// </summary>
    public List<ShipModulePart> GetAttachedModules(Guid moduleId)
    {
        var module = GetModule(moduleId);
        if (module == null) return new List<ShipModulePart>();
        
        return module.AttachedModules
            .Select(id => GetModule(id))
            .Where(m => m != null)
            .Cast<ShipModulePart>()
            .ToList();
    }
}

/// <summary>
/// Simple bounding box structure
/// </summary>
public struct BoundingBox
{
    public Vector3 Min { get; set; }
    public Vector3 Max { get; set; }
    
    public Vector3 Center => (Min + Max) * 0.5f;
    public Vector3 Size => Max - Min;
}
