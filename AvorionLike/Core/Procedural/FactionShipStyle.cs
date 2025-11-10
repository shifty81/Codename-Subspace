using System.Numerics;

namespace AvorionLike.Core.Procedural;

/// <summary>
/// Defines visual and structural style characteristics for faction ships
/// </summary>
public class FactionShipStyle
{
    public string FactionName { get; set; } = "";
    
    // Shape characteristics
    public ShipHullShape PreferredHullShape { get; set; } = ShipHullShape.Blocky;
    public float SymmetryLevel { get; set; } = 0.8f; // 0-1, how symmetrical ships are
    public float Sleekness { get; set; } = 0.3f; // 0-1, how streamlined vs blocky
    
    // Color scheme
    public uint PrimaryColor { get; set; } = 0x808080;
    public uint SecondaryColor { get; set; } = 0x404040;
    public uint AccentColor { get; set; } = 0xFFFFFF;
    
    // Structural preferences
    public float ArmorToHullRatio { get; set; } = 0.3f; // How much armor vs hull
    public float ExternalSystemsPreference { get; set; } = 0.2f; // 0-1, exposed vs internal systems
    public float WeaponDensity { get; set; } = 0.5f; // 0-1, how many weapon mounts
    
    // Material preferences
    public string PreferredMaterial { get; set; } = "Iron";
    
    // Design philosophy
    public DesignPhilosophy Philosophy { get; set; } = DesignPhilosophy.Balanced;
    
    /// <summary>
    /// Get default style for a faction based on common faction types
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
                PrimaryColor = 0x2F4F4F, // Dark Slate Gray
                SecondaryColor = 0x708090, // Slate Gray
                AccentColor = 0xFF0000, // Red
                ArmorToHullRatio = 0.5f,
                ExternalSystemsPreference = 0.3f,
                WeaponDensity = 0.8f,
                Philosophy = DesignPhilosophy.CombatFocused
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
                PrimaryColor = 0xDAA520, // Goldenrod
                SecondaryColor = 0xF0E68C, // Khaki
                AccentColor = 0xFFD700, // Gold
                ArmorToHullRatio = 0.2f,
                ExternalSystemsPreference = 0.4f,
                WeaponDensity = 0.2f,
                Philosophy = DesignPhilosophy.UtilityFocused
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
                PrimaryColor = 0x8B0000, // Dark Red
                SecondaryColor = 0x2F4F4F, // Dark Slate Gray
                AccentColor = 0xFF4500, // Orange Red
                ArmorToHullRatio = 0.35f,
                ExternalSystemsPreference = 0.6f,
                WeaponDensity = 0.7f,
                Philosophy = DesignPhilosophy.CombatFocused
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
                PrimaryColor = 0x4169E1, // Royal Blue
                SecondaryColor = 0xADD8E6, // Light Blue
                AccentColor = 0x00CED1, // Dark Turquoise
                ArmorToHullRatio = 0.15f,
                ExternalSystemsPreference = 0.5f,
                WeaponDensity = 0.2f,
                Philosophy = DesignPhilosophy.ScienceFocused
            };
        }
        
        // Miners - industrial, bulky, utility-focused
        if (normalized.Contains("miner") || normalized.Contains("industrial") || 
            normalized.Contains("mining") || normalized.Contains("construct"))
        {
            return new FactionShipStyle
            {
                FactionName = factionName,
                PreferredHullShape = ShipHullShape.Blocky,
                SymmetryLevel = 0.7f,
                Sleekness = 0.2f,
                PrimaryColor = 0xB8860B, // Dark Goldenrod
                SecondaryColor = 0x696969, // Dim Gray
                AccentColor = 0xFFA500, // Orange
                ArmorToHullRatio = 0.3f,
                ExternalSystemsPreference = 0.7f,
                WeaponDensity = 0.3f,
                Philosophy = DesignPhilosophy.UtilityFocused
            };
        }
        
        // Default - balanced approach
        return new FactionShipStyle
        {
            FactionName = factionName,
            PreferredHullShape = ShipHullShape.Blocky,
            SymmetryLevel = 0.75f,
            Sleekness = 0.5f,
            PrimaryColor = 0x808080, // Gray
            SecondaryColor = 0x696969, // Dim Gray
            AccentColor = 0xC0C0C0, // Silver
            ArmorToHullRatio = 0.3f,
            ExternalSystemsPreference = 0.3f,
            WeaponDensity = 0.5f,
            Philosophy = DesignPhilosophy.Balanced
        };
    }
}

/// <summary>
/// Overall ship hull shape types
/// </summary>
public enum ShipHullShape
{
    Blocky,        // Box-like, industrial, brick-shaped
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
