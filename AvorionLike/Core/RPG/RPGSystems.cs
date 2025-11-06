using System.Text.Json;
using AvorionLike.Core.ECS;
using AvorionLike.Core.Resources;
using AvorionLike.Core.Persistence;

namespace AvorionLike.Core.RPG;

/// <summary>
/// Component for ship progression and stats
/// </summary>
public class ProgressionComponent : IComponent, ISerializable
{
    public Guid EntityId { get; set; }
    public int Level { get; set; } = 1;
    public int Experience { get; set; } = 0;
    public int ExperienceToNextLevel { get; set; } = 100;
    public int SkillPoints { get; set; } = 0;

    /// <summary>
    /// Add experience and check for level up
    /// </summary>
    public bool AddExperience(int amount)
    {
        Experience += amount;
        
        if (Experience >= ExperienceToNextLevel)
        {
            LevelUp();
            return true;
        }
        
        return false;
    }

    private void LevelUp()
    {
        Level++;
        Experience -= ExperienceToNextLevel;
        ExperienceToNextLevel = (int)(ExperienceToNextLevel * 1.5f);
        SkillPoints += 3;
    }

    /// <summary>
    /// Serialize the component to a dictionary
    /// </summary>
    public Dictionary<string, object> Serialize()
    {
        return new Dictionary<string, object>
        {
            ["EntityId"] = EntityId.ToString(),
            ["Level"] = Level,
            ["Experience"] = Experience,
            ["ExperienceToNextLevel"] = ExperienceToNextLevel,
            ["SkillPoints"] = SkillPoints
        };
    }

    /// <summary>
    /// Deserialize the component from a dictionary
    /// </summary>
    public void Deserialize(Dictionary<string, object> data)
    {
        EntityId = Guid.Parse(SerializationHelper.GetValue(data, "EntityId", Guid.Empty.ToString()));
        Level = SerializationHelper.GetValue(data, "Level", 1);
        Experience = SerializationHelper.GetValue(data, "Experience", 0);
        ExperienceToNextLevel = SerializationHelper.GetValue(data, "ExperienceToNextLevel", 100);
        SkillPoints = SerializationHelper.GetValue(data, "SkillPoints", 0);
    }
}

/// <summary>
/// Represents a faction in the game
/// </summary>
public class Faction
{
    public string Name { get; set; } = "Unknown";
    public string Description { get; set; } = "";
    public Dictionary<string, int> Relations { get; set; } = new(); // Faction name -> reputation
}

/// <summary>
/// Component for faction relations
/// </summary>
public class FactionComponent : IComponent, ISerializable
{
    public Guid EntityId { get; set; }
    public string FactionName { get; set; } = "Neutral";
    public Dictionary<string, int> Reputation { get; set; } = new(); // Faction name -> reputation value

    /// <summary>
    /// Modify reputation with a faction
    /// </summary>
    public void ModifyReputation(string factionName, int amount)
    {
        if (!Reputation.ContainsKey(factionName))
        {
            Reputation[factionName] = 0;
        }

        Reputation[factionName] += amount;
        
        // Clamp reputation between -100 and 100
        Reputation[factionName] = Math.Max(-100, Math.Min(100, Reputation[factionName]));
    }

    /// <summary>
    /// Get reputation with a faction
    /// </summary>
    public int GetReputation(string factionName)
    {
        return Reputation.GetValueOrDefault(factionName, 0);
    }

    /// <summary>
    /// Check if friendly with a faction
    /// </summary>
    public bool IsFriendly(string factionName)
    {
        return GetReputation(factionName) >= 50;
    }

    /// <summary>
    /// Check if hostile with a faction
    /// </summary>
    public bool IsHostile(string factionName)
    {
        return GetReputation(factionName) <= -50;
    }

    /// <summary>
    /// Serialize the component to a dictionary
    /// </summary>
    public Dictionary<string, object> Serialize()
    {
        return new Dictionary<string, object>
        {
            ["EntityId"] = EntityId.ToString(),
            ["FactionName"] = FactionName,
            ["Reputation"] = new Dictionary<string, object>(Reputation.ToDictionary(k => k.Key, v => (object)v.Value))
        };
    }

    /// <summary>
    /// Deserialize the component from a dictionary
    /// </summary>
    public void Deserialize(Dictionary<string, object> data)
    {
        EntityId = Guid.Parse(SerializationHelper.GetValue(data, "EntityId", Guid.Empty.ToString()));
        FactionName = SerializationHelper.GetValue(data, "FactionName", "Neutral");
        
        Reputation.Clear();
        if (data.ContainsKey("Reputation"))
        {
            Dictionary<string, object> reputationData;
            
            if (data["Reputation"] is JsonElement jsonElement)
            {
                reputationData = JsonSerializer.Deserialize<Dictionary<string, object>>(jsonElement.GetRawText()) 
                    ?? new Dictionary<string, object>();
            }
            else
            {
                reputationData = (Dictionary<string, object>)data["Reputation"];
            }
            
            Reputation = SerializationHelper.DeserializeStringDictionary<int>(reputationData);
        }
    }
}

/// <summary>
/// Represents a loot drop item
/// </summary>
public class LootDrop
{
    public ResourceType? Resource { get; set; }
    public int Amount { get; set; }
    public float DropChance { get; set; } = 1.0f;
    
    // Pod-specific loot
    public PodUpgrade? PodUpgrade { get; set; }
    public string? AbilityId { get; set; }
}

/// <summary>
/// System for managing loot drops
/// </summary>
public class LootSystem
{
    private readonly Random _random = new();

    /// <summary>
    /// Generate loot drops based on entity level
    /// </summary>
    public List<LootDrop> GenerateLoot(int entityLevel, bool includePodLoot = false)
    {
        var loot = new List<LootDrop>();

        // Credits always drop
        loot.Add(new LootDrop
        {
            Resource = ResourceType.Credits,
            Amount = entityLevel * 100 + _random.Next(50, 200),
            DropChance = 1.0f
        });

        // Random resource drops
        var resourceTypes = new[] 
        { 
            ResourceType.Iron, 
            ResourceType.Titanium, 
            ResourceType.Naonite,
            ResourceType.Trinium
        };

        int dropCount = _random.Next(1, 3);
        for (int i = 0; i < dropCount; i++)
        {
            var resource = resourceTypes[_random.Next(resourceTypes.Length)];
            var amount = _random.Next(10, 50) * entityLevel;
            
            loot.Add(new LootDrop
            {
                Resource = resource,
                Amount = amount,
                DropChance = 0.7f
            });
        }
        
        // Add pod-specific loot
        if (includePodLoot)
        {
            // Pod upgrade drops (rare)
            if (_random.NextDouble() < 0.15f) // 15% chance
            {
                var upgrade = GenerateRandomPodUpgrade(entityLevel);
                loot.Add(new LootDrop
                {
                    PodUpgrade = upgrade,
                    DropChance = 1.0f
                });
            }
            
            // Ability unlock drops (very rare)
            if (entityLevel >= 5 && _random.NextDouble() < 0.05f) // 5% chance for level 5+
            {
                var abilityId = GenerateRandomAbilityId();
                if (!string.IsNullOrEmpty(abilityId))
                {
                    loot.Add(new LootDrop
                    {
                        AbilityId = abilityId,
                        DropChance = 1.0f
                    });
                }
            }
        }

        return loot;
    }
    
    /// <summary>
    /// Generate a random pod upgrade based on entity level
    /// </summary>
    private PodUpgrade GenerateRandomPodUpgrade(int entityLevel)
    {
        var upgradeTypes = Enum.GetValues<PodUpgradeType>();
        var type = upgradeTypes[_random.Next(upgradeTypes.Length)];
        
        // Rarity scales with level (capped at 5)
        int rarity = Math.Min(5, 1 + (entityLevel / 5) + _random.Next(0, 2));
        
        // Effect value scales with rarity
        float effectValue = type switch
        {
            PodUpgradeType.ThrustBoost => 15f + (rarity * 10f),
            PodUpgradeType.ShieldBoost => 50f + (rarity * 25f),
            PodUpgradeType.PowerBoost => 30f + (rarity * 20f),
            PodUpgradeType.EfficiencyBoost => 0.05f + (rarity * 0.02f),
            PodUpgradeType.ExperienceBoost => 0.10f + (rarity * 0.05f),
            PodUpgradeType.SkillBoost => rarity,
            _ => 10f
        };
        
        string name = GenerateUpgradeName(type, rarity);
        string description = GenerateUpgradeDescription(type, effectValue);
        
        return new PodUpgrade(name, description, type, effectValue, rarity);
    }
    
    /// <summary>
    /// Generate a random ability ID for unlocking
    /// </summary>
    private string GenerateRandomAbilityId()
    {
        var abilityIds = new[]
        {
            "shield_overcharge", "emergency_shields",
            "overload_weapons", "precision_strike",
            "afterburner", "emergency_warp",
            "energy_drain", "scan_pulse"
        };
        
        return abilityIds[_random.Next(abilityIds.Length)];
    }
    
    /// <summary>
    /// Generate upgrade name based on type and rarity
    /// </summary>
    private string GenerateUpgradeName(PodUpgradeType type, int rarity)
    {
        var rarityPrefix = rarity switch
        {
            1 => "Basic",
            2 => "Improved",
            3 => "Advanced",
            4 => "Superior",
            5 => "Legendary",
            _ => "Unknown"
        };
        
        var typeName = type switch
        {
            PodUpgradeType.ThrustBoost => "Thruster Module",
            PodUpgradeType.ShieldBoost => "Shield Amplifier",
            PodUpgradeType.PowerBoost => "Power Core",
            PodUpgradeType.EfficiencyBoost => "Efficiency Optimizer",
            PodUpgradeType.ExperienceBoost => "Knowledge Matrix",
            PodUpgradeType.SkillBoost => "Neural Enhancer",
            _ => "Pod Module"
        };
        
        return $"{rarityPrefix} {typeName}";
    }
    
    /// <summary>
    /// Generate upgrade description
    /// </summary>
    private string GenerateUpgradeDescription(PodUpgradeType type, float effectValue)
    {
        return type switch
        {
            PodUpgradeType.ThrustBoost => $"Increases thrust power by {effectValue:F0}N",
            PodUpgradeType.ShieldBoost => $"Increases shield capacity by {effectValue:F0}",
            PodUpgradeType.PowerBoost => $"Increases power generation by {effectValue:F0}W",
            PodUpgradeType.EfficiencyBoost => $"Increases efficiency by {effectValue * 100:F0}%",
            PodUpgradeType.ExperienceBoost => $"Increases experience gain by {effectValue * 100:F0}%",
            PodUpgradeType.SkillBoost => $"Grants {effectValue:F0} bonus skill points per level",
            _ => "Unknown effect"
        };
    }

    /// <summary>
    /// Process loot drops and add to inventory
    /// </summary>
    public void ProcessLoot(List<LootDrop> loot, Inventory inventory)
    {
        foreach (var drop in loot)
        {
            if (_random.NextDouble() <= drop.DropChance && drop.Resource.HasValue)
            {
                inventory.AddResource(drop.Resource.Value, drop.Amount);
            }
        }
    }
    
    /// <summary>
    /// Process loot drops including pod upgrades
    /// Returns list of pod upgrades found
    /// </summary>
    public List<PodUpgrade> ProcessLootWithPodUpgrades(List<LootDrop> loot, Inventory inventory)
    {
        var podUpgrades = new List<PodUpgrade>();
        
        foreach (var drop in loot)
        {
            if (_random.NextDouble() <= drop.DropChance)
            {
                // Add resources
                if (drop.Resource.HasValue)
                {
                    inventory.AddResource(drop.Resource.Value, drop.Amount);
                }
                
                // Collect pod upgrades
                if (drop.PodUpgrade != null)
                {
                    podUpgrades.Add(drop.PodUpgrade);
                }
            }
        }
        
        return podUpgrades;
    }
}

/// <summary>
/// Trading system for buying and selling
/// </summary>
public class TradingSystem
{
    private readonly Dictionary<ResourceType, float> _basePrices = new()
    {
        { ResourceType.Iron, 10f },
        { ResourceType.Titanium, 25f },
        { ResourceType.Naonite, 50f },
        { ResourceType.Trinium, 100f },
        { ResourceType.Xanion, 200f },
        { ResourceType.Ogonite, 400f },
        { ResourceType.Avorion, 800f }
    };

    /// <summary>
    /// Calculate buy price for a resource
    /// </summary>
    public int GetBuyPrice(ResourceType type, int amount)
    {
        return (int)(_basePrices.GetValueOrDefault(type, 10f) * amount * 1.2f);
    }

    /// <summary>
    /// Calculate sell price for a resource
    /// </summary>
    public int GetSellPrice(ResourceType type, int amount)
    {
        return (int)(_basePrices.GetValueOrDefault(type, 10f) * amount * 0.8f);
    }

    /// <summary>
    /// Buy resources from a station
    /// </summary>
    public bool BuyResource(ResourceType type, int amount, Inventory inventory)
    {
        int cost = GetBuyPrice(type, amount);
        
        if (inventory.HasResource(ResourceType.Credits, cost))
        {
            inventory.RemoveResource(ResourceType.Credits, cost);
            inventory.AddResource(type, amount);
            return true;
        }
        
        return false;
    }

    /// <summary>
    /// Sell resources to a station
    /// </summary>
    public bool SellResource(ResourceType type, int amount, Inventory inventory)
    {
        if (inventory.HasResource(type, amount))
        {
            inventory.RemoveResource(type, amount);
            int credits = GetSellPrice(type, amount);
            inventory.AddResource(ResourceType.Credits, credits);
            return true;
        }
        
        return false;
    }
}
