using System.Numerics;
using AvorionLike.Core.Logging;

namespace AvorionLike.Core.Modular;

/// <summary>
/// Generator for X4-inspired ships with equipment and paint
/// Builds on the modular ship system but with X4 design philosophy
/// </summary>
public class X4ShipGenerator
{
    private readonly ModuleLibrary _library;
    private readonly Logger _logger = Logger.Instance;
    private Random _random;
    
    public X4ShipGenerator(ModuleLibrary library)
    {
        _library = library;
        _random = new Random();
    }
    
    /// <summary>
    /// Generate a complete X4-style ship
    /// </summary>
    public X4GeneratedShip GenerateX4Ship(X4ShipConfig config)
    {
        _random = new Random(config.Seed == 0 ? Environment.TickCount : config.Seed);
        
        _logger.Info("X4ShipGen", $"Generating X4-style {config.ShipClass} ({config.DesignStyle})");
        
        var result = new X4GeneratedShip
        {
            Config = config
        };
        
        // Generate base ship hull using modular system
        var baseConfig = ConvertToModularConfig(config);
        var generator = new ModularProceduralShipGenerator(_library, config.Seed);
        var baseShip = generator.GenerateShip(baseConfig);
        
        result.Ship = baseShip.Ship;
        result.Ship.Name = config.ShipName;
        
        // Apply X4-specific customizations
        ApplyDesignStyleModifications(result.Ship, config);
        
        // Add equipment system
        result.Equipment = GenerateEquipmentSystem(result.Ship, config);
        
        // Apply paint scheme
        result.Paint = GeneratePaintScheme(config);
        
        // Generate stats summary
        result.Stats = GenerateShipStats(result.Ship, result.Equipment);
        
        _logger.Info("X4ShipGen", 
            $"Generated {config.ShipClass} with {result.Equipment.EquipmentSlots.Count} equipment slots");
        
        return result;
    }
    
    /// <summary>
    /// Convert X4 config to modular config
    /// </summary>
    private ModularShipConfig ConvertToModularConfig(X4ShipConfig config)
    {
        var modConfig = new ModularShipConfig
        {
            ShipName = config.ShipName,
            Material = config.Material,
            Seed = config.Seed,
            AddWings = true,
            AddWeapons = false, // We'll handle weapons via equipment
            AddCargo = true,
            AddHyperdrive = true
        };
        
        // Map X4 ship class to modular size
        modConfig.Size = config.ShipClass switch
        {
            X4ShipClass.Fighter_Light => ShipSize.Fighter,
            X4ShipClass.Fighter_Heavy => ShipSize.Fighter,
            X4ShipClass.Miner_Small => ShipSize.Corvette,
            X4ShipClass.Corvette => ShipSize.Corvette,
            X4ShipClass.Frigate => ShipSize.Frigate,
            X4ShipClass.Gunboat => ShipSize.Frigate,
            X4ShipClass.Miner_Medium => ShipSize.Frigate,
            X4ShipClass.Freighter_Medium => ShipSize.Frigate,
            X4ShipClass.Destroyer => ShipSize.Destroyer,
            X4ShipClass.Freighter_Large => ShipSize.Cruiser,
            X4ShipClass.Miner_Large => ShipSize.Cruiser,
            X4ShipClass.Battleship => ShipSize.Battleship,
            X4ShipClass.Carrier => ShipSize.Carrier,
            X4ShipClass.Builder => ShipSize.Cruiser,
            _ => ShipSize.Frigate
        };
        
        // Map to role
        modConfig.Role = config.ShipClass switch
        {
            X4ShipClass.Miner_Small or X4ShipClass.Miner_Medium or X4ShipClass.Miner_Large => ShipRole.Mining,
            X4ShipClass.Freighter_Medium or X4ShipClass.Freighter_Large => ShipRole.Trading,
            X4ShipClass.Gunboat or X4ShipClass.Destroyer or X4ShipClass.Battleship => ShipRole.Combat,
            _ => ShipRole.Multipurpose
        };
        
        return modConfig;
    }
    
    /// <summary>
    /// Apply design style modifications to ship
    /// </summary>
    private void ApplyDesignStyleModifications(ModularShipComponent ship, X4ShipConfig config)
    {
        // Adjust ship stats based on design style
        var statsMultiplier = config.DesignStyle switch
        {
            X4DesignStyle.Balanced => (speed: 1.0f, hull: 1.0f, cargo: 1.0f),
            X4DesignStyle.Aggressive => (speed: 1.3f, hull: 0.8f, cargo: 0.7f),
            X4DesignStyle.Durable => (speed: 0.8f, hull: 1.4f, cargo: 1.3f),
            X4DesignStyle.Sleek => (speed: 1.2f, hull: 0.9f, cargo: 0.9f),
            X4DesignStyle.Advanced => (speed: 1.1f, hull: 1.1f, cargo: 1.0f),
            X4DesignStyle.Alien => (speed: 1.0f, hull: 1.0f, cargo: 1.0f),
            _ => (speed: 1.0f, hull: 1.0f, cargo: 1.0f)
        };
        
        // Apply variant modifications
        var variantMultiplier = config.Variant switch
        {
            X4ShipVariant.Sentinel => (speed: 0.85f, hull: 1.2f, cargo: 1.3f),
            X4ShipVariant.Vanguard => (speed: 1.15f, hull: 0.9f, cargo: 0.8f),
            X4ShipVariant.Military => (speed: 0.95f, hull: 1.1f, cargo: 0.9f),
            _ => (speed: 1.0f, hull: 1.0f, cargo: 1.0f)
        };
        
        // Apply multipliers to ship stats
        ship.AggregatedStats.ThrustPower *= statsMultiplier.speed * variantMultiplier.speed;
        ship.AggregatedStats.MaxSpeed *= statsMultiplier.speed * variantMultiplier.speed;
        
        foreach (var module in ship.Modules)
        {
            module.MaxHealth *= statsMultiplier.hull * variantMultiplier.hull;
            module.Health = module.MaxHealth;
        }
        
        ship.AggregatedStats.CargoCapacity *= statsMultiplier.cargo * variantMultiplier.cargo;
        
        ship.RecalculateStats();
    }
    
    /// <summary>
    /// Generate equipment system for ship
    /// </summary>
    private ShipEquipmentComponent GenerateEquipmentSystem(ModularShipComponent ship, X4ShipConfig config)
    {
        var equipment = new ShipEquipmentComponent
        {
            EntityId = ship.EntityId
        };
        
        // Determine equipment slots based on ship class
        var slotConfig = GetEquipmentSlotConfiguration(config.ShipClass);
        
        // Add primary weapon slots
        for (int i = 0; i < slotConfig.primaryWeapons; i++)
        {
            var slot = new EquipmentSlot
            {
                AllowedType = EquipmentType.PrimaryWeapon,
                Position = new Vector3(i % 2 == 0 ? -1.5f : 1.5f, 0, 2),
                MountName = $"Primary Weapon {i + 1}",
                MaxSize = slotConfig.weaponSize
            };
            equipment.AddSlot(slot);
            
            // Auto-equip basic weapons
            if (slotConfig.autoEquipWeapons)
            {
                var weapon = EquipmentFactory.CreatePulseLaser(1);
                equipment.EquipItem(slot.Id, weapon);
            }
        }
        
        // Add turret slots for larger ships
        for (int i = 0; i < slotConfig.turrets; i++)
        {
            var slot = new EquipmentSlot
            {
                AllowedType = EquipmentType.Turret,
                Position = new Vector3(0, 2, -i * 2),
                MountName = $"Turret {i + 1}",
                MaxSize = slotConfig.turretSize
            };
            equipment.AddSlot(slot);
        }
        
        // Add utility slots
        for (int i = 0; i < slotConfig.utilitySlots; i++)
        {
            var slotType = DetermineUtilitySlotType(config.ShipClass, i);
            var slot = new EquipmentSlot
            {
                AllowedType = slotType,
                Position = new Vector3(0, -1, i * 2),
                MountName = GetUtilitySlotName(slotType, i),
                MaxSize = 1
            };
            equipment.AddSlot(slot);
            
            // Auto-equip utility items based on ship role
            if (slotType == EquipmentType.MiningLaser && 
                (config.ShipClass == X4ShipClass.Miner_Small || 
                 config.ShipClass == X4ShipClass.Miner_Medium ||
                 config.ShipClass == X4ShipClass.Miner_Large))
            {
                var miner = EquipmentFactory.CreateMiningLaser(1);
                equipment.EquipItem(slot.Id, miner);
            }
        }
        
        return equipment;
    }
    
    /// <summary>
    /// Get equipment slot configuration for ship class
    /// </summary>
    private (int primaryWeapons, int turrets, int utilitySlots, int weaponSize, int turretSize, bool autoEquipWeapons) 
        GetEquipmentSlotConfiguration(X4ShipClass shipClass)
    {
        return shipClass switch
        {
            X4ShipClass.Fighter_Light => (2, 0, 1, 1, 0, true),
            X4ShipClass.Fighter_Heavy => (4, 0, 2, 1, 0, true),
            X4ShipClass.Miner_Small => (1, 0, 2, 1, 0, false),
            X4ShipClass.Corvette => (2, 0, 2, 1, 0, true),
            X4ShipClass.Frigate => (2, 2, 3, 1, 2, true),
            X4ShipClass.Gunboat => (4, 2, 2, 2, 2, true),
            X4ShipClass.Miner_Medium => (1, 0, 3, 1, 0, false),
            X4ShipClass.Freighter_Medium => (0, 1, 2, 0, 1, false),
            X4ShipClass.Destroyer => (2, 4, 3, 2, 2, true),
            X4ShipClass.Freighter_Large => (0, 2, 2, 0, 2, false),
            X4ShipClass.Miner_Large => (0, 2, 4, 0, 2, false),
            X4ShipClass.Battleship => (4, 8, 4, 3, 3, true),
            X4ShipClass.Carrier => (0, 6, 5, 0, 3, false),
            X4ShipClass.Builder => (0, 2, 4, 0, 1, false),
            _ => (2, 0, 2, 1, 0, true)
        };
    }
    
    /// <summary>
    /// Determine utility slot type based on ship class
    /// </summary>
    private EquipmentType DetermineUtilitySlotType(X4ShipClass shipClass, int slotIndex)
    {
        if (shipClass == X4ShipClass.Miner_Small || 
            shipClass == X4ShipClass.Miner_Medium || 
            shipClass == X4ShipClass.Miner_Large)
        {
            return EquipmentType.MiningLaser;
        }
        
        return slotIndex % 3 switch
        {
            0 => EquipmentType.Scanner,
            1 => EquipmentType.Shield,
            _ => EquipmentType.CounterMeasure
        };
    }
    
    /// <summary>
    /// Get utility slot name
    /// </summary>
    private string GetUtilitySlotName(EquipmentType type, int index)
    {
        return $"{type} Slot {index + 1}";
    }
    
    /// <summary>
    /// Generate paint scheme based on config
    /// </summary>
    private ShipPaintScheme GeneratePaintScheme(X4ShipConfig config)
    {
        // Use config colors if specified
        if (config.PrimaryColor != (128, 128, 128))
        {
            return new ShipPaintScheme
            {
                Name = "Custom",
                Pattern = "Solid",
                PrimaryColor = config.PrimaryColor,
                SecondaryColor = config.SecondaryColor,
                AccentColor = config.AccentColor,
                Quality = PaintQuality.Basic
            };
        }
        
        // Otherwise select based on design style
        var stylePaint = config.DesignStyle switch
        {
            X4DesignStyle.Balanced => PaintLibrary.GetDefaultPaint(),
            X4DesignStyle.Aggressive => PaintLibrary.CreateCustomPaint("Combat", (180, 30, 30), (120, 20, 20), (255, 50, 50)),
            X4DesignStyle.Durable => PaintLibrary.CreateCustomPaint("Industrial", (100, 100, 80), (70, 70, 60), (150, 150, 120)),
            X4DesignStyle.Sleek => PaintLibrary.CreateCustomPaint("Sleek", (240, 240, 240), (200, 200, 200), (255, 255, 255)),
            X4DesignStyle.Advanced => PaintLibrary.CreateCustomPaint("Advanced", (30, 60, 120), (20, 40, 80), (50, 100, 180)),
            X4DesignStyle.Alien => PaintLibrary.CreateCustomPaint("Alien", (100, 50, 150), (70, 30, 120), (150, 100, 200)),
            _ => PaintLibrary.GetDefaultPaint()
        };
        
        return stylePaint;
    }
    
    /// <summary>
    /// Generate ship stats summary
    /// </summary>
    private X4ShipStats GenerateShipStats(ModularShipComponent ship, ShipEquipmentComponent equipment)
    {
        return new X4ShipStats
        {
            Mass = ship.TotalMass,
            Hull = ship.MaxTotalHealth,
            Shield = ship.AggregatedStats.ShieldCapacity,
            Speed = ship.AggregatedStats.MaxSpeed,
            Thrust = ship.AggregatedStats.ThrustPower,
            Cargo = ship.AggregatedStats.CargoCapacity,
            Power = ship.AggregatedStats.PowerGeneration,
            PowerUsage = ship.AggregatedStats.PowerConsumption + equipment.TotalPowerConsumption,
            WeaponDamage = equipment.GetTotalWeaponDamage(),
            MiningPower = equipment.GetTotalMiningPower(),
            EquipmentSlots = equipment.EquipmentSlots.Count,
            EquippedItems = equipment.EquipmentSlots.Count(s => s.IsOccupied)
        };
    }
}

/// <summary>
/// Result of X4 ship generation
/// </summary>
public class X4GeneratedShip
{
    public ModularShipComponent Ship { get; set; } = new();
    public ShipEquipmentComponent Equipment { get; set; } = new();
    public ShipPaintScheme Paint { get; set; } = new();
    public X4ShipStats Stats { get; set; } = new();
    public X4ShipConfig Config { get; set; } = new();
}

/// <summary>
/// Stats summary for X4 ships
/// </summary>
public class X4ShipStats
{
    public float Mass { get; set; }
    public float Hull { get; set; }
    public float Shield { get; set; }
    public float Speed { get; set; }
    public float Thrust { get; set; }
    public float Cargo { get; set; }
    public float Power { get; set; }
    public float PowerUsage { get; set; }
    public float WeaponDamage { get; set; }
    public float MiningPower { get; set; }
    public int EquipmentSlots { get; set; }
    public int EquippedItems { get; set; }
    
    public override string ToString()
    {
        return $"Mass: {Mass:F0}t | Hull: {Hull:F0} | Shield: {Shield:F0}\n" +
               $"Speed: {Speed:F1} m/s | Thrust: {Thrust:F0}N\n" +
               $"Cargo: {Cargo:F0} units | Power: {Power:F0}/{PowerUsage:F0}W\n" +
               $"Weapons: {WeaponDamage:F0} DPS | Mining: {MiningPower:F0}\n" +
               $"Equipment: {EquippedItems}/{EquipmentSlots} slots";
    }
}
