using System.Numerics;

namespace AvorionLike.Core.Modular;

/// <summary>
/// Represents an individual module instance on a station
/// Similar to ShipModulePart but for stations
/// </summary>
public class StationModulePart
{
    public Guid Id { get; set; } = Guid.NewGuid();
    
    /// <summary>
    /// Reference to the module definition (type)
    /// </summary>
    public string ModuleDefinitionId { get; set; } = "";
    
    /// <summary>
    /// Position in station space
    /// </summary>
    public Vector3 Position { get; set; }
    
    /// <summary>
    /// Rotation in degrees (Euler angles)
    /// </summary>
    public Vector3 Rotation { get; set; }
    
    /// <summary>
    /// Scale factor
    /// </summary>
    public Vector3 Scale { get; set; } = Vector3.One;
    
    /// <summary>
    /// Material type (Iron, Titanium, etc.)
    /// </summary>
    public string MaterialType { get; set; } = "Iron";
    
    /// <summary>
    /// Current health
    /// </summary>
    public float Health { get; set; } = 100f;
    
    /// <summary>
    /// Maximum health
    /// </summary>
    public float MaxHealth { get; set; } = 100f;
    
    /// <summary>
    /// Mass of this module
    /// </summary>
    public float Mass { get; set; } = 10f;
    
    /// <summary>
    /// Module category
    /// </summary>
    public StationModuleCategory Category { get; set; } = StationModuleCategory.Hub;
    
    /// <summary>
    /// IDs of modules this module is attached to
    /// </summary>
    public List<Guid> AttachedToModules { get; set; } = new();
    
    /// <summary>
    /// IDs of modules attached to this module
    /// </summary>
    public List<Guid> AttachedModules { get; set; } = new();
    
    /// <summary>
    /// Functional stats for this module instance
    /// </summary>
    public StationFunctionalStats FunctionalStats { get; set; } = new();
    
    /// <summary>
    /// Is this module destroyed?
    /// </summary>
    public bool IsDestroyed => Health <= 0;
    
    /// <summary>
    /// Attach another module to this one
    /// </summary>
    public void AttachModule(Guid moduleId)
    {
        if (!AttachedModules.Contains(moduleId))
        {
            AttachedModules.Add(moduleId);
        }
    }
    
    /// <summary>
    /// Detach a module from this one
    /// </summary>
    public void DetachModule(Guid moduleId)
    {
        AttachedModules.Remove(moduleId);
    }
}

/// <summary>
/// Station module categories
/// </summary>
public enum StationModuleCategory
{
    Hub,            // Central command/core
    Docking,        // Docking bays and hangars
    Production,     // Factories and refineries
    Storage,        // Cargo and warehouses
    Defense,        // Turrets and shields
    Utility,        // Power, life support, sensors
    Habitat,        // Crew quarters and facilities
    Trade,          // Market and trade facilities
    Research,       // Research labs
    Structural      // Support beams and connectors
}
