using System.Text.Json;
using AvorionLike.Core.ECS;
using AvorionLike.Core.Persistence;
using AvorionLike.Core.RPG;

namespace AvorionLike.Core.Fleet;

/// <summary>
/// Ship class determines the role and capabilities of a ship
/// </summary>
public enum ShipClassType
{
    Undefined,      // Not yet classified
    Combat,         // Specialized for combat, high weapon damage
    Industrial,     // Mining and resource gathering
    Exploration,    // Long range scanning, jump drive efficiency
    Salvaging,      // Breaking down wrecks for materials
    Covert          // Stealth operations, cloaking, scouting
}

/// <summary>
/// Component that defines a ship's class and role
/// </summary>
public class ShipClassComponent : IComponent, ISerializable
{
    public Guid EntityId { get; set; }
    
    public ShipClassType ShipClass { get; set; } = ShipClassType.Undefined;
    
    // Class-specific bonuses (multiplicative with subsystems)
    public float ClassBonusMultiplier { get; set; } = 1.0f;
    
    // Mission readiness (0-100%)
    public float MissionReadiness { get; set; } = 100f;
    
    // Current mission status
    public bool IsOnMission { get; set; } = false;
    public Guid? CurrentMissionId { get; set; } = null;
    
    // Class specialization level (increases with successful missions)
    public int SpecializationLevel { get; set; } = 1;
    public int SpecializationXP { get; set; } = 0;
    
    // Class-specific stats
    private Dictionary<string, float> _classStats = new();
    
    public ShipClassComponent()
    {
        InitializeClassStats();
    }
    
    /// <summary>
    /// Set the ship class and initialize bonuses
    /// </summary>
    public void SetShipClass(ShipClassType classType)
    {
        ShipClass = classType;
        InitializeClassStats();
    }
    
    /// <summary>
    /// Initialize class-specific stats
    /// </summary>
    private void InitializeClassStats()
    {
        _classStats.Clear();
        
        switch (ShipClass)
        {
            case ShipClassType.Combat:
                _classStats["WeaponDamage"] = 1.5f;
                _classStats["ShieldCapacity"] = 1.3f;
                _classStats["ArmorEffectiveness"] = 1.4f;
                _classStats["CriticalChance"] = 1.2f;
                ClassBonusMultiplier = 1.2f;
                break;
                
            case ShipClassType.Industrial:
                _classStats["MiningYield"] = 1.8f;
                _classStats["CargoCapacity"] = 1.5f;
                _classStats["ResourceDetection"] = 1.4f;
                _classStats["PowerEfficiency"] = 1.3f;
                ClassBonusMultiplier = 1.15f;
                break;
                
            case ShipClassType.Exploration:
                _classStats["ScannerRange"] = 2.0f;
                _classStats["JumpRange"] = 1.5f;
                _classStats["JumpCooldown"] = 0.7f; // Reduced cooldown
                _classStats["FuelEfficiency"] = 1.4f;
                ClassBonusMultiplier = 1.15f;
                break;
                
            case ShipClassType.Salvaging:
                _classStats["SalvageYield"] = 2.0f;
                _classStats["SalvageSpeed"] = 1.5f;
                _classStats["LootQuality"] = 1.3f;
                _classStats["CargoCapacity"] = 1.3f;
                ClassBonusMultiplier = 1.15f;
                break;
                
            case ShipClassType.Covert:
                _classStats["CloakEfficiency"] = 1.5f;
                _classStats["ScannerEvasion"] = 2.0f;
                _classStats["ThrustPower"] = 1.3f;
                _classStats["DetectionRange"] = 1.6f;
                _classStats["CloakDuration"] = 1.5f;
                ClassBonusMultiplier = 1.1f;
                break;
                
            default:
                ClassBonusMultiplier = 1.0f;
                break;
        }
    }
    
    /// <summary>
    /// Get a class-specific stat multiplier
    /// </summary>
    public float GetClassStat(string statName)
    {
        return _classStats.GetValueOrDefault(statName, 1.0f);
    }
    
    /// <summary>
    /// Get all class stats
    /// </summary>
    public Dictionary<string, float> GetAllClassStats()
    {
        return new Dictionary<string, float>(_classStats);
    }
    
    /// <summary>
    /// Add specialization XP and level up if threshold reached
    /// </summary>
    public bool AddSpecializationXP(int xp)
    {
        SpecializationXP += xp;
        int xpNeeded = SpecializationLevel * 1000; // 1000, 2000, 3000, etc.
        
        if (SpecializationXP >= xpNeeded)
        {
            SpecializationXP -= xpNeeded;
            SpecializationLevel++;
            
            // Increase class bonus multiplier
            ClassBonusMultiplier += 0.05f;
            
            return true; // Leveled up
        }
        
        return false;
    }
    
    /// <summary>
    /// Check if ship can accept a mission
    /// </summary>
    public bool CanAcceptMission()
    {
        return !IsOnMission && MissionReadiness >= 50f;
    }
    
    /// <summary>
    /// Reduce mission readiness (fatigue from missions)
    /// </summary>
    public void ReduceReadiness(float amount)
    {
        MissionReadiness = Math.Max(0f, MissionReadiness - amount);
    }
    
    /// <summary>
    /// Restore mission readiness (recovery over time)
    /// </summary>
    public void RestoreReadiness(float amount)
    {
        MissionReadiness = Math.Min(100f, MissionReadiness + amount);
    }
    
    /// <summary>
    /// Serialize the component
    /// </summary>
    public Dictionary<string, object> Serialize()
    {
        return new Dictionary<string, object>
        {
            ["EntityId"] = EntityId.ToString(),
            ["ShipClass"] = ShipClass.ToString(),
            ["ClassBonusMultiplier"] = ClassBonusMultiplier,
            ["MissionReadiness"] = MissionReadiness,
            ["IsOnMission"] = IsOnMission,
            ["CurrentMissionId"] = CurrentMissionId?.ToString() ?? "",
            ["SpecializationLevel"] = SpecializationLevel,
            ["SpecializationXP"] = SpecializationXP,
            ["ClassStats"] = _classStats
        };
    }
    
    /// <summary>
    /// Deserialize the component
    /// </summary>
    public void Deserialize(Dictionary<string, object> data)
    {
        EntityId = Guid.Parse(data["EntityId"].ToString()!);
        ShipClass = Enum.Parse<ShipClassType>(data["ShipClass"].ToString()!);
        ClassBonusMultiplier = Convert.ToSingle(data["ClassBonusMultiplier"]);
        MissionReadiness = Convert.ToSingle(data["MissionReadiness"]);
        IsOnMission = Convert.ToBoolean(data["IsOnMission"]);
        
        var missionIdStr = data["CurrentMissionId"].ToString();
        CurrentMissionId = string.IsNullOrEmpty(missionIdStr) ? null : Guid.Parse(missionIdStr);
        
        SpecializationLevel = Convert.ToInt32(data["SpecializationLevel"]);
        SpecializationXP = Convert.ToInt32(data["SpecializationXP"]);
        
        // Restore class stats
        if (data.ContainsKey("ClassStats"))
        {
            var statsData = data["ClassStats"];
            if (statsData is JsonElement jsonElement)
            {
                _classStats = JsonSerializer.Deserialize<Dictionary<string, float>>(jsonElement.GetRawText()) 
                    ?? new Dictionary<string, float>();
            }
            else if (statsData is Dictionary<string, float> statsDict)
            {
                _classStats = new Dictionary<string, float>(statsDict);
            }
        }
        
        // Initialize if stats are empty
        if (_classStats.Count == 0)
        {
            InitializeClassStats();
        }
    }
}

/// <summary>
/// Generator for class-specific subsystems
/// </summary>
public static class ClassSpecificSubsystemGenerator
{
    /// <summary>
    /// Generate a subsystem appropriate for a ship class
    /// </summary>
    public static SubsystemUpgrade GenerateForClass(
        ShipClassType shipClass, 
        SubsystemRarity rarity = SubsystemRarity.Common,
        SubsystemQuality quality = SubsystemQuality.Standard)
    {
        var availableTypes = GetSubsystemTypesForClass(shipClass);
        
        if (availableTypes.Length == 0)
        {
            // Fallback to general subsystems
            var allTypes = Enum.GetValues<SubsystemType>();
            return new SubsystemUpgrade(allTypes[Random.Shared.Next(allTypes.Length)], rarity, quality);
        }
        
        var selectedType = availableTypes[Random.Shared.Next(availableTypes.Length)];
        return new SubsystemUpgrade(selectedType, rarity, quality);
    }
    
    /// <summary>
    /// Get subsystem types that are relevant for a ship class
    /// </summary>
    private static SubsystemType[] GetSubsystemTypesForClass(ShipClassType shipClass)
    {
        return shipClass switch
        {
            ShipClassType.Combat => new[]
            {
                SubsystemType.WeaponAmplifier,
                SubsystemType.TargetingComputer,
                SubsystemType.CoolingSystem,
                SubsystemType.ShieldBooster,
                SubsystemType.ShieldRegenerator,
                SubsystemType.ArmorPlating,
                SubsystemType.StructuralReinforcement,
                SubsystemType.PowerAmplifier
            },
            
            ShipClassType.Industrial => new[]
            {
                SubsystemType.PowerEfficiency,
                SubsystemType.CargoExpansion,
                SubsystemType.PowerAmplifier,
                SubsystemType.StructuralReinforcement,
                SubsystemType.ThrustAmplifier
            },
            
            ShipClassType.Exploration => new[]
            {
                SubsystemType.ScannerArray,
                SubsystemType.JumpDriveEnhancer,
                SubsystemType.PowerEfficiency,
                SubsystemType.Capacitor,
                SubsystemType.ThrustAmplifier,
                SubsystemType.CargoExpansion
            },
            
            ShipClassType.Salvaging => new[]
            {
                SubsystemType.CargoExpansion,
                SubsystemType.PowerAmplifier,
                SubsystemType.StructuralReinforcement,
                SubsystemType.ThrustAmplifier
            },
            
            ShipClassType.Covert => new[]
            {
                SubsystemType.PowerEfficiency,
                SubsystemType.ThrustAmplifier,
                SubsystemType.ManeuveringThrusters,
                SubsystemType.ScannerArray,
                SubsystemType.Capacitor
            },
            
            _ => Array.Empty<SubsystemType>()
        };
    }
    
    /// <summary>
    /// Get weighted subsystem types for a ship class (some types more common than others)
    /// </summary>
    public static Dictionary<SubsystemType, float> GetWeightedSubsystemTypes(ShipClassType shipClass)
    {
        var weights = new Dictionary<SubsystemType, float>();
        
        switch (shipClass)
        {
            case ShipClassType.Combat:
                weights[SubsystemType.WeaponAmplifier] = 2.0f;
                weights[SubsystemType.TargetingComputer] = 1.5f;
                weights[SubsystemType.CoolingSystem] = 1.5f;
                weights[SubsystemType.ShieldBooster] = 1.8f;
                weights[SubsystemType.ShieldRegenerator] = 1.5f;
                weights[SubsystemType.ArmorPlating] = 1.3f;
                weights[SubsystemType.StructuralReinforcement] = 1.0f;
                weights[SubsystemType.PowerAmplifier] = 1.2f;
                break;
                
            case ShipClassType.Industrial:
                weights[SubsystemType.CargoExpansion] = 2.0f;
                weights[SubsystemType.PowerEfficiency] = 1.8f;
                weights[SubsystemType.PowerAmplifier] = 1.5f;
                weights[SubsystemType.StructuralReinforcement] = 1.2f;
                weights[SubsystemType.ThrustAmplifier] = 1.0f;
                break;
                
            case ShipClassType.Exploration:
                weights[SubsystemType.ScannerArray] = 2.0f;
                weights[SubsystemType.JumpDriveEnhancer] = 1.8f;
                weights[SubsystemType.PowerEfficiency] = 1.5f;
                weights[SubsystemType.Capacitor] = 1.3f;
                weights[SubsystemType.ThrustAmplifier] = 1.2f;
                weights[SubsystemType.CargoExpansion] = 1.0f;
                break;
                
            case ShipClassType.Salvaging:
                weights[SubsystemType.CargoExpansion] = 2.0f;
                weights[SubsystemType.PowerAmplifier] = 1.5f;
                weights[SubsystemType.StructuralReinforcement] = 1.3f;
                weights[SubsystemType.ThrustAmplifier] = 1.2f;
                break;
                
            case ShipClassType.Covert:
                weights[SubsystemType.PowerEfficiency] = 2.0f;
                weights[SubsystemType.ThrustAmplifier] = 1.8f;
                weights[SubsystemType.ManeuveringThrusters] = 1.6f;
                weights[SubsystemType.ScannerArray] = 1.4f;
                weights[SubsystemType.Capacitor] = 1.3f;
                break;
        }
        
        return weights;
    }
}
