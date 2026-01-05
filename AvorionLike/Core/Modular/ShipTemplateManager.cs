using System.Numerics;
using AvorionLike.Core.Logging;

namespace AvorionLike.Core.Modular;

/// <summary>
/// Manager for predefined ship templates including the Ulysses starter ship
/// </summary>
public class ShipTemplateManager
{
    private readonly Dictionary<string, ShipTemplate> _templates = new();
    private readonly Logger _logger = Logger.Instance;
    
    public ShipTemplateManager()
    {
        InitializeDefaultTemplates();
    }
    
    /// <summary>
    /// Initialize built-in ship templates
    /// </summary>
    private void InitializeDefaultTemplates()
    {
        // Check for Ulysses model and get the correct path
        var (modelExists, modelPath, modelFormat) = UlyssesModelLoader.CheckForUlyssesModel();
        
        _logger.Info("ShipTemplates", modelExists 
            ? $"Ulysses model found: {modelFormat}" 
            : "Ulysses model not found, will use procedural generation");
        
        // Ulysses - The starter ship
        var ulysses = new ShipTemplate
        {
            Id = "ulysses",
            Name = "Ulysses",
            Description = "A reliable starter ship. Multipurpose design suitable for exploration, light combat, and basic trading.",
            ModelPath = modelPath ?? "", // Use found model or empty for procedural
            ShipClass = X4ShipClass.Corvette,
            DesignStyle = X4DesignStyle.Balanced,
            Variant = X4ShipVariant.Standard,
            IsStarterShip = true,
            
            // Base stats
            BaseHull = 1500f,
            BaseMass = 500f,
            BaseSpeed = 80f,
            BaseThrust = 5000f,
            BaseCargo = 50f,
            BasePower = 500f,
            
            // Equipment loadout
            DefaultEquipment = new List<EquipmentLoadout>
            {
                new EquipmentLoadout { SlotName = "Primary 1", Item = EquipmentFactory.CreatePulseLaser(1) },
                new EquipmentLoadout { SlotName = "Primary 2", Item = EquipmentFactory.CreatePulseLaser(1) },
                new EquipmentLoadout { SlotName = "Utility 1", Item = EquipmentFactory.CreateMiningLaser(1) }
            },
            
            // Paint scheme
            DefaultPaint = new ShipPaintScheme
            {
                Name = "Ulysses Standard",
                Pattern = "Solid",
                PrimaryColor = (80, 90, 110),
                SecondaryColor = (60, 70, 85),
                AccentColor = (120, 140, 160),
                GlowColor = (100, 150, 255)
            },
            
            // Equipment slots
            PrimaryWeaponSlots = 2,
            TurretSlots = 0,
            UtilitySlots = 3
        };
        
        _templates[ulysses.Id] = ulysses;
        
        _logger.Info("ShipTemplates", $"Loaded {_templates.Count} ship templates");
    }
    
    /// <summary>
    /// Get a ship template by ID
    /// </summary>
    public ShipTemplate? GetTemplate(string id)
    {
        return _templates.TryGetValue(id, out var template) ? template : null;
    }
    
    /// <summary>
    /// Get the starter ship template
    /// </summary>
    public ShipTemplate? GetStarterShip()
    {
        return _templates.Values.FirstOrDefault(t => t.IsStarterShip);
    }
    
    /// <summary>
    /// Get all available templates
    /// </summary>
    public List<ShipTemplate> GetAllTemplates()
    {
        return _templates.Values.ToList();
    }
    
    /// <summary>
    /// Add a custom template
    /// </summary>
    public void AddTemplate(ShipTemplate template)
    {
        _templates[template.Id] = template;
    }
    
    /// <summary>
    /// Generate a ship from a template
    /// </summary>
    public X4GeneratedShip GenerateFromTemplate(ShipTemplate template, string shipName = "", int seed = 0)
    {
        var config = new X4ShipConfig
        {
            ShipClass = template.ShipClass,
            DesignStyle = template.DesignStyle,
            Variant = template.Variant,
            ShipName = string.IsNullOrEmpty(shipName) ? template.Name : shipName,
            Seed = seed,
            PrimaryWeaponSlots = template.PrimaryWeaponSlots,
            TurretSlots = template.TurretSlots,
            UtilitySlots = template.UtilitySlots
        };
        
        // Use paint from template if provided
        if (template.DefaultPaint != null)
        {
            config.PrimaryColor = template.DefaultPaint.PrimaryColor;
            config.SecondaryColor = template.DefaultPaint.SecondaryColor;
            config.AccentColor = template.DefaultPaint.AccentColor;
        }
        
        // Generate base ship
        var library = new ModuleLibrary();
        library.InitializeBuiltInModules();
        var generator = new X4ShipGenerator(library);
        var ship = generator.GenerateX4Ship(config);
        
        // Apply template-specific model if available
        if (!string.IsNullOrEmpty(template.ModelPath))
        {
            ship.Ship.Name = template.Name;
            // Model path stored for rendering system to use
            _logger.Info("ShipTemplates", $"Generated {template.Name} using model: {template.ModelPath}");
        }
        
        // Apply default equipment from template
        if (template.DefaultEquipment.Count > 0)
        {
            ApplyDefaultEquipment(ship.Equipment, template.DefaultEquipment);
        }
        
        // Override base stats if specified by modifying modules
        if (template.BaseHull > 0)
        {
            // Adjust module health to reach target total hull
            var currentHull = ship.Ship.MaxTotalHealth;
            if (currentHull > 0)
            {
                var multiplier = template.BaseHull / currentHull;
                foreach (var module in ship.Ship.Modules)
                {
                    module.MaxHealth *= multiplier;
                    module.Health = module.MaxHealth;
                }
            }
        }
        
        if (template.BaseMass > 0)
        {
            // Adjust module mass to reach target total mass
            var currentMass = ship.Ship.TotalMass;
            if (currentMass > 0)
            {
                var multiplier = template.BaseMass / currentMass;
                foreach (var module in ship.Ship.Modules)
                {
                    module.Mass *= multiplier;
                }
            }
        }
        
        // Override aggregated stats directly
        if (template.BaseSpeed > 0) ship.Ship.AggregatedStats.MaxSpeed = template.BaseSpeed;
        if (template.BaseThrust > 0) ship.Ship.AggregatedStats.ThrustPower = template.BaseThrust;
        if (template.BaseCargo > 0) ship.Ship.AggregatedStats.CargoCapacity = template.BaseCargo;
        if (template.BasePower > 0) ship.Ship.AggregatedStats.PowerGeneration = template.BasePower;
        
        ship.Ship.RecalculateStats();
        
        return ship;
    }
    
    /// <summary>
    /// Apply default equipment to ship
    /// </summary>
    private void ApplyDefaultEquipment(ShipEquipmentComponent equipment, List<EquipmentLoadout> defaultEquipment)
    {
        foreach (var loadout in defaultEquipment)
        {
            // Find matching slot by name or type
            var slot = equipment.EquipmentSlots.FirstOrDefault(s => 
                s.MountName.Contains(loadout.SlotName, StringComparison.OrdinalIgnoreCase) ||
                s.AllowedType == loadout.Item.Type);
            
            if (slot != null && !slot.IsOccupied)
            {
                equipment.EquipItem(slot.Id, loadout.Item);
            }
        }
    }
}

/// <summary>
/// Ship template definition
/// </summary>
public class ShipTemplate
{
    public string Id { get; set; } = "";
    public string Name { get; set; } = "";
    public string Description { get; set; } = "";
    public string ModelPath { get; set; } = ""; // Path to 3D model file (.blend, .obj, .fbx, etc.)
    
    // Classification
    public X4ShipClass ShipClass { get; set; }
    public X4DesignStyle DesignStyle { get; set; }
    public X4ShipVariant Variant { get; set; }
    public bool IsStarterShip { get; set; } = false;
    
    // Base stats (0 = use generated values)
    public float BaseHull { get; set; } = 0;
    public float BaseMass { get; set; } = 0;
    public float BaseSpeed { get; set; } = 0;
    public float BaseThrust { get; set; } = 0;
    public float BaseCargo { get; set; } = 0;
    public float BasePower { get; set; } = 0;
    
    // Equipment configuration
    public int PrimaryWeaponSlots { get; set; } = 2;
    public int TurretSlots { get; set; } = 0;
    public int UtilitySlots { get; set; } = 2;
    public List<EquipmentLoadout> DefaultEquipment { get; set; } = new();
    
    // Visual customization
    public ShipPaintScheme? DefaultPaint { get; set; }
    
    // Interior configuration
    public bool HasCustomInterior { get; set; } = false;
    public string InteriorLayoutPath { get; set; } = "";
}

/// <summary>
/// Equipment loadout entry for templates
/// </summary>
public class EquipmentLoadout
{
    public string SlotName { get; set; } = "";
    public EquipmentItem Item { get; set; } = new();
}

/// <summary>
/// Helper class for creating starter ships
/// </summary>
public static class StarterShipFactory
{
    /// <summary>
    /// Create the Ulysses starter ship for a new player
    /// </summary>
    public static X4GeneratedShip CreateUlyssesStarterShip(string playerName = "Player")
    {
        var manager = new ShipTemplateManager();
        var template = manager.GetStarterShip();
        
        if (template == null)
        {
            throw new Exception("Starter ship template not found!");
        }
        
        var shipName = $"{playerName}'s {template.Name}";
        return manager.GenerateFromTemplate(template, shipName);
    }
    
    /// <summary>
    /// Create a Ulysses ship with custom name
    /// </summary>
    public static X4GeneratedShip CreateUlysses(string shipName, int seed = 0)
    {
        var manager = new ShipTemplateManager();
        var template = manager.GetTemplate("ulysses");
        
        if (template == null)
        {
            throw new Exception("Ulysses template not found!");
        }
        
        return manager.GenerateFromTemplate(template, shipName, seed);
    }
}
