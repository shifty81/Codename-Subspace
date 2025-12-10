using System.Numerics;
using AvorionLike.Core.Voxel;
using AvorionLike.Core.Logging;

namespace AvorionLike.Core.Procedural;

/// <summary>
/// Station size categories - all stations are massive structures
/// </summary>
public enum StationSize
{
    Small,      // 2000-3000 blocks (minimum)
    Medium,     // 3000-5000 blocks
    Large,      // 5000-8000 blocks
    Massive     // 8000+ blocks
}

/// <summary>
/// Station architectural style - ENHANCED with more creative options
/// </summary>
public enum StationArchitecture
{
    Modular,        // Connected modules and sections
    Ring,           // Ring-shaped station (rotating habitat)
    Tower,          // Tall spire structure
    Industrial,     // Complex industrial framework
    Sprawling,      // Spread-out complex structure
    Organic,        // Bio-inspired flowing shapes
    Crystalline,    // Crystal-like geometric structures
    Spherical,      // Massive sphere-based design
    Helix,          // Double-helix DNA-like structure
    Flower          // Petal-like radiating sections
}

/// <summary>
/// Configuration for station generation
/// </summary>
public class StationGenerationConfig
{
    public StationSize Size { get; set; } = StationSize.Medium;
    public string StationType { get; set; } = "Trading";
    public string Material { get; set; } = "Titanium";
    public StationArchitecture Architecture { get; set; } = StationArchitecture.Modular;
    public int Seed { get; set; } = 0;
    public bool IncludeDockingBays { get; set; } = true;
    public int MinDockingBays { get; set; } = 4;
}

/// <summary>
/// Result of station generation
/// </summary>
public class GeneratedStation
{
    public VoxelStructureComponent Structure { get; set; } = new();
    public StationGenerationConfig Config { get; set; } = new();
    public List<Vector3> DockingPoints { get; set; } = new();
    public List<string> Facilities { get; set; } = new();
    public int BlockCount => Structure.Blocks.Count;
    public float TotalMass => Structure.TotalMass;
}

/// <summary>
/// Procedurally generates massive space stations (minimum 2000 blocks)
/// </summary>
public class ProceduralStationGenerator
{
    private Random _random;
    private readonly Logger _logger = Logger.Instance;
    
    /// <summary>
    /// Create a voxel block with occasional shape variety for stations
    /// </summary>
    private VoxelBlock CreateStationBlock(Vector3 position, Vector3 size, string material, BlockType blockType, float shapeVariety = 0.2f)
    {
        BlockShape shape = BlockShape.Cube;
        BlockOrientation orientation = BlockOrientation.PosY;
        
        // Add shape variety to make stations more architectural
        if (_random.NextDouble() < shapeVariety)
        {
            int shapeChoice = _random.Next(6);
            shape = shapeChoice switch
            {
                0 => BlockShape.Wedge,
                1 => BlockShape.Corner,
                2 => BlockShape.HalfBlock,
                3 => BlockShape.Tetrahedron,
                4 => BlockShape.InnerCorner,
                _ => BlockShape.Cube
            };
            
            if (shape != BlockShape.Cube)
            {
                orientation = (BlockOrientation)_random.Next(6);
            }
        }
        
        return new VoxelBlock(position, size, material, blockType, shape, orientation);
    }
    
    public ProceduralStationGenerator(int seed = 0)
    {
        _random = seed == 0 ? new Random() : new Random(seed);
    }
    
    /// <summary>
    /// Generate a complete massive station
    /// </summary>
    public GeneratedStation GenerateStation(StationGenerationConfig config)
    {
        _random = new Random(config.Seed == 0 ? Environment.TickCount : config.Seed);
        
        var result = new GeneratedStation { Config = config };
        
        _logger.Info("StationGenerator", $"Generating {config.Size} {config.StationType} station with {config.Architecture} architecture");
        
        // Step 1: Determine station dimensions based on size (ensure 2000+ blocks)
        var coreDimensions = GetStationCoreDimensions(config.Size);
        
        // Step 2: Generate core structure based on architecture
        GenerateCoreStructure(result, coreDimensions, config);
        
        // Step 3: Add type-specific facilities
        AddStationFacilities(result, config);
        
        // Step 4: Add docking bays
        if (config.IncludeDockingBays)
        {
            AddDockingBays(result, config);
        }
        
        // Step 5: Add external details and armor
        AddExternalDetails(result, config);
        
        // Step 6: Add internal superstructure for realism
        AddInternalSuperstructure(result, config);
        
        // Step 7: Add visual enhancements (antennas, lights, patterns)
        AddStationVisualEnhancements(result, config);
        
        _logger.Info("StationGenerator", $"Generated station with {result.BlockCount} blocks, {result.DockingPoints.Count} docking bays");
        
        return result;
    }
    
    /// <summary>
    /// Get core dimensions ensuring minimum 2000 block count
    /// </summary>
    private Vector3 GetStationCoreDimensions(StationSize size)
    {
        return size switch
        {
            StationSize.Small => new Vector3(40, 40, 40),      // ~2000-3000 blocks
            StationSize.Medium => new Vector3(60, 60, 60),     // ~3000-5000 blocks
            StationSize.Large => new Vector3(80, 80, 80),      // ~5000-8000 blocks
            StationSize.Massive => new Vector3(120, 120, 120), // ~8000+ blocks
            _ => new Vector3(60, 60, 60)
        };
    }
    
    /// <summary>
    /// Generate core station structure based on architecture
    /// </summary>
    private void GenerateCoreStructure(GeneratedStation station, Vector3 dimensions, StationGenerationConfig config)
    {
        switch (config.Architecture)
        {
            case StationArchitecture.Modular:
                GenerateModularStation(station, dimensions, config);
                break;
            case StationArchitecture.Ring:
                GenerateRingStation(station, dimensions, config);
                break;
            case StationArchitecture.Tower:
                GenerateTowerStation(station, dimensions, config);
                break;
            case StationArchitecture.Industrial:
                GenerateIndustrialStation(station, dimensions, config);
                break;
            case StationArchitecture.Sprawling:
                GenerateSprawlingStation(station, dimensions, config);
                break;
            case StationArchitecture.Organic:
                GenerateOrganicStation(station, dimensions, config);
                break;
            case StationArchitecture.Crystalline:
                GenerateCrystallineStation(station, dimensions, config);
                break;
            case StationArchitecture.Spherical:
                GenerateSphericalStation(station, dimensions, config);
                break;
            case StationArchitecture.Helix:
                GenerateHelixStation(station, dimensions, config);
                break;
            case StationArchitecture.Flower:
                GenerateFlowerStation(station, dimensions, config);
                break;
        }
    }
    
    /// <summary>
    /// Generate modular station with connected sections - ENHANCED with more variety
    /// </summary>
    private void GenerateModularStation(GeneratedStation station, Vector3 dimensions, StationGenerationConfig config)
    {
        // Central hub - varied core shapes
        float coreRadius = Math.Min(dimensions.X, Math.Min(dimensions.Y, dimensions.Z)) / 3;
        int coreStyle = _random.Next(3);
        
        if (coreStyle == 0)
        {
            // Spherical core
            GenerateSphereSection(station, Vector3.Zero, coreRadius, config.Material);
        }
        else if (coreStyle == 1)
        {
            // Cylindrical core
            GenerateCylinder(station, Vector3.Zero, coreRadius * 0.8f, coreRadius * 2f, config.Material);
        }
        else
        {
            // Cubic core
            GenerateBox(station, Vector3.Zero, new Vector3(coreRadius * 1.6f, coreRadius * 1.6f, coreRadius * 1.6f), config.Material);
        }
        
        // Add 6-10 large modules connected to the core - MORE variety
        int moduleCount = 6 + _random.Next(5); // 6-10 modules (increased max)
        float moduleDistance = coreRadius * (2.2f + (float)_random.NextDouble() * 0.8f); // Varied distance
        
        for (int i = 0; i < moduleCount; i++)
        {
            float angle = (float)(i * 2 * Math.PI / moduleCount);
            
            // Vary vertical positioning more dramatically
            float verticalVariation = (i % 3 == 0 ? 1 : (i % 3 == 1 ? -1 : 0)) * moduleDistance * (0.3f + (float)_random.NextDouble() * 0.3f);
            
            Vector3 modulePos = new Vector3(
                MathF.Cos(angle) * moduleDistance,
                MathF.Sin(angle) * moduleDistance * 0.7f + verticalVariation,
                (i % 2 == 0 ? 1 : -1) * moduleDistance * (0.2f + (float)_random.NextDouble() * 0.3f)
            );
            
            // VARIED module shapes and sizes
            int moduleShape = _random.Next(4);
            
            if (moduleShape == 0)
            {
                // Box module
                Vector3 moduleSize = new Vector3(
                    12 + _random.Next(12),
                    12 + _random.Next(12),
                    18 + _random.Next(18)
                );
                GenerateBox(station, modulePos, moduleSize, config.Material);
            }
            else if (moduleShape == 1)
            {
                // Cylindrical module
                float moduleRadius = 8 + _random.Next(6);
                float moduleHeight = 15 + _random.Next(15);
                GenerateCylinder(station, modulePos, moduleRadius, moduleHeight, config.Material);
            }
            else if (moduleShape == 2)
            {
                // Spherical module
                float moduleRadius = 10 + _random.Next(8);
                GenerateSphereSection(station, modulePos, moduleRadius, config.Material);
            }
            else
            {
                // Composite module (multiple smaller sections)
                for (int j = 0; j < 2 + _random.Next(2); j++)
                {
                    Vector3 submoduleOffset = new Vector3(
                        ((float)_random.NextDouble() - 0.5f) * 8,
                        ((float)_random.NextDouble() - 0.5f) * 8,
                        ((float)_random.NextDouble() - 0.5f) * 8
                    );
                    Vector3 submoduleSize = new Vector3(
                        6 + _random.Next(6),
                        6 + _random.Next(6),
                        8 + _random.Next(8)
                    );
                    GenerateBox(station, modulePos + submoduleOffset, submoduleSize, config.Material);
                }
            }
            
            // Connect module to core with corridor - VARIED corridor styles
            int corridorStyle = _random.Next(3);
            
            if (corridorStyle == 0)
            {
                // Single thick corridor
                GenerateCorridor(station, Vector3.Zero, modulePos, 3f + (float)_random.NextDouble() * 1.5f, config.Material);
            }
            else if (corridorStyle == 1)
            {
                // Double parallel corridors
                Vector3 offset = Vector3.Normalize(Vector3.Cross(modulePos, Vector3.UnitY)) * 2f;
                GenerateCorridor(station, Vector3.Zero, modulePos + offset, 2f, config.Material);
                GenerateCorridor(station, Vector3.Zero, modulePos - offset, 2f, config.Material);
            }
            else
            {
                // Curved/articulated corridor with intermediate connection point
                Vector3 midPoint = modulePos * 0.5f + new Vector3(
                    ((float)_random.NextDouble() - 0.5f) * moduleDistance * 0.3f,
                    ((float)_random.NextDouble() - 0.5f) * moduleDistance * 0.3f,
                    ((float)_random.NextDouble() - 0.5f) * moduleDistance * 0.3f
                );
                GenerateCorridor(station, Vector3.Zero, midPoint, 2.5f, config.Material);
                GenerateCorridor(station, midPoint, modulePos, 2.5f, config.Material);
                
                // Add junction box at midpoint
                GenerateBox(station, midPoint, new Vector3(4, 4, 4), config.Material);
            }
        }
    }
    
    /// <summary>
    /// Generate ring-shaped rotating habitat station
    /// FIXED: Consistent block sizes to prevent overlaps
    /// </summary>
    private void GenerateRingStation(GeneratedStation station, Vector3 dimensions, StationGenerationConfig config)
    {
        float ringRadius = dimensions.X / 2;
        float ringThickness = 8f;
        float ringWidth = 12f;
        float blockSize = 2.5f;  // Fixed size, no random variation
        float spacing = 3f;  // Spacing > blockSize to prevent overlap
        
        // Main ring
        int segments = 64;
        for (int i = 0; i < segments; i++)
        {
            float angle1 = (float)(i * 2 * Math.PI / segments);
            
            Vector3 pos1 = new Vector3(
                MathF.Cos(angle1) * ringRadius,
                MathF.Sin(angle1) * ringRadius,
                0
            );
            
            // Ring cross-section blocks
            for (float y = -ringWidth / 2; y <= ringWidth / 2; y += spacing)
            {
                for (float z = -ringThickness / 2; z <= ringThickness / 2; z += spacing)
                {
                    Vector3 blockPos = pos1 + new Vector3(0, y, z);
                    station.Structure.AddBlock(new VoxelBlock(
                        blockPos,
                        new Vector3(blockSize, blockSize, blockSize),
                        config.Material,
                        BlockType.Hull
                    ));
                }
            }
        }
        
        // Central hub with spokes connecting to ring
        GenerateSphereSection(station, Vector3.Zero, 15f, config.Material);
        
        // Add 8 spokes from center to ring
        for (int i = 0; i < 8; i++)
        {
            float angle = (float)(i * 2 * Math.PI / 8);
            Vector3 ringPoint = new Vector3(
                MathF.Cos(angle) * ringRadius,
                MathF.Sin(angle) * ringRadius,
                0
            );
            GenerateCorridor(station, Vector3.Zero, ringPoint, 2.5f, config.Material);
        }
    }
    
    /// <summary>
    /// Generate tall tower/spire station
    /// FIXED: Consistent block sizes to prevent overlaps
    /// </summary>
    private void GenerateTowerStation(GeneratedStation station, Vector3 dimensions, StationGenerationConfig config)
    {
        float height = dimensions.Y;
        float baseRadius = dimensions.X / 4;
        float blockSize = 2.5f;  // Fixed size, no random variation
        float spacing = 3.5f;  // Spacing > blockSize to prevent overlap
        
        // Build tower from bottom to top with tapering
        for (float y = -height / 2; y < height / 2; y += spacing)
        {
            float progress = (y + height / 2) / height;
            float currentRadius = baseRadius * (1.2f - progress * 0.4f);  // Slight taper
            
            // Create cross-section at this height
            int segments = 16;
            for (int i = 0; i < segments; i++)
            {
                float angle = (float)(i * 2 * Math.PI / segments);
                Vector3 pos = new Vector3(
                    MathF.Cos(angle) * currentRadius,
                    y,
                    MathF.Sin(angle) * currentRadius
                );
                
                station.Structure.AddBlock(new VoxelBlock(
                    pos,
                    new Vector3(blockSize, blockSize, blockSize),
                    config.Material,
                    BlockType.Hull
                ));
            }
            
            // Add cross-bracing every 15 units
            if ((int)y % 15 == 0)
            {
                for (int i = 0; i < 4; i++)
                {
                    float angle = (float)(i * Math.PI / 2);
                    Vector3 strutEnd = new Vector3(
                        MathF.Cos(angle) * currentRadius,
                        y,
                        MathF.Sin(angle) * currentRadius
                    );
                    GenerateCorridor(station, new Vector3(0, y, 0), strutEnd, 2.5f, config.Material);
                }
            }
        }
        
        // Add platforms at intervals
        for (float y = -height / 2 + 20; y < height / 2; y += 30)
        {
            float platformRadius = baseRadius * 1.5f;
            GeneratePlatform(station, new Vector3(0, y, 0), platformRadius, config.Material);
        }
    }
    
    /// <summary>
    /// Generate complex industrial framework station
    /// </summary>
    private void GenerateIndustrialStation(GeneratedStation station, Vector3 dimensions, StationGenerationConfig config)
    {
        // Create large framework structure
        GenerateFramework(station, Vector3.Zero, dimensions, config.Material);
        
        // Add industrial modules randomly throughout
        int moduleCount = 20 + _random.Next(10);
        for (int i = 0; i < moduleCount; i++)
        {
            Vector3 modulePos = new Vector3(
                (_random.NextSingle() - 0.5f) * dimensions.X * 0.8f,
                (_random.NextSingle() - 0.5f) * dimensions.Y * 0.8f,
                (_random.NextSingle() - 0.5f) * dimensions.Z * 0.8f
            );
            
            Vector3 moduleSize = new Vector3(
                8 + _random.Next(8),
                8 + _random.Next(8),
                12 + _random.Next(12)
            );
            
            GenerateBox(station, modulePos, moduleSize, config.Material);
        }
        
        // Add large cylindrical storage tanks
        int tankCount = 8 + _random.Next(4);
        for (int i = 0; i < tankCount; i++)
        {
            Vector3 tankPos = new Vector3(
                (_random.NextSingle() - 0.5f) * dimensions.X * 0.9f,
                (_random.NextSingle() - 0.5f) * dimensions.Y * 0.9f,
                (_random.NextSingle() - 0.5f) * dimensions.Z * 0.9f
            );
            
            float tankRadius = 6 + _random.Next(6);
            float tankHeight = 20 + _random.Next(20);
            GenerateCylinder(station, tankPos, tankRadius, tankHeight, config.Material);
        }
    }
    
    /// <summary>
    /// Generate sprawling complex with many interconnected sections - ENHANCED variety
    /// </summary>
    private void GenerateSprawlingStation(GeneratedStation station, Vector3 dimensions, StationGenerationConfig config)
    {
        // Start with central section - VARIED core
        int coreStyle = _random.Next(3);
        if (coreStyle == 0)
        {
            GenerateBox(station, Vector3.Zero, new Vector3(25, 25, 25), config.Material);
        }
        else if (coreStyle == 1)
        {
            GenerateSphereSection(station, Vector3.Zero, 15f, config.Material);
        }
        else
        {
            GenerateCylinder(station, Vector3.Zero, 12f, 25f, config.Material);
        }
        
        // Grow outward with branching sections - MORE variety in section types
        List<Vector3> growthPoints = new List<Vector3> { Vector3.Zero };
        int sectionCount = 35 + _random.Next(25); // 35-59 sections (increased for more sprawl)
        
        for (int i = 0; i < sectionCount; i++)
        {
            // Pick a random existing point to grow from
            Vector3 fromPoint = growthPoints[_random.Next(growthPoints.Count)];
            
            // Generate random direction with bias toward outward growth
            Vector3 direction = new Vector3(
                (_random.NextSingle() - 0.5f) * 2f,
                (_random.NextSingle() - 0.5f) * 2f,
                (_random.NextSingle() - 0.5f) * 2f
            );
            
            // Bias outward from center
            Vector3 outwardBias = Vector3.Normalize(fromPoint);
            if (outwardBias.Length() > 0.1f)
            {
                direction += outwardBias * 0.6f;
            }
            
            direction = Vector3.Normalize(direction);
            
            float distance = 15 + _random.Next(25); // Varied distances
            Vector3 newPoint = fromPoint + direction * distance;
            
            // VARIED section types
            int sectionType = _random.Next(5);
            
            if (sectionType == 0)
            {
                // Standard box
                Vector3 sectionSize = new Vector3(
                    8 + _random.Next(12),
                    8 + _random.Next(12),
                    12 + _random.Next(15)
                );
                GenerateBox(station, newPoint, sectionSize, config.Material);
            }
            else if (sectionType == 1)
            {
                // Small sphere
                float sphereRadius = 6 + _random.Next(6);
                GenerateSphereSection(station, newPoint, sphereRadius, config.Material);
            }
            else if (sectionType == 2)
            {
                // Cylinder
                float cylRadius = 4 + _random.Next(4);
                float cylHeight = 10 + _random.Next(12);
                GenerateCylinder(station, newPoint, cylRadius, cylHeight, config.Material);
            }
            else if (sectionType == 3)
            {
                // Cluster of small boxes
                for (int j = 0; j < 2 + _random.Next(3); j++)
                {
                    Vector3 clusterOffset = new Vector3(
                        (_random.NextSingle() - 0.5f) * 10,
                        (_random.NextSingle() - 0.5f) * 10,
                        (_random.NextSingle() - 0.5f) * 10
                    );
                    GenerateBox(station, newPoint + clusterOffset, 
                        new Vector3(5 + _random.Next(5), 5 + _random.Next(5), 6 + _random.Next(6)),
                        config.Material);
                }
            }
            else
            {
                // Ring section
                float ringRadius = 8 + _random.Next(6);
                for (int angle = 0; angle < 360; angle += 30)
                {
                    float rad = angle * MathF.PI / 180f;
                    Vector3 ringPos = newPoint + new Vector3(
                        ringRadius * MathF.Cos(rad),
                        ringRadius * MathF.Sin(rad),
                        0
                    );
                    station.Structure.AddBlock(new VoxelBlock(
                        ringPos,
                        new Vector3(2.5f, 2.5f, 2.5f),
                        config.Material,
                        BlockType.Hull
                    ));
                }
            }
            
            // Connect with corridor - VARIED corridor thickness
            float corridorThickness = 2f + _random.NextSingle() * 2f;
            GenerateCorridor(station, fromPoint, newPoint, corridorThickness, config.Material);
            
            growthPoints.Add(newPoint);
            
            // Occasionally add cross-connections for more structural integrity
            if (i > 10 && _random.NextDouble() < 0.15)
            {
                // Connect to a nearby growth point (not the one we just grew from)
                var nearbyPoints = growthPoints
                    .Where(p => p != fromPoint && Vector3.Distance(p, newPoint) < 40 && Vector3.Distance(p, newPoint) > 15)
                    .ToList();
                
                if (nearbyPoints.Count > 0)
                {
                    Vector3 connectTo = nearbyPoints[_random.Next(nearbyPoints.Count)];
                    GenerateCorridor(station, newPoint, connectTo, 1.5f, config.Material);
                }
            }
        }
    }
    
    /// <summary>
    /// Add station-type specific facilities
    /// </summary>
    private void AddStationFacilities(GeneratedStation station, StationGenerationConfig config)
    {
        switch (config.StationType.ToLower())
        {
            case "refinery":
                station.Facilities.Add("Ore Processing Plant");
                station.Facilities.Add("Ingot Storage");
                station.Facilities.Add("Quality Control Lab");
                break;
            case "trading":
                station.Facilities.Add("Market Hall");
                station.Facilities.Add("Cargo Storage");
                station.Facilities.Add("Trading Floor");
                break;
            case "military":
                station.Facilities.Add("Barracks");
                station.Facilities.Add("Armory");
                station.Facilities.Add("Command Center");
                break;
            case "shipyard":
                station.Facilities.Add("Construction Bay");
                station.Facilities.Add("Parts Warehouse");
                station.Facilities.Add("Engineering Lab");
                break;
            default:
                station.Facilities.Add("General Purpose Bay");
                break;
        }
    }
    
    /// <summary>
    /// Add docking bays for ships
    /// </summary>
    private void AddDockingBays(GeneratedStation station, StationGenerationConfig config)
    {
        int bayCount = Math.Max(config.MinDockingBays, 4 + _random.Next(8));
        float bayDistance = 40f;
        
        for (int i = 0; i < bayCount; i++)
        {
            float angle = (float)(i * 2 * Math.PI / bayCount);
            Vector3 bayPos = new Vector3(
                MathF.Cos(angle) * bayDistance,
                0,
                MathF.Sin(angle) * bayDistance
            );
            
            station.DockingPoints.Add(bayPos);
            
            // Create docking structure
            Vector3 baySize = new Vector3(12, 8, 15);
            GenerateBox(station, bayPos, baySize, config.Material);
            
            // Add docking arm
            Vector3 armDirection = Vector3.Normalize(bayPos);
            for (float d = 5; d < 15; d += 3)
            {
                Vector3 armPos = bayPos + armDirection * d;
                station.Structure.AddBlock(new VoxelBlock(
                    armPos,
                    new Vector3(2, 2, 6),
                    config.Material,
                    BlockType.Hull
                ));
            }
        }
    }
    
    /// <summary>
    /// Add external details and armor plating
    /// </summary>
    private void AddExternalDetails(GeneratedStation station, StationGenerationConfig config)
    {
        // Add communication arrays
        int arrayCount = 4 + _random.Next(4);
        for (int i = 0; i < arrayCount; i++)
        {
            Vector3 arrayPos = new Vector3(
                (_random.NextSingle() - 0.5f) * 100,
                (_random.NextSingle() - 0.5f) * 100,
                (_random.NextSingle() - 0.5f) * 100
            );
            
            // Dish array
            for (int j = 0; j < 5; j++)
            {
                station.Structure.AddBlock(new VoxelBlock(
                    arrayPos + new Vector3(j * 2, j, 0),
                    new Vector3(3, 3, 1),
                    config.Material,
                    BlockType.Hull
                ));
            }
        }
    }
    
    /// <summary>
    /// Add internal superstructure for structural realism
    /// FIXED: Use consistent sizing to prevent overlaps
    /// </summary>
    private void AddInternalSuperstructure(GeneratedStation station, StationGenerationConfig config)
    {
        // This ensures we meet the minimum block count
        // Add internal framework beams throughout the station
        var existingBlocks = station.Structure.Blocks.ToList();
        int targetBlocks = config.Size switch
        {
            StationSize.Small => 2000,
            StationSize.Medium => 3000,
            StationSize.Large => 5000,
            StationSize.Massive => 8000,
            _ => 3000
        };
        
        float blockSize = 2.5f;  // Fixed size, no random variation
        
        while (station.BlockCount < targetBlocks && existingBlocks.Count > 0)
        {
            // Pick random existing block
            var refBlock = existingBlocks[_random.Next(existingBlocks.Count)];
            
            // Add nearby structural elements with sufficient spacing
            for (int i = 0; i < 3; i++)
            {
                Vector3 offset = new Vector3(
                    (_random.NextSingle() - 0.5f) * 12,  // Increased from 10 for better spacing
                    (_random.NextSingle() - 0.5f) * 12,
                    (_random.NextSingle() - 0.5f) * 12
                );
                
                Vector3 newPos = refBlock.Position + offset;
                
                // Check if too close to existing blocks
                bool tooClose = station.Structure.Blocks.Any(b => 
                    Vector3.Distance(b.Position, newPos) < 3f);  // Minimum 3-unit separation
                
                if (!tooClose)
                {
                    station.Structure.AddBlock(new VoxelBlock(
                        newPos,
                        new Vector3(blockSize, blockSize, blockSize),
                        config.Material,
                        BlockType.Hull
                    ));
                }
                
                if (station.BlockCount >= targetBlocks) break;
            }
        }
    }
    
    // Helper geometry generation methods
    
    private void GenerateSphereSection(GeneratedStation station, Vector3 center, float radius, string material)
    {
        // FIXED: Use consistent block size and proper spacing to prevent overlaps
        float blockSize = 2.5f;  // Fixed size, no random variation
        float spacing = 3f;  // Spacing must be > blockSize to prevent overlap
        
        for (float x = -radius; x <= radius; x += spacing)
        {
            for (float y = -radius; y <= radius; y += spacing)
            {
                for (float z = -radius; z <= radius; z += spacing)
                {
                    float distance = MathF.Sqrt(x * x + y * y + z * z);
                    if (distance <= radius && distance >= radius - 8)  // Hollow shell
                    {
                        station.Structure.AddBlock(new VoxelBlock(
                            center + new Vector3(x, y, z),
                            new Vector3(blockSize, blockSize, blockSize),
                            material,
                            BlockType.Hull
                        ));
                    }
                }
            }
        }
    }
    
    private void GenerateBox(GeneratedStation station, Vector3 center, Vector3 size, string material)
    {
        // FIXED: Use consistent block size and proper spacing to prevent overlaps
        float blockSize = 2.5f;  // Fixed size, no random variation
        float spacing = 3f;  // Spacing must be > blockSize to prevent overlap
        
        // Generate hollow box
        for (float x = -size.X / 2; x <= size.X / 2; x += spacing)
        {
            for (float y = -size.Y / 2; y <= size.Y / 2; y += spacing)
            {
                for (float z = -size.Z / 2; z <= size.Z / 2; z += spacing)
                {
                    // Only create shell
                    bool isEdge = x <= -size.X / 2 + spacing || x >= size.X / 2 - spacing ||
                                  y <= -size.Y / 2 + spacing || y >= size.Y / 2 - spacing ||
                                  z <= -size.Z / 2 + spacing || z >= size.Z / 2 - spacing;
                    
                    if (isEdge)
                    {
                        station.Structure.AddBlock(CreateStationBlock(
                            center + new Vector3(x, y, z),
                            new Vector3(blockSize, blockSize, blockSize),
                            material,
                            BlockType.Hull,
                            0.3f)); // Add shape variety to box edges
                    }
                }
            }
        }
    }
    
    private void GenerateCorridor(GeneratedStation station, Vector3 start, Vector3 end, float thickness, string material)
    {
        // FIXED: Use consistent spacing to prevent overlaps
        Vector3 direction = end - start;
        float length = direction.Length();
        direction = Vector3.Normalize(direction);
        
        float blockSize = Math.Min(thickness, 2.5f);  // Cap at 2.5f
        float spacing = 3f;  // Spacing > blockSize to prevent overlap
        
        for (float d = 0; d < length; d += spacing)
        {
            Vector3 pos = start + direction * d;
            station.Structure.AddBlock(new VoxelBlock(
                pos,
                new Vector3(blockSize, blockSize, blockSize),
                material,
                BlockType.Hull
            ));
        }
    }
    
    private void GenerateCylinder(GeneratedStation station, Vector3 center, float radius, float height, string material)
    {
        // FIXED: Use consistent block size and proper spacing to prevent overlaps
        float blockSize = 2.5f;  // Fixed size, no random variation
        float spacing = 3.5f;  // Spacing > blockSize to prevent overlap
        
        for (float y = -height / 2; y <= height / 2; y += spacing)
        {
            int segments = 16;
            for (int i = 0; i < segments; i++)
            {
                float angle = (float)(i * 2 * Math.PI / segments);
                Vector3 pos = center + new Vector3(
                    MathF.Cos(angle) * radius,
                    y,
                    MathF.Sin(angle) * radius
                );
                
                station.Structure.AddBlock(new VoxelBlock(
                    pos,
                    new Vector3(blockSize, blockSize, blockSize),
                    material,
                    BlockType.Hull
                ));
            }
        }
    }
    
    private void GeneratePlatform(GeneratedStation station, Vector3 center, float radius, string material)
    {
        // FIXED: Use consistent block size and proper spacing to prevent overlaps
        float blockSize = 2.5f;  // Fixed size, no random variation
        float spacing = 3f;  // Spacing > blockSize to prevent overlap
        
        for (float x = -radius; x <= radius; x += spacing)
        {
            for (float z = -radius; z <= radius; z += spacing)
            {
                float distance = MathF.Sqrt(x * x + z * z);
                if (distance <= radius)
                {
                    station.Structure.AddBlock(new VoxelBlock(
                        center + new Vector3(x, 0, z),
                        new Vector3(blockSize, 1.5f, blockSize),
                        material,
                        BlockType.Hull
                    ));
                }
            }
        }
    }
    
    private void GenerateFramework(GeneratedStation station, Vector3 center, Vector3 dimensions, string material)
    {
        // Create large framework beams along all axes
        float spacing = 10f;
        
        // X-axis beams
        for (float y = -dimensions.Y / 2; y <= dimensions.Y / 2; y += spacing)
        {
            for (float z = -dimensions.Z / 2; z <= dimensions.Z / 2; z += spacing)
            {
                for (float x = -dimensions.X / 2; x <= dimensions.X / 2; x += 3)
                {
                    station.Structure.AddBlock(new VoxelBlock(
                        center + new Vector3(x, y, z),
                        new Vector3(2, 2, 2),
                        material,
                        BlockType.Hull
                    ));
                }
            }
        }
        
        // Y-axis beams
        for (float x = -dimensions.X / 2; x <= dimensions.X / 2; x += spacing)
        {
            for (float z = -dimensions.Z / 2; z <= dimensions.Z / 2; z += spacing)
            {
                for (float y = -dimensions.Y / 2; y <= dimensions.Y / 2; y += 3)
                {
                    station.Structure.AddBlock(new VoxelBlock(
                        center + new Vector3(x, y, z),
                        new Vector3(2, 2, 2),
                        material,
                        BlockType.Hull
                    ));
                }
            }
        }
        
        // Z-axis beams
        for (float x = -dimensions.X / 2; x <= dimensions.X / 2; x += spacing)
        {
            for (float y = -dimensions.Y / 2; y <= dimensions.Y / 2; y += spacing)
            {
                for (float z = -dimensions.Z / 2; z <= dimensions.Z / 2; z += 3)
                {
                    station.Structure.AddBlock(new VoxelBlock(
                        center + new Vector3(x, y, z),
                        new Vector3(2, 2, 2),
                        material,
                        BlockType.Hull
                    ));
                }
            }
        }
    }
    
    /// <summary>
    /// Add visual enhancements to stations (antennas, lights, communication arrays, patterns)
    /// </summary>
    private void AddStationVisualEnhancements(GeneratedStation station, StationGenerationConfig config)
    {
        // Add antenna arrays
        AddStationAntennas(station, config);
        
        // Add communication dishes
        AddCommunicationDishes(station, config);
        
        // Add docking bay lights and markers
        AddDockingLights(station, config);
        
        // Add industrial detailing (pipes, vents)
        AddIndustrialDetails(station, config);
        
        // Add color-coded sections for different station types
        AddStationColorScheme(station, config);
    }
    
    /// <summary>
    /// Add antenna arrays to station
    /// </summary>
    private void AddStationAntennas(GeneratedStation station, StationGenerationConfig config)
    {
        int antennaCount = 8 + _random.Next(12); // 8-19 antennas
        
        var edgeBlocks = station.Structure.Blocks
            .OrderByDescending(b => Math.Abs(b.Position.X) + Math.Abs(b.Position.Y) + Math.Abs(b.Position.Z))
            .Take(antennaCount * 3)
            .ToList();
        
        for (int i = 0; i < Math.Min(antennaCount, edgeBlocks.Count); i++)
        {
            var baseBlock = edgeBlocks[i * 3];
            
            // Determine antenna direction (outward from center)
            Vector3 direction = Vector3.Normalize(baseBlock.Position);
            
            // Add tall thin antenna
            float antennaHeight = 10 + (float)_random.NextDouble() * 15; // 10-25 units
            var antennaSize = new Vector3(0.5f, 0.5f, antennaHeight);
            
            // Orient antenna based on position
            if (Math.Abs(direction.Z) > 0.7f)
            {
                antennaSize = new Vector3(0.5f, 0.5f, antennaHeight);
            }
            else if (Math.Abs(direction.Y) > 0.7f)
            {
                antennaSize = new Vector3(0.5f, antennaHeight, 0.5f);
            }
            else
            {
                antennaSize = new Vector3(antennaHeight, 0.5f, 0.5f);
            }
            
            var antenna = new VoxelBlock(
                baseBlock.Position + direction * (antennaHeight / 2 + 2),
                antennaSize,
                config.Material,
                BlockType.TurretMount
            );
            station.Structure.AddBlock(antenna);
        }
    }
    
    /// <summary>
    /// Add communication dishes to station
    /// </summary>
    private void AddCommunicationDishes(GeneratedStation station, StationGenerationConfig config)
    {
        int dishCount = 4 + _random.Next(6); // 4-9 dishes
        
        var outerBlocks = station.Structure.Blocks
            .OrderByDescending(b => (b.Position).Length())
            .Take(dishCount * 5)
            .ToList();
        
        for (int i = 0; i < Math.Min(dishCount, outerBlocks.Count); i++)
        {
            var baseBlock = outerBlocks[i * 5];
            Vector3 direction = Vector3.Normalize(baseBlock.Position);
            
            // Add dish structure (3 blocks forming dish shape)
            float dishSize = 3 + (float)_random.NextDouble() * 2; // 3-5 units
            
            // Base of dish
            var dishBase = new VoxelBlock(
                baseBlock.Position + direction * 2,
                new Vector3(1, 1, 2),
                config.Material,
                BlockType.Hull
            );
            station.Structure.AddBlock(dishBase);
            
            // Dish plate
            var dishPlate = new VoxelBlock(
                baseBlock.Position + direction * (2 + dishSize / 2),
                new Vector3(dishSize, dishSize, 0.5f),
                config.Material,
                BlockType.Hull
            );
            station.Structure.AddBlock(dishPlate);
        }
    }
    
    /// <summary>
    /// Add lights and markers around docking bays
    /// </summary>
    private void AddDockingLights(GeneratedStation station, StationGenerationConfig config)
    {
        foreach (var dockingPoint in station.DockingPoints)
        {
            // Add lights around docking bay (4 corner lights)
            for (int i = 0; i < 4; i++)
            {
                float angle = i * MathF.PI / 2;
                Vector3 offset = new Vector3(
                    MathF.Cos(angle) * 8,
                    MathF.Sin(angle) * 8,
                    0
                );
                
                var light = new VoxelBlock(
                    dockingPoint + offset,
                    new Vector3(1, 1, 1),
                    "Energy", // Use energy material for glow
                    BlockType.Hull
                );
                station.Structure.AddBlock(light);
            }
            
            // Add green approach light
            var approachLight = new VoxelBlock(
                dockingPoint + new Vector3(0, 0, 15),
                new Vector3(1.5f, 1.5f, 1.5f),
                "Energy",
                BlockType.Hull
            );
            station.Structure.AddBlock(approachLight);
        }
    }
    
    /// <summary>
    /// Add industrial details like pipes and vents
    /// </summary>
    private void AddIndustrialDetails(GeneratedStation station, StationGenerationConfig config)
    {
        if (config.StationType != "Trading" && config.StationType != "Industrial") return;
        
        int detailCount = 20 + _random.Next(30); // 20-49 details
        
        var surfaceBlocks = station.Structure.Blocks
            .Where(b => b.BlockType == BlockType.Hull)
            .OrderBy(x => _random.Next())
            .Take(detailCount * 2)
            .ToList();
        
        for (int i = 0; i < Math.Min(detailCount, surfaceBlocks.Count); i++)
        {
            var baseBlock = surfaceBlocks[i * 2];
            
            // Add pipe or vent
            if (_random.NextDouble() < 0.5)
            {
                // Pipe - elongated along a random axis
                var pipeSize = new Vector3(1, 1, 5 + (float)_random.NextDouble() * 5);
                var pipe = new VoxelBlock(
                    baseBlock.Position + new Vector3(0, 0, pipeSize.Z / 2),
                    pipeSize,
                    config.Material,
                    BlockType.Hull
                );
                station.Structure.AddBlock(pipe);
            }
            else
            {
                // Vent - flat panel
                var ventSize = new Vector3(2, 2, 0.5f);
                var vent = new VoxelBlock(
                    baseBlock.Position + new Vector3(0, 0, 1),
                    ventSize,
                    config.Material,
                    BlockType.Hull
                );
                station.Structure.AddBlock(vent);
            }
        }
    }
    
    /// <summary>
    /// Add color scheme to station based on type
    /// </summary>
    private void AddStationColorScheme(GeneratedStation station, StationGenerationConfig config)
    {
        uint primaryColor, secondaryColor, accentColor;
        
        // Choose colors based on station type
        switch (config.StationType.ToLower())
        {
            case "trading":
                primaryColor = 0xDAA520; // Goldenrod
                secondaryColor = 0xF0E68C; // Khaki
                accentColor = 0xFFD700; // Gold
                break;
            case "military":
                primaryColor = 0x2F4F4F; // Dark Slate Gray
                secondaryColor = 0x708090; // Slate Gray
                accentColor = 0xFF0000; // Red
                break;
            case "industrial":
                primaryColor = 0xB8860B; // Dark Goldenrod
                secondaryColor = 0x696969; // Dim Gray
                accentColor = 0xFFA500; // Orange
                break;
            case "research":
                primaryColor = 0x4169E1; // Royal Blue
                secondaryColor = 0xADD8E6; // Light Blue
                accentColor = 0x00CED1; // Dark Turquoise
                break;
            default:
                primaryColor = 0x808080; // Gray
                secondaryColor = 0x696969; // Dim Gray
                accentColor = 0xC0C0C0; // Silver
                break;
        }
        
        // Apply colors to blocks
        foreach (var block in station.Structure.Blocks)
        {
            if (block.BlockType == BlockType.Hull)
            {
                // Randomly use primary or secondary
                block.ColorRGB = _random.NextDouble() < 0.7 ? primaryColor : secondaryColor;
            }
            else if (block.BlockType == BlockType.TurretMount)
            {
                block.ColorRGB = accentColor;
            }
        }
    }
    
    /// <summary>
    /// Generate organic flowing station (bio-inspired design)
    /// </summary>
    private void GenerateOrganicStation(GeneratedStation station, Vector3 dimensions, StationGenerationConfig config)
    {
        float blockSize = 2.5f;
        float spacing = 3f;
        
        // Central "core" - organic bulbous shape
        float coreRadius = Math.Min(dimensions.X, Math.Min(dimensions.Y, dimensions.Z)) / 4;
        GenerateSphereSection(station, Vector3.Zero, coreRadius, config.Material);
        
        // Add 4-6 flowing "tentacles" or arms radiating outward
        int armCount = 4 + _random.Next(3);
        
        for (int i = 0; i < armCount; i++)
        {
            float baseAngle = (float)(i * 2 * Math.PI / armCount);
            float angleVariation = (float)((_random.NextDouble() - 0.5f) * Math.PI / 6);
            float angle = baseAngle + angleVariation;
            
            // Create flowing curved arm
            int segments = 10 + _random.Next(10);
            float armLength = dimensions.X * 0.4f;
            float segmentLength = armLength / segments;
            
            Vector3 currentPos = Vector3.Zero;
            Vector3 currentDir = new Vector3(
                (float)Math.Cos(angle),
                (float)(_random.NextDouble() - 0.5f) * 0.3f,
                (float)Math.Sin(angle)
            );
            
            for (int j = 0; j < segments; j++)
            {
                float progress = j / (float)segments;
                float radius = coreRadius * (1.0f - progress * 0.7f); // Taper
                
                // Add slight curve/wave
                float wave = (float)Math.Sin(progress * Math.PI * 2 + angle) * 5f;
                currentDir = Vector3.Normalize(currentDir + new Vector3(
                    (float)(_random.NextDouble() - 0.5f) * 0.2f,
                    wave * 0.1f,
                    (float)(_random.NextDouble() - 0.5f) * 0.2f
                ));
                
                currentPos += currentDir * segmentLength;
                
                // Generate segment - organic bulbous sections
                for (float r = 0; r < radius; r += spacing)
                {
                    for (float theta = 0; theta < 360; theta += 30)
                    {
                        float rad = theta * MathF.PI / 180f;
                        Vector3 offset = new Vector3(
                            r * MathF.Cos(rad),
                            r * MathF.Sin(rad),
                            0
                        );
                        
                        station.Structure.AddBlock(new VoxelBlock(
                            currentPos + offset,
                            new Vector3(blockSize, blockSize, blockSize),
                            config.Material,
                            BlockType.Hull
                        ));
                    }
                }
                
                // Add occasional bulges
                if (j % 3 == 0)
                {
                    float bulgeRadius = radius * 1.3f;
                    GenerateSphereSection(station, currentPos, bulgeRadius, config.Material);
                }
            }
        }
    }
    
    /// <summary>
    /// Generate crystalline geometric station
    /// </summary>
    private void GenerateCrystallineStation(GeneratedStation station, Vector3 dimensions, StationGenerationConfig config)
    {
        float blockSize = 2.5f;
        float spacing = 3f;
        
        // Central geometric core - octahedron
        float coreSize = Math.Min(dimensions.X, Math.Min(dimensions.Y, dimensions.Z)) / 3;
        GenerateOctahedron(station, Vector3.Zero, coreSize, config.Material);
        
        // Add crystal spikes radiating outward
        int spikeCount = 8 + _random.Next(8);
        
        for (int i = 0; i < spikeCount; i++)
        {
            float angle = (float)(i * 2 * Math.PI / spikeCount);
            float elevation = (float)((_random.NextDouble() - 0.5f) * Math.PI / 3);
            
            Vector3 direction = new Vector3(
                (float)(Math.Cos(angle) * Math.Cos(elevation)),
                (float)Math.Sin(elevation),
                (float)(Math.Sin(angle) * Math.Cos(elevation))
            );
            
            // Create sharp crystal spike
            int spikeLength = 15 + _random.Next(20);
            float baseWidth = 4f + (float)_random.NextDouble() * 3f;
            
            for (int j = 0; j < spikeLength; j++)
            {
                float progress = j / (float)spikeLength;
                float width = baseWidth * (1.0f - progress * 0.9f); // Sharp taper
                
                Vector3 pos = direction * (coreSize + j * spacing);
                
                // Create diamond cross-section
                for (float x = -width; x <= width; x += spacing)
                {
                    for (float y = -width; y <= width; y += spacing)
                    {
                        // Diamond shape filter
                        if (Math.Abs(x) + Math.Abs(y) <= width)
                        {
                            station.Structure.AddBlock(new VoxelBlock(
                                pos + new Vector3(x, y, 0),
                                new Vector3(blockSize, blockSize, blockSize),
                                "Crystal",  // Use crystal material
                                BlockType.Hull
                            ));
                        }
                    }
                }
            }
        }
        
        // Add floating crystal fragments
        int fragmentCount = 10 + _random.Next(15);
        for (int i = 0; i < fragmentCount; i++)
        {
            float distance = dimensions.X * 0.3f + (float)_random.NextDouble() * dimensions.X * 0.3f;
            float angle = (float)(_random.NextDouble() * Math.PI * 2);
            float elevation = (float)((_random.NextDouble() - 0.5f) * Math.PI / 4);
            
            Vector3 pos = new Vector3(
                (float)(Math.Cos(angle) * Math.Cos(elevation)) * distance,
                (float)Math.Sin(elevation) * distance,
                (float)(Math.Sin(angle) * Math.Cos(elevation)) * distance
            );
            
            float fragmentSize = 3f + (float)_random.NextDouble() * 5f;
            GenerateOctahedron(station, pos, fragmentSize, "Crystal");
        }
    }
    
    /// <summary>
    /// Generate massive spherical station (Death Star-style)
    /// </summary>
    private void GenerateSphericalStation(GeneratedStation station, Vector3 dimensions, StationGenerationConfig config)
    {
        float blockSize = 2.5f;
        float spacing = 3.5f;
        
        // Main sphere
        float mainRadius = Math.Min(dimensions.X, Math.Min(dimensions.Y, dimensions.Z)) / 2.5f;
        GenerateSphereSection(station, Vector3.Zero, mainRadius, config.Material);
        
        // Add equatorial trench/ring
        float trenchRadius = mainRadius * 1.05f;
        float trenchWidth = mainRadius * 0.15f;
        
        for (float angle = 0; angle < 360; angle += 10)
        {
            float rad = angle * MathF.PI / 180f;
            
            for (float z = -trenchWidth; z <= trenchWidth; z += spacing)
            {
                Vector3 pos = new Vector3(
                    trenchRadius * MathF.Cos(rad),
                    z,
                    trenchRadius * MathF.Sin(rad)
                );
                
                station.Structure.AddBlock(new VoxelBlock(
                    pos,
                    new Vector3(blockSize, blockSize * 2, blockSize),
                    config.Material,
                    BlockType.Hull
                ));
            }
        }
        
        // Add surface details - panels and structures
        int panelCount = 20 + _random.Next(30);
        for (int i = 0; i < panelCount; i++)
        {
            float angle = (float)(_random.NextDouble() * Math.PI * 2);
            float elevation = (float)((_random.NextDouble() - 0.5f) * Math.PI);
            
            Vector3 surfacePos = new Vector3(
                mainRadius * (float)(Math.Cos(angle) * Math.Cos(elevation)),
                mainRadius * (float)Math.Sin(elevation),
                mainRadius * (float)(Math.Sin(angle) * Math.Cos(elevation))
            );
            
            // Add small surface structure
            float panelSize = 3f + (float)_random.NextDouble() * 4f;
            GenerateBox(station, surfacePos, new Vector3(panelSize, panelSize * 0.3f, panelSize), config.Material);
        }
        
        // Add polar docking ports
        GenerateCylinder(station, new Vector3(0, mainRadius, 0), mainRadius * 0.15f, mainRadius * 0.3f, config.Material);
        GenerateCylinder(station, new Vector3(0, -mainRadius, 0), mainRadius * 0.15f, mainRadius * 0.3f, config.Material);
    }
    
    /// <summary>
    /// Generate helix/DNA-style station
    /// </summary>
    private void GenerateHelixStation(GeneratedStation station, Vector3 dimensions, StationGenerationConfig config)
    {
        float blockSize = 2.5f;
        float spacing = 3f;
        
        float height = dimensions.Y;
        float radius = dimensions.X / 4;
        int turns = 3 + _random.Next(3);  // 3-5 full turns
        
        // Generate two intertwined helixes
        for (int helixIndex = 0; helixIndex < 2; helixIndex++)
        {
            float angleOffset = helixIndex * MathF.PI;  // 180° offset for second helix
            
            for (float y = -height / 2; y < height / 2; y += spacing)
            {
                float progress = (y + height / 2) / height;
                float angle = progress * turns * 2 * MathF.PI + angleOffset;
                
                Vector3 pos = new Vector3(
                    radius * MathF.Cos(angle),
                    y,
                    radius * MathF.Sin(angle)
                );
                
                // Create tube cross-section
                float tubeRadius = 4f + (float)Math.Sin(progress * Math.PI * 4) * 2f;  // Varying thickness
                
                for (float r = 0; r < tubeRadius; r += spacing / 2)
                {
                    for (float theta = 0; theta < 360; theta += 30)
                    {
                        float rad = theta * MathF.PI / 180f;
                        Vector3 offset = new Vector3(
                            r * MathF.Cos(rad) * MathF.Cos(angle),
                            0,
                            r * MathF.Sin(rad)
                        );
                        
                        station.Structure.AddBlock(new VoxelBlock(
                            pos + offset,
                            new Vector3(blockSize, blockSize, blockSize),
                            config.Material,
                            BlockType.Hull
                        ));
                    }
                }
            }
        }
        
        // Add connecting rungs between helixes
        int rungCount = turns * 4;  // 4 rungs per turn
        for (int i = 0; i < rungCount; i++)
        {
            float progress = i / (float)rungCount;
            float y = -height / 2 + progress * height;
            float angle = progress * turns * 2 * MathF.PI;
            
            Vector3 pos1 = new Vector3(
                radius * MathF.Cos(angle),
                y,
                radius * MathF.Sin(angle)
            );
            
            Vector3 pos2 = new Vector3(
                radius * MathF.Cos(angle + MathF.PI),
                y,
                radius * MathF.Sin(angle + MathF.PI)
            );
            
            GenerateCorridor(station, pos1, pos2, 2f, config.Material);
        }
    }
    
    /// <summary>
    /// Generate flower/petal-style radiating station
    /// </summary>
    private void GenerateFlowerStation(GeneratedStation station, Vector3 dimensions, StationGenerationConfig config)
    {
        float blockSize = 2.5f;
        float spacing = 3f;
        
        // Central bulb
        float bulbRadius = Math.Min(dimensions.X, dimensions.Z) / 6;
        GenerateSphereSection(station, Vector3.Zero, bulbRadius, config.Material);
        
        // Generate petals radiating outward
        int petalCount = 5 + _random.Next(4);  // 5-8 petals
        
        for (int i = 0; i < petalCount; i++)
        {
            float angle = (float)(i * 2 * Math.PI / petalCount);
            
            // Each petal is a curved, flattened structure
            int petalSegments = 15;
            float petalLength = dimensions.X * 0.35f;
            float petalMaxWidth = petalLength * 0.4f;
            
            for (int j = 0; j < petalSegments; j++)
            {
                float progress = j / (float)(petalSegments - 1);
                float distance = bulbRadius + progress * petalLength;
                
                // Petal width - wide in middle, narrow at tip
                float widthFactor = (float)Math.Sin(progress * Math.PI);
                float width = petalMaxWidth * widthFactor;
                
                // Upward curve
                float curvature = (float)Math.Sin(progress * Math.PI) * petalLength * 0.2f;
                
                Vector3 petalCenter = new Vector3(
                    (float)Math.Cos(angle) * distance,
                    curvature,
                    (float)Math.Sin(angle) * distance
                );
                
                // Create petal cross-section (flat and wide)
                for (float x = -width; x <= width; x += spacing)
                {
                    // Elliptical cross-section
                    float maxZ = width * 0.3f * (1.0f - Math.Abs(x) / width);
                    
                    for (float z = -maxZ; z <= maxZ; z += spacing)
                    {
                        // Rotate around center
                        float rotX = x * (float)Math.Cos(angle) - z * (float)Math.Sin(angle);
                        float rotZ = x * (float)Math.Sin(angle) + z * (float)Math.Cos(angle);
                        
                        station.Structure.AddBlock(new VoxelBlock(
                            petalCenter + new Vector3(rotX, 0, rotZ),
                            new Vector3(blockSize, blockSize * 0.5f, blockSize),
                            config.Material,
                            BlockType.Hull
                        ));
                    }
                }
            }
        }
        
        // Add connecting structures between petals (stamens)
        for (int i = 0; i < petalCount; i++)
        {
            float angle = (float)(i * 2 * Math.PI / petalCount + Math.PI / petalCount);
            float stamenLength = dimensions.X * 0.2f;
            
            Vector3 stamenTip = new Vector3(
                (float)Math.Cos(angle) * (bulbRadius + stamenLength),
                stamenLength * 0.5f,
                (float)Math.Sin(angle) * (bulbRadius + stamenLength)
            );
            
            // Thin cylindrical stamen
            for (float t = 0; t < stamenLength; t += spacing)
            {
                float progress = t / stamenLength;
                Vector3 pos = Vector3.Lerp(Vector3.Zero, stamenTip, progress);
                
                station.Structure.AddBlock(new VoxelBlock(
                    pos,
                    new Vector3(blockSize * 0.5f, blockSize * 0.5f, blockSize * 0.5f),
                    config.Material,
                    BlockType.Hull
                ));
            }
        }
    }
    
    /// <summary>
    /// Generate octahedron (8-sided polyhedron) - useful for crystal structures
    /// </summary>
    private void GenerateOctahedron(GeneratedStation station, Vector3 center, float size, string material)
    {
        float blockSize = 2.5f;
        float spacing = 3f;
        
        // Octahedron has 6 vertices: ±size on each axis
        Vector3[] vertices = {
            center + new Vector3(size, 0, 0),
            center + new Vector3(-size, 0, 0),
            center + new Vector3(0, size, 0),
            center + new Vector3(0, -size, 0),
            center + new Vector3(0, 0, size),
            center + new Vector3(0, 0, -size)
        };
        
        // Fill octahedron volume
        for (float x = -size; x <= size; x += spacing)
        {
            for (float y = -size; y <= size; y += spacing)
            {
                for (float z = -size; z <= size; z += spacing)
                {
                    // Octahedron SDF: |x| + |y| + |z| <= size
                    if (Math.Abs(x) + Math.Abs(y) + Math.Abs(z) <= size)
                    {
                        station.Structure.AddBlock(new VoxelBlock(
                            center + new Vector3(x, y, z),
                            new Vector3(blockSize, blockSize, blockSize),
                            material,
                            BlockType.Hull
                        ));
                    }
                }
            }
        }
    }
}
