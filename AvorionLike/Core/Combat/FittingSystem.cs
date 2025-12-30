using AvorionLike.Core.ECS;
using AvorionLike.Core.Logging;

namespace AvorionLike.Core.Combat;

/// <summary>
/// System managing ship fitting and module activation
/// Inspired by EVE Online's fitting mechanics
/// </summary>
public class FittingSystem : SystemBase
{
    private readonly EntityManager _entityManager;
    
    public FittingSystem(EntityManager entityManager) : base("FittingSystem")
    {
        _entityManager = entityManager;
    }
    
    public override void Update(float deltaTime)
    {
        var fittings = _entityManager.GetAllComponents<FittingComponent>();
        
        foreach (var fitting in fittings)
        {
            UpdateFitting(fitting, deltaTime);
        }
    }
    
    /// <summary>
    /// Update fitting state (capacitor, cooldowns, etc.)
    /// </summary>
    private void UpdateFitting(FittingComponent fitting, float deltaTime)
    {
        // Recharge capacitor
        fitting.CurrentCapacitor += fitting.CapacitorRechargeRate * deltaTime;
        fitting.CurrentCapacitor = MathF.Min(fitting.CurrentCapacitor, fitting.MaxCapacitor);
        
        // Update module cooldowns
        foreach (var module in fitting.FittedModules)
        {
            if (module.CurrentCooldown > 0)
            {
                module.CurrentCooldown -= deltaTime;
                if (module.CurrentCooldown < 0)
                    module.CurrentCooldown = 0;
            }
        }
    }
    
    /// <summary>
    /// Fit a module to a ship
    /// </summary>
    public bool FitModule(Guid shipEntityId, Module module)
    {
        var fitting = _entityManager.GetComponent<FittingComponent>(shipEntityId);
        if (fitting == null)
        {
            Logger.Instance.Warning("FittingSystem", "Ship has no fitting component");
            return false;
        }
        
        if (!fitting.CanFitModule(module))
        {
            Logger.Instance.Warning("FittingSystem", "Cannot fit module - insufficient resources");
            return false;
        }
        
        fitting.FittedModules.Add(module);
        fitting.UsedPowerGrid += module.PowerGridRequirement;
        fitting.UsedCPU += module.CPURequirement;
        
        Logger.Instance.Info("FittingSystem", $"Fitted module: {module.Name}");
        return true;
    }
    
    /// <summary>
    /// Remove a module from a ship
    /// </summary>
    public bool UnfitModule(Guid shipEntityId, Guid moduleId)
    {
        var fitting = _entityManager.GetComponent<FittingComponent>(shipEntityId);
        if (fitting == null)
            return false;
            
        var module = fitting.FittedModules.FirstOrDefault(m => m.ModuleId == moduleId);
        if (module == null)
            return false;
            
        fitting.FittedModules.Remove(module);
        fitting.UsedPowerGrid -= module.PowerGridRequirement;
        fitting.UsedCPU -= module.CPURequirement;
        
        Logger.Instance.Info("FittingSystem", $"Removed module: {module.Name}");
        return true;
    }
    
    /// <summary>
    /// Activate a module
    /// </summary>
    public bool ActivateModule(Guid shipEntityId, Guid moduleId)
    {
        var fitting = _entityManager.GetComponent<FittingComponent>(shipEntityId);
        if (fitting == null)
            return false;
            
        var module = fitting.FittedModules.FirstOrDefault(m => m.ModuleId == moduleId);
        if (module == null)
            return false;
            
        if (!module.CanActivate(fitting.CurrentCapacitor))
        {
            Logger.Instance.Warning("FittingSystem", "Cannot activate module");
            return false;
        }
        
        // Consume capacitor
        fitting.CurrentCapacitor -= module.CapacitorCost;
        
        // Set cooldown
        module.CurrentCooldown = module.Cooldown;
        module.IsActive = true;
        
        // Apply module effects
        ApplyModuleEffects(fitting, module);
        
        Logger.Instance.Info("FittingSystem", $"Activated module: {module.Name}");
        return true;
    }
    
    /// <summary>
    /// Deactivate a module
    /// </summary>
    public void DeactivateModule(Guid shipEntityId, Guid moduleId)
    {
        var fitting = _entityManager.GetComponent<FittingComponent>(shipEntityId);
        if (fitting == null)
            return;
            
        var module = fitting.FittedModules.FirstOrDefault(m => m.ModuleId == moduleId);
        if (module == null)
            return;
            
        module.IsActive = false;
        
        Logger.Instance.Info("FittingSystem", $"Deactivated module: {module.Name}");
    }
    
    /// <summary>
    /// Apply the effects of an activated module
    /// </summary>
    private void ApplyModuleEffects(FittingComponent fitting, Module module)
    {
        switch (module.Type)
        {
            case ModuleType.ShieldBooster:
                // Boost shields (would integrate with combat system)
                if (module.Attributes.TryGetValue("shieldBoostAmount", out float boostAmount))
                {
                    var combat = _entityManager.GetComponent<CombatComponent>(fitting.EntityId);
                    if (combat != null)
                    {
                        combat.CurrentShields = MathF.Min(
                            combat.CurrentShields + boostAmount, 
                            combat.MaxShields
                        );
                    }
                }
                break;
                
            case ModuleType.ArmorRepairer:
                // Repair armor
                if (module.Attributes.TryGetValue("armorRepairAmount", out float repairAmount))
                {
                    // Would apply to armor component
                }
                break;
                
            case ModuleType.Afterburner:
            case ModuleType.MicroWarpDrive:
                // Increase speed (would integrate with physics)
                break;
                
            case ModuleType.CapacitorBooster:
                // Inject capacitor
                if (module.Attributes.TryGetValue("capacitorBonus", out float capBonus))
                {
                    fitting.CurrentCapacitor += capBonus;
                    fitting.CurrentCapacitor = MathF.Min(fitting.CurrentCapacitor, fitting.MaxCapacitor);
                }
                break;
        }
    }
    
    /// <summary>
    /// Get all modules of a specific type
    /// </summary>
    public IEnumerable<Module> GetModulesByType(Guid shipEntityId, ModuleType type)
    {
        var fitting = _entityManager.GetComponent<FittingComponent>(shipEntityId);
        if (fitting == null)
            return Enumerable.Empty<Module>();
            
        return fitting.FittedModules.Where(m => m.Type == type);
    }
    
    /// <summary>
    /// Get all modules in a specific slot
    /// </summary>
    public IEnumerable<Module> GetModulesBySlot(Guid shipEntityId, ModuleSlot slot)
    {
        var fitting = _entityManager.GetComponent<FittingComponent>(shipEntityId);
        if (fitting == null)
            return Enumerable.Empty<Module>();
            
        return fitting.FittedModules.Where(m => m.SlotType == slot);
    }
    
    /// <summary>
    /// Create a standard module
    /// </summary>
    public Module CreateModule(string name, ModuleType type, ModuleSlot slot, 
        float powerGrid, float cpu, float capCost = 0f)
    {
        return new Module
        {
            Name = name,
            Type = type,
            SlotType = slot,
            PowerGridRequirement = powerGrid,
            CPURequirement = cpu,
            CapacitorCost = capCost
        };
    }
    
    /// <summary>
    /// Validate entire fitting
    /// </summary>
    public (bool isValid, List<string> errors) ValidateFitting(Guid shipEntityId)
    {
        var fitting = _entityManager.GetComponent<FittingComponent>(shipEntityId);
        if (fitting == null)
            return (false, new List<string> { "No fitting component" });
            
        var errors = new List<string>();
        
        if (fitting.UsedPowerGrid > fitting.MaxPowerGrid)
            errors.Add($"Power grid exceeded: {fitting.UsedPowerGrid}/{fitting.MaxPowerGrid} MW");
            
        if (fitting.UsedCPU > fitting.MaxCPU)
            errors.Add($"CPU exceeded: {fitting.UsedCPU}/{fitting.MaxCPU} tf");
            
        if (fitting.FittedModules.Count > fitting.MaxModuleSlots)
            errors.Add($"Too many modules: {fitting.FittedModules.Count}/{fitting.MaxModuleSlots}");
            
        return (errors.Count == 0, errors);
    }
}
