using System.Numerics;
using System.Text.Json;
using AvorionLike.Core.ECS;
using AvorionLike.Core.Persistence;

namespace AvorionLike.Core.RPG;

/// <summary>
/// Types of subsystems that can be equipped to ships and pods
/// </summary>
public enum SubsystemType
{
    // Engine Subsystems
    ThrustAmplifier,      // Increases thrust power
    ManeuveringThrusters, // Improves maneuverability/torque
    
    // Shield Subsystems
    ShieldBooster,        // Increases shield capacity
    ShieldRegenerator,    // Improves shield regeneration rate
    
    // Weapon Subsystems
    WeaponAmplifier,      // Increases weapon damage
    TargetingComputer,    // Improves accuracy/critical chance
    CoolingSystem,        // Improves fire rate
    
    // Power Subsystems
    PowerAmplifier,       // Increases power generation
    PowerEfficiency,      // Reduces power consumption
    Capacitor,            // Increases power storage
    
    // Defense Subsystems
    ArmorPlating,         // Reduces incoming damage
    StructuralReinforcement, // Increases hull durability
    
    // Utility Subsystems
    CargoExpansion,       // Increases cargo capacity
    ScannerArray,         // Improves scanner range
    JumpDriveEnhancer,    // Reduces jump cooldown
    
    // Special Subsystems (rare/legendary)
    ExperienceAccelerator, // Increases XP gain (pod only)
    EfficiencyCore,       // Reduces efficiency penalty (pod only)
    OmniCore              // Small bonus to all stats (rare drop)
}

/// <summary>
/// Rarity of subsystems affecting their stats and drop rate
/// </summary>
public enum SubsystemRarity
{
    Common = 1,     // 60% drop rate, 5-10% stat bonus
    Uncommon = 2,   // 25% drop rate, 10-15% stat bonus
    Rare = 3,       // 10% drop rate, 15-25% stat bonus
    Epic = 4,       // 4% drop rate, 25-35% stat bonus
    Legendary = 5   // 1% drop rate, 35-50% stat bonus
}

/// <summary>
/// Quality level of a subsystem affecting upgrade potential
/// </summary>
public enum SubsystemQuality
{
    Standard = 0,   // No upgrade levels
    Enhanced = 1,   // Can upgrade 1 level
    Superior = 2,   // Can upgrade 2 levels
    Masterwork = 3  // Can upgrade 3 levels
}

/// <summary>
/// Represents a subsystem upgrade that can be equipped to ships or pods
/// </summary>
public class SubsystemUpgrade
{
    public Guid Id { get; set; }
    public string Name { get; set; }
    public string Description { get; set; }
    public SubsystemType Type { get; set; }
    public SubsystemRarity Rarity { get; set; }
    public SubsystemQuality Quality { get; set; }
    
    // Base stat bonus (percentage, e.g., 0.10f = 10% increase)
    public float BaseBonus { get; set; }
    
    // Current upgrade level (0 = not upgraded, max = Quality value)
    public int UpgradeLevel { get; set; }
    
    // Bonus per upgrade level (default 5% of base)
    public float BonusPerUpgrade { get; set; }
    
    // Requirements
    public int MinimumTechLevel { get; set; } // Material tech level requirement
    public int MinimumPlayerLevel { get; set; } // Player level requirement for equipping
    
    // Research and crafting
    public bool IsResearchable { get; set; }
    public bool IsCraftable { get; set; }
    public Dictionary<string, int> CraftingCost { get; set; } // Material costs
    
    public SubsystemUpgrade(
        SubsystemType type, 
        SubsystemRarity rarity = SubsystemRarity.Common,
        SubsystemQuality quality = SubsystemQuality.Standard)
    {
        Id = Guid.NewGuid();
        Type = type;
        Rarity = rarity;
        Quality = quality;
        UpgradeLevel = 0;
        
        // Generate name and description based on type and rarity
        Name = GenerateName(type, rarity);
        Description = GenerateDescription(type);
        
        // Calculate base bonus based on rarity
        BaseBonus = CalculateBaseBonus(rarity);
        BonusPerUpgrade = BaseBonus * 0.5f; // 50% of base per upgrade
        
        // Set requirements
        MinimumTechLevel = (int)rarity; // Rarer items need better tech
        MinimumPlayerLevel = (int)rarity * 5; // Level 5, 10, 15, 20, 25
        
        // Initially not researchable or craftable (only drops)
        IsResearchable = false;
        IsCraftable = false;
        CraftingCost = new Dictionary<string, int>();
    }
    
    /// <summary>
    /// Get the total bonus including upgrades
    /// </summary>
    public float GetTotalBonus()
    {
        return BaseBonus + (BonusPerUpgrade * UpgradeLevel);
    }
    
    /// <summary>
    /// Check if this subsystem can be upgraded
    /// </summary>
    public bool CanUpgrade()
    {
        return UpgradeLevel < (int)Quality;
    }
    
    /// <summary>
    /// Upgrade the subsystem by one level
    /// </summary>
    public bool Upgrade()
    {
        if (!CanUpgrade()) return false;
        
        UpgradeLevel++;
        return true;
    }
    
    /// <summary>
    /// Get upgrade cost in materials
    /// </summary>
    public Dictionary<string, int> GetUpgradeCost()
    {
        var cost = new Dictionary<string, int>();
        
        // Cost increases with rarity and current upgrade level
        int baseCost = 100 * (int)Rarity;
        int levelMultiplier = UpgradeLevel + 1;
        
        // Require materials of appropriate tech level
        var materials = new[] { "Iron", "Titanium", "Naonite", "Trinium", "Xanion", "Ogonite", "Avorion" };
        string requiredMaterial = materials[Math.Min(MinimumTechLevel - 1, materials.Length - 1)];
        
        cost[requiredMaterial] = baseCost * levelMultiplier;
        
        // Higher rarities require additional materials
        if (Rarity >= SubsystemRarity.Rare)
        {
            cost["Credits"] = baseCost * levelMultiplier * 2;
        }
        
        return cost;
    }
    
    /// <summary>
    /// Generate a name based on type and rarity
    /// </summary>
    private static string GenerateName(SubsystemType type, SubsystemRarity rarity)
    {
        string rarityPrefix = rarity switch
        {
            SubsystemRarity.Common => "",
            SubsystemRarity.Uncommon => "Enhanced ",
            SubsystemRarity.Rare => "Advanced ",
            SubsystemRarity.Epic => "Superior ",
            SubsystemRarity.Legendary => "Legendary ",
            _ => ""
        };
        
        string typeName = type switch
        {
            SubsystemType.ThrustAmplifier => "Thrust Amplifier",
            SubsystemType.ManeuveringThrusters => "Maneuvering Thrusters",
            SubsystemType.ShieldBooster => "Shield Booster",
            SubsystemType.ShieldRegenerator => "Shield Regenerator",
            SubsystemType.WeaponAmplifier => "Weapon Amplifier",
            SubsystemType.TargetingComputer => "Targeting Computer",
            SubsystemType.CoolingSystem => "Cooling System",
            SubsystemType.PowerAmplifier => "Power Amplifier",
            SubsystemType.PowerEfficiency => "Power Efficiency Module",
            SubsystemType.Capacitor => "Power Capacitor",
            SubsystemType.ArmorPlating => "Armor Plating",
            SubsystemType.StructuralReinforcement => "Structural Reinforcement",
            SubsystemType.CargoExpansion => "Cargo Expansion",
            SubsystemType.ScannerArray => "Scanner Array",
            SubsystemType.JumpDriveEnhancer => "Jump Drive Enhancer",
            SubsystemType.ExperienceAccelerator => "Experience Accelerator",
            SubsystemType.EfficiencyCore => "Efficiency Core",
            SubsystemType.OmniCore => "Omni Core",
            _ => "Unknown Subsystem"
        };
        
        return rarityPrefix + typeName;
    }
    
    /// <summary>
    /// Generate description based on type
    /// </summary>
    private static string GenerateDescription(SubsystemType type)
    {
        return type switch
        {
            SubsystemType.ThrustAmplifier => "Increases ship thrust power for faster acceleration and higher top speeds.",
            SubsystemType.ManeuveringThrusters => "Improves ship maneuverability and rotation speed.",
            SubsystemType.ShieldBooster => "Increases maximum shield capacity.",
            SubsystemType.ShieldRegenerator => "Improves shield regeneration rate.",
            SubsystemType.WeaponAmplifier => "Increases weapon damage output.",
            SubsystemType.TargetingComputer => "Improves weapon accuracy and critical hit chance.",
            SubsystemType.CoolingSystem => "Reduces weapon heat buildup, increasing fire rate.",
            SubsystemType.PowerAmplifier => "Increases power generation capacity.",
            SubsystemType.PowerEfficiency => "Reduces power consumption of all systems.",
            SubsystemType.Capacitor => "Increases energy storage capacity.",
            SubsystemType.ArmorPlating => "Reduces incoming damage from all sources.",
            SubsystemType.StructuralReinforcement => "Increases hull durability and structural integrity.",
            SubsystemType.CargoExpansion => "Increases cargo hold capacity.",
            SubsystemType.ScannerArray => "Extends scanner range for detecting objects and resources.",
            SubsystemType.JumpDriveEnhancer => "Reduces hyperspace jump cooldown time.",
            SubsystemType.ExperienceAccelerator => "Increases experience gain from all activities.",
            SubsystemType.EfficiencyCore => "Reduces the pod's efficiency penalty.",
            SubsystemType.OmniCore => "Provides small bonuses to all ship systems.",
            _ => "Unknown subsystem effect."
        };
    }
    
    /// <summary>
    /// Calculate base bonus based on rarity
    /// </summary>
    private static float CalculateBaseBonus(SubsystemRarity rarity)
    {
        return rarity switch
        {
            SubsystemRarity.Common => 0.05f + (Random.Shared.NextSingle() * 0.05f),      // 5-10%
            SubsystemRarity.Uncommon => 0.10f + (Random.Shared.NextSingle() * 0.05f),    // 10-15%
            SubsystemRarity.Rare => 0.15f + (Random.Shared.NextSingle() * 0.10f),        // 15-25%
            SubsystemRarity.Epic => 0.25f + (Random.Shared.NextSingle() * 0.10f),        // 25-35%
            SubsystemRarity.Legendary => 0.35f + (Random.Shared.NextSingle() * 0.15f),   // 35-50%
            _ => 0.05f
        };
    }
    
    /// <summary>
    /// Serialize the subsystem to a dictionary
    /// </summary>
    public Dictionary<string, object> Serialize()
    {
        return new Dictionary<string, object>
        {
            ["Id"] = Id.ToString(),
            ["Name"] = Name,
            ["Description"] = Description,
            ["Type"] = Type.ToString(),
            ["Rarity"] = Rarity.ToString(),
            ["Quality"] = Quality.ToString(),
            ["BaseBonus"] = BaseBonus,
            ["UpgradeLevel"] = UpgradeLevel,
            ["BonusPerUpgrade"] = BonusPerUpgrade,
            ["MinimumTechLevel"] = MinimumTechLevel,
            ["MinimumPlayerLevel"] = MinimumPlayerLevel,
            ["IsResearchable"] = IsResearchable,
            ["IsCraftable"] = IsCraftable,
            ["CraftingCost"] = CraftingCost
        };
    }
    
    /// <summary>
    /// Deserialize a subsystem from a dictionary
    /// </summary>
    public static SubsystemUpgrade Deserialize(Dictionary<string, object> data)
    {
        var type = Enum.Parse<SubsystemType>(data["Type"].ToString()!);
        var rarity = Enum.Parse<SubsystemRarity>(data["Rarity"].ToString()!);
        var quality = Enum.Parse<SubsystemQuality>(data["Quality"].ToString()!);
        
        var subsystem = new SubsystemUpgrade(type, rarity, quality)
        {
            Id = Guid.Parse(data["Id"].ToString()!),
            Name = data["Name"].ToString()!,
            Description = data["Description"].ToString()!,
            BaseBonus = Convert.ToSingle(data["BaseBonus"]),
            UpgradeLevel = Convert.ToInt32(data["UpgradeLevel"]),
            BonusPerUpgrade = Convert.ToSingle(data["BonusPerUpgrade"]),
            MinimumTechLevel = Convert.ToInt32(data["MinimumTechLevel"]),
            MinimumPlayerLevel = Convert.ToInt32(data["MinimumPlayerLevel"]),
            IsResearchable = Convert.ToBoolean(data["IsResearchable"]),
            IsCraftable = Convert.ToBoolean(data["IsCraftable"])
        };
        
        // Handle crafting cost
        if (data.ContainsKey("CraftingCost"))
        {
            var costData = data["CraftingCost"];
            if (costData is JsonElement jsonElement)
            {
                subsystem.CraftingCost = JsonSerializer.Deserialize<Dictionary<string, int>>(jsonElement.GetRawText()) 
                    ?? new Dictionary<string, int>();
            }
            else if (costData is Dictionary<string, int> costDict)
            {
                subsystem.CraftingCost = new Dictionary<string, int>(costDict);
            }
        }
        
        return subsystem;
    }
}

/// <summary>
/// Component for managing subsystems on ships
/// </summary>
public class ShipSubsystemComponent : IComponent, ISerializable
{
    public Guid EntityId { get; set; }
    
    // Subsystem slots
    public int MaxSubsystemSlots { get; set; } = 8;
    public List<SubsystemUpgrade?> EquippedSubsystems { get; set; } = new();
    
    // Subsystem bonuses cache
    private Dictionary<SubsystemType, float> _bonusCache = new();
    private bool _cacheValid = false;
    
    public ShipSubsystemComponent()
    {
        // Initialize slots
        for (int i = 0; i < MaxSubsystemSlots; i++)
        {
            EquippedSubsystems.Add(null);
        }
    }
    
    /// <summary>
    /// Equip a subsystem to a specific slot
    /// </summary>
    public bool EquipSubsystem(SubsystemUpgrade subsystem, int slot)
    {
        if (slot < 0 || slot >= MaxSubsystemSlots)
            return false;
        
        EquippedSubsystems[slot] = subsystem;
        _cacheValid = false;
        return true;
    }
    
    /// <summary>
    /// Unequip a subsystem from a specific slot
    /// </summary>
    public SubsystemUpgrade? UnequipSubsystem(int slot)
    {
        if (slot < 0 || slot >= MaxSubsystemSlots)
            return null;
        
        var subsystem = EquippedSubsystems[slot];
        EquippedSubsystems[slot] = null;
        _cacheValid = false;
        return subsystem;
    }
    
    /// <summary>
    /// Get total bonus for a specific subsystem type
    /// </summary>
    public float GetSubsystemBonus(SubsystemType type)
    {
        if (!_cacheValid)
            RebuildBonusCache();
        
        return _bonusCache.GetValueOrDefault(type, 0f);
    }
    
    /// <summary>
    /// Get total bonus for all OmniCore subsystems
    /// </summary>
    public float GetOmniCoreBonus()
    {
        return GetSubsystemBonus(SubsystemType.OmniCore);
    }
    
    /// <summary>
    /// Rebuild the bonus cache from equipped subsystems
    /// </summary>
    private void RebuildBonusCache()
    {
        _bonusCache.Clear();
        
        foreach (var subsystem in EquippedSubsystems)
        {
            if (subsystem == null) continue;
            
            var type = subsystem.Type;
            var bonus = subsystem.GetTotalBonus();
            
            if (_bonusCache.ContainsKey(type))
                _bonusCache[type] += bonus;
            else
                _bonusCache[type] = bonus;
        }
        
        _cacheValid = true;
    }
    
    /// <summary>
    /// Get all equipped subsystems
    /// </summary>
    public List<SubsystemUpgrade> GetEquippedSubsystems()
    {
        return EquippedSubsystems.Where(s => s != null).Cast<SubsystemUpgrade>().ToList();
    }
    
    /// <summary>
    /// Get number of free slots
    /// </summary>
    public int GetFreeSlots()
    {
        return EquippedSubsystems.Count(s => s == null);
    }
    
    /// <summary>
    /// Serialize the component
    /// </summary>
    public Dictionary<string, object> Serialize()
    {
        var subsystemsData = new List<Dictionary<string, object>?>();
        foreach (var subsystem in EquippedSubsystems)
        {
            subsystemsData.Add(subsystem?.Serialize());
        }
        
        return new Dictionary<string, object>
        {
            ["EntityId"] = EntityId.ToString(),
            ["MaxSubsystemSlots"] = MaxSubsystemSlots,
            ["EquippedSubsystems"] = subsystemsData
        };
    }
    
    /// <summary>
    /// Deserialize the component
    /// </summary>
    public void Deserialize(Dictionary<string, object> data)
    {
        EntityId = Guid.Parse(data["EntityId"].ToString()!);
        MaxSubsystemSlots = Convert.ToInt32(data["MaxSubsystemSlots"]);
        
        EquippedSubsystems.Clear();
        
        if (data.ContainsKey("EquippedSubsystems"))
        {
            var subsystemsData = data["EquippedSubsystems"];
            
            if (subsystemsData is JsonElement jsonElement && jsonElement.ValueKind == JsonValueKind.Array)
            {
                foreach (var subsystemElement in jsonElement.EnumerateArray())
                {
                    if (subsystemElement.ValueKind == JsonValueKind.Null)
                    {
                        EquippedSubsystems.Add(null);
                    }
                    else
                    {
                        var subsystemDict = JsonSerializer.Deserialize<Dictionary<string, object>>(subsystemElement.GetRawText());
                        if (subsystemDict != null)
                        {
                            EquippedSubsystems.Add(SubsystemUpgrade.Deserialize(subsystemDict));
                        }
                        else
                        {
                            EquippedSubsystems.Add(null);
                        }
                    }
                }
            }
            else if (subsystemsData is List<object> subsystemsList)
            {
                foreach (var subsystemObj in subsystemsList)
                {
                    if (subsystemObj == null)
                    {
                        EquippedSubsystems.Add(null);
                    }
                    else if (subsystemObj is Dictionary<string, object> subsystemDict)
                    {
                        EquippedSubsystems.Add(SubsystemUpgrade.Deserialize(subsystemDict));
                    }
                    else
                    {
                        EquippedSubsystems.Add(null);
                    }
                }
            }
        }
        
        // Ensure we have the right number of slots
        while (EquippedSubsystems.Count < MaxSubsystemSlots)
        {
            EquippedSubsystems.Add(null);
        }
        
        _cacheValid = false;
    }
}

/// <summary>
/// Component for managing subsystems on pods (extends pod upgrades)
/// </summary>
public class PodSubsystemComponent : IComponent, ISerializable
{
    public Guid EntityId { get; set; }
    
    // Pod has fewer subsystem slots than ships
    public int MaxSubsystemSlots { get; set; } = 4;
    public List<SubsystemUpgrade?> EquippedSubsystems { get; set; } = new();
    
    // Subsystem bonuses cache
    private Dictionary<SubsystemType, float> _bonusCache = new();
    private bool _cacheValid = false;
    
    public PodSubsystemComponent()
    {
        // Initialize slots
        for (int i = 0; i < MaxSubsystemSlots; i++)
        {
            EquippedSubsystems.Add(null);
        }
    }
    
    /// <summary>
    /// Equip a subsystem to a specific slot
    /// </summary>
    public bool EquipSubsystem(SubsystemUpgrade subsystem, int slot)
    {
        if (slot < 0 || slot >= MaxSubsystemSlots)
            return false;
        
        EquippedSubsystems[slot] = subsystem;
        _cacheValid = false;
        return true;
    }
    
    /// <summary>
    /// Unequip a subsystem from a specific slot
    /// </summary>
    public SubsystemUpgrade? UnequipSubsystem(int slot)
    {
        if (slot < 0 || slot >= MaxSubsystemSlots)
            return null;
        
        var subsystem = EquippedSubsystems[slot];
        EquippedSubsystems[slot] = null;
        _cacheValid = false;
        return subsystem;
    }
    
    /// <summary>
    /// Get total bonus for a specific subsystem type
    /// </summary>
    public float GetSubsystemBonus(SubsystemType type)
    {
        if (!_cacheValid)
            RebuildBonusCache();
        
        return _bonusCache.GetValueOrDefault(type, 0f);
    }
    
    /// <summary>
    /// Get total bonus for all OmniCore subsystems
    /// </summary>
    public float GetOmniCoreBonus()
    {
        return GetSubsystemBonus(SubsystemType.OmniCore);
    }
    
    /// <summary>
    /// Rebuild the bonus cache from equipped subsystems
    /// </summary>
    private void RebuildBonusCache()
    {
        _bonusCache.Clear();
        
        foreach (var subsystem in EquippedSubsystems)
        {
            if (subsystem == null) continue;
            
            var type = subsystem.Type;
            var bonus = subsystem.GetTotalBonus();
            
            if (_bonusCache.ContainsKey(type))
                _bonusCache[type] += bonus;
            else
                _bonusCache[type] = bonus;
        }
        
        _cacheValid = true;
    }
    
    /// <summary>
    /// Get all equipped subsystems
    /// </summary>
    public List<SubsystemUpgrade> GetEquippedSubsystems()
    {
        return EquippedSubsystems.Where(s => s != null).Cast<SubsystemUpgrade>().ToList();
    }
    
    /// <summary>
    /// Get number of free slots
    /// </summary>
    public int GetFreeSlots()
    {
        return EquippedSubsystems.Count(s => s == null);
    }
    
    /// <summary>
    /// Serialize the component
    /// </summary>
    public Dictionary<string, object> Serialize()
    {
        var subsystemsData = new List<Dictionary<string, object>?>();
        foreach (var subsystem in EquippedSubsystems)
        {
            subsystemsData.Add(subsystem?.Serialize());
        }
        
        return new Dictionary<string, object>
        {
            ["EntityId"] = EntityId.ToString(),
            ["MaxSubsystemSlots"] = MaxSubsystemSlots,
            ["EquippedSubsystems"] = subsystemsData
        };
    }
    
    /// <summary>
    /// Deserialize the component
    /// </summary>
    public void Deserialize(Dictionary<string, object> data)
    {
        EntityId = Guid.Parse(data["EntityId"].ToString()!);
        MaxSubsystemSlots = Convert.ToInt32(data["MaxSubsystemSlots"]);
        
        EquippedSubsystems.Clear();
        
        if (data.ContainsKey("EquippedSubsystems"))
        {
            var subsystemsData = data["EquippedSubsystems"];
            
            if (subsystemsData is JsonElement jsonElement && jsonElement.ValueKind == JsonValueKind.Array)
            {
                foreach (var subsystemElement in jsonElement.EnumerateArray())
                {
                    if (subsystemElement.ValueKind == JsonValueKind.Null)
                    {
                        EquippedSubsystems.Add(null);
                    }
                    else
                    {
                        var subsystemDict = JsonSerializer.Deserialize<Dictionary<string, object>>(subsystemElement.GetRawText());
                        if (subsystemDict != null)
                        {
                            EquippedSubsystems.Add(SubsystemUpgrade.Deserialize(subsystemDict));
                        }
                        else
                        {
                            EquippedSubsystems.Add(null);
                        }
                    }
                }
            }
            else if (subsystemsData is List<object> subsystemsList)
            {
                foreach (var subsystemObj in subsystemsList)
                {
                    if (subsystemObj == null)
                    {
                        EquippedSubsystems.Add(null);
                    }
                    else if (subsystemObj is Dictionary<string, object> subsystemDict)
                    {
                        EquippedSubsystems.Add(SubsystemUpgrade.Deserialize(subsystemDict));
                    }
                    else
                    {
                        EquippedSubsystems.Add(null);
                    }
                }
            }
        }
        
        // Ensure we have the right number of slots
        while (EquippedSubsystems.Count < MaxSubsystemSlots)
        {
            EquippedSubsystems.Add(null);
        }
        
        _cacheValid = false;
    }
}
