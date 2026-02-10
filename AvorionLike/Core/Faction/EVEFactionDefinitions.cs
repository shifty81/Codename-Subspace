using AvorionLike.Core.Procedural;

namespace AvorionLike.Core.Faction;

/// <summary>
/// Faction identity for EVE-inspired factions (0-3)
/// </summary>
public enum EVEFactionId
{
    SanctumHegemony = 0,   // Amarr analog - Religious/Armor
    CoreNexus = 1,         // Caldari analog - Corp/Shield
    VanguardRepublic = 2,  // Gallente analog - Liberal/Hybrid
    RustScrapCoalition = 3 // Minmatar analog - Tribal/Projectile
}

/// <summary>
/// Bloodline options within each faction
/// </summary>
public enum Bloodline
{
    // Sanctum Hegemony bloodlines
    TrueBloods = 0,
    Khanid = 1,
    NiKunni = 2,

    // Core Nexus bloodlines
    Deteis = 3,
    Civire = 4,
    Achura = 5,

    // Vanguard Republic bloodlines
    Gallente = 6,
    Intaki = 7,
    JinMei = 8,

    // Rust-Scrap Coalition bloodlines
    Brutor = 9,
    Sebiestor = 10,
    Vherokior = 11
}

/// <summary>
/// Education path that determines initial skill points
/// </summary>
public enum Education
{
    Engineering,
    Gunnery,
    Navigation,
    Drones,
    Electronics,
    MissileOperations
}

/// <summary>
/// Defines the four EVE-inspired factions with their lore, visual styles, and gameplay focuses.
/// Built on the "four pillars" of EVE style:
///   Religious/Armor, Corp/Shield, Liberal/Hybrid, Tribal/Projectile.
/// </summary>
public class EVEFactionDefinitions
{
    private static readonly Dictionary<EVEFactionId, EVEFactionProfile> _profiles = new();

    static EVEFactionDefinitions()
    {
        InitializeProfiles();
    }

    /// <summary>
    /// Get the faction profile for a given faction ID
    /// </summary>
    public static EVEFactionProfile GetProfile(EVEFactionId factionId)
    {
        return _profiles[factionId];
    }

    /// <summary>
    /// Get all faction profiles
    /// </summary>
    public static IReadOnlyDictionary<EVEFactionId, EVEFactionProfile> GetAllProfiles()
    {
        return _profiles;
    }

    /// <summary>
    /// Get the bloodlines available for a given faction
    /// </summary>
    public static Bloodline[] GetBloodlines(EVEFactionId factionId)
    {
        return factionId switch
        {
            EVEFactionId.SanctumHegemony => new[] { Bloodline.TrueBloods, Bloodline.Khanid, Bloodline.NiKunni },
            EVEFactionId.CoreNexus => new[] { Bloodline.Deteis, Bloodline.Civire, Bloodline.Achura },
            EVEFactionId.VanguardRepublic => new[] { Bloodline.Gallente, Bloodline.Intaki, Bloodline.JinMei },
            EVEFactionId.RustScrapCoalition => new[] { Bloodline.Brutor, Bloodline.Sebiestor, Bloodline.Vherokior },
            _ => Array.Empty<Bloodline>()
        };
    }

    /// <summary>
    /// Get the faction ship style for procedural ship generation
    /// </summary>
    public static FactionShipStyle GetShipStyle(EVEFactionId factionId)
    {
        return _profiles[factionId].ShipStyle;
    }

    private static void InitializeProfiles()
    {
        // ============================================================
        // The Sanctum Hegemony (Amarr Analog)
        // ============================================================
        _profiles[EVEFactionId.SanctumHegemony] = new EVEFactionProfile
        {
            FactionId = EVEFactionId.SanctumHegemony,
            Name = "The Sanctum Hegemony",
            Description = "A feudal, expansionist theocracy that believes it is destined to rule the stars. " +
                          "They utilize vast, golden, cathedral-like ships.",
            Lore = "For millennia, the Sanctum Hegemony has spread its faith across the stars, " +
                   "subjugating entire civilizations under its divine mandate. Their rulers claim " +
                   "direct lineage from the first colonists, granting them absolute authority. " +
                   "The Hegemony's ships are monuments to their faith—golden, symmetrical, and imposing.",
            GovernmentStyle = GovernmentType.Theocracy,
            PrimaryEthic = FactionEthics.Spiritualist,
            SecondaryEthic = FactionEthics.Authoritarian,
            VisualStyle = "Gold plating, symmetrical, shiny, slow, heavy armor",
            PVEFocus = "Energy turret and drone users. Slow to kill, but rarely need to dock for repairs due to high armor HP.",
            WeaponPreference = WeaponPreference.EnergyTurrets,
            DefensePreference = DefensePreference.HeavyArmor,
            StarterShipArchetype = "frigate",
            StarterStation = "Sanctum Prime Citadel",
            ShipStyle = new FactionShipStyle
            {
                FactionName = "Sanctum Hegemony",
                PreferredHullShape = ShipHullShape.Angular,
                SymmetryLevel = 0.95f,
                Sleekness = 0.6f,
                BlockComplexity = 0.8f,
                VolumeScaling = 1.2f,
                UseAngledBlocks = true,
                UseBoxAesthetic = false,
                PrimaryColor = 0xDAA520,   // Golden
                SecondaryColor = 0xFFD700, // Gold
                AccentColor = 0xFFF8DC,    // Cornsilk (light gold glow)
                ArmorToHullRatio = 0.6f,
                ExternalSystemsPreference = 0.2f,
                WeaponDensity = 0.6f,
                Philosophy = DesignPhilosophy.DefenseFocused,
                PreferredMaterial = "Xanion",
                RequireIntegrityField = true,
                RequirePowerCore = true,
                EnginePlacementDepth = 0.9f,
                TargetUpgradeSlots = 7,
                UseModularSections = true,
                ModularSectionCount = 4
            }
        };

        // ============================================================
        // The Core Nexus (Caldari Analog)
        // ============================================================
        _profiles[EVEFactionId.CoreNexus] = new EVEFactionProfile
        {
            FactionId = EVEFactionId.CoreNexus,
            Name = "The Core Nexus",
            Description = "A collection of mega-corporations operating as a state. " +
                          "Efficiency, profit, and 1984-style order are paramount.",
            Lore = "The Core Nexus is not a nation but a network of corporate entities, each " +
                   "competing for market dominance. Citizens are employees, borders are trade zones, " +
                   "and warfare is just another line item on the budget. Their ships are functional, " +
                   "angular, and built for maximum efficiency—no wasted space, no wasted credits.",
            GovernmentStyle = GovernmentType.Corporate,
            PrimaryEthic = FactionEthics.Materialist,
            SecondaryEthic = FactionEthics.Industrialist,
            VisualStyle = "Angular, functional, dark/grey, bulky, industrial looking",
            PVEFocus = "Shield tanking missile boats. Long-range, high damage, but vulnerable if the shield breaks.",
            WeaponPreference = WeaponPreference.Missiles,
            DefensePreference = DefensePreference.ShieldTank,
            StarterShipArchetype = "corvette",
            StarterStation = "Nexus Corporate Hub",
            ShipStyle = new FactionShipStyle
            {
                FactionName = "Core Nexus",
                PreferredHullShape = ShipHullShape.Angular,
                SymmetryLevel = 0.85f,
                Sleekness = 0.3f,
                BlockComplexity = 0.7f,
                VolumeScaling = 1.1f,
                UseAngledBlocks = true,
                UseBoxAesthetic = true,
                PrimaryColor = 0x2F2F2F,   // Dark grey
                SecondaryColor = 0x708090,  // Slate grey
                AccentColor = 0x4682B4,     // Steel blue
                ArmorToHullRatio = 0.2f,
                ExternalSystemsPreference = 0.3f,
                WeaponDensity = 0.7f,
                Philosophy = DesignPhilosophy.CombatFocused,
                PreferredMaterial = "Titanium",
                RequireIntegrityField = true,
                RequirePowerCore = true,
                EnginePlacementDepth = 0.85f,
                TargetUpgradeSlots = 8,
                UseModularSections = true,
                ModularSectionCount = 3
            }
        };

        // ============================================================
        // The Vanguard Republic (Gallente Analog)
        // ============================================================
        _profiles[EVEFactionId.VanguardRepublic] = new EVEFactionProfile
        {
            FactionId = EVEFactionId.VanguardRepublic,
            Name = "The Vanguard Republic",
            Description = "A hedonistic, democratic society championing freedom and individuality. " +
                          "They are technologically advanced but politically chaotic.",
            Lore = "Born from revolution, the Vanguard Republic is a beacon of personal freedom " +
                   "in a galaxy of authoritarians and profiteers. Their citizens enjoy unprecedented " +
                   "liberties, but their government is a labyrinth of competing interests. Their ships " +
                   "reflect their culture: organic, beautiful, and deceptively deadly.",
            GovernmentStyle = GovernmentType.Democracy,
            PrimaryEthic = FactionEthics.Egalitarian,
            SecondaryEthic = FactionEthics.Xenophile,
            VisualStyle = "Organic, curved, green/blue, asymmetrical shapes",
            PVEFocus = "Drone boats and hybrid weapons. Heavy drone usage allows for versatile engagements.",
            WeaponPreference = WeaponPreference.DronesAndHybrids,
            DefensePreference = DefensePreference.ArmorShieldHybrid,
            StarterShipArchetype = "corvette",
            StarterStation = "Liberty Freeport",
            ShipStyle = new FactionShipStyle
            {
                FactionName = "Vanguard Republic",
                PreferredHullShape = ShipHullShape.Organic,
                SymmetryLevel = 0.6f,
                Sleekness = 0.8f,
                BlockComplexity = 0.75f,
                VolumeScaling = 1.0f,
                UseAngledBlocks = false,
                UseBoxAesthetic = false,
                PrimaryColor = 0x2E8B57,   // Sea green
                SecondaryColor = 0x4682B4,  // Steel blue
                AccentColor = 0x00CED1,     // Dark turquoise
                ArmorToHullRatio = 0.35f,
                ExternalSystemsPreference = 0.4f,
                WeaponDensity = 0.5f,
                Philosophy = DesignPhilosophy.Balanced,
                PreferredMaterial = "Trinium",
                RequireIntegrityField = true,
                RequirePowerCore = true,
                EnginePlacementDepth = 0.8f,
                TargetUpgradeSlots = 6,
                UseModularSections = true,
                ModularSectionCount = 3
            }
        };

        // ============================================================
        // The Rust-Scrap Coalition (Minmatar Analog)
        // ============================================================
        _profiles[EVEFactionId.RustScrapCoalition] = new EVEFactionProfile
        {
            FactionId = EVEFactionId.RustScrapCoalition,
            Name = "The Rust-Scrap Coalition",
            Description = "A tribal society that recently broke free from the Sanctum Hegemony. " +
                          "They are masters of improvisation.",
            Lore = "The Rust-Scrap Coalition is held together by shared suffering and stubborn defiance. " +
                   "Once enslaved by the Sanctum Hegemony, these tribes now forge their own destiny with " +
                   "whatever they can salvage, steal, or jury-rig. Their motto: 'In Rust We Trust'. " +
                   "Their ships are fast, modular, and look like they might fall apart—but never do.",
            GovernmentStyle = GovernmentType.Democracy,
            PrimaryEthic = FactionEthics.Militarist,
            SecondaryEthic = FactionEthics.Traditionalist,
            VisualStyle = "Red/Black, exposed metal, asymmetrical, 'rusty', fast, modular",
            PVEFocus = "Projectile weaponry and shield/armor hybrids. Fast-paced, high speed, 'In Rust We Trust'.",
            WeaponPreference = WeaponPreference.ProjectileWeapons,
            DefensePreference = DefensePreference.ShieldArmorHybrid,
            StarterShipArchetype = "fighter",
            StarterStation = "Rustyard Assembly",
            ShipStyle = new FactionShipStyle
            {
                FactionName = "Rust-Scrap Coalition",
                PreferredHullShape = ShipHullShape.Irregular,
                SymmetryLevel = 0.4f,
                Sleekness = 0.2f,
                BlockComplexity = 0.6f,
                VolumeScaling = 0.9f,
                UseAngledBlocks = true,
                UseBoxAesthetic = false,
                PrimaryColor = 0x8B0000,   // Dark red
                SecondaryColor = 0x1C1C1C, // Very dark grey (black)
                AccentColor = 0xB87333,    // Copper/rust
                ArmorToHullRatio = 0.3f,
                ExternalSystemsPreference = 0.7f,
                WeaponDensity = 0.65f,
                Philosophy = DesignPhilosophy.SpeedFocused,
                PreferredMaterial = "Iron",
                RequireIntegrityField = false,
                RequirePowerCore = true,
                EnginePlacementDepth = 0.7f,
                TargetUpgradeSlots = 5,
                UseModularSections = true,
                ModularSectionCount = 3
            }
        };
    }
}

/// <summary>
/// Complete profile for an EVE-inspired faction including lore, style, and gameplay data
/// </summary>
public class EVEFactionProfile
{
    public EVEFactionId FactionId { get; set; }
    public string Name { get; set; } = "";
    public string Description { get; set; } = "";
    public string Lore { get; set; } = "";
    public GovernmentType GovernmentStyle { get; set; }
    public FactionEthics PrimaryEthic { get; set; }
    public FactionEthics SecondaryEthic { get; set; }
    public string VisualStyle { get; set; } = "";
    public string PVEFocus { get; set; } = "";
    public WeaponPreference WeaponPreference { get; set; }
    public DefensePreference DefensePreference { get; set; }
    public string StarterShipArchetype { get; set; } = "fighter";
    public string StarterStation { get; set; } = "";
    public FactionShipStyle ShipStyle { get; set; } = new();

    /// <summary>
    /// Get a summary of this faction's gameplay identity
    /// </summary>
    public string GetSummary()
    {
        return $"{Name}\n" +
               $"  Government: {GovernmentStyle}\n" +
               $"  Ethics: {PrimaryEthic}/{SecondaryEthic}\n" +
               $"  Style: {VisualStyle}\n" +
               $"  PVE: {PVEFocus}\n" +
               $"  Weapons: {WeaponPreference}\n" +
               $"  Defense: {DefensePreference}\n" +
               $"  Starter Ship: {StarterShipArchetype}\n" +
               $"  Starter Station: {StarterStation}";
    }
}

/// <summary>
/// Weapon preference for a faction
/// </summary>
public enum WeaponPreference
{
    EnergyTurrets,
    Missiles,
    DronesAndHybrids,
    ProjectileWeapons
}

/// <summary>
/// Defense preference for a faction
/// </summary>
public enum DefensePreference
{
    HeavyArmor,
    ShieldTank,
    ArmorShieldHybrid,
    ShieldArmorHybrid
}
