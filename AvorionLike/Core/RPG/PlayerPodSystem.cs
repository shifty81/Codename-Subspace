using System.Numerics;
using System.Text.Json;
using AvorionLike.Core.ECS;
using AvorionLike.Core.Persistence;
using AvorionLike.Core.Voxel;

namespace AvorionLike.Core.RPG;

/// <summary>
/// Represents an upgrade that can be equipped to the player pod
/// </summary>
public class PodUpgrade
{
    public string Name { get; set; } = "Unknown Upgrade";
    public string Description { get; set; } = "";
    public PodUpgradeType Type { get; set; }
    public float EffectValue { get; set; } = 0f;
    public int Rarity { get; set; } = 1; // 1-5 scale
    
    public PodUpgrade(string name, string description, PodUpgradeType type, float effectValue, int rarity)
    {
        Name = name;
        Description = description;
        Type = type;
        EffectValue = effectValue;
        Rarity = rarity;
    }
}

/// <summary>
/// Types of upgrades that can be applied to the pod
/// </summary>
public enum PodUpgradeType
{
    ThrustBoost,       // Increases thrust power
    ShieldBoost,       // Increases shield capacity
    PowerBoost,        // Increases power generation
    EfficiencyBoost,   // Reduces efficiency penalty
    ExperienceBoost,   // Increases experience gain
    SkillBoost         // Additional skill points per level
}

/// <summary>
/// Component representing the player's pod - functions as the playable character
/// The pod is a multi-purpose utility ship with all necessary systems at half efficiency
/// </summary>
public class PlayerPodComponent : IComponent, ISerializable
{
    public Guid EntityId { get; set; }
    
    // Pod operates at 0.5x efficiency compared to built ships
    public float BaseEfficiencyMultiplier { get; set; } = 0.5f;
    
    // Pod's inherent capabilities (before upgrades)
    public float BaseThrustPower { get; set; } = 50f;
    public float BasePowerGeneration { get; set; } = 100f;
    public float BaseShieldCapacity { get; set; } = 200f;
    public float BaseTorque { get; set; } = 20f;
    
    // Equipped upgrades
    public List<PodUpgrade> EquippedUpgrades { get; set; } = new();
    public int MaxUpgradeSlots { get; set; } = 5;
    
    // Docking state
    public bool IsDocked { get; set; } = false;
    public Guid? DockedShipId { get; set; } = null;
    
    /// <summary>
    /// Calculate the total efficiency multiplier including upgrades
    /// </summary>
    public float GetTotalEfficiencyMultiplier()
    {
        float total = BaseEfficiencyMultiplier;
        foreach (var upgrade in EquippedUpgrades)
        {
            if (upgrade.Type == PodUpgradeType.EfficiencyBoost)
            {
                total += upgrade.EffectValue;
            }
        }
        return Math.Min(total, 1.0f); // Cap at 1.0 (100% efficiency)
    }
    
    /// <summary>
    /// Calculate total thrust power including upgrades
    /// </summary>
    public float GetTotalThrust()
    {
        float total = BaseThrustPower;
        foreach (var upgrade in EquippedUpgrades)
        {
            if (upgrade.Type == PodUpgradeType.ThrustBoost)
            {
                total += upgrade.EffectValue;
            }
        }
        return total * GetTotalEfficiencyMultiplier();
    }
    
    /// <summary>
    /// Calculate total power generation including upgrades
    /// </summary>
    public float GetTotalPowerGeneration()
    {
        float total = BasePowerGeneration;
        foreach (var upgrade in EquippedUpgrades)
        {
            if (upgrade.Type == PodUpgradeType.PowerBoost)
            {
                total += upgrade.EffectValue;
            }
        }
        return total * GetTotalEfficiencyMultiplier();
    }
    
    /// <summary>
    /// Calculate total shield capacity including upgrades
    /// </summary>
    public float GetTotalShieldCapacity()
    {
        float total = BaseShieldCapacity;
        foreach (var upgrade in EquippedUpgrades)
        {
            if (upgrade.Type == PodUpgradeType.ShieldBoost)
            {
                total += upgrade.EffectValue;
            }
        }
        return total * GetTotalEfficiencyMultiplier();
    }
    
    /// <summary>
    /// Calculate total torque including upgrades
    /// </summary>
    public float GetTotalTorque()
    {
        return BaseTorque * GetTotalEfficiencyMultiplier();
    }
    
    /// <summary>
    /// Get experience multiplier from upgrades
    /// </summary>
    public float GetExperienceMultiplier()
    {
        float multiplier = 1.0f;
        foreach (var upgrade in EquippedUpgrades)
        {
            if (upgrade.Type == PodUpgradeType.ExperienceBoost)
            {
                multiplier += upgrade.EffectValue;
            }
        }
        return multiplier;
    }
    
    /// <summary>
    /// Get additional skill points per level from upgrades
    /// </summary>
    public int GetBonusSkillPoints()
    {
        int bonus = 0;
        foreach (var upgrade in EquippedUpgrades)
        {
            if (upgrade.Type == PodUpgradeType.SkillBoost)
            {
                bonus += (int)upgrade.EffectValue;
            }
        }
        return bonus;
    }
    
    /// <summary>
    /// Equip an upgrade to the pod
    /// </summary>
    public bool EquipUpgrade(PodUpgrade upgrade)
    {
        if (EquippedUpgrades.Count >= MaxUpgradeSlots)
        {
            return false;
        }
        
        EquippedUpgrades.Add(upgrade);
        return true;
    }
    
    /// <summary>
    /// Remove an upgrade from the pod
    /// </summary>
    public bool UnequipUpgrade(PodUpgrade upgrade)
    {
        return EquippedUpgrades.Remove(upgrade);
    }
    
    /// <summary>
    /// Serialize the component to a dictionary
    /// </summary>
    public Dictionary<string, object> Serialize()
    {
        var upgradesData = new List<Dictionary<string, object>>();
        foreach (var upgrade in EquippedUpgrades)
        {
            upgradesData.Add(new Dictionary<string, object>
            {
                ["Name"] = upgrade.Name,
                ["Description"] = upgrade.Description,
                ["Type"] = upgrade.Type.ToString(),
                ["EffectValue"] = upgrade.EffectValue,
                ["Rarity"] = upgrade.Rarity
            });
        }
        
        return new Dictionary<string, object>
        {
            ["EntityId"] = EntityId.ToString(),
            ["BaseEfficiencyMultiplier"] = BaseEfficiencyMultiplier,
            ["BaseThrustPower"] = BaseThrustPower,
            ["BasePowerGeneration"] = BasePowerGeneration,
            ["BaseShieldCapacity"] = BaseShieldCapacity,
            ["BaseTorque"] = BaseTorque,
            ["EquippedUpgrades"] = upgradesData,
            ["MaxUpgradeSlots"] = MaxUpgradeSlots,
            ["IsDocked"] = IsDocked,
            ["DockedShipId"] = DockedShipId?.ToString() ?? ""
        };
    }
    
    /// <summary>
    /// Deserialize the component from a dictionary
    /// </summary>
    public void Deserialize(Dictionary<string, object> data)
    {
        EntityId = Guid.Parse(SerializationHelper.GetValue(data, "EntityId", Guid.Empty.ToString()));
        BaseEfficiencyMultiplier = SerializationHelper.GetValue(data, "BaseEfficiencyMultiplier", 0.5f);
        BaseThrustPower = SerializationHelper.GetValue(data, "BaseThrustPower", 50f);
        BasePowerGeneration = SerializationHelper.GetValue(data, "BasePowerGeneration", 100f);
        BaseShieldCapacity = SerializationHelper.GetValue(data, "BaseShieldCapacity", 200f);
        BaseTorque = SerializationHelper.GetValue(data, "BaseTorque", 20f);
        MaxUpgradeSlots = SerializationHelper.GetValue(data, "MaxUpgradeSlots", 5);
        IsDocked = SerializationHelper.GetValue(data, "IsDocked", false);
        
        var dockedShipIdStr = SerializationHelper.GetValue(data, "DockedShipId", "");
        DockedShipId = string.IsNullOrEmpty(dockedShipIdStr) ? null : Guid.Parse(dockedShipIdStr);
        
        EquippedUpgrades.Clear();
        if (data.ContainsKey("EquippedUpgrades"))
        {
            var upgradesData = data["EquippedUpgrades"];
            
            if (upgradesData is JsonElement jsonElement && jsonElement.ValueKind == JsonValueKind.Array)
            {
                foreach (var upgradeElement in jsonElement.EnumerateArray())
                {
                    var name = upgradeElement.GetProperty("Name").GetString() ?? "Unknown";
                    var description = upgradeElement.GetProperty("Description").GetString() ?? "";
                    var typeStr = upgradeElement.GetProperty("Type").GetString() ?? "ThrustBoost";
                    var type = Enum.Parse<PodUpgradeType>(typeStr);
                    var effectValue = upgradeElement.GetProperty("EffectValue").GetSingle();
                    var rarity = upgradeElement.GetProperty("Rarity").GetInt32();
                    
                    EquippedUpgrades.Add(new PodUpgrade(name, description, type, effectValue, rarity));
                }
            }
        }
    }
}

/// <summary>
/// Component for ships that can dock a player pod
/// </summary>
public class DockingComponent : IComponent, ISerializable
{
    public Guid EntityId { get; set; }
    
    // Whether this ship has a docking port
    public bool HasDockingPort { get; set; } = false;
    
    // Currently docked pod (if any)
    public Guid? DockedPodId { get; set; } = null;
    
    // Position of docking port relative to ship center
    public Vector3 DockingPortPosition { get; set; } = Vector3.Zero;
    
    /// <summary>
    /// Check if a pod is currently docked
    /// </summary>
    public bool HasDockedPod()
    {
        return DockedPodId.HasValue;
    }
    
    /// <summary>
    /// Serialize the component to a dictionary
    /// </summary>
    public Dictionary<string, object> Serialize()
    {
        return new Dictionary<string, object>
        {
            ["EntityId"] = EntityId.ToString(),
            ["HasDockingPort"] = HasDockingPort,
            ["DockedPodId"] = DockedPodId?.ToString() ?? "",
            ["DockingPortPosition"] = new Dictionary<string, object>
            {
                ["X"] = DockingPortPosition.X,
                ["Y"] = DockingPortPosition.Y,
                ["Z"] = DockingPortPosition.Z
            }
        };
    }
    
    /// <summary>
    /// Deserialize the component from a dictionary
    /// </summary>
    public void Deserialize(Dictionary<string, object> data)
    {
        EntityId = Guid.Parse(SerializationHelper.GetValue(data, "EntityId", Guid.Empty.ToString()));
        HasDockingPort = SerializationHelper.GetValue(data, "HasDockingPort", false);
        
        var dockedPodIdStr = SerializationHelper.GetValue(data, "DockedPodId", "");
        DockedPodId = string.IsNullOrEmpty(dockedPodIdStr) ? null : Guid.Parse(dockedPodIdStr);
        
        if (data.ContainsKey("DockingPortPosition"))
        {
            var posData = data["DockingPortPosition"];
            if (posData is JsonElement jsonElement)
            {
                DockingPortPosition = new Vector3(
                    jsonElement.GetProperty("X").GetSingle(),
                    jsonElement.GetProperty("Y").GetSingle(),
                    jsonElement.GetProperty("Z").GetSingle()
                );
            }
            else if (posData is Dictionary<string, object> posDict)
            {
                DockingPortPosition = new Vector3(
                    Convert.ToSingle(posDict["X"]),
                    Convert.ToSingle(posDict["Y"]),
                    Convert.ToSingle(posDict["Z"])
                );
            }
        }
    }
}

/// <summary>
/// System for managing pod docking and stat bonuses
/// </summary>
public class PodDockingSystem
{
    private readonly EntityManager _entityManager;
    
    public PodDockingSystem(EntityManager entityManager)
    {
        _entityManager = entityManager;
    }
    
    /// <summary>
    /// Dock a pod to a ship - transfers pod stats to the ship
    /// </summary>
    public bool DockPod(Guid podEntityId, Guid shipEntityId)
    {
        var podComponent = _entityManager.GetComponent<PlayerPodComponent>(podEntityId);
        var dockingComponent = _entityManager.GetComponent<DockingComponent>(shipEntityId);
        
        if (podComponent == null || dockingComponent == null)
        {
            return false;
        }
        
        if (!dockingComponent.HasDockingPort || dockingComponent.HasDockedPod())
        {
            return false;
        }
        
        if (podComponent.IsDocked)
        {
            return false;
        }
        
        // Update docking state
        podComponent.IsDocked = true;
        podComponent.DockedShipId = shipEntityId;
        dockingComponent.DockedPodId = podEntityId;
        
        // Apply pod bonuses to ship
        ApplyPodBonuses(podEntityId, shipEntityId);
        
        return true;
    }
    
    /// <summary>
    /// Undock a pod from a ship - removes pod stat bonuses
    /// </summary>
    public bool UndockPod(Guid podEntityId)
    {
        var podComponent = _entityManager.GetComponent<PlayerPodComponent>(podEntityId);
        
        if (podComponent == null || !podComponent.IsDocked || !podComponent.DockedShipId.HasValue)
        {
            return false;
        }
        
        var shipEntityId = podComponent.DockedShipId.Value;
        var dockingComponent = _entityManager.GetComponent<DockingComponent>(shipEntityId);
        
        if (dockingComponent == null)
        {
            return false;
        }
        
        // Remove pod bonuses from ship
        RemovePodBonuses(podEntityId, shipEntityId);
        
        // Update docking state
        podComponent.IsDocked = false;
        podComponent.DockedShipId = null;
        dockingComponent.DockedPodId = null;
        
        return true;
    }
    
    /// <summary>
    /// Apply pod stat bonuses to the docked ship
    /// </summary>
    private void ApplyPodBonuses(Guid podEntityId, Guid shipEntityId)
    {
        var podComponent = _entityManager.GetComponent<PlayerPodComponent>(podEntityId);
        var shipVoxel = _entityManager.GetComponent<VoxelStructureComponent>(shipEntityId);
        
        if (podComponent == null || shipVoxel == null)
        {
            return;
        }
        
        // Pod bonuses are already calculated, they affect the ship when piloted
        // The actual stat application happens when the ship's systems are queried
        // This method is a placeholder for future enhancements
    }
    
    /// <summary>
    /// Remove pod stat bonuses from the ship
    /// </summary>
    private void RemovePodBonuses(Guid podEntityId, Guid shipEntityId)
    {
        // Remove bonuses when undocking
        // This method is a placeholder for future enhancements
    }
    
    /// <summary>
    /// Get the effective stats for a ship with a docked pod
    /// </summary>
    public ShipStats GetEffectiveShipStats(Guid shipEntityId)
    {
        var dockingComponent = _entityManager.GetComponent<DockingComponent>(shipEntityId);
        var shipVoxel = _entityManager.GetComponent<VoxelStructureComponent>(shipEntityId);
        
        var stats = new ShipStats();
        
        if (shipVoxel != null)
        {
            stats.TotalThrust = shipVoxel.TotalThrust;
            stats.TotalTorque = shipVoxel.TotalTorque;
            stats.PowerGeneration = shipVoxel.PowerGeneration;
            stats.ShieldCapacity = shipVoxel.ShieldCapacity;
        }
        
        // Apply pod bonuses if docked
        if (dockingComponent != null && dockingComponent.HasDockedPod() && dockingComponent.DockedPodId.HasValue)
        {
            var podComponent = _entityManager.GetComponent<PlayerPodComponent>(dockingComponent.DockedPodId.Value);
            
            if (podComponent != null)
            {
                // Pod skills and abilities considerably affect the ship
                var podProgression = _entityManager.GetComponent<ProgressionComponent>(dockingComponent.DockedPodId.Value);
                float levelBonus = podProgression != null ? 1.0f + (podProgression.Level * 0.05f) : 1.0f;
                
                stats.TotalThrust *= levelBonus;
                stats.TotalTorque *= levelBonus;
                stats.PowerGeneration *= levelBonus;
                stats.ShieldCapacity *= levelBonus;
                
                // Add pod's inherent bonuses
                stats.TotalThrust += podComponent.GetTotalThrust();
                stats.TotalTorque += podComponent.GetTotalTorque();
                stats.PowerGeneration += podComponent.GetTotalPowerGeneration();
                stats.ShieldCapacity += podComponent.GetTotalShieldCapacity();
            }
        }
        
        return stats;
    }
}

/// <summary>
/// Represents ship statistics (with or without pod bonuses)
/// </summary>
public class ShipStats
{
    public float TotalThrust { get; set; }
    public float TotalTorque { get; set; }
    public float PowerGeneration { get; set; }
    public float ShieldCapacity { get; set; }
}
