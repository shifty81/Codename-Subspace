using System;
using System.Numerics;
using AvorionLike.Core.ECS;
using AvorionLike.Core.Graphics;
using AvorionLike.Core.Logging;

namespace AvorionLike.Examples;

/// <summary>
/// Example demonstrating 3D model loading and mesh rendering
/// Shows how to use AssetManager, ModelLoader, and MeshRenderer
/// </summary>
public class MeshRenderingExample
{
    private readonly Logger _logger = Logger.Instance;
    
    public void Run()
    {
        Console.WriteLine("\n=== 3D Model Loading and Mesh Rendering Example ===\n");
        Console.WriteLine("This example demonstrates the new 3D model loading system.");
        Console.WriteLine("Note: This is a code example - actual rendering requires GraphicsWindow.\n");
        
        // Example 1: Using AssetManager
        Console.WriteLine("Example 1: AssetManager");
        Console.WriteLine("========================");
        DemonstrateAssetManager();
        
        Console.WriteLine("\n");
        
        // Example 2: Creating placeholder meshes
        Console.WriteLine("Example 2: Placeholder Meshes");
        Console.WriteLine("=============================");
        DemonstratePlaceholderMeshes();
        
        Console.WriteLine("\n");
        
        // Example 3: MeshRenderer usage (conceptual)
        Console.WriteLine("Example 3: MeshRenderer Integration");
        Console.WriteLine("===================================");
        DemonstrateMeshRendererUsage();
        
        Console.WriteLine("\n=== Example Complete ===\n");
        Console.WriteLine("Next steps:");
        Console.WriteLine("  1. Place 3D model files (OBJ, FBX, GLTF) in Assets/Models/");
        Console.WriteLine("  2. Update ModuleLibrary to reference actual model files");
        Console.WriteLine("  3. Integrate MeshRenderer with GraphicsWindow");
        Console.WriteLine("  4. Replace VoxelRenderer calls with MeshRenderer for modular ships");
    }
    
    private void DemonstrateAssetManager()
    {
        var assetManager = AssetManager.Instance;
        
        Console.WriteLine($"Asset base path: {assetManager.GetAssetPath("")}");
        Console.WriteLine($"Models directory: {assetManager.GetAssetPath("Models")}");
        
        // Check for available models
        var availableModels = assetManager.GetAvailableModels();
        Console.WriteLine($"\nAvailable models: {availableModels.Length}");
        
        if (availableModels.Length > 0)
        {
            Console.WriteLine("Found models:");
            foreach (var model in availableModels)
            {
                Console.WriteLine($"  - {model}");
            }
            
            // Try to load the first model
            try
            {
                Console.WriteLine($"\nAttempting to load: {availableModels[0]}");
                var meshes = assetManager.LoadModel(availableModels[0]);
                Console.WriteLine($"Successfully loaded {meshes.Count} meshes!");
                
                foreach (var mesh in meshes)
                {
                    Console.WriteLine($"  Mesh: {mesh.Name}");
                    Console.WriteLine($"    Vertices: {mesh.VertexCount}");
                    Console.WriteLine($"    Triangles: {mesh.TriangleCount}");
                    var (min, max) = mesh.GetBounds();
                    Console.WriteLine($"    Bounds: Min{min} Max{max}");
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine($"Error loading model: {ex.Message}");
            }
        }
        else
        {
            Console.WriteLine("No models found. Place .obj, .fbx, or .gltf files in Assets/Models/");
        }
        
        // Get cache stats
        var (modelCount, totalMeshes, memory) = assetManager.GetCacheStats();
        Console.WriteLine($"\nCache statistics:");
        Console.WriteLine($"  Models cached: {modelCount}");
        Console.WriteLine($"  Total meshes: {totalMeshes}");
        Console.WriteLine($"  Estimated memory: {memory / 1024.0:F2} KB");
    }
    
    private void DemonstratePlaceholderMeshes()
    {
        var assetManager = AssetManager.Instance;
        
        // Create a placeholder cube
        Console.WriteLine("Creating placeholder cube mesh...");
        var cube = assetManager.CreatePlaceholderCube(2.0f);
        
        Console.WriteLine($"Cube mesh created:");
        Console.WriteLine($"  Name: {cube.Name}");
        Console.WriteLine($"  Vertices: {cube.VertexCount}");
        Console.WriteLine($"  Triangles: {cube.TriangleCount}");
        Console.WriteLine($"  Size: {cube.GetSize()}");
        Console.WriteLine($"  Center: {cube.GetCenter()}");
        
        // Validate the mesh
        if (cube.IsValid(out string errorMessage))
        {
            Console.WriteLine($"  Validation: ✓ Mesh is valid");
        }
        else
        {
            Console.WriteLine($"  Validation: ✗ {errorMessage}");
        }
        
        Console.WriteLine("\nPlaceholder meshes are useful for:");
        Console.WriteLine("  - Testing rendering pipeline");
        Console.WriteLine("  - Fallback when models fail to load");
        Console.WriteLine("  - Debugging and development");
    }
    
    private void DemonstrateMeshRendererUsage()
    {
        Console.WriteLine("MeshRenderer integration (conceptual code):");
        Console.WriteLine();
        Console.WriteLine("// In GraphicsWindow initialization:");
        Console.WriteLine("var meshRenderer = new MeshRenderer(_gl);");
        Console.WriteLine();
        Console.WriteLine("// In rendering loop for modular ships:");
        Console.WriteLine("foreach (var module in ship.Modules)");
        Console.WriteLine("{");
        Console.WriteLine("    // Get mesh from AssetManager");
        Console.WriteLine("    var meshes = AssetManager.Instance.LoadModel(module.Definition.ModelPath);");
        Console.WriteLine("    ");
        Console.WriteLine("    foreach (var mesh in meshes)");
        Console.WriteLine("    {");
        Console.WriteLine("        // Create transform matrix");
        Console.WriteLine("        var transform = CreateModuleTransform(module);");
        Console.WriteLine("        ");
        Console.WriteLine("        // Render the mesh");
        Console.WriteLine("        meshRenderer.RenderMesh(");
        Console.WriteLine("            mesh,");
        Console.WriteLine("            transform,");
        Console.WriteLine("            module.Color,");
        Console.WriteLine("            camera.ViewMatrix,");
        Console.WriteLine("            camera.ProjectionMatrix,");
        Console.WriteLine("            camera.Position");
        Console.WriteLine("        );");
        Console.WriteLine("    }");
        Console.WriteLine("}");
        Console.WriteLine();
        Console.WriteLine("This replaces the current VoxelRenderer cube rendering with");
        Console.WriteLine("actual 3D models for each ship module!");
    }
    
    /// <summary>
    /// Demonstrates how to integrate with ModuleLibrary
    /// </summary>
    public void ShowModuleIntegrationExample()
    {
        Console.WriteLine("\n=== Module Integration Example ===\n");
        Console.WriteLine("To integrate 3D models with the modular ship system:");
        Console.WriteLine();
        Console.WriteLine("1. Update ShipModuleDefinition model paths:");
        Console.WriteLine("   module.ModelPath = \"ships/cockpit_fighter.obj\";");
        Console.WriteLine();
        Console.WriteLine("2. In ModuleLibrary initialization:");
        Console.WriteLine("   var cockpit = new ShipModuleDefinition");
        Console.WriteLine("   {");
        Console.WriteLine("       Id = \"cockpit_basic\",");
        Console.WriteLine("       Name = \"Basic Cockpit\",");
        Console.WriteLine("       ModelPath = \"ships/modules/cockpit_basic.fbx\",  // 3D model!");
        Console.WriteLine("       Category = ModuleCategory.Hull,");
        Console.WriteLine("       // ... other properties");
        Console.WriteLine("   };");
        Console.WriteLine();
        Console.WriteLine("3. When rendering, load the model:");
        Console.WriteLine("   var meshes = AssetManager.Instance.LoadModel(module.ModelPath);");
        Console.WriteLine();
        Console.WriteLine("4. Render with MeshRenderer:");
        Console.WriteLine("   meshRenderer.RenderMesh(meshes[0], transform, color, view, proj, camPos);");
    }
}
