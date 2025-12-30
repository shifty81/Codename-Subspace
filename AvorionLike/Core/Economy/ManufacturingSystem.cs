using AvorionLike.Core.ECS;
using AvorionLike.Core.Resources;
using AvorionLike.Core.Logging;

namespace AvorionLike.Core.Economy;

/// <summary>
/// System managing manufacturing and blueprint research
/// Inspired by EVE Online's industry system
/// </summary>
public class ManufacturingSystem : SystemBase
{
    private readonly EntityManager _entityManager;
    
    public ManufacturingSystem(EntityManager entityManager) : base("ManufacturingSystem")
    {
        _entityManager = entityManager;
    }
    
    public override void Update(float deltaTime)
    {
        // Update all manufacturing facilities
        var facilities = _entityManager.GetAllComponents<ManufacturingFacilityComponent>();
        
        foreach (var facility in facilities)
        {
            UpdateManufacturingJobs(facility, deltaTime);
        }
    }
    
    /// <summary>
    /// Update manufacturing jobs progress
    /// </summary>
    private void UpdateManufacturingJobs(ManufacturingFacilityComponent facility, float deltaTime)
    {
        foreach (var job in facility.ActiveJobs.ToList())
        {
            if (job.IsComplete)
                continue;
                
            job.TimeRemaining -= deltaTime;
            
            if (job.TimeRemaining <= 0)
            {
                CompleteManufacturingJob(facility, job);
            }
        }
    }
    
    /// <summary>
    /// Complete a manufacturing job and deliver the product
    /// </summary>
    private void CompleteManufacturingJob(ManufacturingFacilityComponent facility, ManufacturingJob job)
    {
        job.IsComplete = true;
        
        var blueprint = _entityManager.GetComponent<BlueprintComponent>(job.BlueprintId);
        if (blueprint == null)
        {
            Logger.Instance.Warning("ManufacturingSystem", "Blueprint not found for completed job");
            return;
        }
        
        // Decrement runs if it's a copy
        if (!blueprint.IsOriginal && blueprint.RunsRemaining > 0)
        {
            blueprint.RunsRemaining--;
            
            if (blueprint.RunsRemaining <= 0)
            {
                Logger.Instance.Info("ManufacturingSystem", $"Blueprint {blueprint.Name} exhausted");
            }
        }
        
        // Add manufactured items to owner's inventory
        var ownerInventory = _entityManager.GetComponent<InventoryComponent>(job.OwnerId);
        if (ownerInventory != null)
        {
            // Simplified: would create actual items/ships
            Logger.Instance.Info("ManufacturingSystem", 
                $"Manufacturing job completed: {blueprint.Name} x{job.Runs}");
        }
        
        facility.ActiveJobs.Remove(job);
    }
    
    /// <summary>
    /// Start a new manufacturing job
    /// </summary>
    public bool StartManufacturingJob(Guid facilityId, Guid blueprintId, Guid ownerId, int runs)
    {
        var facility = _entityManager.GetComponent<ManufacturingFacilityComponent>(facilityId);
        if (facility == null)
        {
            Logger.Instance.Warning("ManufacturingSystem", "Manufacturing facility not found");
            return false;
        }
        
        if (facility.ActiveJobs.Count >= facility.MaxJobs)
        {
            Logger.Instance.Warning("ManufacturingSystem", "Manufacturing facility at capacity");
            return false;
        }
        
        var blueprint = _entityManager.GetComponent<BlueprintComponent>(blueprintId);
        if (blueprint == null)
        {
            Logger.Instance.Warning("ManufacturingSystem", "Blueprint not found");
            return false;
        }
        
        // Check if blueprint has enough runs
        if (!blueprint.IsOriginal && blueprint.RunsRemaining < runs)
        {
            Logger.Instance.Warning("ManufacturingSystem", "Not enough runs on blueprint");
            return false;
        }
        
        var ownerInventory = _entityManager.GetComponent<InventoryComponent>(ownerId);
        if (ownerInventory == null)
        {
            Logger.Instance.Warning("ManufacturingSystem", "Owner inventory not found");
            return false;
        }
        
        // Check material requirements
        var materials = blueprint.GetActualMaterialRequirements();
        foreach (var material in materials)
        {
            int required = material.Value * runs;
            if (!ownerInventory.Inventory.HasResource(material.Key, required))
            {
                Logger.Instance.Warning("ManufacturingSystem", 
                    $"Insufficient materials: need {required} {material.Key}");
                return false;
            }
        }
        
        // Consume materials
        foreach (var material in materials)
        {
            int required = material.Value * runs;
            ownerInventory.Inventory.RemoveResource(material.Key, required);
        }
        
        // Create manufacturing job
        float productionTime = blueprint.GetActualProductionTime();
        productionTime *= (1.0f - facility.TimeBonus); // Apply facility bonus
        productionTime *= runs;
        
        var job = new ManufacturingJob
        {
            BlueprintId = blueprintId,
            FacilityId = facilityId,
            OwnerId = ownerId,
            Runs = runs,
            TimeRemaining = productionTime,
            TotalTime = productionTime,
            StartTime = DateTime.UtcNow
        };
        
        facility.ActiveJobs.Add(job);
        
        Logger.Instance.Info("ManufacturingSystem", 
            $"Started manufacturing job: {blueprint.Name} x{runs} ({productionTime:F0}s)");
        
        return true;
    }
    
    /// <summary>
    /// Research blueprint for material efficiency
    /// </summary>
    public bool ResearchMaterialEfficiency(Guid blueprintId, int pointsToAdd)
    {
        var blueprint = _entityManager.GetComponent<BlueprintComponent>(blueprintId);
        if (blueprint == null)
            return false;
            
        if (!blueprint.IsOriginal)
        {
            Logger.Instance.Warning("ManufacturingSystem", "Cannot research blueprint copies");
            return false;
        }
        
        blueprint.MaterialResearchPoints += pointsToAdd;
        
        // Level up based on research points
        int newLevel = CalculateResearchLevel(blueprint.MaterialResearchPoints);
        if (newLevel > blueprint.MaterialEfficiency)
        {
            blueprint.MaterialEfficiency = newLevel;
            Logger.Instance.Info("ManufacturingSystem", 
                $"Blueprint {blueprint.Name} material efficiency improved to level {newLevel}");
        }
        
        return true;
    }
    
    /// <summary>
    /// Research blueprint for time efficiency
    /// </summary>
    public bool ResearchTimeEfficiency(Guid blueprintId, int pointsToAdd)
    {
        var blueprint = _entityManager.GetComponent<BlueprintComponent>(blueprintId);
        if (blueprint == null)
            return false;
            
        if (!blueprint.IsOriginal)
        {
            Logger.Instance.Warning("ManufacturingSystem", "Cannot research blueprint copies");
            return false;
        }
        
        blueprint.TimeResearchPoints += pointsToAdd;
        
        // Level up based on research points
        int newLevel = CalculateResearchLevel(blueprint.TimeResearchPoints);
        if (newLevel > blueprint.TimeEfficiency)
        {
            blueprint.TimeEfficiency = newLevel;
            Logger.Instance.Info("ManufacturingSystem", 
                $"Blueprint {blueprint.Name} time efficiency improved to level {newLevel}");
        }
        
        return true;
    }
    
    /// <summary>
    /// Copy a blueprint
    /// </summary>
    public Guid? CopyBlueprint(Guid originalBlueprintId, int runs, Guid ownerId)
    {
        var original = _entityManager.GetComponent<BlueprintComponent>(originalBlueprintId);
        if (original == null || !original.IsOriginal)
            return null;
            
        var copyEntity = _entityManager.CreateEntity($"{original.Name} (Copy)");
        
        var copy = new BlueprintComponent
        {
            EntityId = copyEntity.Id,
            Name = original.Name,
            Type = original.Type,
            MaterialEfficiency = original.MaterialEfficiency,
            TimeEfficiency = original.TimeEfficiency,
            RunsRemaining = runs,
            IsOriginal = false,
            MaterialRequirements = new Dictionary<ResourceType, int>(original.MaterialRequirements),
            BaseProductionTime = original.BaseProductionTime
        };
        
        _entityManager.AddComponent(copyEntity.Id, copy);
        
        Logger.Instance.Info("ManufacturingSystem", 
            $"Created blueprint copy: {copy.Name} with {runs} runs");
        
        return copyEntity.Id;
    }
    
    /// <summary>
    /// Calculate research level from points
    /// </summary>
    private int CalculateResearchLevel(int points)
    {
        // Exponential cost: level 1 = 100 points, level 2 = 250, level 3 = 500, etc.
        int level = 0;
        int pointsNeeded = 0;
        
        while (pointsNeeded <= points && level < 10)
        {
            level++;
            pointsNeeded += (int)(100 * Math.Pow(2, level - 1));
        }
        
        return Math.Max(0, level - 1);
    }
    
    /// <summary>
    /// Get all active jobs for an owner
    /// </summary>
    public IEnumerable<ManufacturingJob> GetJobsForOwner(Guid ownerId)
    {
        var facilities = _entityManager.GetAllComponents<ManufacturingFacilityComponent>();
        
        return facilities.SelectMany(f => f.ActiveJobs)
                        .Where(j => j.OwnerId == ownerId);
    }
    
    /// <summary>
    /// Cancel a manufacturing job and refund materials
    /// </summary>
    public bool CancelJob(Guid jobId)
    {
        var facilities = _entityManager.GetAllComponents<ManufacturingFacilityComponent>();
        
        foreach (var facility in facilities)
        {
            var job = facility.ActiveJobs.FirstOrDefault(j => j.JobId == jobId);
            if (job != null)
            {
                // Refund partial materials based on progress
                float progressPercent = 1.0f - (job.TimeRemaining / job.TotalTime);
                float refundPercent = 1.0f - (progressPercent * 0.5f); // Lose 50% of consumed materials
                
                var blueprint = _entityManager.GetComponent<BlueprintComponent>(job.BlueprintId);
                var ownerInventory = _entityManager.GetComponent<InventoryComponent>(job.OwnerId);
                
                if (blueprint != null && ownerInventory != null)
                {
                    var materials = blueprint.GetActualMaterialRequirements();
                    foreach (var material in materials)
                    {
                        int refundAmount = (int)(material.Value * job.Runs * refundPercent);
                        ownerInventory.Inventory.AddResource(material.Key, refundAmount);
                    }
                }
                
                facility.ActiveJobs.Remove(job);
                Logger.Instance.Info("ManufacturingSystem", $"Cancelled manufacturing job {jobId}");
                return true;
            }
        }
        
        return false;
    }
}
