using System;
using System.Numerics;
using AvorionLike.Core.ECS;
using AvorionLike.Core.Graphics;
using AvorionLike.Core.Physics;
using AvorionLike.Core.Procedural;
using AvorionLike.Core.Voxel;

namespace AvorionLike.Examples;

/// <summary>
/// Demonstrates NPR (Non-Photorealistic Rendering) and PBR rendering modes
/// This example showcases the visual fixes for blocks across all demos
/// 
/// Key features tested:
/// - Edge detection for better block visibility
/// - Ambient occlusion between adjacent blocks
/// - Cel-shading for stylized look
/// - Per-material properties
/// - Hybrid mode combining PBR with NPR techniques
/// </summary>
public class RenderingModeDemo
{
    private readonly EntityManager _entityManager;
    private readonly RenderingConfiguration _config;
    
    public RenderingModeDemo(EntityManager entityManager)
    {
        _entityManager = entityManager;
        _config = RenderingConfiguration.Instance;
    }
    
    /// <summary>
    /// Run the rendering mode demonstration
    /// </summary>
    public void RunDemo()
    {
        Console.WriteLine("\n╔════════════════════════════════════════════════════════════════╗");
        Console.WriteLine("║     NPR / PBR RENDERING MODE DEMONSTRATION                     ║");
        Console.WriteLine("║     Visual Fixes for Voxel Blocks                               ║");
        Console.WriteLine("╚════════════════════════════════════════════════════════════════╝\n");
        
        Console.WriteLine("This demo addresses visual issues on blocks by providing flexible");
        Console.WriteLine("rendering options including NPR (Non-Photorealistic Rendering),");
        Console.WriteLine("PBR (Physically Based Rendering), and Hybrid modes.\n");
        
        // Show current configuration
        PrintCurrentConfiguration();
        
        // Interactive menu
        bool done = false;
        while (!done)
        {
            Console.WriteLine("\n=== RENDERING OPTIONS ===");
            Console.WriteLine("1. Apply PBR Realistic Preset (professional, realistic look)");
            Console.WriteLine("2. Apply NPR Stylized Preset (cel-shading, outlines)");
            Console.WriteLine("3. Apply Hybrid Balanced Preset (RECOMMENDED for voxels)");
            Console.WriteLine("4. Apply Performance Preset (minimal effects)");
            Console.WriteLine("5. Toggle Edge Detection");
            Console.WriteLine("6. Toggle Ambient Occlusion");
            Console.WriteLine("7. Toggle Cel-Shading");
            Console.WriteLine("8. Toggle Block Glow Effects");
            Console.WriteLine("9. Generate Test Ships to View Changes");
            Console.WriteLine("0. Return to Main Menu");
            Console.Write("\nSelect option: ");
            
            var choice = Console.ReadLine();
            
            switch (choice)
            {
                case "1":
                    _config.ApplyPreset(RenderingPreset.RealisticPBR);
                    Console.WriteLine("\n✓ Applied PBR Realistic preset");
                    PrintCurrentConfiguration();
                    break;
                    
                case "2":
                    _config.ApplyPreset(RenderingPreset.StylizedNPR);
                    Console.WriteLine("\n✓ Applied NPR Stylized preset");
                    PrintCurrentConfiguration();
                    break;
                    
                case "3":
                    _config.ApplyPreset(RenderingPreset.HybridBalanced);
                    Console.WriteLine("\n✓ Applied Hybrid Balanced preset (RECOMMENDED)");
                    PrintCurrentConfiguration();
                    break;
                    
                case "4":
                    _config.ApplyPreset(RenderingPreset.Performance);
                    Console.WriteLine("\n✓ Applied Performance preset");
                    PrintCurrentConfiguration();
                    break;
                    
                case "5":
                    _config.EnableEdgeDetection = !_config.EnableEdgeDetection;
                    Console.WriteLine($"\n✓ Edge Detection: {(_config.EnableEdgeDetection ? "ON" : "OFF")}");
                    break;
                    
                case "6":
                    _config.EnableAmbientOcclusion = !_config.EnableAmbientOcclusion;
                    Console.WriteLine($"\n✓ Ambient Occlusion: {(_config.EnableAmbientOcclusion ? "ON" : "OFF")}");
                    break;
                    
                case "7":
                    _config.EnableCelShading = !_config.EnableCelShading;
                    _config.Mode = _config.EnableCelShading ? RenderingMode.NPR : RenderingMode.Hybrid;
                    Console.WriteLine($"\n✓ Cel-Shading: {(_config.EnableCelShading ? "ON" : "OFF")}");
                    break;
                    
                case "8":
                    _config.EnableBlockGlow = !_config.EnableBlockGlow;
                    Console.WriteLine($"\n✓ Block Glow Effects: {(_config.EnableBlockGlow ? "ON" : "OFF")}");
                    break;
                    
                case "9":
                    GenerateTestShips();
                    break;
                    
                case "0":
                    done = true;
                    break;
                    
                default:
                    Console.WriteLine("Invalid option!");
                    break;
            }
        }
    }
    
    /// <summary>
    /// Print the current rendering configuration
    /// </summary>
    private void PrintCurrentConfiguration()
    {
        Console.WriteLine("\n--- Current Rendering Configuration ---");
        Console.WriteLine($"  Rendering Mode: {_config.Mode}");
        Console.WriteLine();
        Console.WriteLine("  NPR Features:");
        Console.WriteLine($"    Edge Detection: {(_config.EnableEdgeDetection ? "ON" : "OFF")} (thickness: {_config.EdgeThickness:F1})");
        Console.WriteLine($"    Cel-Shading: {(_config.EnableCelShading ? "ON" : "OFF")} ({_config.CelShadingBands} bands)");
        Console.WriteLine();
        Console.WriteLine("  PBR Features:");
        Console.WriteLine($"    Ambient Occlusion: {(_config.EnableAmbientOcclusion ? "ON" : "OFF")} (strength: {_config.AmbientOcclusionStrength:F2})");
        Console.WriteLine($"    Per-Material Props: {(_config.EnablePerMaterialProperties ? "ON" : "OFF")}");
        Console.WriteLine($"    Procedural Details: {(_config.EnableProceduralDetails ? "ON" : "OFF")} (strength: {_config.ProceduralDetailStrength:F2})");
        Console.WriteLine();
        Console.WriteLine("  Visual Effects:");
        Console.WriteLine($"    Block Glow: {(_config.EnableBlockGlow ? "ON" : "OFF")} (intensity: {_config.BlockGlowIntensity:F2})");
        Console.WriteLine($"    Rim Lighting: {(_config.EnableRimLighting ? "ON" : "OFF")} (strength: {_config.RimLightingStrength:F2})");
        Console.WriteLine($"    Environment Reflections: {(_config.EnableEnvironmentReflections ? "ON" : "OFF")}");
    }
    
    /// <summary>
    /// Generate test ships to visualize the rendering changes
    /// </summary>
    private void GenerateTestShips()
    {
        Console.WriteLine("\n=== Generating Test Ships ===");
        Console.WriteLine("Creating ships with different block types to test rendering...\n");
        
        var generator = new ProceduralShipGenerator(Environment.TickCount);
        
        // Configuration for test ships
        var configs = new[]
        {
            new { Name = "Hull Test Ship", Size = ShipSize.Corvette, Role = ShipRole.Multipurpose, Material = "Titanium" },
            new { Name = "Engine Test Ship", Size = ShipSize.Fighter, Role = ShipRole.Combat, Material = "Ogonite" },
            new { Name = "Shield Test Ship", Size = ShipSize.Frigate, Role = ShipRole.Combat, Material = "Naonite" },
            new { Name = "Generator Test Ship", Size = ShipSize.Corvette, Role = ShipRole.Trading, Material = "Xanion" }
        };
        
        int index = 0;
        foreach (var cfg in configs)
        {
            var shipConfig = new ShipGenerationConfig
            {
                Size = cfg.Size,
                Role = cfg.Role,
                Material = cfg.Material,
                Style = FactionShipStyle.GetDefaultStyle("Military"),
                Seed = Environment.TickCount + index
            };
            
            var generatedShip = generator.GenerateShip(shipConfig);
            var entity = _entityManager.CreateEntity(cfg.Name);
            
            _entityManager.AddComponent(entity.Id, generatedShip.Structure);
            
            var physics = new PhysicsComponent
            {
                Position = new Vector3(index * 50 - 75, 0, 0),
                Velocity = Vector3.Zero,
                Mass = generatedShip.TotalMass
            };
            _entityManager.AddComponent(entity.Id, physics);
            
            Console.WriteLine($"  ✓ Created '{cfg.Name}':");
            Console.WriteLine($"      Position: ({physics.Position.X}, {physics.Position.Y}, {physics.Position.Z})");
            Console.WriteLine($"      Blocks: {generatedShip.Structure.Blocks.Count}");
            Console.WriteLine($"      Material: {cfg.Material}");
            
            // Count block types
            int hullCount = generatedShip.Structure.Blocks.Count(b => b.BlockType == BlockType.Hull);
            int engineCount = generatedShip.Structure.Blocks.Count(b => b.BlockType == BlockType.Engine);
            int shieldCount = generatedShip.Structure.Blocks.Count(b => b.BlockType == BlockType.ShieldGenerator);
            int generatorCount = generatedShip.Structure.Blocks.Count(b => b.BlockType == BlockType.Generator);
            
            Console.WriteLine($"      Block Types: Hull={hullCount}, Engine={engineCount}, Shield={shieldCount}, Generator={generatorCount}");
            Console.WriteLine();
            
            index++;
        }
        
        Console.WriteLine("✓ All test ships created!");
        Console.WriteLine("\nRun the 3D Graphics Demo or New Game to see the rendering effects.");
        Console.WriteLine("The current rendering configuration will be applied to all block rendering.");
    }
    
    /// <summary>
    /// Display information about the rendering modes
    /// </summary>
    public static void DisplayRenderingModeInfo()
    {
        Console.WriteLine("\n╔════════════════════════════════════════════════════════════════╗");
        Console.WriteLine("║     RENDERING MODE INFORMATION                                  ║");
        Console.WriteLine("╚════════════════════════════════════════════════════════════════╝\n");
        
        Console.WriteLine("=== PBR (Physically Based Rendering) ===");
        Console.WriteLine("  • Realistic lighting and materials");
        Console.WriteLine("  • Metallic/roughness workflow");
        Console.WriteLine("  • Environment reflections");
        Console.WriteLine("  • Best for: Professional, realistic games");
        Console.WriteLine();
        
        Console.WriteLine("=== NPR (Non-Photorealistic Rendering) ===");
        Console.WriteLine("  • Stylized cel-shading with discrete light bands");
        Console.WriteLine("  • Edge detection for cartoon outlines");
        Console.WriteLine("  • Artistic expression over physical accuracy");
        Console.WriteLine("  • Best for: Games with unique visual style (comic/anime)");
        Console.WriteLine();
        
        Console.WriteLine("=== Hybrid Mode (RECOMMENDED for Voxel Games) ===");
        Console.WriteLine("  • Combines PBR lighting with NPR edge detection");
        Console.WriteLine("  • Better block visibility through subtle outlines");
        Console.WriteLine("  • Ambient occlusion between adjacent blocks");
        Console.WriteLine("  • Maintains realistic materials while improving readability");
        Console.WriteLine("  • Best for: Voxel-based games with modular components");
        Console.WriteLine();
        
        Console.WriteLine("=== Key Technical Features ===");
        Console.WriteLine("  ✓ Edge Detection: Highlights block boundaries");
        Console.WriteLine("  ✓ Ambient Occlusion: Adds depth between blocks");
        Console.WriteLine("  ✓ Cel-Shading: Quantizes lighting to bands");
        Console.WriteLine("  ✓ Per-Material Properties: Different blocks look different");
        Console.WriteLine("  ✓ Block Glow: Functional blocks emit light");
        Console.WriteLine("  ✓ Rim Lighting: Dramatic edge highlights");
    }
}
