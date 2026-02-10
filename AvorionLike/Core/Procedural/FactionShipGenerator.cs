using System.Numerics;
using AvorionLike.Core.Faction;
using AvorionLike.Core.Logging;
using AvorionLike.Core.Voxel;

namespace AvorionLike.Core.Procedural;

/// <summary>
/// Bounding box for collision checking between placed modules
/// </summary>
public struct ModuleBounds
{
    public Vector3 Min { get; set; }
    public Vector3 Max { get; set; }

    public ModuleBounds(Vector3 center, Vector3 size)
    {
        Min = center - size * 0.5f;
        Max = center + size * 0.5f;
    }

    /// <summary>
    /// Check if this bounding box overlaps with another
    /// </summary>
    public bool Overlaps(ModuleBounds other)
    {
        return Min.X < other.Max.X && Max.X > other.Min.X &&
               Min.Y < other.Max.Y && Max.Y > other.Min.Y &&
               Min.Z < other.Max.Z && Max.Z > other.Min.Z;
    }
}

/// <summary>
/// Tracks a placed module with its position and bounds for constraint checking
/// </summary>
public class PlacedModule
{
    public ShipModule Module { get; set; } = null!;
    public Vector3 Position { get; set; }
    public ModuleBounds Bounds { get; set; }
}

/// <summary>
/// Configuration for faction-aware procedural ship generation with seed-based determinism,
/// snap-point assembly, constraint checking, and role-based module prioritization.
/// </summary>
public class FactionShipGenerationConfig
{
    public EVEFactionId FactionId { get; set; }
    public ShipSize Size { get; set; } = ShipSize.Frigate;
    public ShipRole Role { get; set; } = ShipRole.Multipurpose;
    public int Seed { get; set; } = 0;

    /// <summary>
    /// When true, extra greeble/detail modules are added for visual complexity
    /// </summary>
    public bool AddGreebles { get; set; } = true;

    /// <summary>
    /// When true, role-based module prioritization is applied
    /// (e.g., more cargo for haulers, more weapons for fighters)
    /// </summary>
    public bool UseRolePrioritization { get; set; } = true;
}

/// <summary>
/// Faction-aware modular ship generator that produces EVE-style ships using:
/// - Seed-based deterministic generation
/// - Snap-point/socket based module assembly
/// - Bounding-box constraint checking to avoid overlapping geometry
/// - Role-based module prioritization
/// - Faction-specific visual styling and component selection
///
/// Assembly Pipeline:
///   1. Hull Assembly — Choose core shape based on faction style
///   2. Component Mounting — Algorithmically attach hardpoints, engines, turrets
///   3. Procedural Painting — Apply faction-specific textures and colors
/// </summary>
public class FactionShipGenerator
{
    private Random _random;
    private readonly Logger _logger = Logger.Instance;

    public FactionShipGenerator(int seed = 0)
    {
        _random = seed == 0 ? new Random() : new Random(seed);
    }

    /// <summary>
    /// Generate a faction-specific ship with full assembly pipeline
    /// </summary>
    public GeneratedShip GenerateFactionShip(FactionShipGenerationConfig config)
    {
        // Use seed for deterministic generation
        _random = config.Seed == 0 ? new Random() : new Random(config.Seed);

        var profile = EVEFactionDefinitions.GetProfile(config.FactionId);
        var style = profile.ShipStyle;
        var moduleStyle = MapFactionToModuleStyle(config.FactionId);
        var material = style.PreferredMaterial;

        _logger.Info("FactionShipGenerator",
            $"Generating {config.Size} {config.Role} ship for {profile.Name} (seed={config.Seed})");

        var ship = new GeneratedShip
        {
            Config = new ShipGenerationConfig
            {
                Size = config.Size,
                Role = config.Role,
                Style = style,
                Seed = config.Seed,
                Material = material
            }
        };

        var placedModules = new List<PlacedModule>();
        var availableAttachments = new List<(ShipModule module, AttachmentPoint point)>();

        // --------------------------------------------------
        // Pipeline Step 1: Hull Assembly
        // --------------------------------------------------
        int sizeClass = GetSizeClass(config.Size);
        var coreHull = ModuleFactory.CreateCoreHull(moduleStyle, sizeClass, material);
        PlaceModuleWithConstraints(ship, coreHull, Vector3.Zero, placedModules);
        CollectAttachments(coreHull, availableAttachments, Vector3.Zero);

        // --------------------------------------------------
        // Pipeline Step 2: Component Mounting
        // --------------------------------------------------

        // 2a. Cockpit at front
        var cockpit = ModuleFactory.CreateCockpit(moduleStyle, material);
        var frontAp = coreHull.AttachmentPoints.FirstOrDefault(a => a.Id == "front");
        if (frontAp != null)
        {
            var cockpitPos = frontAp.Position + frontAp.Normal * 2f;
            PlaceModuleWithConstraints(ship, cockpit, cockpitPos, placedModules);
            frontAp.IsOccupied = true;
        }

        // 2b. Engines at rear
        int engineCount = GetEngineCount(config.Size, config.FactionId);
        var rearAp = coreHull.AttachmentPoints.FirstOrDefault(a => a.Id == "rear");
        if (rearAp != null)
        {
            float spread = 3f * sizeClass;
            for (int i = 0; i < engineCount; i++)
            {
                float xOffset = (i - (engineCount - 1) / 2f) * spread;
                var engine = ModuleFactory.CreateEngineNacelle(moduleStyle, Math.Max(1, sizeClass - 1), material);
                var enginePos = rearAp.Position + new Vector3(xOffset, 0, -3f);

                if (!CheckCollision(enginePos, engine.Size, placedModules))
                {
                    PlaceModuleWithConstraints(ship, engine, enginePos, placedModules);
                    CollectAttachments(engine, availableAttachments, enginePos);
                }
            }
            rearAp.IsOccupied = true;
        }

        // 2c. Wings (faction-dependent — Sanctum gets symmetrical, Rust gets asymmetrical)
        bool hasWings = style.Sleekness > 0.5f || config.Role == ShipRole.Combat;
        if (hasWings)
        {
            var leftWingAp = coreHull.AttachmentPoints.FirstOrDefault(a => a.Id == "left_wing");
            var rightWingAp = coreHull.AttachmentPoints.FirstOrDefault(a => a.Id == "right_wing");

            if (leftWingAp != null)
            {
                var leftWing = ModuleFactory.CreateWing(moduleStyle, false, material);
                PlaceModuleWithConstraints(ship, leftWing, leftWingAp.Position, placedModules);
                leftWingAp.IsOccupied = true;
            }
            if (rightWingAp != null)
            {
                // For asymmetrical factions (Rust-Scrap), vary the right wing
                bool mirror = style.SymmetryLevel > 0.5f;
                var rightWing = ModuleFactory.CreateWing(moduleStyle, mirror, material);
                PlaceModuleWithConstraints(ship, rightWing, rightWingAp.Position, placedModules);
                rightWingAp.IsOccupied = true;
            }
        }

        // 2d. Role-based module prioritization
        if (config.UseRolePrioritization)
        {
            var rolePlan = GetRoleModulePlan(config.Role, config.FactionId, sizeClass);
            foreach (var (moduleType, count) in rolePlan)
            {
                for (int i = 0; i < count; i++)
                {
                    var module = CreateModuleOfType(moduleType, moduleStyle, material);
                    if (module == null) continue;

                    var ap = FindUnoccupiedAttachment(availableAttachments, moduleType);
                    if (ap.point != null)
                    {
                        var pos = ap.point.Position + ap.point.Normal * 2f;
                        pos.X += (i - count / 2f) * 2.5f;

                        if (!CheckCollision(pos, module.Size, placedModules))
                        {
                            PlaceModuleWithConstraints(ship, module, pos, placedModules);
                            ap.point.IsOccupied = true;
                        }
                    }
                }
            }
        }

        // 2e. Greebles / detail modules
        if (config.AddGreebles)
        {
            int greebleCount = _random.Next(1, 3 + sizeClass);
            for (int i = 0; i < greebleCount; i++)
            {
                var greeble = ModuleFactory.CreateSensorArray(moduleStyle, material);
                var ap = FindUnoccupiedAttachment(availableAttachments, ModuleType.SensorArray);
                if (ap.point != null)
                {
                    var pos = ap.point.Position + ap.point.Normal * 1.5f;
                    if (!CheckCollision(pos, greeble.Size, placedModules))
                    {
                        PlaceModuleWithConstraints(ship, greeble, pos, placedModules);
                        ap.point.IsOccupied = true;
                    }
                }
            }
        }

        // --------------------------------------------------
        // Pipeline Step 3: Procedural Painting
        // --------------------------------------------------
        ApplyFactionColors(ship, style);

        // Calculate final stats
        CalculateShipStats(ship, placedModules);

        _logger.Info("FactionShipGenerator",
            $"Generated {profile.Name} {config.Size} ship: " +
            $"{ship.Structure.Blocks.Count} blocks, {placedModules.Count} modules");

        return ship;
    }

    /// <summary>
    /// Generate a starter ship for a new character based on their faction
    /// </summary>
    public GeneratedShip GenerateStarterShip(EVEFactionId factionId, int seed = 0)
    {
        var profile = EVEFactionDefinitions.GetProfile(factionId);
        var starterSize = factionId switch
        {
            EVEFactionId.RustScrapCoalition => ShipSize.Fighter,
            _ => ShipSize.Corvette
        };

        return GenerateFactionShip(new FactionShipGenerationConfig
        {
            FactionId = factionId,
            Size = starterSize,
            Role = ShipRole.Multipurpose,
            Seed = seed,
            AddGreebles = true,
            UseRolePrioritization = true
        });
    }

    // ========== Constraint Checking ==========

    /// <summary>
    /// Check if a newly placed module would collide with existing modules.
    /// Compares bounding boxes and returns true if overlap is detected.
    /// </summary>
    public static bool CheckCollision(Vector3 position, Vector3 moduleSize, List<PlacedModule> existingModules)
    {
        var newBounds = new ModuleBounds(position, moduleSize);

        foreach (var existing in existingModules)
        {
            if (newBounds.Overlaps(existing.Bounds))
            {
                return true;
            }
        }

        return false;
    }

    // ========== Private helpers ==========

    private void PlaceModuleWithConstraints(GeneratedShip ship, ShipModule module,
        Vector3 position, List<PlacedModule> placedModules)
    {
        var transformed = module.CreateTransformedCopy(position, Quaternion.Identity);
        foreach (var block in transformed.Blocks)
        {
            ship.Structure.AddBlock(block);
        }

        placedModules.Add(new PlacedModule
        {
            Module = module,
            Position = position,
            Bounds = new ModuleBounds(position, module.Size)
        });
    }

    private void CollectAttachments(ShipModule module,
        List<(ShipModule module, AttachmentPoint point)> attachments, Vector3 offset)
    {
        foreach (var ap in module.AttachmentPoints.Where(a => !a.IsOccupied))
        {
            var offsetAp = new AttachmentPoint
            {
                Id = ap.Id,
                Position = ap.Position + offset,
                Normal = ap.Normal,
                CompatibleTypes = new List<ModuleType>(ap.CompatibleTypes),
                SizeClass = ap.SizeClass,
                IsOccupied = ap.IsOccupied
            };
            attachments.Add((module, offsetAp));
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

    private ModuleStyle MapFactionToModuleStyle(EVEFactionId factionId)
    {
        return factionId switch
        {
            EVEFactionId.SanctumHegemony => ModuleStyle.Ancient,
            EVEFactionId.CoreNexus => ModuleStyle.Military,
            EVEFactionId.VanguardRepublic => ModuleStyle.Sleek,
            EVEFactionId.RustScrapCoalition => ModuleStyle.Pirate,
            _ => ModuleStyle.Civilian
        };
    }

    private int GetSizeClass(ShipSize size)
    {
        return size switch
        {
            ShipSize.Fighter => 1,
            ShipSize.Corvette => 1,
            ShipSize.Frigate => 2,
            ShipSize.Destroyer => 2,
            ShipSize.Cruiser => 3,
            ShipSize.Battleship => 3,
            ShipSize.Carrier => 4,
            _ => 2
        };
    }

    private int GetEngineCount(ShipSize size, EVEFactionId factionId)
    {
        int baseCount = size switch
        {
            ShipSize.Fighter => 1,
            ShipSize.Corvette => 1,
            ShipSize.Frigate => 2,
            ShipSize.Destroyer => 2,
            ShipSize.Cruiser => 3,
            ShipSize.Battleship => 4,
            ShipSize.Carrier => 4,
            _ => 2
        };

        // Rust-Scrap ships are fast — extra engines
        if (factionId == EVEFactionId.RustScrapCoalition)
            baseCount = Math.Min(baseCount + 1, 4);

        return baseCount;
    }

    /// <summary>
    /// Determines extra modules to add based on ship role and faction.
    /// Fighters get more weapons, haulers get more cargo, etc.
    /// </summary>
    private List<(ModuleType type, int count)> GetRoleModulePlan(
        ShipRole role, EVEFactionId factionId, int sizeClass)
    {
        var plan = new List<(ModuleType type, int count)>();

        switch (role)
        {
            case ShipRole.Combat:
                plan.Add((ModuleType.WeaponMount, 2 + sizeClass));
                plan.Add((ModuleType.ShieldEmitter, 1));
                break;
            case ShipRole.Mining:
                plan.Add((ModuleType.WeaponMount, 1 + sizeClass));
                plan.Add((ModuleType.CargoContainer, 2 + sizeClass));
                break;
            case ShipRole.Trading:
                plan.Add((ModuleType.CargoContainer, 3 + sizeClass));
                plan.Add((ModuleType.ShieldEmitter, 1));
                break;
            case ShipRole.Exploration:
                plan.Add((ModuleType.SensorArray, 2));
                plan.Add((ModuleType.WeaponMount, 1));
                break;
            default: // Multipurpose
                plan.Add((ModuleType.WeaponMount, 1 + sizeClass));
                plan.Add((ModuleType.CargoContainer, 1));
                break;
        }

        // Faction-specific additions
        switch (factionId)
        {
            case EVEFactionId.SanctumHegemony:
                // Extra armor focus — no extra modules, but heavy armor in painting step
                break;
            case EVEFactionId.CoreNexus:
                plan.Add((ModuleType.ShieldEmitter, 1));
                break;
            case EVEFactionId.VanguardRepublic:
                plan.Add((ModuleType.SensorArray, 1)); // Drone control
                break;
            case EVEFactionId.RustScrapCoalition:
                plan.Add((ModuleType.WeaponMount, 1)); // Extra projectile mount
                break;
        }

        return plan;
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

    /// <summary>
    /// Apply faction-specific color scheme to all blocks
    /// </summary>
    private void ApplyFactionColors(GeneratedShip ship, FactionShipStyle style)
    {
        foreach (var block in ship.Structure.Blocks)
        {
            if (block.BlockType == BlockType.Hull)
            {
                block.ColorRGB = style.PrimaryColor;
            }
            else if (block.BlockType == BlockType.Armor)
            {
                block.ColorRGB = style.SecondaryColor;
            }
            else if (block.BlockType == BlockType.Engine || block.BlockType == BlockType.TurretMount)
            {
                block.ColorRGB = style.AccentColor;
            }
        }
    }

    private void CalculateShipStats(GeneratedShip ship, List<PlacedModule> placedModules)
    {
        ship.TotalThrust = 0;
        ship.TotalPowerGeneration = 0;
        ship.TotalShieldCapacity = 0;
        ship.WeaponMountCount = 0;
        ship.CargoBlockCount = 0;

        foreach (var pm in placedModules)
        {
            ship.TotalThrust += pm.Module.Thrust;
            ship.TotalPowerGeneration += pm.Module.PowerGeneration;
            ship.CargoBlockCount += (int)(pm.Module.CargoCapacity / 100f);
        }

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
        ship.Stats["ModuleCount"] = placedModules.Count;
    }
}
