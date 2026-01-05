using System.Numerics;

namespace AvorionLike.Core.Modular;

/// <summary>
/// Types of equipment that can be mounted on ships (X4-inspired)
/// </summary>
public enum EquipmentType
{
    // Weapons
    PrimaryWeapon,      // Forward-facing weapons (pulse lasers, bolt repeaters, etc.)
    Turret,             // 360-degree turrets for larger ships
    Missile,            // Missile launchers
    
    // Utility tools
    MiningLaser,        // For asteroid mining
    SalvageBeam,        // For salvaging wrecks
    TractorBeam,        // For cargo/wreck manipulation
    
    // Defensive
    Shield,             // Shield generators
    CounterMeasure,     // Flares, chaff, etc.
    
    // Support
    Scanner,            // Enhanced sensors
    Drone,              // Deployable drones
    RepairBot           // Self-repair systems
}

/// <summary>
/// Equipment slot on a ship
/// </summary>
public class EquipmentSlot
{
    public Guid Id { get; set; } = Guid.NewGuid();
    public EquipmentType AllowedType { get; set; }
    public Vector3 Position { get; set; }
    public Vector3 Rotation { get; set; }
    public string MountName { get; set; } = "";
    public EquipmentItem? EquippedItem { get; set; }
    public bool IsOccupied => EquippedItem != null;
    
    // Size restrictions (small, medium, large)
    public int MaxSize { get; set; } = 1;
}

/// <summary>
/// An equipment item that can be installed on a ship
/// </summary>
public class EquipmentItem
{
    public Guid Id { get; set; } = Guid.NewGuid();
    public string Name { get; set; } = "Unknown Equipment";
    public EquipmentType Type { get; set; }
    public int Size { get; set; } = 1; // 1=small, 2=medium, 3=large
    
    // Stats
    public float Damage { get; set; } = 0;
    public float Range { get; set; } = 0;
    public float FireRate { get; set; } = 0; // Shots per second
    public float PowerConsumption { get; set; } = 0;
    public float HeatGeneration { get; set; } = 0;
    public float MiningPower { get; set; } = 0;
    public float SalvagePower { get; set; } = 0;
    
    // Requirements
    public int TechLevel { get; set; } = 1;
    public float Mass { get; set; } = 100f;
    
    // Visual
    public string ModelPath { get; set; } = "";
    public (int R, int G, int B) Color { get; set; } = (255, 255, 255);
}

/// <summary>
/// Component for ship equipment system (attached to ModularShipComponent entities)
/// </summary>
public class ShipEquipmentComponent
{
    public Guid EntityId { get; set; }
    
    /// <summary>
    /// All equipment slots on this ship
    /// </summary>
    public List<EquipmentSlot> EquipmentSlots { get; set; } = new();
    
    /// <summary>
    /// Current power consumption from all equipment
    /// </summary>
    public float TotalPowerConsumption
    {
        get
        {
            return EquipmentSlots
                .Where(s => s.EquippedItem != null)
                .Sum(s => s.EquippedItem!.PowerConsumption);
        }
    }
    
    /// <summary>
    /// Add an equipment slot to the ship
    /// </summary>
    public void AddSlot(EquipmentSlot slot)
    {
        EquipmentSlots.Add(slot);
    }
    
    /// <summary>
    /// Equip an item in a slot
    /// </summary>
    public bool EquipItem(Guid slotId, EquipmentItem item)
    {
        var slot = EquipmentSlots.FirstOrDefault(s => s.Id == slotId);
        if (slot == null) return false;
        
        // Check if item fits
        if (item.Type != slot.AllowedType) return false;
        if (item.Size > slot.MaxSize) return false;
        
        slot.EquippedItem = item;
        return true;
    }
    
    /// <summary>
    /// Unequip an item from a slot
    /// </summary>
    public EquipmentItem? UnequipItem(Guid slotId)
    {
        var slot = EquipmentSlots.FirstOrDefault(s => s.Id == slotId);
        if (slot == null) return null;
        
        var item = slot.EquippedItem;
        slot.EquippedItem = null;
        return item;
    }
    
    /// <summary>
    /// Get all equipped weapons
    /// </summary>
    public List<EquipmentItem> GetEquippedWeapons()
    {
        return EquipmentSlots
            .Where(s => s.EquippedItem != null && 
                       (s.EquippedItem.Type == EquipmentType.PrimaryWeapon ||
                        s.EquippedItem.Type == EquipmentType.Turret ||
                        s.EquippedItem.Type == EquipmentType.Missile))
            .Select(s => s.EquippedItem!)
            .ToList();
    }
    
    /// <summary>
    /// Get all mining equipment
    /// </summary>
    public List<EquipmentItem> GetMiningEquipment()
    {
        return EquipmentSlots
            .Where(s => s.EquippedItem != null && 
                       s.EquippedItem.Type == EquipmentType.MiningLaser)
            .Select(s => s.EquippedItem!)
            .ToList();
    }
    
    /// <summary>
    /// Get total mining power
    /// </summary>
    public float GetTotalMiningPower()
    {
        return GetMiningEquipment().Sum(e => e.MiningPower);
    }
    
    /// <summary>
    /// Get total weapon damage
    /// </summary>
    public float GetTotalWeaponDamage()
    {
        return GetEquippedWeapons().Sum(e => e.Damage);
    }
}

/// <summary>
/// Factory for creating X4-style equipment items
/// </summary>
public static class EquipmentFactory
{
    /// <summary>
    /// Create a basic pulse laser (primary weapon)
    /// </summary>
    public static EquipmentItem CreatePulseLaser(int tier = 1)
    {
        return new EquipmentItem
        {
            Name = $"Pulse Laser Mk{tier}",
            Type = EquipmentType.PrimaryWeapon,
            Size = 1,
            Damage = 50f * tier,
            Range = 1000f + (200f * tier),
            FireRate = 5f,
            PowerConsumption = 20f * tier,
            HeatGeneration = 15f * tier,
            TechLevel = tier,
            Mass = 50f,
            ModelPath = "equipment/weapons/pulse_laser.obj",
            Color = (100, 150, 255)
        };
    }
    
    /// <summary>
    /// Create a mining laser
    /// </summary>
    public static EquipmentItem CreateMiningLaser(int tier = 1)
    {
        return new EquipmentItem
        {
            Name = $"Mining Laser Mk{tier}",
            Type = EquipmentType.MiningLaser,
            Size = 1,
            MiningPower = 100f * tier,
            Range = 500f + (100f * tier),
            PowerConsumption = 30f * tier,
            HeatGeneration = 20f * tier,
            TechLevel = tier,
            Mass = 75f,
            ModelPath = "equipment/tools/mining_laser.obj",
            Color = (255, 200, 50)
        };
    }
    
    /// <summary>
    /// Create a salvage beam
    /// </summary>
    public static EquipmentItem CreateSalvageBeam(int tier = 1)
    {
        return new EquipmentItem
        {
            Name = $"Salvage Beam Mk{tier}",
            Type = EquipmentType.SalvageBeam,
            Size = 1,
            SalvagePower = 80f * tier,
            Range = 400f + (100f * tier),
            PowerConsumption = 25f * tier,
            HeatGeneration = 10f * tier,
            TechLevel = tier,
            Mass = 60f,
            ModelPath = "equipment/tools/salvage_beam.obj",
            Color = (50, 255, 100)
        };
    }
    
    /// <summary>
    /// Create a beam turret (like X4)
    /// </summary>
    public static EquipmentItem CreateBeamTurret(int tier = 1)
    {
        return new EquipmentItem
        {
            Name = $"Beam Turret Mk{tier}",
            Type = EquipmentType.Turret,
            Size = 2,
            Damage = 40f * tier, // Lower than primary weapons
            Range = 1500f + (300f * tier),
            FireRate = 10f, // Continuous beam
            PowerConsumption = 35f * tier,
            HeatGeneration = 5f * tier, // Low heat
            TechLevel = tier,
            Mass = 200f,
            ModelPath = "equipment/weapons/beam_turret.obj",
            Color = (255, 100, 100)
        };
    }
}
