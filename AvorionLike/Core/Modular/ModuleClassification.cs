using System;

namespace AvorionLike.Core.Modular;

/// <summary>
/// Ship class classification for module compatibility
/// Defines which types of ships can use which modules
/// </summary>
[Flags]
public enum ShipClass
{
    None = 0,
    Fighter = 1 << 0,      // Small, agile combat ships (1-2 crew)
    Corvette = 1 << 1,     // Light escort ships (3-10 crew)
    Frigate = 1 << 2,      // Medium combat ships (20-50 crew)
    Destroyer = 1 << 3,    // Heavy combat ships (100-200 crew)
    Cruiser = 1 << 4,      // Major warships (200-500 crew)
    Battleship = 1 << 5,   // Capital warships (500-1000 crew)
    Carrier = 1 << 6,      // Fighter carriers (1000+ crew)
    
    // Industrial classes
    Miner = 1 << 7,        // Mining vessels
    Hauler = 1 << 8,       // Cargo transport
    Salvager = 1 << 9,     // Salvage ships
    Refinery = 1 << 10,    // Processing ships
    Constructor = 1 << 11, // Station builders
    
    // Special classes
    Scout = 1 << 12,       // Fast exploration
    Science = 1 << 13,     // Research vessels
    Support = 1 << 14,     // Repair/supply ships
    
    // Convenience groupings
    AllCombat = Fighter | Corvette | Frigate | Destroyer | Cruiser | Battleship | Carrier,
    AllCapital = Battleship | Carrier,
    AllIndustrial = Miner | Hauler | Salvager | Refinery | Constructor,
    AllCivilian = Scout | Science | Support | AllIndustrial,
    All = ~0
}

/// <summary>
/// Module size classification
/// Affects compatibility, power requirements, and visual scale
/// </summary>
public enum ModuleSize
{
    Tiny,       // Smallest modules (fighter-only components)
    Small,      // Fighter/Corvette modules
    Medium,     // Frigate/Destroyer modules
    Large,      // Cruiser/Battleship modules
    Huge,       // Capital-only modules
    Massive     // Station/super-capital only
}

/// <summary>
/// Module visibility determines where it appears in the ship editor
/// </summary>
public enum ModuleVisibility
{
    External,   // Visible on ship exterior (wings, engines, weapons)
    Internal,   // Internal systems (cargo, crew quarters, shield generators)
    Both        // Can be external or internal (sensors, power cores)
}

/// <summary>
/// Extended module definition properties for class-specific modules
/// This extends ShipModuleDefinition with new classification properties
/// </summary>
public class ModuleClassificationInfo
{
    /// <summary>
    /// Which ship classes can use this module
    /// </summary>
    public ShipClass CompatibleClasses { get; set; } = ShipClass.All;
    
    /// <summary>
    /// Size classification of this module
    /// </summary>
    public ModuleSize Size { get; set; } = ModuleSize.Medium;
    
    /// <summary>
    /// Where this module appears in the ship editor
    /// </summary>
    public ModuleVisibility Visibility { get; set; } = ModuleVisibility.External;
    
    /// <summary>
    /// Visual style variant (military, industrial, sleek, etc.)
    /// </summary>
    public string StyleVariant { get; set; } = "standard";
    
    /// <summary>
    /// Is this module required for ship to function
    /// </summary>
    public bool IsRequired { get; set; } = false;
    
    /// <summary>
    /// Maximum number of this module type per ship (0 = unlimited)
    /// </summary>
    public int MaxPerShip { get; set; } = 0;
    
    /// <summary>
    /// Minimum number of this module type per ship
    /// </summary>
    public int MinPerShip { get; set; } = 0;
    
    /// <summary>
    /// Can this module be damaged/destroyed separately
    /// </summary>
    public bool IsDestructible { get; set; } = true;
    
    /// <summary>
    /// Priority for auto-targeting (for weapons/turrets)
    /// </summary>
    public int TargetPriority { get; set; } = 0;
    
    /// <summary>
    /// Check if this module is compatible with a given ship class
    /// </summary>
    public bool IsCompatibleWith(ShipClass shipClass)
    {
        return (CompatibleClasses & shipClass) != 0;
    }
    
    /// <summary>
    /// Get display name for size
    /// </summary>
    public string GetSizeDisplayName()
    {
        return Size switch
        {
            ModuleSize.Tiny => "Tiny",
            ModuleSize.Small => "Small",
            ModuleSize.Medium => "Medium",
            ModuleSize.Large => "Large",
            ModuleSize.Huge => "Huge",
            ModuleSize.Massive => "Massive",
            _ => "Unknown"
        };
    }
    
    /// <summary>
    /// Get recommended ship classes as a readable string
    /// </summary>
    public string GetCompatibleClassesString()
    {
        if (CompatibleClasses == ShipClass.All)
            return "All Classes";
        if (CompatibleClasses == ShipClass.AllCombat)
            return "All Combat";
        if (CompatibleClasses == ShipClass.AllCapital)
            return "Capital Only";
        if (CompatibleClasses == ShipClass.AllIndustrial)
            return "Industrial Only";
            
        var classes = new List<string>();
        foreach (ShipClass value in Enum.GetValues(typeof(ShipClass)))
        {
            if (value != ShipClass.None && value != ShipClass.All && 
                value != ShipClass.AllCombat && value != ShipClass.AllCapital && 
                value != ShipClass.AllIndustrial && value != ShipClass.AllCivilian)
            {
                if ((CompatibleClasses & value) != 0)
                    classes.Add(value.ToString());
            }
        }
        
        return classes.Count > 0 ? string.Join(", ", classes) : "None";
    }
}

/// <summary>
/// Helper class for module filtering and classification
/// </summary>
public static class ModuleClassificationHelper
{
    /// <summary>
    /// Get all modules compatible with a ship class
    /// </summary>
    public static List<ShipModuleDefinition> FilterByShipClass(
        IEnumerable<ShipModuleDefinition> modules, 
        ShipClass shipClass)
    {
        return modules.Where(m => 
        {
            // Check if module has classification info
            if (m.Tags.Contains("class_info"))
            {
                // Parse classification from tags or use default
                return true; // TODO: Implement proper tag parsing
            }
            return true; // Default: all modules compatible
        }).ToList();
    }
    
    /// <summary>
    /// Get all external modules (visible on ship exterior)
    /// </summary>
    public static List<ShipModuleDefinition> GetExternalModules(
        IEnumerable<ShipModuleDefinition> modules)
    {
        var externalCategories = new[] 
        { 
            ModuleCategory.Hull, 
            ModuleCategory.Wing, 
            ModuleCategory.Tail,
            ModuleCategory.Engine, 
            ModuleCategory.Thruster,
            ModuleCategory.WeaponMount,
            ModuleCategory.Antenna
        };
        
        return modules.Where(m => externalCategories.Contains(m.Category)).ToList();
    }
    
    /// <summary>
    /// Get all internal modules (ship interior systems)
    /// </summary>
    public static List<ShipModuleDefinition> GetInternalModules(
        IEnumerable<ShipModuleDefinition> modules)
    {
        var internalCategories = new[] 
        { 
            ModuleCategory.PowerCore, 
            ModuleCategory.Shield, 
            ModuleCategory.Cargo,
            ModuleCategory.CrewQuarters, 
            ModuleCategory.Hyperdrive,
            ModuleCategory.Sensor
        };
        
        return modules.Where(m => internalCategories.Contains(m.Category)).ToList();
    }
    
    /// <summary>
    /// Get recommended module size for a ship class
    /// </summary>
    public static ModuleSize GetRecommendedSizeForClass(ShipClass shipClass)
    {
        return shipClass switch
        {
            ShipClass.Fighter => ModuleSize.Tiny,
            ShipClass.Corvette => ModuleSize.Small,
            ShipClass.Frigate => ModuleSize.Medium,
            ShipClass.Destroyer => ModuleSize.Medium,
            ShipClass.Cruiser => ModuleSize.Large,
            ShipClass.Battleship => ModuleSize.Huge,
            ShipClass.Carrier => ModuleSize.Huge,
            ShipClass.Miner => ModuleSize.Medium,
            ShipClass.Hauler => ModuleSize.Large,
            _ => ModuleSize.Medium
        };
    }
    
    /// <summary>
    /// Get display name for ship class
    /// </summary>
    public static string GetShipClassDisplayName(ShipClass shipClass)
    {
        return shipClass switch
        {
            ShipClass.Fighter => "Fighter",
            ShipClass.Corvette => "Corvette",
            ShipClass.Frigate => "Frigate",
            ShipClass.Destroyer => "Destroyer",
            ShipClass.Cruiser => "Cruiser",
            ShipClass.Battleship => "Battleship",
            ShipClass.Carrier => "Carrier",
            ShipClass.Miner => "Mining Ship",
            ShipClass.Hauler => "Cargo Hauler",
            ShipClass.Salvager => "Salvage Ship",
            ShipClass.Refinery => "Refinery Ship",
            ShipClass.Constructor => "Constructor Ship",
            ShipClass.Scout => "Scout Ship",
            ShipClass.Science => "Science Vessel",
            ShipClass.Support => "Support Ship",
            _ => shipClass.ToString()
        };
    }
}
