using System.Numerics;

namespace AvorionLike.Core.Procedural;

/// <summary>
/// Defines visual and structural style characteristics for faction ships
/// Following Avorion-style block-based design principles
/// </summary>
public class FactionShipStyle
{
    public string FactionName { get; set; } = "";
    
    // Shape characteristics
    public ShipHullShape PreferredHullShape { get; set; } = ShipHullShape.Blocky;
    public float SymmetryLevel { get; set; } = 0.8f; // 0-1, how symmetrical ships are
    public float Sleekness { get; set; } = 0.3f; // 0-1, how streamlined vs blocky
    
    // Avorion-style block complexity
    public float BlockComplexity { get; set; } = 0.5f; // 0-1, high = more blocks, low = simpler/bulkier
    public float VolumeScaling { get; set; } = 1.0f; // Multiplier for overall ship volume
    public bool UseAngledBlocks { get; set; } = true; // Use angled/wedge blocks for aesthetics
    public bool UseBoxAesthetic { get; set; } = true; // Avorion box-like appearance
    
    // Color scheme
    public uint PrimaryColor { get; set; } = 0x808080;
    public uint SecondaryColor { get; set; } = 0x404040;
    public uint AccentColor { get; set; } = 0xFFFFFF;
    
    // Structural preferences
    public float ArmorToHullRatio { get; set; } = 0.3f; // How much armor vs hull
    public float ExternalSystemsPreference { get; set; } = 0.2f; // 0-1, exposed vs internal systems
    public float WeaponDensity { get; set; } = 0.5f; // 0-1, how many weapon mounts
    
    // Material preferences (affects upgrade slots and efficiency per Avorion rules)
    public string PreferredMaterial { get; set; } = "Iron";
    
    // Design philosophy
    public DesignPhilosophy Philosophy { get; set; } = DesignPhilosophy.Balanced;
    
    // Avorion-specific core component placement
    public bool RequireIntegrityField { get; set; } = true;
    public bool RequirePowerCore { get; set; } = true;
    public float EnginePlacementDepth { get; set; } = 0.8f; // How far back engines are placed (0-1)
    public int TargetUpgradeSlots { get; set; } = 5; // Base upgrade slots (can be increased with computer blocks)
    
    // Modularity settings
    public bool UseModularSections { get; set; } = true; // Break ship into distinct modules
    public int ModularSectionCount { get; set; } = 3; // Number of major sections (front/mid/rear)
    
    /// <summary>
    /// Get default style for a faction based on common faction types
    /// Enhanced with Avorion-style properties
    /// </summary>
    public static FactionShipStyle GetDefaultStyle(string factionName)
    {
        // Normalize faction name for matching
        var normalized = factionName.ToLower();
        
        // Create styles based on common faction archetypes
        // Militaristic - angular, armored, weapon-heavy
        if (normalized.Contains("military") || normalized.Contains("empire") || 
            normalized.Contains("federation") || normalized.Contains("defense"))
        {
            return new FactionShipStyle
            {
                FactionName = factionName,
                PreferredHullShape = ShipHullShape.Angular,
                SymmetryLevel = 0.9f,
                Sleekness = 0.4f,
                BlockComplexity = 0.7f, // High detail
                VolumeScaling = 1.1f, // Slightly larger
                UseAngledBlocks = true,
                UseBoxAesthetic = true,
                PrimaryColor = 0x2F4F4F, // Dark Slate Gray
                SecondaryColor = 0x708090, // Slate Gray
                AccentColor = 0xFF0000, // Red
                ArmorToHullRatio = 0.5f,
                ExternalSystemsPreference = 0.3f,
                WeaponDensity = 0.8f,
                Philosophy = DesignPhilosophy.CombatFocused,
                PreferredMaterial = "Titanium", // Good balance for military
                RequireIntegrityField = true,
                RequirePowerCore = true,
                EnginePlacementDepth = 0.85f,
                TargetUpgradeSlots = 8,
                UseModularSections = true,
                ModularSectionCount = 4
            };
        }
        
        // Traders - practical, cargo-focused, efficient
        if (normalized.Contains("trad") || normalized.Contains("merchant") || 
            normalized.Contains("commerce") || normalized.Contains("cargo"))
        {
            return new FactionShipStyle
            {
                FactionName = factionName,
                PreferredHullShape = ShipHullShape.Cylindrical,
                SymmetryLevel = 0.95f,
                Sleekness = 0.6f,
                BlockComplexity = 0.4f, // Simpler, bulkier design
                VolumeScaling = 1.3f, // Larger for cargo
                UseAngledBlocks = false,
                UseBoxAesthetic = true,
                PrimaryColor = 0xDAA520, // Goldenrod
                SecondaryColor = 0xF0E68C, // Khaki
                AccentColor = 0xFFD700, // Gold
                ArmorToHullRatio = 0.2f,
                ExternalSystemsPreference = 0.4f,
                WeaponDensity = 0.2f,
                Philosophy = DesignPhilosophy.UtilityFocused,
                PreferredMaterial = "Iron", // Cheap and practical
                RequireIntegrityField = true,
                RequirePowerCore = true,
                EnginePlacementDepth = 0.9f,
                TargetUpgradeSlots = 4,
                UseModularSections = true,
                ModularSectionCount = 3
            };
        }
        
        // Pirates - asymmetric, cobbled-together, aggressive
        if (normalized.Contains("pirate") || normalized.Contains("raider") || 
            normalized.Contains("outlaw") || normalized.Contains("bandit"))
        {
            return new FactionShipStyle
            {
                FactionName = factionName,
                PreferredHullShape = ShipHullShape.Irregular,
                SymmetryLevel = 0.4f,
                Sleekness = 0.2f,
                BlockComplexity = 0.6f,
                VolumeScaling = 0.9f,
                UseAngledBlocks = true,
                UseBoxAesthetic = false,
                PrimaryColor = 0x8B0000, // Dark Red
                SecondaryColor = 0x2F4F4F, // Dark Slate Gray
                AccentColor = 0xFF4500, // Orange Red
                ArmorToHullRatio = 0.35f,
                ExternalSystemsPreference = 0.6f,
                WeaponDensity = 0.7f,
                Philosophy = DesignPhilosophy.CombatFocused,
                PreferredMaterial = "Iron", // Cheap salvaged materials
                RequireIntegrityField = false, // Cobbled together
                RequirePowerCore = true,
                EnginePlacementDepth = 0.7f,
                TargetUpgradeSlots = 3,
                UseModularSections = false,
                ModularSectionCount = 2
            };
        }
        
        // Scientists/Explorers - sleek, sensor-heavy, minimal weapons
        if (normalized.Contains("scien") || normalized.Contains("explor") || 
            normalized.Contains("research") || normalized.Contains("academic"))
        {
            return new FactionShipStyle
            {
                FactionName = factionName,
                PreferredHullShape = ShipHullShape.Sleek,
                SymmetryLevel = 0.85f,
                Sleekness = 0.8f,
                BlockComplexity = 0.8f, // High detail for sensors
                VolumeScaling = 0.9f, // More compact
                UseAngledBlocks = true,
                UseBoxAesthetic = false,
                PrimaryColor = 0x4169E1, // Royal Blue
                SecondaryColor = 0xADD8E6, // Light Blue
                AccentColor = 0x00CED1, // Dark Turquoise
                ArmorToHullRatio = 0.15f,
                ExternalSystemsPreference = 0.5f,
                WeaponDensity = 0.2f,
                Philosophy = DesignPhilosophy.ScienceFocused,
                PreferredMaterial = "Trinium", // High efficiency for sensors
                RequireIntegrityField = true,
                RequirePowerCore = true,
                EnginePlacementDepth = 0.75f,
                TargetUpgradeSlots = 10, // Many upgrade slots for equipment
                UseModularSections = true,
                ModularSectionCount = 3
            };
        }
        
        // Miners - industrial, bulky, utility-focused
        // Enhanced for angular, blocky voxel-based mining ship aesthetics
        if (normalized.Contains("miner") || normalized.Contains("mining"))
        {
            return new FactionShipStyle
            {
                FactionName = factionName,
                PreferredHullShape = ShipHullShape.Blocky,
                SymmetryLevel = 0.7f,
                Sleekness = 0.15f, // Very industrial, not sleek
                BlockComplexity = 0.6f, // Higher complexity for exposed frameworks
                VolumeScaling = 1.3f, // Larger for ore storage and equipment
                UseAngledBlocks = true, // Angular panels on blocky hull
                UseBoxAesthetic = true, // Very boxy industrial look
                PrimaryColor = 0x4A4A50, // Industrial dark gray
                SecondaryColor = 0x8B7355, // Industrial tan/rust
                AccentColor = 0xFFA500, // Safety orange
                ArmorToHullRatio = 0.35f, // More armor for protection
                ExternalSystemsPreference = 0.8f, // Exposed gantries and equipment
                WeaponDensity = 0.4f, // Some mining lasers as weapons
                Philosophy = DesignPhilosophy.UtilityFocused,
                PreferredMaterial = "Iron", // Cheap and abundant
                RequireIntegrityField = true,
                RequirePowerCore = true,
                EnginePlacementDepth = 0.9f,
                TargetUpgradeSlots = 6,
                UseModularSections = true,
                ModularSectionCount = 4 // More sections for equipment modules
            };
        }
        
        // Industrial - dark utilitarian ships with hazard stripes (like Republic of Thule: Liberator)
        // Matches the aesthetic from 1234.PNG: dark green/brown hulls, cyan engines, yellow hazard stripes
        if (normalized.Contains("industrial") || normalized.Contains("construct") ||
            normalized.Contains("republic") || normalized.Contains("thule"))
        {
            return new FactionShipStyle
            {
                FactionName = factionName,
                PreferredHullShape = ShipHullShape.Angular,
                SymmetryLevel = 0.85f,
                Sleekness = 0.25f, // Utilitarian and angular
                BlockComplexity = 0.65f, // Detailed with wing structures
                VolumeScaling = 1.1f, // Slightly larger
                UseAngledBlocks = true, // Sharp angular wings
                UseBoxAesthetic = true, // Blocky Avorion aesthetic
                PrimaryColor = 0x3A4A3A, // Dark olive-green (matches 1234.PNG hull)
                SecondaryColor = 0x4A3F30, // Dark brown (matches 1234.PNG)
                AccentColor = 0x00CED1, // Cyan for glowing engines (matches 1234.PNG)
                ArmorToHullRatio = 0.4f, // Heavy armor
                ExternalSystemsPreference = 0.5f, // Visible wing structures
                WeaponDensity = 0.6f, // Combat capable
                Philosophy = DesignPhilosophy.CombatFocused,
                PreferredMaterial = "Titanium", // Strong industrial material
                RequireIntegrityField = true,
                RequirePowerCore = true,
                EnginePlacementDepth = 0.85f,
                TargetUpgradeSlots = 7,
                UseModularSections = true,
                ModularSectionCount = 3
            };
        }
        
        // Default - balanced approach
        return new FactionShipStyle
        {
            FactionName = factionName,
            PreferredHullShape = ShipHullShape.Blocky,
            SymmetryLevel = 0.75f,
            Sleekness = 0.5f,
            BlockComplexity = 0.5f,
            VolumeScaling = 1.0f,
            UseAngledBlocks = true,
            UseBoxAesthetic = true,
            PrimaryColor = 0x808080, // Gray
            SecondaryColor = 0x696969, // Dim Gray
            AccentColor = 0xC0C0C0, // Silver
            ArmorToHullRatio = 0.3f,
            ExternalSystemsPreference = 0.3f,
            WeaponDensity = 0.5f,
            Philosophy = DesignPhilosophy.Balanced,
            PreferredMaterial = "Iron",
            RequireIntegrityField = true,
            RequirePowerCore = true,
            EnginePlacementDepth = 0.8f,
            TargetUpgradeSlots = 5,
            UseModularSections = true,
            ModularSectionCount = 3
        };
    }
}

/// <summary>
/// Overall ship hull shape types
/// </summary>
public enum ShipHullShape
{
    Blocky,        // Box-like, industrial, brick-shaped (Avorion classic)
    Angular,       // Sharp angles, wedge-shaped, military
    Cylindrical,   // Tube-like, symmetric, cargo-focused
    Sleek,         // Streamlined, aerodynamic-looking
    Irregular,     // Asymmetric, cobbled-together
    Organic        // Curved, flowing shapes
}

/// <summary>
/// Design philosophy affects component placement priorities
/// </summary>
public enum DesignPhilosophy
{
    Balanced,         // Equal focus on all systems
    CombatFocused,    // Maximize weapons and defenses
    UtilityFocused,   // Maximize cargo and utility
    SpeedFocused,     // Maximize engines and minimize mass
    ScienceFocused,   // Sensors and special systems
    DefenseFocused    // Heavy armor and shields
}
