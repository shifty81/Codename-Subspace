using System.Numerics;
using AvorionLike.Core.Modular;
using AvorionLike.Core.Logging;

namespace AvorionLike.Examples;

/// <summary>
/// Test and demonstration of the enhanced small ship modules
/// Builds several small fighter craft using the new detailed module variants
/// </summary>
public class SmallShipModulesTest
{
    private readonly ModuleLibrary _moduleLibrary;
    private readonly Logger _logger = Logger.Instance;
    
    public class TestShipDisplay
    {
        public string Name { get; set; } = "";
        public ModularShipComponent Ship { get; set; } = null!;
        public Vector3 Position { get; set; }
        public string Description { get; set; } = "";
        public int ModuleCount { get; set; }
    }
    
    public SmallShipModulesTest()
    {
        _moduleLibrary = new ModuleLibrary();
        _moduleLibrary.InitializeBuiltInModules();
    }
    
    /// <summary>
    /// Test the new small ship modules by building various fighter configurations
    /// </summary>
    public List<TestShipDisplay> BuildTestFighters()
    {
        _logger.Info("SmallShipTest", "=== SMALL SHIP MODULES TEST ===");
        _logger.Info("SmallShipTest", "Building test fighters with enhanced detail modules...\n");
        
        var testShips = new List<TestShipDisplay>();
        
        // Test 1: Basic fighter (cockpit + hull + engine)
        testShips.Add(BuildBasicFighter(new Vector3(-30, 0, 0)));
        
        // Test 2: Fighter with wings
        testShips.Add(BuildWingedFighter(new Vector3(-10, 0, 0)));
        
        // Test 3: Fighter with wings and thrusters
        testShips.Add(BuildFullFighter(new Vector3(10, 0, 0)));
        
        // Test 4: Multi-engine fighter
        testShips.Add(BuildHeavyFighter(new Vector3(30, 0, 0)));
        
        // Display results
        _logger.Info("SmallShipTest", $"\nSuccessfully built {testShips.Count} test fighters:");
        foreach (var ship in testShips)
        {
            _logger.Info("SmallShipTest", $"  {ship.Name}: {ship.ModuleCount} modules - {ship.Description}");
            _logger.Info("SmallShipTest", $"    Position: {ship.Position}");
            _logger.Info("SmallShipTest", $"    Mass: {ship.Ship.TotalMass:F1} kg");
            _logger.Info("SmallShipTest", $"    Thrust: {ship.Ship.AggregatedStats.ThrustPower:F0} N");
            _logger.Info("SmallShipTest", $"    Health: {ship.Ship.Modules.Sum(m => m.Health):F0} HP");
            _logger.Info("SmallShipTest", "");
        }
        
        return testShips;
    }
    
    /// <summary>
    /// Build a basic fighter: cockpit + hull + engine
    /// </summary>
    private TestShipDisplay BuildBasicFighter(Vector3 position)
    {
        var ship = new ModularShipComponent
        {
            Name = "Basic Fighter"
        };
        
        // Add cockpit (core module)
        var cockpit = CreateModule("cockpit_small", Vector3.Zero, "Titanium");
        ship.AddModule(cockpit);
        ship.CoreModuleId = cockpit.Id;
        
        // Add hull section behind cockpit
        var hull = CreateModule("hull_section_small", new Vector3(0, 0, -6), "Titanium");
        ship.AddModule(hull);
        
        // Add engine at rear
        var engine = CreateModule("engine_small", new Vector3(0, 0, -10), "Titanium");
        ship.AddModule(engine);
        
        return new TestShipDisplay
        {
            Name = "Basic Fighter",
            Ship = ship,
            Position = position,
            Description = "Minimal fighter configuration",
            ModuleCount = ship.Modules.Count
        };
    }
    
    /// <summary>
    /// Build a winged fighter: cockpit + hull + wings + engine
    /// </summary>
    private TestShipDisplay BuildWingedFighter(Vector3 position)
    {
        var ship = new ModularShipComponent
        {
            Name = "Winged Fighter"
        };
        
        // Add cockpit
        var cockpit = CreateModule("cockpit_small", Vector3.Zero, "Titanium");
        ship.AddModule(cockpit);
        ship.CoreModuleId = cockpit.Id;
        
        // Add hull section
        var hull = CreateModule("hull_section_small", new Vector3(0, 0, -6), "Titanium");
        ship.AddModule(hull);
        
        // Add wings (left and right)
        var wingLeft = CreateModule("wing_small_left", new Vector3(-2, 0, -6), "Titanium");
        ship.AddModule(wingLeft);
        
        var wingRight = CreateModule("wing_small_right", new Vector3(2, 0, -6), "Titanium");
        ship.AddModule(wingRight);
        
        // Add engine
        var engine = CreateModule("engine_small", new Vector3(0, 0, -10), "Titanium");
        ship.AddModule(engine);
        
        return new TestShipDisplay
        {
            Name = "Winged Fighter",
            Ship = ship,
            Position = position,
            Description = "Fighter with wing modules",
            ModuleCount = ship.Modules.Count
        };
    }
    
    /// <summary>
    /// Build a full fighter: cockpit + hull + wings + engine + thrusters
    /// </summary>
    private TestShipDisplay BuildFullFighter(Vector3 position)
    {
        var ship = new ModularShipComponent
        {
            Name = "Full Fighter"
        };
        
        // Add cockpit
        var cockpit = CreateModule("cockpit_small", Vector3.Zero, "Naonite");
        ship.AddModule(cockpit);
        ship.CoreModuleId = cockpit.Id;
        
        // Add hull section
        var hull = CreateModule("hull_section_small", new Vector3(0, 0, -6), "Naonite");
        ship.AddModule(hull);
        
        // Add wings
        var wingLeft = CreateModule("wing_small_left", new Vector3(-2, 0, -6), "Naonite");
        ship.AddModule(wingLeft);
        
        var wingRight = CreateModule("wing_small_right", new Vector3(2, 0, -6), "Naonite");
        ship.AddModule(wingRight);
        
        // Add maneuvering thrusters
        var thrusterTop = CreateModule("thruster_small", new Vector3(0, 1.8f, -6), "Naonite");
        ship.AddModule(thrusterTop);
        
        var thrusterBottom = CreateModule("thruster_small", new Vector3(0, -1.8f, -6), "Naonite");
        ship.AddModule(thrusterBottom);
        
        // Add main engine
        var engine = CreateModule("engine_small", new Vector3(0, 0, -10), "Naonite");
        ship.AddModule(engine);
        
        return new TestShipDisplay
        {
            Name = "Full Fighter",
            Ship = ship,
            Position = position,
            Description = "Complete fighter with thrusters",
            ModuleCount = ship.Modules.Count
        };
    }
    
    /// <summary>
    /// Build a heavy fighter: cockpit + 2 hulls + wings + 2 engines + thrusters
    /// </summary>
    private TestShipDisplay BuildHeavyFighter(Vector3 position)
    {
        var ship = new ModularShipComponent
        {
            Name = "Heavy Fighter"
        };
        
        // Add cockpit
        var cockpit = CreateModule("cockpit_small", Vector3.Zero, "Trinium");
        ship.AddModule(cockpit);
        ship.CoreModuleId = cockpit.Id;
        
        // Add two hull sections
        var hull1 = CreateModule("hull_section_small", new Vector3(0, 0, -6), "Trinium");
        ship.AddModule(hull1);
        
        var hull2 = CreateModule("hull_section_small", new Vector3(0, 0, -12), "Trinium");
        ship.AddModule(hull2);
        
        // Add wings
        var wingLeft = CreateModule("wing_small_left", new Vector3(-2, 0, -6), "Trinium");
        ship.AddModule(wingLeft);
        
        var wingRight = CreateModule("wing_small_right", new Vector3(2, 0, -6), "Trinium");
        ship.AddModule(wingRight);
        
        // Add thrusters
        var thrusterTop = CreateModule("thruster_small", new Vector3(0, 1.8f, -9), "Trinium");
        ship.AddModule(thrusterTop);
        
        var thrusterBottom = CreateModule("thruster_small", new Vector3(0, -1.8f, -9), "Trinium");
        ship.AddModule(thrusterBottom);
        
        var thrusterLeft = CreateModule("thruster_small", new Vector3(-2.2f, 0, -9), "Trinium");
        ship.AddModule(thrusterLeft);
        
        var thrusterRight = CreateModule("thruster_small", new Vector3(2.2f, 0, -9), "Trinium");
        ship.AddModule(thrusterRight);
        
        // Add dual engines
        var engineLeft = CreateModule("engine_small", new Vector3(-1, 0, -16), "Trinium");
        ship.AddModule(engineLeft);
        
        var engineRight = CreateModule("engine_small", new Vector3(1, 0, -16), "Trinium");
        ship.AddModule(engineRight);
        
        return new TestShipDisplay
        {
            Name = "Heavy Fighter",
            Ship = ship,
            Position = position,
            Description = "Heavy fighter with dual engines",
            ModuleCount = ship.Modules.Count
        };
    }
    
    /// <summary>
    /// Create a ship module part from a definition
    /// </summary>
    private ShipModulePart CreateModule(string definitionId, Vector3 position, string materialType)
    {
        var definition = _moduleLibrary.GetDefinition(definitionId);
        if (definition == null)
        {
            throw new Exception($"Module definition not found: {definitionId}");
        }
        
        var module = new ShipModulePart(definitionId, position, materialType)
        {
            MaxHealth = definition.GetHealthForMaterial(materialType),
            Health = definition.GetHealthForMaterial(materialType),
            Mass = definition.GetMassForMaterial(materialType),
            FunctionalStats = definition.GetStatsForMaterial(materialType)
        };
        
        return module;
    }
    
    /// <summary>
    /// Verify all small ship modules are available in the library
    /// </summary>
    public void VerifyModuleAvailability()
    {
        _logger.Info("SmallShipTest", "=== MODULE AVAILABILITY CHECK ===");
        
        var requiredModules = new[]
        {
            "cockpit_small",
            "hull_section_small",
            "engine_small",
            "thruster_small",
            "wing_small_left",
            "wing_small_right"
        };
        
        bool allAvailable = true;
        foreach (var moduleId in requiredModules)
        {
            var definition = _moduleLibrary.GetDefinition(moduleId);
            if (definition != null)
            {
                _logger.Info("SmallShipTest", $"✓ {moduleId}: Available ({definition.Name})");
                _logger.Info("SmallShipTest", $"    Model: {definition.ModelPath}");
                _logger.Info("SmallShipTest", $"    Size: {definition.Size}");
                _logger.Info("SmallShipTest", $"    Mass: {definition.BaseMass} kg");
            }
            else
            {
                _logger.Error("SmallShipTest", $"✗ {moduleId}: NOT FOUND");
                allAvailable = false;
            }
        }
        
        _logger.Info("SmallShipTest", "");
        if (allAvailable)
        {
            _logger.Info("SmallShipTest", "All small ship modules are available!");
        }
        else
        {
            _logger.Error("SmallShipTest", "Some modules are missing!");
        }
        _logger.Info("SmallShipTest", "");
    }
}
