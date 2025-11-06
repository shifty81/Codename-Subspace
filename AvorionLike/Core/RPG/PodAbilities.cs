using System.Numerics;
using System.Text.Json;
using AvorionLike.Core.ECS;
using AvorionLike.Core.Persistence;

namespace AvorionLike.Core.RPG;

/// <summary>
/// Represents the type of active ability
/// </summary>
public enum AbilityType
{
    Shield,      // Shield-based abilities
    Weapon,      // Weapon-based abilities
    Mobility,    // Movement abilities
    Utility      // Misc utility abilities
}

/// <summary>
/// Represents an active ability that the pod can use
/// </summary>
public class PodAbility
{
    public string Id { get; set; } = "";
    public string Name { get; set; } = "";
    public string Description { get; set; } = "";
    public AbilityType Type { get; set; }
    
    // Resource costs
    public float EnergyCost { get; set; } = 0f;
    public float Cooldown { get; set; } = 0f; // In seconds
    public float Duration { get; set; } = 0f; // In seconds (0 = instant)
    
    // Ability effects
    public float EffectValue { get; set; } = 0f;
    public float Range { get; set; } = 0f; // Range in meters (0 = self)
    
    // State
    public DateTime LastUsed { get; set; } = DateTime.MinValue;
    public bool IsActive { get; set; } = false;
    public DateTime ActiveUntil { get; set; } = DateTime.MinValue;
    
    public PodAbility(string id, string name, string description, AbilityType type,
                      float energyCost, float cooldown, float duration = 0f, 
                      float effectValue = 0f, float range = 0f)
    {
        Id = id;
        Name = name;
        Description = description;
        Type = type;
        EnergyCost = energyCost;
        Cooldown = cooldown;
        Duration = duration;
        EffectValue = effectValue;
        Range = range;
    }
    
    /// <summary>
    /// Check if the ability is ready to use
    /// </summary>
    public bool IsReady()
    {
        var timeSinceLastUse = (DateTime.Now - LastUsed).TotalSeconds;
        return timeSinceLastUse >= Cooldown;
    }
    
    /// <summary>
    /// Get remaining cooldown time in seconds
    /// </summary>
    public float GetRemainingCooldown()
    {
        var timeSinceLastUse = (DateTime.Now - LastUsed).TotalSeconds;
        var remaining = Cooldown - timeSinceLastUse;
        return (float)Math.Max(0, remaining);
    }
    
    /// <summary>
    /// Check if the ability is currently active
    /// </summary>
    public bool IsCurrentlyActive()
    {
        if (Duration == 0f) return false; // Instant ability
        return IsActive && DateTime.Now < ActiveUntil;
    }
    
    /// <summary>
    /// Use the ability
    /// </summary>
    public void Use()
    {
        LastUsed = DateTime.Now;
        
        if (Duration > 0f)
        {
            IsActive = true;
            ActiveUntil = DateTime.Now.AddSeconds(Duration);
        }
    }
    
    /// <summary>
    /// Update ability state (call each frame)
    /// </summary>
    public void Update()
    {
        if (IsActive && DateTime.Now >= ActiveUntil)
        {
            IsActive = false;
        }
    }
}

/// <summary>
/// Component managing the pod's active abilities
/// </summary>
public class PodAbilitiesComponent : IComponent, ISerializable
{
    public Guid EntityId { get; set; }
    
    // All available abilities
    public Dictionary<string, PodAbility> Abilities { get; set; } = new();
    
    // Equipped abilities (limited slots)
    public List<string> EquippedAbilityIds { get; set; } = new();
    public int MaxEquippedAbilities { get; set; } = 4;
    
    public PodAbilitiesComponent()
    {
        InitializeAbilities();
    }
    
    /// <summary>
    /// Initialize all pod abilities
    /// </summary>
    private void InitializeAbilities()
    {
        // Shield Abilities
        Abilities["shield_overcharge"] = new PodAbility(
            "shield_overcharge",
            "Shield Overcharge",
            "Temporarily boosts shield capacity by 50% for 10 seconds",
            AbilityType.Shield,
            energyCost: 50f,
            cooldown: 30f,
            duration: 10f,
            effectValue: 0.5f
        );
        
        Abilities["emergency_shields"] = new PodAbility(
            "emergency_shields",
            "Emergency Shields",
            "Instantly restores 30% of shield capacity",
            AbilityType.Shield,
            energyCost: 40f,
            cooldown: 45f,
            effectValue: 0.3f
        );
        
        // Weapon Abilities
        Abilities["overload_weapons"] = new PodAbility(
            "overload_weapons",
            "Weapon Overload",
            "Increases weapon damage by 75% for 8 seconds",
            AbilityType.Weapon,
            energyCost: 60f,
            cooldown: 40f,
            duration: 8f,
            effectValue: 0.75f
        );
        
        Abilities["precision_strike"] = new PodAbility(
            "precision_strike",
            "Precision Strike",
            "Next shot deals 200% damage and always critical hits",
            AbilityType.Weapon,
            energyCost: 30f,
            cooldown: 25f,
            effectValue: 2.0f
        );
        
        // Mobility Abilities
        Abilities["afterburner"] = new PodAbility(
            "afterburner",
            "Afterburner",
            "Increases thrust by 100% for 5 seconds",
            AbilityType.Mobility,
            energyCost: 35f,
            cooldown: 20f,
            duration: 5f,
            effectValue: 1.0f
        );
        
        Abilities["emergency_warp"] = new PodAbility(
            "emergency_warp",
            "Emergency Warp",
            "Instantly teleport 500 meters in facing direction",
            AbilityType.Mobility,
            energyCost: 70f,
            cooldown: 60f,
            range: 500f
        );
        
        // Utility Abilities
        Abilities["energy_drain"] = new PodAbility(
            "energy_drain",
            "Energy Drain",
            "Drains 50 energy from target within 200m and restores it to self",
            AbilityType.Utility,
            energyCost: 20f,
            cooldown: 15f,
            effectValue: 50f,
            range: 200f
        );
        
        Abilities["scan_pulse"] = new PodAbility(
            "scan_pulse",
            "Scan Pulse",
            "Reveals all objects within 1000m for 15 seconds",
            AbilityType.Utility,
            energyCost: 25f,
            cooldown: 30f,
            duration: 15f,
            range: 1000f
        );
    }
    
    /// <summary>
    /// Equip an ability to an active slot
    /// </summary>
    public bool EquipAbility(string abilityId)
    {
        if (!Abilities.ContainsKey(abilityId))
        {
            return false;
        }
        
        if (EquippedAbilityIds.Contains(abilityId))
        {
            return false; // Already equipped
        }
        
        if (EquippedAbilityIds.Count >= MaxEquippedAbilities)
        {
            return false; // No slots available
        }
        
        EquippedAbilityIds.Add(abilityId);
        return true;
    }
    
    /// <summary>
    /// Unequip an ability
    /// </summary>
    public bool UnequipAbility(string abilityId)
    {
        return EquippedAbilityIds.Remove(abilityId);
    }
    
    /// <summary>
    /// Use an equipped ability
    /// </summary>
    public bool UseAbility(string abilityId, float availableEnergy)
    {
        if (!Abilities.ContainsKey(abilityId))
        {
            return false;
        }
        
        if (!EquippedAbilityIds.Contains(abilityId))
        {
            return false; // Not equipped
        }
        
        var ability = Abilities[abilityId];
        
        if (!ability.IsReady())
        {
            return false; // On cooldown
        }
        
        if (availableEnergy < ability.EnergyCost)
        {
            return false; // Not enough energy
        }
        
        ability.Use();
        return true;
    }
    
    /// <summary>
    /// Get all equipped abilities
    /// </summary>
    public List<PodAbility> GetEquippedAbilities()
    {
        return EquippedAbilityIds
            .Where(id => Abilities.ContainsKey(id))
            .Select(id => Abilities[id])
            .ToList();
    }
    
    /// <summary>
    /// Get abilities by type
    /// </summary>
    public List<PodAbility> GetAbilitiesByType(AbilityType type)
    {
        return Abilities.Values.Where(a => a.Type == type).ToList();
    }
    
    /// <summary>
    /// Update all abilities (call each frame)
    /// </summary>
    public void Update()
    {
        foreach (var ability in Abilities.Values)
        {
            ability.Update();
        }
    }
    
    /// <summary>
    /// Serialize the component
    /// </summary>
    public Dictionary<string, object> Serialize()
    {
        var abilitiesData = new List<Dictionary<string, object>>();
        foreach (var ability in Abilities.Values)
        {
            abilitiesData.Add(new Dictionary<string, object>
            {
                ["Id"] = ability.Id,
                ["LastUsed"] = ability.LastUsed.ToString("o"),
                ["IsActive"] = ability.IsActive,
                ["ActiveUntil"] = ability.ActiveUntil.ToString("o")
            });
        }
        
        return new Dictionary<string, object>
        {
            ["EntityId"] = EntityId.ToString(),
            ["EquippedAbilityIds"] = EquippedAbilityIds.ToList(),
            ["MaxEquippedAbilities"] = MaxEquippedAbilities,
            ["Abilities"] = abilitiesData
        };
    }
    
    /// <summary>
    /// Deserialize the component
    /// </summary>
    public void Deserialize(Dictionary<string, object> data)
    {
        EntityId = Guid.Parse(SerializationHelper.GetValue(data, "EntityId", Guid.Empty.ToString()));
        MaxEquippedAbilities = SerializationHelper.GetValue(data, "MaxEquippedAbilities", 4);
        
        EquippedAbilityIds.Clear();
        if (data.ContainsKey("EquippedAbilityIds"))
        {
            var idsData = data["EquippedAbilityIds"];
            if (idsData is JsonElement jsonElement && jsonElement.ValueKind == JsonValueKind.Array)
            {
                foreach (var idElement in jsonElement.EnumerateArray())
                {
                    var id = idElement.GetString();
                    if (!string.IsNullOrEmpty(id))
                    {
                        EquippedAbilityIds.Add(id);
                    }
                }
            }
        }
        
        if (data.ContainsKey("Abilities"))
        {
            var abilitiesData = data["Abilities"];
            if (abilitiesData is JsonElement jsonElement && jsonElement.ValueKind == JsonValueKind.Array)
            {
                foreach (var abilityElement in jsonElement.EnumerateArray())
                {
                    var id = abilityElement.GetProperty("Id").GetString() ?? "";
                    if (Abilities.ContainsKey(id))
                    {
                        var ability = Abilities[id];
                        
                        // Parse LastUsed with fallback
                        var lastUsedStr = abilityElement.GetProperty("LastUsed").GetString();
                        if (DateTime.TryParse(lastUsedStr, out var lastUsed))
                        {
                            ability.LastUsed = lastUsed;
                        }
                        else
                        {
                            ability.LastUsed = DateTime.MinValue;
                        }
                        
                        ability.IsActive = abilityElement.GetProperty("IsActive").GetBoolean();
                        
                        // Parse ActiveUntil with fallback
                        var activeUntilStr = abilityElement.GetProperty("ActiveUntil").GetString();
                        if (DateTime.TryParse(activeUntilStr, out var activeUntil))
                        {
                            ability.ActiveUntil = activeUntil;
                        }
                        else
                        {
                            ability.ActiveUntil = DateTime.MinValue;
                        }
                    }
                }
            }
        }
    }
}

/// <summary>
/// System for managing pod abilities
/// </summary>
public class PodAbilitySystem
{
    private readonly EntityManager _entityManager;
    
    public PodAbilitySystem(EntityManager entityManager)
    {
        _entityManager = entityManager;
    }
    
    /// <summary>
    /// Update all pod abilities (call each frame)
    /// </summary>
    public void Update()
    {
        var allAbilityComponents = _entityManager.GetAllComponents<PodAbilitiesComponent>();
        
        foreach (var abilitiesComponent in allAbilityComponents)
        {
            abilitiesComponent.Update();
        }
    }
    
    /// <summary>
    /// Use a pod ability
    /// </summary>
    public bool UsePodAbility(Guid podEntityId, string abilityId)
    {
        var abilitiesComponent = _entityManager.GetComponent<PodAbilitiesComponent>(podEntityId);
        var podComponent = _entityManager.GetComponent<PlayerPodComponent>(podEntityId);
        
        if (abilitiesComponent == null || podComponent == null)
        {
            return false;
        }
        
        // Get available energy (simplified - assumes power generation is available energy)
        float availableEnergy = podComponent.GetTotalPowerGeneration();
        
        return abilitiesComponent.UseAbility(abilityId, availableEnergy);
    }
    
    /// <summary>
    /// Get the effect multiplier for a currently active ability
    /// </summary>
    public float GetActiveAbilityMultiplier(Guid podEntityId, AbilityType type)
    {
        var abilitiesComponent = _entityManager.GetComponent<PodAbilitiesComponent>(podEntityId);
        
        if (abilitiesComponent == null)
        {
            return 0f;
        }
        
        float totalMultiplier = 0f;
        
        foreach (var ability in abilitiesComponent.GetEquippedAbilities())
        {
            if (ability.Type == type && ability.IsCurrentlyActive())
            {
                totalMultiplier += ability.EffectValue;
            }
        }
        
        return totalMultiplier;
    }
}
