using AvorionLike.Core.Logging;
using AvorionLike.Core.Modular;

namespace AvorionLike.Core.Examples;

/// <summary>
/// Test for ship refinement improvements
/// Tests:
/// - Module spacing (no overlapping)
/// - Texture loading for Ulysses
/// - Ship generation with proper positioning
/// </summary>
public class ShipRefinementTest
{
    private readonly Logger _logger = Logger.Instance;
    
    /// <summary>
    /// Test that modules don't overlap
    /// </summary>
    public void TestModuleSpacing()
    {
        _logger.Info("ShipRefinementTest", "=== Testing Module Spacing ===");
        
        var library = new ModuleLibrary();
        library.InitializeBuiltInModules();
        
        var generator = new ModularProceduralShipGenerator(library, seed: 12345);
        
        // Generate a corvette
        var config = new ModularShipConfig
        {
            ShipName = "Test Corvette",
            Size = ShipSize.Corvette,
            Role = ShipRole.Multipurpose,
            Material = "Iron",
            Seed = 12345
        };
        
        var result = generator.GenerateShip(config);
        
        _logger.Info("ShipRefinementTest", $"Generated ship with {result.Ship.Modules.Count} modules");
        
        // Check for overlapping modules
        bool hasOverlap = false;
        for (int i = 0; i < result.Ship.Modules.Count; i++)
        {
            for (int j = i + 1; j < result.Ship.Modules.Count; j++)
            {
                var module1 = result.Ship.Modules[i];
                var module2 = result.Ship.Modules[j];
                
                var def1 = library.GetDefinition(module1.ModuleDefinitionId);
                var def2 = library.GetDefinition(module2.ModuleDefinitionId);
                
                if (def1 == null || def2 == null) continue;
                
                // Simple AABB overlap check
                var min1 = module1.Position - def1.Size / 2f;
                var max1 = module1.Position + def1.Size / 2f;
                var min2 = module2.Position - def2.Size / 2f;
                var max2 = module2.Position + def2.Size / 2f;
                
                bool overlaps = 
                    min1.X < max2.X && max1.X > min2.X &&
                    min1.Y < max2.Y && max1.Y > min2.Y &&
                    min1.Z < max2.Z && max1.Z > min2.Z;
                
                if (overlaps)
                {
                    _logger.Warning("ShipRefinementTest", 
                        $"Module overlap detected: {module1.ModuleDefinitionId} at {module1.Position} " +
                        $"overlaps with {module2.ModuleDefinitionId} at {module2.Position}");
                    hasOverlap = true;
                }
            }
        }
        
        if (!hasOverlap)
        {
            _logger.Info("ShipRefinementTest", "✓ No module overlaps detected - spacing looks good!");
        }
        else
        {
            _logger.Error("ShipRefinementTest", "✗ Module overlaps detected - spacing needs adjustment");
        }
        
        // Log module positions for debugging
        _logger.Info("ShipRefinementTest", "\nModule Positions:");
        foreach (var module in result.Ship.Modules)
        {
            var def = library.GetDefinition(module.ModuleDefinitionId);
            _logger.Info("ShipRefinementTest", 
                $"  {module.ModuleDefinitionId}: pos={module.Position}, size={def?.Size}");
        }
    }
    
    /// <summary>
    /// Test Ulysses model loading
    /// </summary>
    public void TestUlyssesModelLoading()
    {
        _logger.Info("ShipRefinementTest", "\n=== Testing Ulysses Model Loading ===");
        
        // Check if Ulysses model exists
        var (exists, path, format) = UlyssesModelLoader.CheckForUlyssesModel();
        
        if (exists)
        {
            _logger.Info("ShipRefinementTest", $"✓ Ulysses model found: {path} ({format})");
            
            // Try to load the model
            var meshes = UlyssesModelLoader.LoadUlyssesModel();
            
            if (meshes != null && meshes.Count > 0)
            {
                _logger.Info("ShipRefinementTest", $"✓ Ulysses model loaded successfully: {meshes.Count} mesh(es)");
                
                foreach (var mesh in meshes)
                {
                    _logger.Info("ShipRefinementTest", 
                        $"  - {mesh.Name}: {mesh.VertexCount} vertices, {mesh.TriangleCount} triangles");
                }
            }
            else
            {
                _logger.Error("ShipRefinementTest", "✗ Failed to load Ulysses model");
            }
        }
        else
        {
            _logger.Warning("ShipRefinementTest", "✗ Ulysses model not found");
            _logger.Info("ShipRefinementTest", "Expected location: Assets/Models/ships/Ulysses/source/ulysses.blend");
        }
    }
    
    /// <summary>
    /// Test Ulysses ship generation
    /// </summary>
    public void TestUlyssesShipGeneration()
    {
        _logger.Info("ShipRefinementTest", "\n=== Testing Ulysses Ship Generation ===");
        
        try
        {
            var ship = StarterShipFactory.CreateUlyssesStarterShip("Test Player");
            
            _logger.Info("ShipRefinementTest", $"✓ Ulysses ship generated: {ship.Ship.Name}");
            _logger.Info("ShipRefinementTest", $"  Modules: {ship.Ship.Modules.Count}");
            _logger.Info("ShipRefinementTest", $"  Equipment slots: {ship.Equipment.EquipmentSlots.Count}");
            _logger.Info("ShipRefinementTest", $"  Paint: {ship.Paint.Name}");
            _logger.Info("ShipRefinementTest", $"  Stats: Hull={ship.Stats.Hull:F0}, Speed={ship.Stats.Speed:F1}m/s");
        }
        catch (Exception ex)
        {
            _logger.Error("ShipRefinementTest", $"✗ Failed to generate Ulysses ship: {ex.Message}", ex);
        }
    }
    
    /// <summary>
    /// Run all tests
    /// </summary>
    public void RunAllTests()
    {
        _logger.Info("ShipRefinementTest", "╔════════════════════════════════════════╗");
        _logger.Info("ShipRefinementTest", "║   Ship Refinement Test Suite          ║");
        _logger.Info("ShipRefinementTest", "╚════════════════════════════════════════╝");
        
        TestModuleSpacing();
        TestUlyssesModelLoading();
        TestUlyssesShipGeneration();
        
        _logger.Info("ShipRefinementTest", "\n=== All Tests Complete ===");
    }
}
