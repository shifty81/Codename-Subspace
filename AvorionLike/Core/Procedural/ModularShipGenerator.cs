using System.Numerics;
using AvorionLike.Core.Voxel;
using AvorionLike.Core.Logging;

namespace AvorionLike.Core.Procedural;

/// <summary>
/// Ship archetype for modular generation
/// Defines the high-level structure of a ship type
/// </summary>
public class ShipArchetype
{
    public string Name { get; set; } = "";
    public ShipSize Size { get; set; } = ShipSize.Frigate;
    public ShipRole Role { get; set; } = ShipRole.Multipurpose;
    
    /// <summary>
    /// Required module types for this archetype
    /// </summary>
    public List<ModuleType> RequiredModules { get; set; } = new()
    {
        ModuleType.Cockpit,
        ModuleType.CoreHull,
        ModuleType.EngineNacelle
    };
    
    /// <summary>
    /// Optional modules and their probability
    /// </summary>
    public Dictionary<ModuleType, float> OptionalModules { get; set; } = new();
    
    /// <summary>
    /// Preferred styles for this archetype
    /// </summary>
    public List<ModuleStyle> PreferredStyles { get; set; } = new();
    
    /// <summary>
    /// Number of engine nacelles (0 = single main engine)
    /// </summary>
    public int EngineCount { get; set; } = 2;
    
    /// <summary>
    /// Whether ship has wings
    /// </summary>
    public bool HasWings { get; set; } = false;
    
    /// <summary>
    /// Number of weapon mounts
    /// </summary>
    public int WeaponMountCount { get; set; } = 2;
    
    /// <summary>
    /// Size class for main modules
    /// </summary>
    public int MainSizeClass { get; set; } = 2;
}

/// <summary>
/// Modular ship generator that builds ships from pre-defined modules
/// Similar to No Man's Sky's approach of combining ship parts
/// </summary>
public class ModularShipGenerator
{
    private Random _random;
    private readonly Logger _logger = Logger.Instance;
    
    // Pre-defined archetypes
    private static readonly Dictionary<string, ShipArchetype> _archetypes = new();
    
    static ModularShipGenerator()
    {
        InitializeArchetypes();
    }
    
    public ModularShipGenerator(int seed = 0)
    {
        _random = seed == 0 ? new Random() : new Random(seed);
    }
    
    /// <summary>
    /// Initialize ship archetypes
    /// </summary>
    private static void InitializeArchetypes()
    {
        // Fighter archetype
        _archetypes["fighter"] = new ShipArchetype
        {
            Name = "Fighter",
            Size = ShipSize.Fighter,
            Role = ShipRole.Combat,
            RequiredModules = new() { ModuleType.Cockpit, ModuleType.CoreHull, ModuleType.EngineNacelle },
            OptionalModules = new() { { ModuleType.Wing, 0.8f }, { ModuleType.WeaponMount, 0.9f } },
            PreferredStyles = new() { ModuleStyle.Military, ModuleStyle.Sleek },
            EngineCount = 1,
            HasWings = true,
            WeaponMountCount = 2,
            MainSizeClass = 1
        };
        
        // Heavy fighter
        _archetypes["heavy_fighter"] = new ShipArchetype
        {
            Name = "Heavy Fighter",
            Size = ShipSize.Fighter,
            Role = ShipRole.Combat,
            RequiredModules = new() { ModuleType.Cockpit, ModuleType.CoreHull, ModuleType.EngineNacelle },
            OptionalModules = new() { { ModuleType.Wing, 0.9f }, { ModuleType.WeaponMount, 1.0f }, { ModuleType.ShieldEmitter, 0.5f } },
            PreferredStyles = new() { ModuleStyle.Military },
            EngineCount = 2,
            HasWings = true,
            WeaponMountCount = 4,
            MainSizeClass = 1
        };
        
        // Hauler/trader
        _archetypes["hauler"] = new ShipArchetype
        {
            Name = "Hauler",
            Size = ShipSize.Frigate,
            Role = ShipRole.Trading,
            RequiredModules = new() { ModuleType.Cockpit, ModuleType.CoreHull, ModuleType.EngineNacelle, ModuleType.CargoContainer },
            OptionalModules = new() { { ModuleType.CargoContainer, 0.7f }, { ModuleType.SensorArray, 0.3f } },
            PreferredStyles = new() { ModuleStyle.Industrial, ModuleStyle.Civilian },
            EngineCount = 2,
            HasWings = false,
            WeaponMountCount = 1,
            MainSizeClass = 2
        };
        
        // Explorer/shuttle
        _archetypes["explorer"] = new ShipArchetype
        {
            Name = "Explorer",
            Size = ShipSize.Corvette,
            Role = ShipRole.Exploration,
            RequiredModules = new() { ModuleType.Cockpit, ModuleType.CoreHull, ModuleType.EngineNacelle, ModuleType.SensorArray },
            OptionalModules = new() { { ModuleType.Wing, 0.6f }, { ModuleType.Antenna, 0.7f } },
            PreferredStyles = new() { ModuleStyle.Sleek, ModuleStyle.Civilian },
            EngineCount = 1,
            HasWings = true,
            WeaponMountCount = 1,
            MainSizeClass = 1
        };
        
        // Shuttle
        _archetypes["shuttle"] = new ShipArchetype
        {
            Name = "Shuttle",
            Size = ShipSize.Fighter,
            Role = ShipRole.Multipurpose,
            RequiredModules = new() { ModuleType.Cockpit, ModuleType.CoreHull, ModuleType.EngineNacelle },
            OptionalModules = new() { { ModuleType.CargoContainer, 0.4f } },
            PreferredStyles = new() { ModuleStyle.Civilian, ModuleStyle.Industrial },
            EngineCount = 1,
            HasWings = false,
            WeaponMountCount = 0,
            MainSizeClass = 1
        };
        
        // Corvette
        _archetypes["corvette"] = new ShipArchetype
        {
            Name = "Corvette",
            Size = ShipSize.Corvette,
            Role = ShipRole.Combat,
            RequiredModules = new() { ModuleType.Cockpit, ModuleType.CoreHull, ModuleType.MidSection, ModuleType.EngineNacelle },
            OptionalModules = new() { { ModuleType.Wing, 0.7f }, { ModuleType.WeaponMount, 0.9f }, { ModuleType.ShieldEmitter, 0.6f } },
            PreferredStyles = new() { ModuleStyle.Military },
            EngineCount = 2,
            HasWings = true,
            WeaponMountCount = 4,
            MainSizeClass = 2
        };
        
        // Frigate
        _archetypes["frigate"] = new ShipArchetype
        {
            Name = "Frigate",
            Size = ShipSize.Frigate,
            Role = ShipRole.Multipurpose,
            RequiredModules = new() { ModuleType.Cockpit, ModuleType.CoreHull, ModuleType.MidSection, ModuleType.EngineNacelle },
            OptionalModules = new() { { ModuleType.WeaponMount, 0.8f }, { ModuleType.CargoContainer, 0.5f }, { ModuleType.SensorArray, 0.4f } },
            PreferredStyles = new() { ModuleStyle.Military, ModuleStyle.Industrial },
            EngineCount = 2,
            HasWings = false,
            WeaponMountCount = 4,
            MainSizeClass = 2
        };
        
        // Raider (pirate)
        _archetypes["raider"] = new ShipArchetype
        {
            Name = "Raider",
            Size = ShipSize.Corvette,
            Role = ShipRole.Combat,
            RequiredModules = new() { ModuleType.Cockpit, ModuleType.CoreHull, ModuleType.EngineNacelle },
            OptionalModules = new() { { ModuleType.Wing, 0.5f }, { ModuleType.WeaponMount, 0.95f } },
            PreferredStyles = new() { ModuleStyle.Pirate },
            EngineCount = 2,
            HasWings = true,
            WeaponMountCount = 4,
            MainSizeClass = 1
        };
        
        // Miner
        _archetypes["miner"] = new ShipArchetype
        {
            Name = "Miner",
            Size = ShipSize.Frigate,
            Role = ShipRole.Mining,
            RequiredModules = new() { ModuleType.Cockpit, ModuleType.CoreHull, ModuleType.EngineNacelle, ModuleType.CargoContainer },
            OptionalModules = new() { { ModuleType.CargoContainer, 0.8f }, { ModuleType.WeaponMount, 0.6f } },
            PreferredStyles = new() { ModuleStyle.Industrial },
            EngineCount = 2,
            HasWings = false,
            WeaponMountCount = 2,
            MainSizeClass = 2
        };
    }
    
    /// <summary>
    /// Generate a modular ship based on an archetype
    /// </summary>
    public GeneratedShip GenerateModularShip(string archetypeName, ModuleStyle? overrideStyle = null, int? seed = null)
    {
        if (seed.HasValue)
        {
            _random = new Random(seed.Value);
        }
        
        if (!_archetypes.TryGetValue(archetypeName.ToLower(), out var archetype))
        {
            archetype = _archetypes["fighter"]; // Default
        }
        
        _logger.Info("ModularShipGenerator", $"Generating modular {archetype.Name} ship");
        
        // Pick style
        ModuleStyle style = overrideStyle ?? PickRandomStyle(archetype.PreferredStyles);
        
        // Create the ship
        var ship = new GeneratedShip
        {
            Config = new ShipGenerationConfig
            {
                Size = archetype.Size,
                Role = archetype.Role,
                Seed = seed ?? _random.Next()
            }
        };
        
        // Track placed modules and their attachment points
        var placedModules = new List<ShipModule>();
        var availableAttachments = new List<(ShipModule module, AttachmentPoint point)>();
        
        // Material based on style
        string material = GetMaterialForStyle(style);
        
        // Step 1: Create core hull first (this is the root)
        var coreHull = ModuleFactory.CreateCoreHull(style, archetype.MainSizeClass, material);
        PlaceModule(ship, coreHull, Vector3.Zero, Quaternion.Identity);
        placedModules.Add(coreHull);
        UpdateAvailableAttachments(coreHull, availableAttachments);
        
        // Step 2: Attach cockpit to front
        var cockpit = ModuleFactory.CreateCockpit(style, material);
        var frontAttachment = coreHull.AttachmentPoints.FirstOrDefault(ap => ap.Id == "front");
        if (frontAttachment != null)
        {
            AttachModule(ship, cockpit, "rear", frontAttachment, placedModules, availableAttachments);
        }
        
        // Step 3: Attach engines to rear
        var rearAttachment = coreHull.AttachmentPoints.FirstOrDefault(ap => ap.Id == "rear");
        if (rearAttachment != null)
        {
            if (archetype.EngineCount <= 1)
            {
                // Single main engine
                var engine = ModuleFactory.CreateEngineNacelle(style, archetype.MainSizeClass, material);
                AttachModule(ship, engine, "front", rearAttachment, placedModules, availableAttachments);
            }
            else
            {
                // Multiple engine nacelles
                float spread = 4f * archetype.MainSizeClass;
                for (int i = 0; i < archetype.EngineCount; i++)
                {
                    float xOffset = (i - (archetype.EngineCount - 1) / 2f) * spread;
                    var engine = ModuleFactory.CreateEngineNacelle(style, 1, material);
                    var enginePos = rearAttachment.Position + new Vector3(xOffset, 0, -4);
                    PlaceModule(ship, engine, enginePos, Quaternion.Identity);
                    placedModules.Add(engine);
                    UpdateAvailableAttachments(engine, availableAttachments, enginePos);
                }
            }
        }
        
        // Step 4: Attach wings if applicable
        if (archetype.HasWings)
        {
            var leftWingAp = coreHull.AttachmentPoints.FirstOrDefault(ap => ap.Id == "left_wing");
            var rightWingAp = coreHull.AttachmentPoints.FirstOrDefault(ap => ap.Id == "right_wing");
            
            if (leftWingAp != null)
            {
                var leftWing = ModuleFactory.CreateWing(style, false, material);
                AttachModuleAtPoint(ship, leftWing, leftWingAp.Position, Quaternion.Identity, false, placedModules);
            }
            if (rightWingAp != null)
            {
                var rightWing = ModuleFactory.CreateWing(style, true, material);
                AttachModuleAtPoint(ship, rightWing, rightWingAp.Position, Quaternion.Identity, false, placedModules);
            }
        }
        
        // Step 5: Add weapon mounts
        for (int i = 0; i < archetype.WeaponMountCount; i++)
        {
            var topAp = FindUnoccupiedAttachment(availableAttachments, ModuleType.WeaponMount);
            if (topAp.module != null && topAp.point != null)
            {
                var weapon = ModuleFactory.CreateWeaponMount(style, material);
                var weaponPos = topAp.point.Position + topAp.point.Normal * 2f;
                // Add some spread
                weaponPos.X += (i - archetype.WeaponMountCount / 2f) * 3f;
                PlaceModule(ship, weapon, weaponPos, Quaternion.Identity);
                placedModules.Add(weapon);
                topAp.point.IsOccupied = true;
            }
        }
        
        // Step 6: Add optional modules based on probabilities
        foreach (var optional in archetype.OptionalModules)
        {
            if (_random.NextDouble() < optional.Value)
            {
                var optionalModule = CreateModuleOfType(optional.Key, style, material);
                if (optionalModule != null)
                {
                    var ap = FindUnoccupiedAttachment(availableAttachments, optional.Key);
                    if (ap.module != null && ap.point != null)
                    {
                        var pos = ap.point.Position + ap.point.Normal * 2f;
                        PlaceModule(ship, optionalModule, pos, Quaternion.Identity);
                        placedModules.Add(optionalModule);
                        ap.point.IsOccupied = true;
                    }
                }
            }
        }
        
        // Step 7: Apply color scheme based on style
        ApplyStyleColors(ship, style);
        
        // Step 8: Calculate stats
        CalculateModularShipStats(ship, placedModules);
        
        _logger.Info("ModularShipGenerator", $"Generated {archetype.Name} with {ship.Structure.Blocks.Count} blocks from {placedModules.Count} modules");
        
        return ship;
    }
    
    /// <summary>
    /// Generate a completely random ship by mixing modules
    /// </summary>
    public GeneratedShip GenerateRandomModularShip(int? seed = null)
    {
        if (seed.HasValue)
        {
            _random = new Random(seed.Value);
        }
        
        // Pick random archetype
        var archetypeKeys = _archetypes.Keys.ToList();
        var archetypeName = archetypeKeys[_random.Next(archetypeKeys.Count)];
        
        // Pick random style
        var styles = Enum.GetValues<ModuleStyle>();
        var style = styles[_random.Next(styles.Length)];
        
        return GenerateModularShip(archetypeName, style, seed);
    }
    
    /// <summary>
    /// Get all available archetypes
    /// </summary>
    public static IReadOnlyDictionary<string, ShipArchetype> GetArchetypes() => _archetypes;
    
    // ========== Private helper methods ==========
    
    private ModuleStyle PickRandomStyle(List<ModuleStyle> preferred)
    {
        if (preferred.Count > 0)
        {
            return preferred[_random.Next(preferred.Count)];
        }
        var styles = Enum.GetValues<ModuleStyle>();
        return styles[_random.Next(styles.Length)];
    }
    
    private string GetMaterialForStyle(ModuleStyle style)
    {
        return style switch
        {
            ModuleStyle.Military => "Titanium",
            ModuleStyle.Industrial => "Iron",
            ModuleStyle.Sleek => "Trinium",
            ModuleStyle.Ancient => "Xanion",
            ModuleStyle.Pirate => "Iron",
            _ => "Iron"
        };
    }
    
    private void PlaceModule(GeneratedShip ship, ShipModule module, Vector3 position, Quaternion rotation)
    {
        var transformedModule = module.CreateTransformedCopy(position, rotation);
        foreach (var block in transformedModule.Blocks)
        {
            ship.Structure.AddBlock(block);
        }
    }
    
    private void AttachModule(GeneratedShip ship, ShipModule module, string moduleAttachmentId, 
        AttachmentPoint targetPoint, List<ShipModule> placedModules, 
        List<(ShipModule module, AttachmentPoint point)> availableAttachments)
    {
        // Find the module's attachment point
        var moduleAp = module.AttachmentPoints.FirstOrDefault(ap => ap.Id == moduleAttachmentId);
        if (moduleAp == null) return;
        
        // Calculate position so attachment points meet
        Vector3 position = targetPoint.Position - moduleAp.Position;
        
        // Calculate rotation to align normals (they should face opposite directions)
        var rotation = Quaternion.Identity;
        // For now, use identity rotation - could calculate proper alignment
        
        PlaceModule(ship, module, position, rotation);
        placedModules.Add(module);
        targetPoint.IsOccupied = true;
        
        UpdateAvailableAttachments(module, availableAttachments, position);
    }
    
    private void AttachModuleAtPoint(GeneratedShip ship, ShipModule module, Vector3 position, 
        Quaternion rotation, bool mirror, List<ShipModule> placedModules)
    {
        var transformedModule = module.CreateTransformedCopy(position, rotation, mirror);
        foreach (var block in transformedModule.Blocks)
        {
            ship.Structure.AddBlock(block);
        }
        placedModules.Add(transformedModule);
    }
    
    private void UpdateAvailableAttachments(ShipModule module, 
        List<(ShipModule module, AttachmentPoint point)> availableAttachments, 
        Vector3 offset = default)
    {
        foreach (var ap in module.AttachmentPoints.Where(ap => !ap.IsOccupied))
        {
            var offsetAp = new AttachmentPoint
            {
                Id = ap.Id,
                Position = ap.Position + offset,
                Normal = ap.Normal,
                CompatibleTypes = ap.CompatibleTypes,
                SizeClass = ap.SizeClass,
                IsOccupied = ap.IsOccupied
            };
            availableAttachments.Add((module, offsetAp));
        }
    }
    
    private (ShipModule? module, AttachmentPoint? point) FindUnoccupiedAttachment(
        List<(ShipModule module, AttachmentPoint point)> attachments, ModuleType targetType)
    {
        foreach (var (module, point) in attachments)
        {
            if (!point.IsOccupied && point.CompatibleTypes.Contains(targetType))
            {
                return (module, point);
            }
        }
        return (null, null);
    }
    
    private ShipModule? CreateModuleOfType(ModuleType type, ModuleStyle style, string material)
    {
        return type switch
        {
            ModuleType.Cockpit => ModuleFactory.CreateCockpit(style, material),
            ModuleType.CoreHull => ModuleFactory.CreateCoreHull(style, 1, material),
            ModuleType.EngineNacelle => ModuleFactory.CreateEngineNacelle(style, 1, material),
            ModuleType.Wing => ModuleFactory.CreateWing(style, _random.Next(2) == 0, material),
            ModuleType.CargoContainer => ModuleFactory.CreateCargoContainer(style, 1, material),
            ModuleType.WeaponMount => ModuleFactory.CreateWeaponMount(style, material),
            ModuleType.MidSection => ModuleFactory.CreateMidSection(style, 2, material),
            ModuleType.SensorArray => ModuleFactory.CreateSensorArray(style, material),
            _ => null
        };
    }
    
    private void ApplyStyleColors(GeneratedShip ship, ModuleStyle style)
    {
        uint primary, secondary, accent;
        
        (primary, secondary, accent) = style switch
        {
            ModuleStyle.Military => (0x2F4F4Fu, 0x708090u, 0xFF0000u),
            ModuleStyle.Industrial => (0x4A4A50u, 0x8B7355u, 0xFFA500u),
            ModuleStyle.Sleek => (0xE8E8E8u, 0xC0C0C0u, 0x00CED1u),
            ModuleStyle.Pirate => (0x8B0000u, 0x2F2F2Fu, 0xFF4500u),
            ModuleStyle.Ancient => (0xDAA520u, 0x8B7355u, 0x00FF00u),
            ModuleStyle.Organic => (0x4B0082u, 0x800080u, 0xFF1493u),
            _ => (0x808080u, 0x696969u, 0xC0C0C0u)
        };
        
        foreach (var block in ship.Structure.Blocks)
        {
            if (block.BlockType == BlockType.Hull)
            {
                block.ColorRGB = primary;
            }
            else if (block.BlockType == BlockType.Armor)
            {
                block.ColorRGB = secondary;
            }
            else if (block.BlockType == BlockType.Engine || block.BlockType == BlockType.TurretMount)
            {
                block.ColorRGB = accent;
            }
        }
    }
    
    private void CalculateModularShipStats(GeneratedShip ship, List<ShipModule> modules)
    {
        ship.TotalThrust = 0;
        ship.TotalPowerGeneration = 0;
        ship.TotalShieldCapacity = 0;
        ship.WeaponMountCount = 0;
        ship.CargoBlockCount = 0;
        
        foreach (var module in modules)
        {
            ship.TotalThrust += module.Thrust;
            ship.TotalPowerGeneration += module.PowerGeneration;
            ship.CargoBlockCount += (int)(module.CargoCapacity / 100f);
        }
        
        // Count weapons and cargo from blocks
        foreach (var block in ship.Structure.Blocks)
        {
            ship.TotalThrust += block.ThrustPower;
            ship.TotalPowerGeneration += block.PowerGeneration;
            ship.TotalShieldCapacity += block.ShieldCapacity;
            
            if (block.BlockType == BlockType.TurretMount) ship.WeaponMountCount++;
            if (block.BlockType == BlockType.Cargo) ship.CargoBlockCount++;
        }
        
        ship.Stats["TotalMass"] = ship.TotalMass;
        ship.Stats["TotalThrust"] = ship.TotalThrust;
        ship.Stats["PowerGeneration"] = ship.TotalPowerGeneration;
        ship.Stats["ShieldCapacity"] = ship.TotalShieldCapacity;
        ship.Stats["ModuleCount"] = modules.Count;
    }
}
