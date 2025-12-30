using AvorionLike.Core.ECS;

namespace AvorionLike.Core.Combat;

/// <summary>
/// Component for ship fitting with power grid, CPU, and capacitor constraints
/// Inspired by EVE Online's fitting system
/// </summary>
public class FittingComponent : IComponent
{
    public Guid EntityId { get; set; }
    
    /// <summary>
    /// Maximum power grid available (MW)
    /// </summary>
    public float MaxPowerGrid { get; set; } = 1000f;
    
    /// <summary>
    /// Current power grid used (MW)
    /// </summary>
    public float UsedPowerGrid { get; set; } = 0f;
    
    /// <summary>
    /// Maximum CPU available (tf)
    /// </summary>
    public float MaxCPU { get; set; } = 500f;
    
    /// <summary>
    /// Current CPU used (tf)
    /// </summary>
    public float UsedCPU { get; set; } = 0f;
    
    /// <summary>
    /// Maximum capacitor energy (GJ)
    /// </summary>
    public float MaxCapacitor { get; set; } = 1000f;
    
    /// <summary>
    /// Current capacitor energy (GJ)
    /// </summary>
    public float CurrentCapacitor { get; set; } = 1000f;
    
    /// <summary>
    /// Capacitor recharge rate (GJ/s)
    /// </summary>
    public float CapacitorRechargeRate { get; set; } = 10f;
    
    /// <summary>
    /// Fitted modules
    /// </summary>
    public List<Module> FittedModules { get; set; } = new();
    
    /// <summary>
    /// Maximum number of module slots
    /// </summary>
    public int MaxModuleSlots { get; set; } = 8;
    
    /// <summary>
    /// Check if a module can be fitted
    /// </summary>
    public bool CanFitModule(Module module)
    {
        if (FittedModules.Count >= MaxModuleSlots)
            return false;
            
        if (UsedPowerGrid + module.PowerGridRequirement > MaxPowerGrid)
            return false;
            
        if (UsedCPU + module.CPURequirement > MaxCPU)
            return false;
            
        return true;
    }
    
    /// <summary>
    /// Get available power grid
    /// </summary>
    public float AvailablePowerGrid => MaxPowerGrid - UsedPowerGrid;
    
    /// <summary>
    /// Get available CPU
    /// </summary>
    public float AvailableCPU => MaxCPU - UsedCPU;
    
    /// <summary>
    /// Get capacitor percentage
    /// </summary>
    public float CapacitorPercent => (CurrentCapacitor / MaxCapacitor) * 100f;
}

/// <summary>
/// Represents a fitted module
/// </summary>
public class Module
{
    public Guid ModuleId { get; set; } = Guid.NewGuid();
    public string Name { get; set; } = "Unknown Module";
    public FittingModuleType Type { get; set; }
    public ModuleSlot SlotType { get; set; }
    
    /// <summary>
    /// Power grid requirement (MW)
    /// </summary>
    public float PowerGridRequirement { get; set; } = 10f;
    
    /// <summary>
    /// CPU requirement (tf)
    /// </summary>
    public float CPURequirement { get; set; } = 5f;
    
    /// <summary>
    /// Capacitor usage per activation (GJ)
    /// </summary>
    public float CapacitorCost { get; set; } = 0f;
    
    /// <summary>
    /// Activation time (seconds)
    /// </summary>
    public float ActivationTime { get; set; } = 1f;
    
    /// <summary>
    /// Cooldown between activations (seconds)
    /// </summary>
    public float Cooldown { get; set; } = 0f;
    
    /// <summary>
    /// Current cooldown remaining
    /// </summary>
    public float CurrentCooldown { get; set; } = 0f;
    
    /// <summary>
    /// Whether the module is currently active
    /// </summary>
    public bool IsActive { get; set; } = false;
    
    /// <summary>
    /// Module attributes (bonuses, effects, etc.)
    /// </summary>
    public Dictionary<string, float> Attributes { get; set; } = new();
    
    /// <summary>
    /// Check if module can be activated
    /// </summary>
    public bool CanActivate(float currentCapacitor)
    {
        if (CurrentCooldown > 0)
            return false;
            
        if (CapacitorCost > currentCapacitor)
            return false;
            
        return true;
    }
}

/// <summary>
/// Type of fitting module
/// </summary>
public enum FittingModuleType
{
    // Weapons
    Turret,
    Launcher,
    
    // Defense
    ShieldBooster,
    ArmorRepairer,
    HullRepairer,
    
    // Propulsion
    Afterburner,
    MicroWarpDrive,
    
    // Electronic Warfare
    SensorDampener,
    TargetPainter,
    EnergyNeutralizer,
    WarpScrambler,
    
    // Engineering
    CapacitorBooster,
    PowerDiagnostic,
    ReactorControlUnit,
    
    // Utility
    CargoExpander,
    ScannerArray,
    CloakingDevice
}

/// <summary>
/// Module slot category
/// </summary>
public enum ModuleSlot
{
    High,      // Weapons and utility
    Medium,    // Defense and propulsion
    Low,       // Engineering and passive bonuses
    Rig        // Permanent modifications
}
