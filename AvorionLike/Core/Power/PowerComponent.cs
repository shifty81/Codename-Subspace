using AvorionLike.Core.ECS;
using AvorionLike.Core.Persistence;

namespace AvorionLike.Core.Power;

/// <summary>
/// Component for managing ship power generation and consumption
/// Powers weapons, shields, engines, and other systems
/// </summary>
public class PowerComponent : IComponent, ISerializable
{
    public Guid EntityId { get; set; }
    
    // Power generation
    public float MaxPowerGeneration { get; set; } = 0f;
    public float CurrentPowerGeneration { get; set; } = 0f;
    
    // Power consumption
    public float TotalPowerConsumption { get; set; } = 0f;
    public float WeaponsPowerConsumption { get; set; } = 0f;
    public float ShieldsPowerConsumption { get; set; } = 0f;
    public float EnginesPowerConsumption { get; set; } = 0f;
    public float SystemsPowerConsumption { get; set; } = 0f;
    
    // Power storage (capacitors)
    public float MaxStoredPower { get; set; } = 100f;
    public float CurrentStoredPower { get; set; } = 100f;
    
    // Power efficiency
    public float Efficiency { get; set; } = 1.0f; // 1.0 = 100% efficient
    
    // System states
    public bool WeaponsEnabled { get; set; } = true;
    public bool ShieldsEnabled { get; set; } = true;
    public bool EnginesEnabled { get; set; } = true;
    public bool SystemsEnabled { get; set; } = true;
    
    // Priority system (1 = highest, 4 = lowest)
    public int WeaponsPriority { get; set; } = 2;
    public int ShieldsPriority { get; set; } = 1;
    public int EnginesPriority { get; set; } = 3;
    public int SystemsPriority { get; set; } = 4;
    
    /// <summary>
    /// Get available power after consumption
    /// </summary>
    public float GetAvailablePower()
    {
        float generated = CurrentPowerGeneration * Efficiency;
        return Math.Max(0, generated - TotalPowerConsumption);
    }
    
    /// <summary>
    /// Check if there's enough power for a specific system
    /// </summary>
    public bool HasEnoughPower(float requiredPower)
    {
        return GetAvailablePower() >= requiredPower;
    }
    
    /// <summary>
    /// Get power deficit (negative available power)
    /// </summary>
    public float GetPowerDeficit()
    {
        return Math.Max(0, TotalPowerConsumption - (CurrentPowerGeneration * Efficiency));
    }
    
    /// <summary>
    /// Check if ship is in low power state
    /// </summary>
    public bool IsLowPower()
    {
        return GetPowerDeficit() > 0;
    }
    
    /// <summary>
    /// Update total power consumption from all systems
    /// </summary>
    public void UpdateTotalConsumption()
    {
        TotalPowerConsumption = 0;
        
        if (WeaponsEnabled) TotalPowerConsumption += WeaponsPowerConsumption;
        if (ShieldsEnabled) TotalPowerConsumption += ShieldsPowerConsumption;
        if (EnginesEnabled) TotalPowerConsumption += EnginesPowerConsumption;
        if (SystemsEnabled) TotalPowerConsumption += SystemsPowerConsumption;
    }
    
    /// <summary>
    /// Toggle a system on/off
    /// </summary>
    public void ToggleSystem(PowerSystemType systemType)
    {
        switch (systemType)
        {
            case PowerSystemType.Weapons:
                WeaponsEnabled = !WeaponsEnabled;
                break;
            case PowerSystemType.Shields:
                ShieldsEnabled = !ShieldsEnabled;
                break;
            case PowerSystemType.Engines:
                EnginesEnabled = !EnginesEnabled;
                break;
            case PowerSystemType.Systems:
                SystemsEnabled = !SystemsEnabled;
                break;
        }
        UpdateTotalConsumption();
    }
    
    /// <summary>
    /// Serialize component data
    /// </summary>
    public Dictionary<string, object> Serialize()
    {
        return new Dictionary<string, object>
        {
            ["EntityId"] = EntityId.ToString(),
            ["MaxPowerGeneration"] = MaxPowerGeneration,
            ["CurrentPowerGeneration"] = CurrentPowerGeneration,
            ["WeaponsPowerConsumption"] = WeaponsPowerConsumption,
            ["ShieldsPowerConsumption"] = ShieldsPowerConsumption,
            ["EnginesPowerConsumption"] = EnginesPowerConsumption,
            ["SystemsPowerConsumption"] = SystemsPowerConsumption,
            ["MaxStoredPower"] = MaxStoredPower,
            ["CurrentStoredPower"] = CurrentStoredPower,
            ["Efficiency"] = Efficiency,
            ["WeaponsEnabled"] = WeaponsEnabled,
            ["ShieldsEnabled"] = ShieldsEnabled,
            ["EnginesEnabled"] = EnginesEnabled,
            ["SystemsEnabled"] = SystemsEnabled,
            ["WeaponsPriority"] = WeaponsPriority,
            ["ShieldsPriority"] = ShieldsPriority,
            ["EnginesPriority"] = EnginesPriority,
            ["SystemsPriority"] = SystemsPriority
        };
    }
    
    /// <summary>
    /// Deserialize component data
    /// </summary>
    public void Deserialize(Dictionary<string, object> data)
    {
        EntityId = Guid.Parse(data["EntityId"].ToString() ?? Guid.Empty.ToString());
        MaxPowerGeneration = Convert.ToSingle(data["MaxPowerGeneration"]);
        CurrentPowerGeneration = Convert.ToSingle(data["CurrentPowerGeneration"]);
        WeaponsPowerConsumption = Convert.ToSingle(data["WeaponsPowerConsumption"]);
        ShieldsPowerConsumption = Convert.ToSingle(data["ShieldsPowerConsumption"]);
        EnginesPowerConsumption = Convert.ToSingle(data["EnginesPowerConsumption"]);
        SystemsPowerConsumption = Convert.ToSingle(data["SystemsPowerConsumption"]);
        MaxStoredPower = Convert.ToSingle(data["MaxStoredPower"]);
        CurrentStoredPower = Convert.ToSingle(data["CurrentStoredPower"]);
        Efficiency = Convert.ToSingle(data["Efficiency"]);
        WeaponsEnabled = Convert.ToBoolean(data["WeaponsEnabled"]);
        ShieldsEnabled = Convert.ToBoolean(data["ShieldsEnabled"]);
        EnginesEnabled = Convert.ToBoolean(data["EnginesEnabled"]);
        SystemsEnabled = Convert.ToBoolean(data["SystemsEnabled"]);
        WeaponsPriority = Convert.ToInt32(data["WeaponsPriority"]);
        ShieldsPriority = Convert.ToInt32(data["ShieldsPriority"]);
        EnginesPriority = Convert.ToInt32(data["EnginesPriority"]);
        SystemsPriority = Convert.ToInt32(data["SystemsPriority"]);
    }
}

/// <summary>
/// Types of power systems on a ship
/// </summary>
public enum PowerSystemType
{
    Weapons,
    Shields,
    Engines,
    Systems
}
