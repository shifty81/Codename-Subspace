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
/// Ship size category classification
/// S = Small, M = Medium, L = Large, XL = Capital
/// This is separate from ShipClass (Fighter, Corvette, etc.)
/// </summary>
public enum ShipSizeCategory
{
    S,      // Small ships (Fighters, Scouts, small Corvettes)
    M,      // Medium ships (Large Corvettes, Frigates, Destroyers, Miners)
    L,      // Large ships (Cruisers, Large Industrial, Science vessels)
    XL      // Capital ships (Battleships, Carriers, Capital Industrial)
}

/// <summary>
/// Module size classification
/// Affects compatibility, power requirements, and visual scale
/// Aligned with ship size system
/// </summary>
public enum ModuleSize
{
    S,      // Small modules - for S-class ships
    M,      // Medium modules - for M-class ships
    L,      // Large modules - for L-class ships
    XL      // Capital modules - for XL-class ships only
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
    public ModuleSize Size { get; set; } = ModuleSize.M;
    
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
        return ModuleClassificationHelper.GetModuleSizeDisplayName(Size);
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
    /// Maps ship classes to S/M/L/XL sizing system
    /// </summary>
    public static ModuleSize GetRecommendedSizeForClass(ShipClass shipClass)
    {
        return shipClass switch
        {
            // Small (S) - Fighters, Scouts
            ShipClass.Fighter => ModuleSize.S,
            ShipClass.Scout => ModuleSize.S,
            
            // Medium (M) - Corvettes, Frigates, Destroyers, basic Industrial
            ShipClass.Corvette => ModuleSize.M,
            ShipClass.Frigate => ModuleSize.M,
            ShipClass.Destroyer => ModuleSize.M,
            ShipClass.Miner => ModuleSize.M,
            ShipClass.Salvager => ModuleSize.M,
            
            // Large (L) - Cruisers, large Industrial, Science
            ShipClass.Cruiser => ModuleSize.L,
            ShipClass.Hauler => ModuleSize.L,
            ShipClass.Refinery => ModuleSize.L,
            ShipClass.Science => ModuleSize.L,
            ShipClass.Support => ModuleSize.L,
            
            // Capital (XL) - Battleships, Carriers, Constructor
            ShipClass.Battleship => ModuleSize.XL,
            ShipClass.Carrier => ModuleSize.XL,
            ShipClass.Constructor => ModuleSize.XL,
            
            _ => ModuleSize.M  // Default to medium
        };
    }
    
    /// <summary>
    /// Get ship size classification from ship class
    /// </summary>
    public static ShipSizeCategory GetShipSizeFromClass(ShipClass shipClass)
    {
        return shipClass switch
        {
            ShipClass.Fighter => ShipSizeCategory.S,
            ShipClass.Scout => ShipSizeCategory.S,
            
            ShipClass.Corvette => ShipSizeCategory.M,
            ShipClass.Frigate => ShipSizeCategory.M,
            ShipClass.Destroyer => ShipSizeCategory.M,
            ShipClass.Miner => ShipSizeCategory.M,
            ShipClass.Salvager => ShipSizeCategory.M,
            
            ShipClass.Cruiser => ShipSizeCategory.L,
            ShipClass.Hauler => ShipSizeCategory.L,
            ShipClass.Refinery => ShipSizeCategory.L,
            ShipClass.Science => ShipSizeCategory.L,
            ShipClass.Support => ShipSizeCategory.L,
            
            ShipClass.Battleship => ShipSizeCategory.XL,
            ShipClass.Carrier => ShipSizeCategory.XL,
            ShipClass.Constructor => ShipSizeCategory.XL,
            
            _ => ShipSizeCategory.M
        };
    }
    
    /// <summary>
    /// Check if a module size is compatible with a ship size
    /// </summary>
    public static bool IsModuleSizeCompatible(ModuleSize moduleSize, ShipSizeCategory shipSize)
    {
        // Exact match is always compatible
        if ((int)moduleSize == (int)shipSize)
            return true;
            
        // S modules can be used on any ship (universal small components)
        if (moduleSize == ModuleSize.S)
            return true;
            
        // M modules can be used on M, L, and XL ships
        if (moduleSize == ModuleSize.M && shipSize >= ShipSizeCategory.M)
            return true;
            
        // L modules can be used on L and XL ships
        if (moduleSize == ModuleSize.L && shipSize >= ShipSizeCategory.L)
            return true;
            
        // XL modules only on XL ships
        if (moduleSize == ModuleSize.XL && shipSize == ShipSizeCategory.XL)
            return true;
            
        return false;
    }
    
    /// <summary>
    /// Get display name for ship size
    /// </summary>
    public static string GetShipSizeDisplayName(ShipSizeCategory size)
    {
        return size switch
        {
            ShipSizeCategory.S => "Small (S)",
            ShipSizeCategory.M => "Medium (M)",
            ShipSizeCategory.L => "Large (L)",
            ShipSizeCategory.XL => "Capital (XL)",
            _ => size.ToString()
        };
    }
    
    /// <summary>
    /// Get display name for module size
    /// </summary>
    public static string GetModuleSizeDisplayName(ModuleSize size)
    {
        return size switch
        {
            ModuleSize.S => "Small (S)",
            ModuleSize.M => "Medium (M)",
            ModuleSize.L => "Large (L)",
            ModuleSize.XL => "Capital (XL)",
            _ => size.ToString()
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
