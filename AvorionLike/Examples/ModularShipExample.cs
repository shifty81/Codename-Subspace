using System.Numerics;
using AvorionLike.Core.Logging;
using AvorionLike.Core.Graphics;
using AvorionLike.Core.Procedural;

namespace AvorionLike.Examples;

/// <summary>
/// Demonstrates the NMS-style modular ship generation system and enhanced textures
/// </summary>
public class ModularShipExample
{
    private readonly Logger _logger = Logger.Instance;
    
    public void RunExample()
    {
        Console.WriteLine("\n" + new string('=', 60));
        Console.WriteLine("       MODULAR SHIP GENERATION DEMO (NMS-Style)");
        Console.WriteLine(new string('=', 60));
        
        // Demonstrate modular ship generation
        DemonstrateModularGeneration();
        
        // Demonstrate enhanced texture generation
        DemonstrateEnhancedTextures();
        
        // Show interactive menu for generating ships
        ShowInteractiveMenu();
    }
    
    private void DemonstrateModularGeneration()
    {
        Console.WriteLine("\n--- MODULAR SHIP GENERATION ---\n");
        Console.WriteLine("Unlike traditional procedural generation that builds ships block-by-block,");
        Console.WriteLine("the modular approach uses pre-designed 'modules' that snap together,");
        Console.WriteLine("similar to No Man's Sky's ship generation system.\n");
        
        var generator = new ModularShipGenerator(12345);
        
        // Show available archetypes
        Console.WriteLine("Available Ship Archetypes:");
        Console.WriteLine("--------------------------");
        var archetypes = ModularShipGenerator.GetArchetypes();
        foreach (var (name, archetype) in archetypes)
        {
            Console.WriteLine($"  • {archetype.Name,-15} - {archetype.Role,-12} - Size: {archetype.Size}");
        }
        
        Console.WriteLine("\nGenerating sample ships from each archetype...\n");
        
        var styles = new[] { ModuleStyle.Military, ModuleStyle.Industrial, ModuleStyle.Sleek, ModuleStyle.Pirate };
        int styleIndex = 0;
        
        foreach (var (name, archetype) in archetypes.Take(5))
        {
            var style = styles[styleIndex % styles.Length];
            var ship = generator.GenerateModularShip(name, style, styleIndex * 1000);
            
            Console.WriteLine($"Generated: {archetype.Name} ({style} style)");
            Console.WriteLine($"  Blocks: {ship.Structure.Blocks.Count}");
            Console.WriteLine($"  Thrust: {ship.TotalThrust:F0}");
            Console.WriteLine($"  Weapons: {ship.WeaponMountCount}");
            Console.WriteLine($"  Cargo: {ship.CargoBlockCount}");
            Console.WriteLine();
            
            styleIndex++;
        }
    }
    
    private void DemonstrateEnhancedTextures()
    {
        Console.WriteLine("\n--- ENHANCED TEXTURE GENERATION ---\n");
        Console.WriteLine("The new texture system adds visual complexity through:");
        Console.WriteLine("  • Panel lines and seams");
        Console.WriteLine("  • Greebling (surface detail)");
        Console.WriteLine("  • Wear and weathering (scratches, rust, scorch marks)");
        Console.WriteLine("  • Style-specific patterns (camo, oil stains, geometric glyphs)");
        Console.WriteLine("  • Emissive accents and running lights\n");
        
        var texGen = new EnhancedTextureGenerator(42);
        Vector3 samplePos = new Vector3(10f, 5f, 20f);
        Vector3 baseColor = new Vector3(0.6f, 0.6f, 0.65f); // Gray-blue base
        
        Console.WriteLine("Texture colors at sample position for each style:");
        Console.WriteLine("(Base color: Steel Gray RGB 153, 153, 166)\n");
        
        foreach (var style in Enum.GetValues<EnhancedTextureGenerator.TextureStyle>())
        {
            var color = texGen.GenerateEnhancedColor(samplePos, baseColor, style);
            
            // Convert to 0-255 range
            int r = (int)Math.Clamp(color.X * 255, 0, 255);
            int g = (int)Math.Clamp(color.Y * 255, 0, 255);
            int b = (int)Math.Clamp(color.Z * 255, 0, 255);
            
            Console.WriteLine($"  {style,-12}: RGB({r,3}, {g,3}, {b,3}) - #{r:X2}{g:X2}{b:X2}");
        }
        
        Console.WriteLine("\n--- STYLE CHARACTERISTICS ---\n");
        
        Console.WriteLine("  Clean:       Factory-fresh, minimal weathering");
        Console.WriteLine("  Military:    Camo patterns, armor plates, warning marks");
        Console.WriteLine("  Industrial:  Dirty, oily, rust stains, exposed rivets");
        Console.WriteLine("  Sleek:       High-tech panels, glowing accents, clean seams");
        Console.WriteLine("  Pirate:      Heavy wear, patches, mismatched panels");
        Console.WriteLine("  Ancient:     Geometric patterns, mysterious glowing glyphs");
        Console.WriteLine("  Organic:     Biomechanical veins, pulsing patterns");
        Console.WriteLine("  Crystalline: Faceted surfaces, prismatic color shifting");
    }
    
    private void ShowInteractiveMenu()
    {
        Console.WriteLine("\n" + new string('=', 60));
        Console.WriteLine("          INTERACTIVE SHIP GENERATOR");
        Console.WriteLine(new string('=', 60));
        
        var generator = new ModularShipGenerator();
        var archetypes = ModularShipGenerator.GetArchetypes().Keys.ToList();
        
        while (true)
        {
            Console.WriteLine("\nOptions:");
            Console.WriteLine("  1. Generate random modular ship");
            Console.WriteLine("  2. Generate specific archetype");
            Console.WriteLine("  3. Show all archetypes with details");
            Console.WriteLine("  4. Compare module styles");
            Console.WriteLine("  5. Generate fleet (10 random ships)");
            Console.WriteLine("  0. Return to main menu\n");
            
            Console.Write("Select option: ");
            var input = Console.ReadLine()?.Trim();
            
            switch (input)
            {
                case "1":
                    GenerateRandomShip(generator);
                    break;
                    
                case "2":
                    GenerateSpecificShip(generator, archetypes);
                    break;
                    
                case "3":
                    ShowArchetypeDetails();
                    break;
                    
                case "4":
                    CompareModuleStyles(generator);
                    break;
                    
                case "5":
                    GenerateFleet(generator);
                    break;
                    
                case "0":
                    return;
                    
                default:
                    Console.WriteLine("Invalid option. Please try again.");
                    break;
            }
        }
    }
    
    private void GenerateRandomShip(ModularShipGenerator generator)
    {
        Console.WriteLine("\n--- Generating Random Ship ---\n");
        
        var seed = Random.Shared.Next();
        var ship = generator.GenerateRandomModularShip(seed);
        
        DisplayShipDetails(ship, seed);
    }
    
    private void GenerateSpecificShip(ModularShipGenerator generator, List<string> archetypes)
    {
        Console.WriteLine("\n--- Available Archetypes ---");
        for (int i = 0; i < archetypes.Count; i++)
        {
            Console.WriteLine($"  {i + 1}. {archetypes[i]}");
        }
        
        Console.Write("\nSelect archetype (1-" + archetypes.Count + "): ");
        var input = Console.ReadLine()?.Trim();
        
        if (int.TryParse(input, out int index) && index >= 1 && index <= archetypes.Count)
        {
            var archetype = archetypes[index - 1];
            
            Console.WriteLine("\n--- Available Styles ---");
            var styles = Enum.GetValues<ModuleStyle>();
            for (int i = 0; i < styles.Length; i++)
            {
                Console.WriteLine($"  {i + 1}. {styles[i]}");
            }
            
            Console.Write("\nSelect style (1-" + styles.Length + "): ");
            var styleInput = Console.ReadLine()?.Trim();
            
            ModuleStyle? style = null;
            if (int.TryParse(styleInput, out int styleIndex) && styleIndex >= 1 && styleIndex <= styles.Length)
            {
                style = styles[styleIndex - 1];
            }
            
            var seed = Random.Shared.Next();
            var ship = generator.GenerateModularShip(archetype, style, seed);
            
            DisplayShipDetails(ship, seed);
        }
        else
        {
            Console.WriteLine("Invalid selection.");
        }
    }
    
    private void ShowArchetypeDetails()
    {
        Console.WriteLine("\n" + new string('=', 60));
        Console.WriteLine("              SHIP ARCHETYPE DETAILS");
        Console.WriteLine(new string('=', 60));
        
        var archetypes = ModularShipGenerator.GetArchetypes();
        
        foreach (var (name, arch) in archetypes)
        {
            Console.WriteLine($"\n{arch.Name.ToUpper()}");
            Console.WriteLine(new string('-', 40));
            Console.WriteLine($"  Size:           {arch.Size}");
            Console.WriteLine($"  Role:           {arch.Role}");
            Console.WriteLine($"  Engines:        {arch.EngineCount}");
            Console.WriteLine($"  Has Wings:      {(arch.HasWings ? "Yes" : "No")}");
            Console.WriteLine($"  Weapons:        {arch.WeaponMountCount}");
            Console.WriteLine($"  Main Size:      Class {arch.MainSizeClass}");
            Console.WriteLine($"  Required:       {string.Join(", ", arch.RequiredModules)}");
            Console.WriteLine($"  Preferred:      {string.Join(", ", arch.PreferredStyles)}");
        }
    }
    
    private void CompareModuleStyles(ModularShipGenerator generator)
    {
        Console.WriteLine("\n--- MODULE STYLE COMPARISON ---");
        Console.WriteLine("Generating Fighter in each style...\n");
        
        Console.WriteLine($"{"Style",-12} {"Blocks",-8} {"Thrust",-10} {"Weapons",-8} {"Color Scheme"}");
        Console.WriteLine(new string('-', 60));
        
        foreach (var style in Enum.GetValues<ModuleStyle>())
        {
            var ship = generator.GenerateModularShip("fighter", style, (int)style * 1000);
            
            string colorScheme = style switch
            {
                ModuleStyle.Military => "Dark Slate + Red",
                ModuleStyle.Industrial => "Gray + Orange",
                ModuleStyle.Sleek => "White/Silver + Cyan",
                ModuleStyle.Pirate => "Dark Red + Orange",
                ModuleStyle.Ancient => "Gold + Green",
                ModuleStyle.Organic => "Purple + Magenta",
                _ => "Gray + Silver"
            };
            
            Console.WriteLine($"{style,-12} {ship.Structure.Blocks.Count,-8} {ship.TotalThrust,-10:F0} {ship.WeaponMountCount,-8} {colorScheme}");
        }
    }
    
    private void GenerateFleet(ModularShipGenerator generator)
    {
        Console.WriteLine("\n--- GENERATING FLEET (10 Ships) ---\n");
        
        Console.WriteLine($"{"#",-3} {"Type",-15} {"Style",-12} {"Blocks",-8} {"Thrust",-10} {"Weapons",-8}");
        Console.WriteLine(new string('-', 70));
        
        var archetypes = ModularShipGenerator.GetArchetypes().Keys.ToList();
        var styles = Enum.GetValues<ModuleStyle>();
        var random = new Random();
        
        for (int i = 1; i <= 10; i++)
        {
            var archetype = archetypes[random.Next(archetypes.Count)];
            var style = styles[random.Next(styles.Length)];
            var seed = random.Next();
            
            var ship = generator.GenerateModularShip(archetype, style, seed);
            
            Console.WriteLine($"{i,-3} {archetype,-15} {style,-12} {ship.Structure.Blocks.Count,-8} {ship.TotalThrust,-10:F0} {ship.WeaponMountCount,-8}");
        }
        
        Console.WriteLine("\nFleet generation complete!");
    }
    
    private void DisplayShipDetails(GeneratedShip ship, int seed)
    {
        Console.WriteLine($"\n{'=',-50}");
        Console.WriteLine($"  GENERATED SHIP - Seed: {seed}");
        Console.WriteLine($"{'=',-50}");
        
        Console.WriteLine($"\n  Size:             {ship.Config.Size}");
        Console.WriteLine($"  Role:             {ship.Config.Role}");
        Console.WriteLine($"  Total Blocks:     {ship.Structure.Blocks.Count}");
        Console.WriteLine($"  Total Mass:       {ship.TotalMass:F0} kg");
        Console.WriteLine($"  Total Thrust:     {ship.TotalThrust:F0}");
        Console.WriteLine($"  Power Gen:        {ship.TotalPowerGeneration:F0}");
        Console.WriteLine($"  Shield Cap:       {ship.TotalShieldCapacity:F0}");
        Console.WriteLine($"  Weapon Mounts:    {ship.WeaponMountCount}");
        Console.WriteLine($"  Cargo Space:      {ship.CargoBlockCount}");
        
        // Block type breakdown
        var blockTypes = ship.Structure.Blocks
            .GroupBy(b => b.BlockType)
            .OrderByDescending(g => g.Count());
        
        Console.WriteLine("\n  Block Breakdown:");
        foreach (var group in blockTypes)
        {
            Console.WriteLine($"    {group.Key,-18}: {group.Count()}");
        }
        
        // Warnings
        if (ship.Warnings.Count > 0)
        {
            Console.WriteLine("\n  Warnings:");
            foreach (var warning in ship.Warnings.Take(5))
            {
                Console.WriteLine($"    ! {warning}");
            }
        }
        
        Console.WriteLine();
    }
}
