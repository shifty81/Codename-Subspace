using System.Numerics;
using AvorionLike.Core.Procedural;
using AvorionLike.Core.Graphics;
using AvorionLike.Core.Voxel;
using AvorionLike.Core.ECS;
using AvorionLike.Core.Physics;

namespace AvorionLike.Examples;

/// <summary>
/// Example demonstrating the procedural ship generation and texture systems
/// </summary>
public class ShipGenerationExample
{
    private readonly EntityManager _entityManager;
    private readonly ProceduralShipGenerator _shipGenerator;
    private readonly CelestialTextureGenerator _textureGenerator;
    
    public ShipGenerationExample(EntityManager entityManager, int seed = 0)
    {
        _entityManager = entityManager;
        _shipGenerator = new ProceduralShipGenerator(seed);
        _textureGenerator = new CelestialTextureGenerator(seed);
    }
    
    /// <summary>
    /// Generate example ships for different factions
    /// </summary>
    public void GenerateExampleShips()
    {
        Console.WriteLine("\n=== PROCEDURAL SHIP GENERATION DEMO ===\n");
        
        // Example 1: Military Frigate
        Console.WriteLine("1. Generating Military Frigate...");
        var militaryShip = GenerateFactionShip(
            "Military", 
            ShipSize.Frigate, 
            ShipRole.Combat,
            new Vector3(0, 0, 0)
        );
        PrintShipStats("Military Frigate", militaryShip);
        
        // Example 2: Trader Corvette
        Console.WriteLine("\n2. Generating Trader Corvette...");
        var traderShip = GenerateFactionShip(
            "Traders",
            ShipSize.Corvette,
            ShipRole.Trading,
            new Vector3(100, 0, 0)
        );
        PrintShipStats("Trader Corvette", traderShip);
        
        // Example 3: Pirate Fighter
        Console.WriteLine("\n3. Generating Pirate Fighter...");
        var pirateShip = GenerateFactionShip(
            "Pirates",
            ShipSize.Fighter,
            ShipRole.Combat,
            new Vector3(-100, 0, 0)
        );
        PrintShipStats("Pirate Fighter", pirateShip);
        
        // Example 4: Explorer Destroyer
        Console.WriteLine("\n4. Generating Explorer Destroyer...");
        var explorerShip = GenerateFactionShip(
            "Explorers",
            ShipSize.Destroyer,
            ShipRole.Exploration,
            new Vector3(0, 100, 0)
        );
        PrintShipStats("Explorer Destroyer", explorerShip);
        
        // Example 5: Mining Industrial Ship
        Console.WriteLine("\n5. Generating Mining Ship...");
        var minerShip = GenerateFactionShip(
            "Miners",
            ShipSize.Frigate,
            ShipRole.Mining,
            new Vector3(0, -100, 0)
        );
        PrintShipStats("Mining Ship", minerShip);
        
        Console.WriteLine("\n=== SHIP GENERATION COMPLETE ===\n");
        Console.WriteLine($"Total ships created: 5");
        Console.WriteLine($"Total blocks placed: {CountTotalBlocks()}");
    }
    
    /// <summary>
    /// Generate a ship for a specific faction
    /// </summary>
    private GeneratedShip GenerateFactionShip(string factionName, ShipSize size, ShipRole role, Vector3 position)
    {
        // Get faction style
        var style = FactionShipStyle.GetDefaultStyle(factionName);
        
        // Configure ship generation
        var config = new ShipGenerationConfig
        {
            Size = size,
            Role = role,
            Material = style.PreferredMaterial,
            Style = style,
            Seed = Environment.TickCount,
            RequireHyperdrive = size >= ShipSize.Frigate,
            RequireCargo = true,
            MinimumWeaponMounts = role == ShipRole.Combat ? 4 : 2
        };
        
        // Generate ship
        var generatedShip = _shipGenerator.GenerateShip(config);
        
        // Create entity and add components
        var entity = _entityManager.CreateEntity($"{factionName} {size} {role}");
        
        // Add voxel structure
        _entityManager.AddComponent(entity.Id, generatedShip.Structure);
        
        // Add physics
        var physics = new PhysicsComponent
        {
            EntityId = entity.Id,
            Position = position,
            Mass = generatedShip.TotalMass,
            Velocity = Vector3.Zero
        };
        _entityManager.AddComponent(entity.Id, physics);
        
        return generatedShip;
    }
    
    /// <summary>
    /// Print ship statistics
    /// </summary>
    private void PrintShipStats(string name, GeneratedShip ship)
    {
        Console.WriteLine($"  Name: {name}");
        Console.WriteLine($"  Size: {ship.Config.Size}");
        Console.WriteLine($"  Role: {ship.Config.Role}");
        Console.WriteLine($"  Hull Shape: {ship.Config.Style.PreferredHullShape}");
        Console.WriteLine($"  Blocks: {ship.Structure.Blocks.Count}");
        Console.WriteLine($"  Mass: {ship.TotalMass:F0} kg");
        Console.WriteLine($"  Thrust: {ship.TotalThrust:F0} N");
        Console.WriteLine($"  Power: {ship.TotalPowerGeneration:F0} W");
        Console.WriteLine($"  Shields: {ship.TotalShieldCapacity:F0}");
        Console.WriteLine($"  Weapons: {ship.WeaponMountCount}");
        Console.WriteLine($"  Cargo: {ship.CargoBlockCount}");
        Console.WriteLine($"  Thrust/Mass: {(ship.TotalMass > 0 ? ship.TotalThrust / ship.TotalMass : 0):F2}");
        
        if (ship.Warnings.Count > 0)
        {
            Console.WriteLine($"  Warnings: {ship.Warnings.Count}");
            foreach (var warning in ship.Warnings)
            {
                Console.WriteLine($"    - {warning}");
            }
        }
    }
    
    /// <summary>
    /// Demonstrate texture generation for various celestial bodies
    /// </summary>
    public void DemonstrateTextureGeneration()
    {
        Console.WriteLine("\n=== PROCEDURAL TEXTURE GENERATION DEMO ===\n");
        
        // Gas Giant Examples
        Console.WriteLine("1. Gas Giant Textures:");
        DemonstrateGasGiant("Jupiter-like", "jupiter", new Vector3(0, 50, 0));
        DemonstrateGasGiant("Neptune-like", "neptune", new Vector3(0, -50, 0));
        DemonstrateGasGiant("Toxic Gas Giant", "toxic", new Vector3(0, 100, 0));
        
        // Rocky Planet Examples
        Console.WriteLine("\n2. Rocky Planet Textures:");
        DemonstrateRockyPlanet("Desert World", "desert", 50f, 0.8f, 0.2f);
        DemonstrateRockyPlanet("Earth-like World", "earthlike", 100f, 0.6f, 0.7f);
        DemonstrateRockyPlanet("Volcanic World", "volcanic", 20f, 0.9f, 0.1f);
        DemonstrateRockyPlanet("Ice World", "ice", 300f, 0.1f, 0.3f);
        
        // Asteroid Examples
        Console.WriteLine("\n3. Asteroid Textures:");
        DemonstrateAsteroid("Resource-Poor Asteroid", 0.1f);
        DemonstrateAsteroid("Resource-Rich Asteroid", 0.8f);
        
        // Nebula Examples
        Console.WriteLine("\n4. Nebula Textures:");
        DemonstrateNebula("Pink Nebula", "nebula_pink");
        DemonstrateNebula("Blue Nebula", "nebula_blue");
        
        // Station/Ship Hull
        Console.WriteLine("\n5. Industrial Textures:");
        DemonstrateStationHull("New Station Hull", new Vector3(0.7f, 0.7f, 0.7f), false);
        DemonstrateStationHull("Weathered Ship Hull", new Vector3(0.5f, 0.5f, 0.5f), true);
        
        Console.WriteLine("\n=== TEXTURE GENERATION COMPLETE ===\n");
    }
    
    private void DemonstrateGasGiant(string name, string paletteType, Vector3 samplePos)
    {
        var color = _textureGenerator.GenerateGasGiantTexture(samplePos, paletteType, 0f);
        Console.WriteLine($"  {name} at {samplePos}:");
        Console.WriteLine($"    RGB: ({color.X:F2}, {color.Y:F2}, {color.Z:F2})");
        Console.WriteLine($"    Hex: #{ColorToHex(color)}");
    }
    
    private void DemonstrateRockyPlanet(string name, string paletteType, float altitude, float temp, float moisture)
    {
        var samplePos = new Vector3(0, altitude, 0);
        var color = _textureGenerator.GenerateRockyPlanetTexture(samplePos, paletteType, altitude, temp, moisture);
        Console.WriteLine($"  {name} (alt:{altitude}, temp:{temp:F1}, moisture:{moisture:F1}):");
        Console.WriteLine($"    RGB: ({color.X:F2}, {color.Y:F2}, {color.Z:F2})");
        Console.WriteLine($"    Hex: #{ColorToHex(color)}");
    }
    
    private void DemonstrateAsteroid(string name, float resourceDensity)
    {
        var samplePos = new Vector3(10, 20, 30);
        var color = _textureGenerator.GenerateAsteroidTexture(samplePos, resourceDensity);
        Console.WriteLine($"  {name} (resource density: {resourceDensity:F1}):");
        Console.WriteLine($"    RGB: ({color.X:F2}, {color.Y:F2}, {color.Z:F2})");
        Console.WriteLine($"    Hex: #{ColorToHex(color)}");
    }
    
    private void DemonstrateNebula(string name, string paletteType)
    {
        var samplePos = new Vector3(5, 10, 15);
        var color = _textureGenerator.GenerateNebulaTexture(samplePos, paletteType, 0f);
        Console.WriteLine($"  {name}:");
        Console.WriteLine($"    RGB: ({color.X:F2}, {color.Y:F2}, {color.Z:F2})");
        Console.WriteLine($"    Hex: #{ColorToHex(color)}");
    }
    
    private void DemonstrateStationHull(string name, Vector3 baseColor, bool weathered)
    {
        var samplePos = new Vector3(2, 3, 4);
        var color = _textureGenerator.GenerateStationTexture(samplePos, baseColor, weathered);
        Console.WriteLine($"  {name} (weathered: {weathered}):");
        Console.WriteLine($"    RGB: ({color.X:F2}, {color.Y:F2}, {color.Z:F2})");
        Console.WriteLine($"    Hex: #{ColorToHex(color)}");
    }
    
    /// <summary>
    /// Demonstrate material properties and patterns
    /// </summary>
    public void DemonstrateMaterialLibrary()
    {
        Console.WriteLine("\n=== MATERIAL LIBRARY ===\n");
        
        var materials = MaterialLibrary.GetAllMaterials();
        
        Console.WriteLine($"Total Materials: {materials.Count}\n");
        
        foreach (var kvp in materials)
        {
            var material = kvp.Value;
            Console.WriteLine($"{material.Name} ({kvp.Key}):");
            Console.WriteLine($"  Base Color: ({material.BaseColor.X:F2}, {material.BaseColor.Y:F2}, {material.BaseColor.Z:F2})");
            Console.WriteLine($"  Roughness: {material.Roughness:F2}");
            Console.WriteLine($"  Metallic: {material.Metallic:F2}");
            Console.WriteLine($"  Emissive: {material.Emissive:F2}");
            Console.WriteLine($"  Pattern: {material.Pattern}");
            Console.WriteLine($"  Opacity: {material.Opacity:F2}");
            
            if (material.Animated)
            {
                Console.WriteLine($"  Animated: Yes (speed: {material.AnimationSpeed:F1}x)");
            }
            
            Console.WriteLine();
        }
    }
    
    /// <summary>
    /// Demonstrate how textures work with procedural ship generation
    /// </summary>
    public void DemonstrateShipWithTextures()
    {
        Console.WriteLine("\n=== SHIP WITH TEXTURES DEMO ===\n");
        
        // Generate a ship
        var style = FactionShipStyle.GetDefaultStyle("Military");
        var config = new ShipGenerationConfig
        {
            Size = ShipSize.Frigate,
            Role = ShipRole.Combat,
            Material = "Titanium",
            Style = style,
            Seed = 12345
        };
        
        var ship = _shipGenerator.GenerateShip(config);
        
        Console.WriteLine($"Generated {config.Size} {config.Role} ship:");
        Console.WriteLine($"  Blocks: {ship.Structure.Blocks.Count}");
        Console.WriteLine($"  Faction Style: {style.FactionName}");
        Console.WriteLine($"  Hull Shape: {style.PreferredHullShape}");
        Console.WriteLine();
        
        // Sample textures at different block positions
        Console.WriteLine("Sampling textures at block positions:");
        
        var textureGen = new ProceduralTextureGenerator(12345);
        int sampleCount = Math.Min(5, ship.Structure.Blocks.Count);
        
        for (int i = 0; i < sampleCount; i++)
        {
            var block = ship.Structure.Blocks[i];
            
            // Determine material type based on block type
            MaterialType matType = block.BlockType switch
            {
                BlockType.Hull => MaterialType.Hull,
                BlockType.Armor => MaterialType.Armor,
                BlockType.Engine => MaterialType.Metal,
                BlockType.Thruster => MaterialType.Metal,
                _ => MaterialType.Metal
            };
            
            var material = MaterialLibrary.GetMaterial(matType);
            var color = textureGen.GenerateTextureColor(block.Position, material);
            
            Console.WriteLine($"  Block {i + 1} ({block.BlockType} at {block.Position}):");
            Console.WriteLine($"    Material: {material.Name}");
            Console.WriteLine($"    Color: ({color.X:F2}, {color.Y:F2}, {color.Z:F2})");
            Console.WriteLine($"    Hex: #{ColorToHex(color)}");
        }
        
        Console.WriteLine("\n=== DEMO COMPLETE ===\n");
    }
    
    /// <summary>
    /// Show all available celestial palettes
    /// </summary>
    public void ShowAvailablePalettes()
    {
        Console.WriteLine("\n=== AVAILABLE CELESTIAL PALETTES ===\n");
        
        var palettes = _textureGenerator.GetAvailablePalettes();
        
        foreach (var palette in palettes)
        {
            Console.WriteLine($"  - {palette}");
        }
        
        Console.WriteLine();
    }
    
    // Helper methods
    
    private int CountTotalBlocks()
    {
        int total = 0;
        var entities = _entityManager.GetAllEntities();
        
        foreach (var entity in entities)
        {
            var voxelComp = _entityManager.GetComponent<VoxelStructureComponent>(entity.Id);
            if (voxelComp != null)
            {
                total += voxelComp.Blocks.Count;
            }
        }
        
        return total;
    }
    
    private string ColorToHex(Vector3 color)
    {
        int r = (int)(Math.Clamp(color.X, 0f, 1f) * 255);
        int g = (int)(Math.Clamp(color.Y, 0f, 1f) * 255);
        int b = (int)(Math.Clamp(color.Z, 0f, 1f) * 255);
        return $"{r:X2}{g:X2}{b:X2}";
    }
}
