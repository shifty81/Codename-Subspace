namespace AvorionLike.Core.Modular;

/// <summary>
/// X4-inspired ship classes and their characteristics
/// Based on X4 Foundations ship classification system
/// Names are changed to avoid copyright issues but maintain visual styling
/// </summary>
public enum X4ShipClass
{
    // Small (S) class - Fast, agile, low cargo
    Fighter_Light,      // Like X4 Scouts/Interceptors
    Fighter_Heavy,      // Like X4 Heavy Fighters
    Miner_Small,        // Like X4 S Miners
    
    // Medium (M) class - Balanced versatility
    Corvette,           // Like X4 Corvettes
    Frigate,            // Like X4 Frigates
    Gunboat,            // Like X4 Gunboats
    Miner_Medium,       // Like X4 M Miners (Drill Vanguard style)
    Freighter_Medium,   // Like X4 M Transporters
    
    // Large (L) class - Heavy combat/cargo
    Destroyer,          // Like X4 Destroyers
    Freighter_Large,    // Like X4 L Freighters
    Miner_Large,        // Like X4 L Miners (Roqual style)
    
    // Extra Large (XL) class - Capital ships
    Battleship,         // Like X4 Battleships
    Carrier,            // Like X4 Carriers
    Builder             // Like X4 Builder ships
}

/// <summary>
/// X4-inspired racial/faction design philosophies
/// Visual styles without using actual X4 race names
/// </summary>
public enum X4DesignStyle
{
    Balanced,       // Balanced stats (like Argon) - angular, industrial
    Aggressive,     // Speed focused (like Split) - sharp, aggressive lines
    Durable,        // Tank/cargo focused (like Teladi) - bulky, reinforced
    Sleek,          // Fast and elegant (like Paranid) - smooth curves
    Advanced,       // High-tech (like Terran) - advanced plating, clean lines
    Alien           // Unique aesthetics (like Xenon/Khaak) - unconventional
}

/// <summary>
/// Ship variant types like X4's Sentinel/Vanguard system
/// </summary>
public enum X4ShipVariant
{
    Standard,   // Balanced variant
    Sentinel,   // More hull/cargo, slower
    Vanguard,   // Faster, lighter defense/cargo
    Military    // Combat-oriented variant
}

/// <summary>
/// Configuration for generating X4-style ships
/// </summary>
public class X4ShipConfig
{
    public X4ShipClass ShipClass { get; set; } = X4ShipClass.Corvette;
    public X4DesignStyle DesignStyle { get; set; } = X4DesignStyle.Balanced;
    public X4ShipVariant Variant { get; set; } = X4ShipVariant.Standard;
    public string ShipName { get; set; } = "Unnamed Ship";
    public string Material { get; set; } = "Iron";
    public int Seed { get; set; } = 0;
    
    // Equipment slots based on ship class
    public int PrimaryWeaponSlots { get; set; } = 2;
    public int TurretSlots { get; set; } = 0;
    public int UtilitySlots { get; set; } = 2; // For mining lasers, salvage beams, etc.
    
    // Color customization (RGB 0-255)
    public (int R, int G, int B) PrimaryColor { get; set; } = (128, 128, 128);
    public (int R, int G, int B) SecondaryColor { get; set; } = (64, 64, 64);
    public (int R, int G, int B) AccentColor { get; set; } = (255, 128, 0);
}
