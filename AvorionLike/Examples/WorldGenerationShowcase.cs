using System;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using AvorionLike.Core.ECS;
using AvorionLike.Core.Graphics;
using AvorionLike.Core.Physics;
using AvorionLike.Core.Procedural;
using AvorionLike.Core.Voxel;

namespace AvorionLike.Examples;

/// <summary>
/// Visual showcase demonstrating all world generation options
/// Shows the visual impact of different rendering modes, generation styles, and configuration options
/// </summary>
public class WorldGenerationShowcase
{
    private readonly EntityManager _entityManager;
    private readonly RenderingConfiguration _renderingConfig;
    private readonly List<ShowcaseItem> _showcaseItems = new();

    public class ShowcaseItem
    {
        public string Name { get; set; } = "";
        public string Category { get; set; } = "";
        public string Description { get; set; } = "";
        public Guid EntityId { get; set; }
        public Vector3 Position { get; set; }
        public int BlockCount { get; set; }
        public Dictionary<string, object> Properties { get; set; } = new();
    }

    public WorldGenerationShowcase(EntityManager entityManager)
    {
        _entityManager = entityManager;
        _renderingConfig = RenderingConfiguration.Instance;
    }

    /// <summary>
    /// Run the complete visual showcase
    /// </summary>
    public void RunShowcase()
    {
        PrintHeader();
        
        bool done = false;
        while (!done)
        {
            PrintMenu();
            var choice = Console.ReadLine();

            switch (choice)
            {
                case "1":
                    ShowRenderingModesComparison();
                    break;
                case "2":
                    ShowShipGenerationStyles();
                    break;
                case "3":
                    ShowStationArchitectures();
                    break;
                case "4":
                    ShowGalaxySectorGeneration();
                    break;
                case "5":
                    ShowAsteroidVariety();
                    break;
                case "6":
                    ShowMiningShipStyles();
                    break;
                case "7":
                    ShowAllGenerationOptions();
                    break;
                case "8":
                    PrintShowcaseSummary();
                    break;
                case "9":
                    ExportShowcaseReport();
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

    private void PrintHeader()
    {
        Console.WriteLine();
        Console.WriteLine("╔══════════════════════════════════════════════════════════════════════════╗");
        Console.WriteLine("║           WORLD GENERATION OPTIONS - VISUAL SHOWCASE                     ║");
        Console.WriteLine("║   Demonstrating NPR/PBR Rendering Modes & Procedural Generation          ║");
        Console.WriteLine("╚══════════════════════════════════════════════════════════════════════════╝");
        Console.WriteLine();
        Console.WriteLine("This showcase demonstrates all world generation options available in the");
        Console.WriteLine("Codename:Subspace engine, including the new NPR/PBR rendering modes that");
        Console.WriteLine("fix visual issues on voxel blocks.");
        Console.WriteLine();
    }

    private void PrintMenu()
    {
        Console.WriteLine();
        Console.WriteLine("═══════════════════════════════════════════════════════════════════════════");
        Console.WriteLine("SHOWCASE OPTIONS:");
        Console.WriteLine("═══════════════════════════════════════════════════════════════════════════");
        Console.WriteLine("  1  - Rendering Modes Comparison (PBR vs NPR vs Hybrid)");
        Console.WriteLine("  2  - Ship Generation Styles (Military, Industrial, Organic, etc.)");
        Console.WriteLine("  3  - Station Architectures (Modular, Ring, Tower, Industrial, Sprawling)");
        Console.WriteLine("  4  - Galaxy Sector Generation (Stars, Planets, Asteroid Belts)");
        Console.WriteLine("  5  - Asteroid Variety (Different types and resources)");
        Console.WriteLine("  6  - Mining Ship Styles (Angular, Blocky Industrial Ships)");
        Console.WriteLine("  7  - Generate All Options (Complete showcase)");
        Console.WriteLine("  8  - View Showcase Summary");
        Console.WriteLine("  9  - Export Showcase Report");
        Console.WriteLine("  0  - Return to Main Menu");
        Console.WriteLine();
        Console.Write("Select option: ");
    }

    /// <summary>
    /// Show comparison of different rendering modes
    /// </summary>
    public void ShowRenderingModesComparison()
    {
        Console.WriteLine();
        Console.WriteLine("╔══════════════════════════════════════════════════════════════╗");
        Console.WriteLine("║         RENDERING MODES COMPARISON                            ║");
        Console.WriteLine("╚══════════════════════════════════════════════════════════════╝");
        Console.WriteLine();

        Console.WriteLine("Generating identical ships with different rendering configurations...\n");

        int baseSeed = 42424242;
        var presets = new[]
        {
            (RenderingPreset.RealisticPBR, "PBR Realistic", "Metallic/roughness workflow with reflections"),
            (RenderingPreset.StylizedNPR, "NPR Stylized", "Cel-shading with cartoon outlines"),
            (RenderingPreset.HybridBalanced, "Hybrid Balanced", "PBR + edge detection (RECOMMENDED)"),
            (RenderingPreset.Performance, "Performance", "Minimal effects for speed")
        };

        float xOffset = -150f;
        foreach (var (preset, name, description) in presets)
        {
            // Apply preset temporarily to show configuration
            var previousMode = _renderingConfig.Mode;
            var previousEdge = _renderingConfig.EnableEdgeDetection;
            var previousCel = _renderingConfig.EnableCelShading;
            var previousAO = _renderingConfig.EnableAmbientOcclusion;

            _renderingConfig.ApplyPreset(preset);

            // Generate a test ship with this configuration
            var generator = new ProceduralShipGenerator(baseSeed);
            var config = new ShipGenerationConfig
            {
                Size = ShipSize.Frigate,
                Role = ShipRole.Combat,
                Material = "Titanium",
                Style = FactionShipStyle.GetDefaultStyle("Military"),
                Seed = baseSeed
            };

            var ship = generator.GenerateShip(config);
            var entity = _entityManager.CreateEntity($"Render Demo - {name}");
            _entityManager.AddComponent(entity.Id, ship.Structure);

            var physics = new PhysicsComponent
            {
                Position = new Vector3(xOffset, 0, 0),
                Mass = ship.TotalMass
            };
            _entityManager.AddComponent(entity.Id, physics);

            var item = new ShowcaseItem
            {
                Name = name,
                Category = "Rendering Modes",
                Description = description,
                EntityId = entity.Id,
                Position = physics.Position,
                BlockCount = ship.Structure.Blocks.Count,
                Properties = new Dictionary<string, object>
                {
                    ["RenderingMode"] = _renderingConfig.Mode.ToString(),
                    ["EdgeDetection"] = _renderingConfig.EnableEdgeDetection,
                    ["CelShading"] = _renderingConfig.EnableCelShading,
                    ["AmbientOcclusion"] = _renderingConfig.EnableAmbientOcclusion,
                    ["EdgeThickness"] = _renderingConfig.EdgeThickness,
                    ["AOStrength"] = _renderingConfig.AmbientOcclusionStrength
                }
            };
            _showcaseItems.Add(item);

            Console.WriteLine($"┌─────────────────────────────────────────────────────────────┐");
            Console.WriteLine($"│ {name,-58}│");
            Console.WriteLine($"├─────────────────────────────────────────────────────────────┤");
            Console.WriteLine($"│ {description,-58}│");
            Console.WriteLine($"│ Mode: {_renderingConfig.Mode,-52}│");
            Console.WriteLine($"│ Edge Detection: {(_renderingConfig.EnableEdgeDetection ? "ON" : "OFF"),-42}│");
            Console.WriteLine($"│ Cel-Shading: {(_renderingConfig.EnableCelShading ? "ON" : "OFF"),-45}│");
            Console.WriteLine($"│ Ambient Occlusion: {(_renderingConfig.EnableAmbientOcclusion ? "ON" : "OFF"),-39}│");
            Console.WriteLine($"│ Blocks: {ship.Structure.Blocks.Count,-49}│");
            Console.WriteLine($"│ Position: ({physics.Position.X:F0}, {physics.Position.Y:F0}, {physics.Position.Z:F0}){new string(' ', 37)}│");
            Console.WriteLine($"└─────────────────────────────────────────────────────────────┘");
            Console.WriteLine();

            // Restore previous settings
            _renderingConfig.Mode = previousMode;
            _renderingConfig.EnableEdgeDetection = previousEdge;
            _renderingConfig.EnableCelShading = previousCel;
            _renderingConfig.EnableAmbientOcclusion = previousAO;

            xOffset += 100f;
        }

        Console.WriteLine("✓ Generated 4 ships demonstrating different rendering modes");
        Console.WriteLine("  Run '3D Graphics Demo' (option 11) to see visual differences");
    }

    /// <summary>
    /// Show different ship generation styles
    /// </summary>
    public void ShowShipGenerationStyles()
    {
        Console.WriteLine();
        Console.WriteLine("╔══════════════════════════════════════════════════════════════╗");
        Console.WriteLine("║         SHIP GENERATION STYLES                                ║");
        Console.WriteLine("╚══════════════════════════════════════════════════════════════╝");
        Console.WriteLine();

        var styles = new[]
        {
            ("Military", "Sharp angular hulls with weapon hardpoints"),
            ("Industrial", "Blocky utilitarian designs"),
            ("Trader", "Sleek efficient cargo vessels"),
            ("Pirate", "Asymmetric aggressive configurations"),
            ("Science", "Elegant research vessel layouts"),
            ("Default", "Balanced general-purpose ships")
        };

        var sizes = new[] { ShipSize.Fighter, ShipSize.Corvette, ShipSize.Frigate, ShipSize.Destroyer };

        Console.WriteLine($"Generating {styles.Length} faction styles × {sizes.Length} ship sizes...\n");

        int baseSeed = Environment.TickCount;
        float xSpacing = 80f;
        float zSpacing = 60f;

        for (int styleIdx = 0; styleIdx < styles.Length; styleIdx++)
        {
            var (styleName, styleDesc) = styles[styleIdx];
            var factionStyle = FactionShipStyle.GetDefaultStyle(styleName);

            Console.WriteLine($"── {styleName} Style: {styleDesc} ──");

            for (int sizeIdx = 0; sizeIdx < sizes.Length; sizeIdx++)
            {
                var size = sizes[sizeIdx];
                var generator = new ProceduralShipGenerator(baseSeed + styleIdx * 100 + sizeIdx);

                var config = new ShipGenerationConfig
                {
                    Size = size,
                    Role = ShipRole.Multipurpose,
                    Material = "Titanium",
                    Style = factionStyle,
                    Seed = baseSeed + styleIdx * 100 + sizeIdx
                };

                var ship = generator.GenerateShip(config);
                var entity = _entityManager.CreateEntity($"{styleName} {size}");
                _entityManager.AddComponent(entity.Id, ship.Structure);

                var position = new Vector3(styleIdx * xSpacing - 200, 0, sizeIdx * zSpacing);
                var physics = new PhysicsComponent
                {
                    Position = position,
                    Mass = ship.TotalMass
                };
                _entityManager.AddComponent(entity.Id, physics);

                var item = new ShowcaseItem
                {
                    Name = $"{styleName} {size}",
                    Category = "Ship Styles",
                    Description = styleDesc,
                    EntityId = entity.Id,
                    Position = position,
                    BlockCount = ship.Structure.Blocks.Count,
                    Properties = new Dictionary<string, object>
                    {
                        ["FactionStyle"] = styleName,
                        ["ShipSize"] = size.ToString(),
                        ["UseAngledBlocks"] = factionStyle.UseAngledBlocks,
                        ["Symmetry"] = factionStyle.SymmetryLevel
                    }
                };
                _showcaseItems.Add(item);

                Console.Write($"  {size,-12} ");
            }
            Console.WriteLine();
        }

        int totalShips = styles.Length * sizes.Length;
        Console.WriteLine($"\n✓ Generated {totalShips} ships across {styles.Length} faction styles");
    }

    /// <summary>
    /// Show different station architectures
    /// </summary>
    public void ShowStationArchitectures()
    {
        Console.WriteLine();
        Console.WriteLine("╔══════════════════════════════════════════════════════════════╗");
        Console.WriteLine("║         STATION ARCHITECTURES                                 ║");
        Console.WriteLine("╚══════════════════════════════════════════════════════════════╝");
        Console.WriteLine();

        var architectures = new[]
        {
            (StationArchitecture.Modular, "Modular", "Connected modules and sections"),
            (StationArchitecture.Ring, "Ring", "Rotating ring-shaped habitat"),
            (StationArchitecture.Tower, "Tower", "Tall spire structure"),
            (StationArchitecture.Industrial, "Industrial", "Complex industrial framework"),
            (StationArchitecture.Sprawling, "Sprawling", "Spread-out complex structure")
        };

        var stationTypes = new[] { "Trading", "Military", "Research", "Refinery" };

        Console.WriteLine($"Generating {architectures.Length} architectures × {stationTypes.Length} types...\n");

        int baseSeed = 777777;
        var stationGenerator = new ProceduralStationGenerator(baseSeed);

        float xSpacing = 300f;
        float zSpacing = 200f;

        for (int archIdx = 0; archIdx < architectures.Length; archIdx++)
        {
            var (architecture, archName, archDesc) = architectures[archIdx];

            Console.WriteLine($"┌─ {archName}: {archDesc}");

            for (int typeIdx = 0; typeIdx < stationTypes.Length; typeIdx++)
            {
                var stationType = stationTypes[typeIdx];

                var config = new StationGenerationConfig
                {
                    Size = StationSize.Medium,
                    StationType = stationType,
                    Material = "Titanium",
                    Architecture = architecture,
                    Seed = baseSeed + archIdx * 100 + typeIdx,
                    IncludeDockingBays = true,
                    MinDockingBays = 4
                };

                var station = stationGenerator.GenerateStation(config);
                var entity = _entityManager.CreateEntity($"{archName} {stationType} Station");
                _entityManager.AddComponent(entity.Id, station.Structure);

                var position = new Vector3(archIdx * xSpacing - 600, 0, typeIdx * zSpacing);
                var physics = new PhysicsComponent
                {
                    Position = position,
                    Mass = station.TotalMass
                };
                _entityManager.AddComponent(entity.Id, physics);

                var item = new ShowcaseItem
                {
                    Name = $"{archName} {stationType}",
                    Category = "Stations",
                    Description = archDesc,
                    EntityId = entity.Id,
                    Position = position,
                    BlockCount = station.BlockCount,
                    Properties = new Dictionary<string, object>
                    {
                        ["Architecture"] = archName,
                        ["StationType"] = stationType,
                        ["DockingBays"] = station.DockingPoints.Count,
                        ["Facilities"] = string.Join(", ", station.Facilities)
                    }
                };
                _showcaseItems.Add(item);

                Console.Write($"│   {stationType,-10}: {station.BlockCount,5} blocks, {station.DockingPoints.Count} docks  ");
            }
            Console.WriteLine();
            Console.WriteLine("└────────────────────────────────────────────────────────────");
        }

        int totalStations = architectures.Length * stationTypes.Length;
        Console.WriteLine($"\n✓ Generated {totalStations} stations across {architectures.Length} architectures");
    }

    /// <summary>
    /// Show galaxy sector generation
    /// </summary>
    public void ShowGalaxySectorGeneration()
    {
        Console.WriteLine();
        Console.WriteLine("╔══════════════════════════════════════════════════════════════╗");
        Console.WriteLine("║         GALAXY SECTOR GENERATION                              ║");
        Console.WriteLine("╚══════════════════════════════════════════════════════════════╝");
        Console.WriteLine();

        var starSystemGenerator = new StarSystemGenerator(12345);

        Console.WriteLine("Generating sample solar systems at different galaxy coordinates...\n");

        var coordinates = new[]
        {
            (new AvorionLike.Core.Procedural.Vector3Int(0, 0, 0), "Center (Core)"),
            (new AvorionLike.Core.Procedural.Vector3Int(5, 0, 0), "Inner Ring"),
            (new AvorionLike.Core.Procedural.Vector3Int(15, 5, 0), "Mid Ring"),
            (new AvorionLike.Core.Procedural.Vector3Int(30, 10, 5), "Outer Frontier")
        };

        foreach (var (coord, name) in coordinates)
        {
            var system = starSystemGenerator.GenerateSystem(coord);

            Console.WriteLine($"┌─────────────────────────────────────────────────────────────┐");
            Console.WriteLine($"│ {name,-28} @ ({coord.X},{coord.Y},{coord.Z})              │");
            Console.WriteLine($"├─────────────────────────────────────────────────────────────┤");
            Console.WriteLine($"│ System: {system.Name,-49}│");
            Console.WriteLine($"│ Type: {system.Type,-51}│");
            Console.WriteLine($"│ Danger Level: {system.DangerLevel,-44}│");
            Console.WriteLine($"│ Star: {system.CentralStar?.Type.ToString() ?? "Unknown",-51}│");
            Console.WriteLine($"│ Planets: {system.Planets.Count,-49}│");
            Console.WriteLine($"│ Asteroid Belts: {system.AsteroidBelts.Count,-42}│");
            Console.WriteLine($"│ Stations: {system.Stations.Count,-48}│");
            Console.WriteLine($"└─────────────────────────────────────────────────────────────┘");
            Console.WriteLine();

            var item = new ShowcaseItem
            {
                Name = system.Name,
                Category = "Solar Systems",
                Description = $"{system.Type} at {name}",
                BlockCount = 0,
                Properties = new Dictionary<string, object>
                {
                    ["SystemType"] = system.Type.ToString(),
                    ["DangerLevel"] = system.DangerLevel,
                    ["StarType"] = system.CentralStar?.Type.ToString() ?? "Unknown",
                    ["Planets"] = system.Planets.Count,
                    ["AsteroidBelts"] = system.AsteroidBelts.Count,
                    ["Stations"] = system.Stations.Count
                }
            };
            _showcaseItems.Add(item);

            // Show planet details
            if (system.Planets.Count > 0)
            {
                Console.WriteLine("  Planets:");
                foreach (var planet in system.Planets.Take(4))
                {
                    Console.WriteLine($"    • {planet.Name}: {planet.Type}, Size={planet.Size:F0}");
                }
                if (system.Planets.Count > 4)
                    Console.WriteLine($"    ... and {system.Planets.Count - 4} more");
                Console.WriteLine();
            }
        }

        Console.WriteLine($"✓ Generated {coordinates.Length} sample solar systems");
    }

    /// <summary>
    /// Show asteroid variety
    /// </summary>
    public void ShowAsteroidVariety()
    {
        Console.WriteLine();
        Console.WriteLine("╔══════════════════════════════════════════════════════════════╗");
        Console.WriteLine("║         ASTEROID VARIETY                                      ║");
        Console.WriteLine("╚══════════════════════════════════════════════════════════════╝");
        Console.WriteLine();

        var massiveAsteroidGenerator = new MassiveAsteroidGenerator(55555);

        var asteroidTypes = Enum.GetValues<MassiveAsteroidType>();

        Console.WriteLine($"Generating {asteroidTypes.Length} massive asteroid types...\n");

        float xOffset = -200f;
        foreach (var asteroidType in asteroidTypes)
        {
            var config = new MassiveAsteroidConfig
            {
                Type = asteroidType,
                Seed = 55555 + (int)asteroidType,
                MinSize = 1000f,
                MaxSize = 2000f
            };

            var asteroid = massiveAsteroidGenerator.GenerateAsteroid(config);
            var entity = _entityManager.CreateEntity($"Massive {asteroidType} Asteroid");
            _entityManager.AddComponent(entity.Id, asteroid.Structure);

            var position = new Vector3(xOffset, 0, 0);
            var physics = new PhysicsComponent
            {
                Position = position,
                Mass = asteroid.Structure.TotalMass
            };
            _entityManager.AddComponent(entity.Id, physics);

            var item = new ShowcaseItem
            {
                Name = $"{asteroidType} Asteroid",
                Category = "Asteroids",
                Description = $"Claimable massive asteroid",
                EntityId = entity.Id,
                Position = position,
                BlockCount = asteroid.BlockCount,
                Properties = new Dictionary<string, object>
                {
                    ["AsteroidType"] = asteroidType.ToString(),
                    ["DockingPoints"] = asteroid.DockingPoints.Count,
                    ["Size"] = config.MaxSize
                }
            };
            _showcaseItems.Add(item);

            Console.WriteLine($"  {asteroidType,-20}: {asteroid.BlockCount,5} blocks, {asteroid.DockingPoints.Count} docking points");

            xOffset += 150f;
        }

        Console.WriteLine($"\n✓ Generated {asteroidTypes.Length} massive asteroids");
    }

    /// <summary>
    /// Show mining ship styles
    /// </summary>
    public void ShowMiningShipStyles()
    {
        Console.WriteLine();
        Console.WriteLine("╔══════════════════════════════════════════════════════════════╗");
        Console.WriteLine("║         INDUSTRIAL MINING SHIP STYLES                         ║");
        Console.WriteLine("╚══════════════════════════════════════════════════════════════╝");
        Console.WriteLine();

        var showcase = new IndustrialMiningShipShowcase(_entityManager);
        var ships = showcase.GenerateShowcase(99999);

        foreach (var ship in ships)
        {
            var item = new ShowcaseItem
            {
                Name = ship.ShipData.ShipName,
                Category = "Mining Ships",
                Description = ship.Description,
                EntityId = ship.EntityId,
                Position = ship.Position,
                BlockCount = ship.ShipData.Structure.Blocks.Count,
                Properties = new Dictionary<string, object>
                {
                    ["ShipSize"] = ship.ShipData.Config.Size.ToString(),
                    ["MiningLasers"] = ship.ShipData.MiningLaserCount,
                    ["CargoCapacity"] = ship.ShipData.CargoCapacity,
                    ["ExposedFramework"] = ship.ShipData.Config.UseExposedFramework,
                    ["AsymmetricArms"] = ship.ShipData.Config.UseAsymmetricMiningArms
                }
            };
            _showcaseItems.Add(item);
        }

        Console.WriteLine($"\n✓ Generated {ships.Count} industrial mining ships");
    }

    /// <summary>
    /// Generate all showcase options
    /// </summary>
    public void ShowAllGenerationOptions()
    {
        Console.WriteLine();
        Console.WriteLine("╔══════════════════════════════════════════════════════════════╗");
        Console.WriteLine("║         GENERATING COMPLETE SHOWCASE                          ║");
        Console.WriteLine("╚══════════════════════════════════════════════════════════════╝");
        Console.WriteLine();

        Console.WriteLine("This will generate examples from all categories...\n");

        ShowRenderingModesComparison();
        Console.WriteLine();
        
        ShowShipGenerationStyles();
        Console.WriteLine();
        
        ShowStationArchitectures();
        Console.WriteLine();
        
        ShowGalaxySectorGeneration();
        Console.WriteLine();
        
        ShowAsteroidVariety();
        Console.WriteLine();
        
        ShowMiningShipStyles();

        Console.WriteLine();
        Console.WriteLine("═══════════════════════════════════════════════════════════════");
        Console.WriteLine($"COMPLETE SHOWCASE GENERATED: {_showcaseItems.Count} total items");
        Console.WriteLine("═══════════════════════════════════════════════════════════════");
    }

    /// <summary>
    /// Print summary of all showcase items
    /// </summary>
    public void PrintShowcaseSummary()
    {
        Console.WriteLine();
        Console.WriteLine("╔══════════════════════════════════════════════════════════════╗");
        Console.WriteLine("║         SHOWCASE SUMMARY                                      ║");
        Console.WriteLine("╚══════════════════════════════════════════════════════════════╝");
        Console.WriteLine();

        if (_showcaseItems.Count == 0)
        {
            Console.WriteLine("No items generated yet. Use options 1-7 to generate showcase items.");
            return;
        }

        // Group by category
        var categories = _showcaseItems.GroupBy(i => i.Category);

        int totalBlocks = 0;
        foreach (var category in categories)
        {
            Console.WriteLine($"┌─ {category.Key} ({category.Count()} items) ─────────────────────────────────");
            foreach (var item in category)
            {
                Console.WriteLine($"│  • {item.Name}: {item.BlockCount} blocks");
                totalBlocks += item.BlockCount;
            }
            Console.WriteLine($"└────────────────────────────────────────────────────────────────");
            Console.WriteLine();
        }

        Console.WriteLine($"TOTAL: {_showcaseItems.Count} items, {totalBlocks:N0} blocks");
    }

    /// <summary>
    /// Export showcase report
    /// </summary>
    public void ExportShowcaseReport()
    {
        Console.WriteLine();
        Console.WriteLine("╔══════════════════════════════════════════════════════════════╗");
        Console.WriteLine("║         SHOWCASE REPORT                                       ║");
        Console.WriteLine("╚══════════════════════════════════════════════════════════════╝");
        Console.WriteLine();

        if (_showcaseItems.Count == 0)
        {
            Console.WriteLine("No items to export. Generate showcase items first.");
            return;
        }

        Console.WriteLine("WORLD GENERATION OPTIONS - VISUAL SHOWCASE REPORT");
        Console.WriteLine("================================================");
        Console.WriteLine($"Generated: {DateTime.Now:yyyy-MM-dd HH:mm:ss}");
        Console.WriteLine($"Total Items: {_showcaseItems.Count}");
        Console.WriteLine();

        Console.WriteLine("RENDERING MODES:");
        Console.WriteLine("----------------");
        Console.WriteLine("  PBR (Physically Based Rendering):");
        Console.WriteLine("    - Realistic lighting and materials");
        Console.WriteLine("    - Metallic/roughness workflow");
        Console.WriteLine("    - Environment reflections");
        Console.WriteLine();
        Console.WriteLine("  NPR (Non-Photorealistic Rendering):");
        Console.WriteLine("    - Cel-shading with discrete light bands");
        Console.WriteLine("    - Edge detection for cartoon outlines");
        Console.WriteLine("    - Stylized artistic look");
        Console.WriteLine();
        Console.WriteLine("  Hybrid Mode (RECOMMENDED):");
        Console.WriteLine("    - PBR lighting with NPR edge detection");
        Console.WriteLine("    - Better block visibility through outlines");
        Console.WriteLine("    - Ambient occlusion between blocks");
        Console.WriteLine();

        Console.WriteLine("GENERATION OPTIONS:");
        Console.WriteLine("-------------------");
        
        var categories = _showcaseItems.GroupBy(i => i.Category);
        foreach (var category in categories)
        {
            Console.WriteLine($"\n  {category.Key}:");
            foreach (var item in category)
            {
                Console.WriteLine($"    - {item.Name}");
                foreach (var prop in item.Properties.Take(3))
                {
                    Console.WriteLine($"        {prop.Key}: {prop.Value}");
                }
            }
        }

        Console.WriteLine();
        Console.WriteLine("Run '3D Graphics Demo' (option 11) to visualize all generated items.");
        Console.WriteLine("Use 'NPR/PBR Rendering Mode Demo' (option 30) to adjust rendering settings.");
    }

    /// <summary>
    /// Get all generated showcase items
    /// </summary>
    public List<ShowcaseItem> GetShowcaseItems() => _showcaseItems;
}
