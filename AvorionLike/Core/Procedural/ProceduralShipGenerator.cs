using System.Numerics;
using AvorionLike.Core.Voxel;
using AvorionLike.Core.Logging;

namespace AvorionLike.Core.Procedural;

/// <summary>
/// Ship size categories
/// </summary>
public enum ShipSize
{
    Fighter,      // 10-20 blocks, single pilot
    Corvette,     // 20-50 blocks, small crew
    Frigate,      // 50-100 blocks, medium crew
    Destroyer,    // 100-200 blocks, large crew
    Cruiser,      // 200-400 blocks, major vessel
    Battleship,   // 400-800 blocks, capital ship
    Carrier       // 800+ blocks, massive carrier
}

/// <summary>
/// Ship role determines functional block distribution
/// </summary>
public enum ShipRole
{
    Multipurpose,  // Balanced stats
    Combat,        // Heavy weapons, strong shields
    Mining,        // Mining lasers, large cargo
    Trading,       // Huge cargo, minimal weapons
    Exploration,   // Long range sensors, hyperdrive
    Salvage        // Salvage beams, cargo space
}

/// <summary>
/// Configuration for ship generation following Avorion-style design principles
/// </summary>
public class ShipGenerationConfig
{
    public ShipSize Size { get; set; } = ShipSize.Frigate;
    public ShipRole Role { get; set; } = ShipRole.Multipurpose;
    public string Material { get; set; } = "Iron";
    public FactionShipStyle Style { get; set; } = FactionShipStyle.GetDefaultStyle("Default");
    public int Seed { get; set; } = 0;
    
    // Functional requirements
    public bool RequireHyperdrive { get; set; } = true;
    public bool RequireCargo { get; set; } = true;
    public int MinimumWeaponMounts { get; set; } = 2;
    
    // Avorion-style block configuration
    public float BlockComplexity { get; set; } = 0.5f; // 0-1, affects number of blocks
    public float VolumeScaling { get; set; } = 1.0f; // Overall size multiplier
    public bool UseAngledBlocks { get; set; } = true; // Use wedge/angled blocks
    public bool UseBoxAesthetic { get; set; } = true; // Avorion box-like look
    
    // Core component requirements
    public bool RequireIntegrityField { get; set; } = true;
    public bool RequirePowerCore { get; set; } = true;
    public int TargetUpgradeSlots { get; set; } = 5;
    
    // Modularity
    public bool UseModularSections { get; set; } = true;
    public int ModularSectionCount { get; set; } = 3;
    
    /// <summary>
    /// Apply style settings to this config
    /// </summary>
    public void ApplyStyle()
    {
        if (Style == null) return;
        
        BlockComplexity = Style.BlockComplexity;
        VolumeScaling = Style.VolumeScaling;
        UseAngledBlocks = Style.UseAngledBlocks;
        UseBoxAesthetic = Style.UseBoxAesthetic;
        RequireIntegrityField = Style.RequireIntegrityField;
        RequirePowerCore = Style.RequirePowerCore;
        TargetUpgradeSlots = Style.TargetUpgradeSlots;
        UseModularSections = Style.UseModularSections;
        ModularSectionCount = Style.ModularSectionCount;
        
        // Use material from style if not explicitly set
        if (Material == "Iron" && !string.IsNullOrEmpty(Style.PreferredMaterial))
        {
            Material = Style.PreferredMaterial;
        }
    }
}

/// <summary>
/// Result of ship generation containing the structure and metadata
/// </summary>
public class GeneratedShip
{
    public VoxelStructureComponent Structure { get; set; } = new();
    public ShipGenerationConfig Config { get; set; } = new();
    public Dictionary<string, float> Stats { get; set; } = new();
    public List<string> Warnings { get; set; } = new();
    
    public float TotalMass => Structure.TotalMass;
    public float TotalThrust { get; set; }
    public float TotalPowerGeneration { get; set; }
    public float TotalShieldCapacity { get; set; }
    public int WeaponMountCount { get; set; }
    public int CargoBlockCount { get; set; }
    public int UpgradeSlots { get; set; }
    public int ComputerBlockCount { get; set; }
}

/// <summary>
/// Procedurally generates functional ships based on faction styles and requirements
/// Following Avorion-style block-based design principles
/// </summary>
public class ProceduralShipGenerator
{
    private Random _random;
    private readonly Logger _logger = Logger.Instance;
    
    // Visual effect colors
    private const uint ENGINE_COLOR = 0x3366FF;      // Blue tint for engines
    private const uint NOZZLE_COLOR = 0x2244CC;      // Darker blue for nozzles
    private const uint ENGINE_GLOW_COLOR = 0x00FFFF; // Cyan primary glow
    private const uint OUTER_GLOW_COLOR = 0x0088CC;  // Dimmer blue outer glow
    
    public ProceduralShipGenerator(int seed = 0)
    {
        _random = seed == 0 ? new Random() : new Random(seed);
    }
    
    /// <summary>
    /// Generate a complete ship based on configuration
    /// Uses Avorion-style block-based design with modular sections
    /// </summary>
    public GeneratedShip GenerateShip(ShipGenerationConfig config)
    {
        // Initialize random with seed for reproducible generation
        _random = new Random(config.Seed == 0 ? Environment.TickCount : config.Seed);
        
        // Apply style settings to config
        config.ApplyStyle();
        
        var result = new GeneratedShip { Config = config };
        
        _logger.Info("ShipGenerator", $"Generating {config.Size} {config.Role} ship for {config.Style.FactionName} (Seed: {config.Seed})");
        
        // Step 1: Determine ship dimensions based on size and volume scaling
        var dimensions = GetShipDimensions(config.Size);
        dimensions *= config.VolumeScaling; // Apply Avorion-style volume scaling
        
        // Step 2: Generate core hull structure using Avorion-style modular approach
        GenerateHullStructure(result, dimensions, config);
        
        // Step 3: Place core components (generators, power core, integrity field)
        PlaceCoreComponents(result, dimensions, config);
        
        // Step 4: Place functional components (engines, thrusters, etc.)
        PlaceFunctionalComponents(result, config);
        
        // Step 5: Add weapons based on role
        PlaceWeaponMounts(result, config);
        
        // Step 6: Add utility blocks (cargo, hyperdrive, computer blocks for upgrade slots)
        PlaceUtilityBlocks(result, config);
        
        // Step 7: Add armor plating based on faction style
        AddArmorPlating(result, config);
        
        // Step 8: Apply faction color scheme
        ApplyColorScheme(result, config);
        
        // Step 9: Add surface detailing (greebles, antennas, vents) based on complexity
        if (config.BlockComplexity > 0.3f)
        {
            AddSurfaceDetailing(result, config);
        }
        
        // Step 10: Calculate final statistics including upgrade slots
        CalculateShipStats(result);
        
        // Step 11: Validate ship is functional
        ValidateShip(result, config);
        
        // Step 12: Validate structural integrity
        ValidateStructuralIntegrity(result, config);
        
        // Step 13: Validate functional requirements
        ValidateFunctionalRequirements(result, config);
        
        // Step 14: Validate aesthetic guidelines
        ValidateAesthetics(result, config);
        
        _logger.Info("ShipGenerator", $"Generated ship with {result.Structure.Blocks.Count} blocks, " +
            $"{result.TotalThrust:F0} thrust, {result.TotalPowerGeneration:F0} power, {result.WeaponMountCount} weapons");
        
        return result;
    }
    
    /// <summary>
    /// Get dimensions for a ship based on size category
    /// Enhanced with 1.5-2x block count increase for better aesthetics and detail
    /// </summary>
    private Vector3 GetShipDimensions(ShipSize size)
    {
        return size switch
        {
            ShipSize.Fighter => new Vector3(9, 6, 15),      // 1.5x increase
            ShipSize.Corvette => new Vector3(12, 8, 21),    // 1.5x increase
            ShipSize.Frigate => new Vector3(18, 9, 30),     // 1.5x increase
            ShipSize.Destroyer => new Vector3(24, 12, 42),  // 1.5x increase
            ShipSize.Cruiser => new Vector3(33, 18, 57),    // 1.5x increase
            ShipSize.Battleship => new Vector3(45, 24, 75), // 1.5x increase
            ShipSize.Carrier => new Vector3(60, 33, 98),    // 1.5x increase
            _ => new Vector3(18, 9, 30)
        };
    }
    
    /// <summary>
    /// Get a randomized block size for aesthetic variety
    /// Returns STANDARDIZED sizes (2x2x2) for consistent connectivity
    /// Variation comes from shape (angular blocks) not size
    /// </summary>
    private Vector3 GetRandomBlockSize()
    {
        // Use standard 2x2x2 blocks for consistent spacing and connectivity
        // This prevents jittery appearance and ensures blocks connect properly
        return new Vector3(2, 2, 2);
    }
    
    /// <summary>
    /// Get an angular/wedge block size for more interesting shapes
    /// Creates non-square blocks for angular, sleek designs
    /// UPDATED: Smaller, more refined blocks for sleeker appearance
    /// </summary>
    private Vector3 GetAngularBlockSize()
    {
        // Create sleeker angular blocks with minimal size variation
        // Using consistent 2-unit base for better connectivity
        int shapeType = _random.Next(4);
        
        return shapeType switch
        {
            0 => new Vector3(2, 1, 2),      // Flat panel (horizontal) - sleeker
            1 => new Vector3(2, 2, 1),      // Tall panel (vertical) - sleeker
            2 => new Vector3(3, 1, 2),      // Wide wedge - reduced from 4
            _ => new Vector3(2, 2, 2)       // Standard block for consistency
        };
    }
    
    /// <summary>
    /// Get a stretched block size for long structural elements
    /// Creates elongated blocks for beams, panels, and architectural details
    /// UPDATED: Sleeker proportions for refined appearance
    /// </summary>
    private Vector3 GetStretchedBlockSize(string axis)
    {
        // Use sleeker proportions - thinner beams, better visual flow
        float baseSize = 2f;
        float stretchAmount = 3f;  // Reduced from 4 for sleeker look
        
        return axis.ToLower() switch
        {
            "x" => new Vector3(stretchAmount, baseSize, baseSize),
            "y" => new Vector3(baseSize, stretchAmount, baseSize),
            "z" => new Vector3(baseSize, baseSize, stretchAmount),
            _ => new Vector3(baseSize, baseSize, baseSize)
        };
    }
    
    /// <summary>
    /// Get a random shape for visual variety
    /// Increases shape variation to avoid "just blocks" appearance
    /// </summary>
    private BlockShape GetRandomShape(float wedgeProbability = 0.3f, float cornerProbability = 0.15f)
    {
        float roll = (float)_random.NextDouble();
        
        if (roll < wedgeProbability)
            return BlockShape.Wedge;
        else if (roll < wedgeProbability + cornerProbability)
            return BlockShape.Corner;
        else if (roll < wedgeProbability + cornerProbability + 0.1f)
            return BlockShape.HalfBlock;
        else if (roll < wedgeProbability + cornerProbability + 0.15f)
            return BlockShape.Tetrahedron;
        else if (roll < wedgeProbability + cornerProbability + 0.2f)
            return BlockShape.InnerCorner;
        else
            return BlockShape.Cube;
    }
    
    /// <summary>
    /// Helper to create a VoxelBlock with shape variety
    /// </summary>
    private VoxelBlock CreateBlockWithShapeVariety(Vector3 position, Vector3 size, string material, BlockType blockType, float shapeVariety = 0.3f)
    {
        BlockShape shape = BlockShape.Cube;
        BlockOrientation orientation = BlockOrientation.PosY;
        
        if (_random.NextDouble() < shapeVariety)
        {
            shape = GetRandomShape(0.4f, 0.2f);
            if (shape != BlockShape.Cube)
            {
                orientation = GetRandomOrientation();
            }
        }
        
        return new VoxelBlock(position, size, material, blockType, shape, orientation);
    }
    
    /// <summary>
    /// Get a random orientation for shaped blocks
    /// </summary>
    private BlockOrientation GetRandomOrientation()
    {
        int orientations = 6;
        int choice = _random.Next(orientations);
        
        return choice switch
        {
            0 => BlockOrientation.PosX,
            1 => BlockOrientation.NegX,
            2 => BlockOrientation.PosY,
            3 => BlockOrientation.NegY,
            4 => BlockOrientation.PosZ,
            5 => BlockOrientation.NegZ,
            _ => BlockOrientation.PosY
        };
    }
    
    /// <summary>
    /// Get edge-appropriate shape for hull boundaries
    /// Returns wedges/corners oriented to create smooth transitions
    /// </summary>
    private (BlockShape shape, BlockOrientation orientation) GetEdgeShape(Vector3 position, Vector3 dimensions)
    {
        float halfX = dimensions.X / 2;
        float halfY = dimensions.Y / 2;
        float halfZ = dimensions.Z / 2;
        
        // Check if near edges
        bool nearLeft = position.X < -halfX + 2;
        bool nearRight = position.X > halfX - 2;
        bool nearTop = position.Y > halfY - 2;
        bool nearBottom = position.Y < -halfY + 2;
        bool nearFront = position.Z > halfZ - 2;
        bool nearBack = position.Z < -halfZ + 2;
        
        // Return appropriate shapes for edges
        if (nearFront)
        {
            if (nearTop) return (BlockShape.Corner, BlockOrientation.PosZ);
            if (nearBottom) return (BlockShape.Corner, BlockOrientation.NegZ);
            return (BlockShape.Wedge, BlockOrientation.PosZ);
        }
        if (nearBack)
        {
            if (nearTop) return (BlockShape.Corner, BlockOrientation.NegZ);
            return (BlockShape.Wedge, BlockOrientation.NegZ);
        }
        if (nearTop && (nearLeft || nearRight))
        {
            return (BlockShape.Corner, nearLeft ? BlockOrientation.NegX : BlockOrientation.PosX);
        }
        
        // Default to cube
        return (BlockShape.Cube, BlockOrientation.PosY);
    }
    
    /// <summary>
    /// Generate the basic hull structure based on faction style
    /// </summary>
    private void GenerateHullStructure(GeneratedShip ship, Vector3 dimensions, ShipGenerationConfig config)
    {
        var style = config.Style;
        var shape = style.PreferredHullShape;
        
        // Apply ship type-specific guidelines for better spaceship appearance
        dimensions = ApplyShipTypeGuidelines(dimensions, config);
        
        // Generate hull based on shape preference
        switch (shape)
        {
            case ShipHullShape.Blocky:
                GenerateBlockyHull(ship, dimensions, config);
                break;
            case ShipHullShape.Angular:
                GenerateAngularHull(ship, dimensions, config);
                break;
            case ShipHullShape.Cylindrical:
                GenerateCylindricalHull(ship, dimensions, config);
                break;
            case ShipHullShape.Sleek:
                GenerateSleekHull(ship, dimensions, config);
                break;
            case ShipHullShape.Irregular:
                GenerateIrregularHull(ship, dimensions, config);
                break;
            default:
                GenerateBlockyHull(ship, dimensions, config);
                break;
        }
    }
    
    /// <summary>
    /// Apply ship type-specific guidelines to ensure proper spaceship appearance
    /// Ships should generally be longer than they are wide or tall
    /// </summary>
    private Vector3 ApplyShipTypeGuidelines(Vector3 dimensions, ShipGenerationConfig config)
    {
        // Enforce aspect ratio guidelines for better spaceship appearance
        // Length (Z) should typically be 1.5-3x the width (X) and 2-4x the height (Y)
        
        switch (config.Role)
        {
            case ShipRole.Combat:
                // Combat ships: Wider and more aggressive stance
                dimensions.Z = Math.Max(dimensions.Z, dimensions.X * 1.8f);
                dimensions.Y = Math.Max(dimensions.Y, dimensions.X * 0.5f);
                break;
                
            case ShipRole.Trading:
                // Trading ships: Bulkier, cylindrical appearance
                dimensions.Y = Math.Max(dimensions.Y, dimensions.X * 0.8f);
                dimensions.Z = Math.Max(dimensions.Z, dimensions.X * 2.0f);
                break;
                
            case ShipRole.Exploration:
                // Exploration ships: Sleek and elongated
                dimensions.Z = Math.Max(dimensions.Z, dimensions.X * 2.5f);
                dimensions.Y = Math.Max(dimensions.Y, dimensions.X * 0.4f);
                break;
                
            case ShipRole.Mining:
                // Mining ships: Wider front, industrial look
                dimensions.X = Math.Max(dimensions.X, dimensions.Y * 1.2f);
                dimensions.Z = Math.Max(dimensions.Z, dimensions.X * 1.5f);
                break;
                
            default:
                // Default: Balanced proportions
                dimensions.Z = Math.Max(dimensions.Z, dimensions.X * 1.5f);
                dimensions.Y = Math.Max(dimensions.Y, dimensions.X * 0.5f);
                break;
        }
        
        return dimensions;
    }
    
    /// <summary>
    /// Generate an Avorion-style blocky hull (industrial, utilitarian design)
    /// Creates modular sections with box-like aesthetic common to Avorion ships
    /// </summary>
    private void GenerateBlockyHull(GeneratedShip ship, Vector3 dimensions, ShipGenerationConfig config)
    {
        float blockSize = 2f;
        var style = config.Style;
        
        // Apply volume scaling
        dimensions *= config.VolumeScaling;
        
        // Determine block density based on complexity
        float blockSpacing = blockSize * (1.0f + (1.0f - config.BlockComplexity) * 0.5f);
        
        // AVORION STYLE: Generate modular sections
        if (config.UseModularSections && config.ModularSectionCount >= 2)
        {
            GenerateModularBlockyHull(ship, dimensions, config, blockSize, blockSpacing);
        }
        else
        {
            GenerateSolidBlockyHull(ship, dimensions, config, blockSize, blockSpacing);
        }
    }
    
    /// <summary>
    /// Generate modular blocky hull with distinct sections (front, mid, rear)
    /// Following Avorion's modular ship design
    /// </summary>
    private void GenerateModularBlockyHull(GeneratedShip ship, Vector3 dimensions, ShipGenerationConfig config, float blockSize, float blockSpacing)
    {
        int sectionCount = config.ModularSectionCount;
        float sectionLength = dimensions.Z / sectionCount;
        
        for (int section = 0; section < sectionCount; section++)
        {
            float sectionStart = -dimensions.Z / 2 + section * sectionLength;
            float sectionEnd = sectionStart + sectionLength;
            float sectionCenter = (sectionStart + sectionEnd) / 2;
            
            // Calculate section dimensions with taper
            float sectionProgress = (float)section / (sectionCount - 1);
            float widthFactor = 1.0f - sectionProgress * 0.3f; // Tapers toward front
            float heightFactor = 1.0f - sectionProgress * 0.2f;
            
            float sectionWidth = dimensions.X * widthFactor;
            float sectionHeight = dimensions.Y * heightFactor;
            
            // Generate solid box for each section
            GenerateBoxSection(ship, 
                new Vector3(0, 0, sectionCenter), 
                new Vector3(sectionWidth, sectionHeight, sectionLength * 0.9f),
                config, blockSize, blockSpacing);
            
            // Add connecting blocks between sections
            if (section < sectionCount - 1)
            {
                float nextWidthFactor = 1.0f - (section + 1f) / (sectionCount - 1) * 0.3f;
                float connectWidth = Math.Min(sectionWidth, dimensions.X * nextWidthFactor);
                float connectHeight = Math.Min(sectionHeight, dimensions.Y * (1.0f - (section + 1f) / (sectionCount - 1) * 0.2f));
                
                // Connector blocks
                for (float x = -connectWidth / 2 + blockSize; x < connectWidth / 2 - blockSize; x += blockSize * 2)
                {
                    for (float y = -connectHeight / 2 + blockSize; y < connectHeight / 2 - blockSize; y += blockSize * 2)
                    {
                        ship.Structure.AddBlock(new VoxelBlock(
                            new Vector3(x, y, sectionEnd - blockSize / 2),
                            new Vector3(blockSize, blockSize, blockSize),
                            config.Material, BlockType.Hull));
                    }
                }
            }
        }
        
        // Add bridge/cockpit on top of front section
        float bridgeZ = dimensions.Z / 2 - sectionLength * 0.5f;
        float bridgeWidth = dimensions.X * 0.4f;
        float bridgeHeight = dimensions.Y * 0.3f;
        
        for (float x = -bridgeWidth / 2; x < bridgeWidth / 2; x += blockSize)
        {
            for (float z = bridgeZ - blockSize * 2; z < bridgeZ + blockSize * 2; z += blockSize)
            {
                ship.Structure.AddBlock(new VoxelBlock(
                    new Vector3(x, dimensions.Y / 2 * 0.7f + bridgeHeight / 2, z),
                    new Vector3(blockSize, bridgeHeight, blockSize),
                    config.Material, BlockType.Hull));
            }
        }
    }
    
    /// <summary>
    /// Generate a solid box section (part of modular hull)
    /// </summary>
    private void GenerateBoxSection(GeneratedShip ship, Vector3 center, Vector3 size, 
        ShipGenerationConfig config, float blockSize, float blockSpacing)
    {
        // Create solid box with Avorion-style aesthetics
        // Fill the box with blocks (not hollow)
        
        float halfX = size.X / 2;
        float halfY = size.Y / 2;
        float halfZ = size.Z / 2;
        
        // Generate filled box with block spacing based on complexity
        for (float x = -halfX + blockSize / 2; x < halfX; x += blockSpacing)
        {
            for (float y = -halfY + blockSize / 2; y < halfY; y += blockSpacing)
            {
                for (float z = -halfZ + blockSize / 2; z < halfZ; z += blockSpacing)
                {
                    // Skip interior blocks to save resources (create shell with some interior)
                    bool isEdge = Math.Abs(x) > halfX - blockSize * 1.5f ||
                                  Math.Abs(y) > halfY - blockSize * 1.5f ||
                                  Math.Abs(z) > halfZ - blockSize * 1.5f;
                    
                    // Include some interior structure based on complexity
                    bool isInterior = !isEdge && _random.NextDouble() < config.BlockComplexity * 0.3f;
                    
                    if (isEdge || isInterior)
                    {
                        // Use shape variety for edges to make ships more interesting
                        ship.Structure.AddBlock(CreateBlockWithShapeVariety(
                            center + new Vector3(x, y, z),
                            new Vector3(blockSize, blockSize, blockSize),
                            config.Material, BlockType.Hull, isEdge ? 0.4f : 0.1f));
                    }
                }
            }
        }
        
        // Add angled blocks at edges if enabled
        if (config.UseAngledBlocks)
        {
            AddAngledEdgeBlocks(ship, center, size, config, blockSize);
        }
    }
    
    /// <summary>
    /// Add angled/sloped blocks at edges for more interesting Avorion aesthetics
    /// Now uses proper wedge shapes for visual angles
    /// </summary>
    private void AddAngledEdgeBlocks(GeneratedShip ship, Vector3 center, Vector3 size,
        ShipGenerationConfig config, float blockSize)
    {
        float halfX = size.X / 2;
        float halfY = size.Y / 2;
        float halfZ = size.Z / 2;
        
        // Add wedge blocks at front edges - actual sloped geometry
        for (float x = -halfX + blockSize; x < halfX - blockSize; x += blockSize * 2)
        {
            // Top front edge wedges - slope facing forward (+Z)
            ship.Structure.AddBlock(new VoxelBlock(
                center + new Vector3(x, halfY, halfZ),
                new Vector3(blockSize, blockSize, blockSize),
                config.Material, BlockType.Hull, BlockShape.Wedge, BlockOrientation.PosZ));
            
            // Bottom front edge wedges - slope facing forward but inverted
            ship.Structure.AddBlock(new VoxelBlock(
                center + new Vector3(x, -halfY, halfZ),
                new Vector3(blockSize, blockSize, blockSize),
                config.Material, BlockType.Hull, BlockShape.Wedge, BlockOrientation.NegY));
        }
        
        // Add wedge blocks at rear edges
        for (float x = -halfX + blockSize; x < halfX - blockSize; x += blockSize * 2)
        {
            // Top rear edge wedges - slope facing backward
            ship.Structure.AddBlock(new VoxelBlock(
                center + new Vector3(x, halfY, -halfZ),
                new Vector3(blockSize, blockSize, blockSize),
                config.Material, BlockType.Hull, BlockShape.Wedge, BlockOrientation.NegZ));
        }
        
        // Add side wedges for beveled edges
        for (float z = -halfZ + blockSize; z < halfZ - blockSize; z += blockSize * 2)
        {
            // Right side wedges
            ship.Structure.AddBlock(new VoxelBlock(
                center + new Vector3(halfX, 0, z),
                new Vector3(blockSize, blockSize, blockSize),
                config.Material, BlockType.Hull, BlockShape.Wedge, BlockOrientation.PosX));
            
            // Left side wedges
            ship.Structure.AddBlock(new VoxelBlock(
                center + new Vector3(-halfX, 0, z),
                new Vector3(blockSize, blockSize, blockSize),
                config.Material, BlockType.Hull, BlockShape.Wedge, BlockOrientation.NegX));
        }
        
        // Add corner wedges at the front corners
        ship.Structure.AddBlock(new VoxelBlock(
            center + new Vector3(halfX, halfY, halfZ),
            new Vector3(blockSize, blockSize, blockSize),
            config.Material, BlockType.Hull, BlockShape.Corner, BlockOrientation.PosZ));
        ship.Structure.AddBlock(new VoxelBlock(
            center + new Vector3(-halfX, halfY, halfZ),
            new Vector3(blockSize, blockSize, blockSize),
            config.Material, BlockType.Hull, BlockShape.Corner, BlockOrientation.PosZ));
    }
    
    /// <summary>
    /// Generate solid blocky hull without modular sections
    /// </summary>
    private void GenerateSolidBlockyHull(GeneratedShip ship, Vector3 dimensions, ShipGenerationConfig config, 
        float blockSize, float blockSpacing)
    {
        // Create a single solid box hull with front taper
        float halfX = dimensions.X / 2;
        float halfY = dimensions.Y / 2;
        float halfZ = dimensions.Z / 2;
        
        for (float z = -halfZ; z < halfZ; z += blockSpacing)
        {
            // Calculate taper factor (narrower toward front)
            float zProgress = (z + halfZ) / dimensions.Z;
            float taperFactor = 1.0f - zProgress * 0.4f;
            
            float currentWidth = dimensions.X * taperFactor;
            float currentHeight = dimensions.Y * taperFactor;
            
            for (float x = -currentWidth / 2; x < currentWidth / 2; x += blockSpacing)
            {
                for (float y = -currentHeight / 2; y < currentHeight / 2; y += blockSpacing)
                {
                    // Create shell with some interior
                    bool isEdge = Math.Abs(x) > currentWidth / 2 - blockSize * 1.5f ||
                                  Math.Abs(y) > currentHeight / 2 - blockSize * 1.5f ||
                                  Math.Abs(z) > halfZ - blockSize * 2f ||
                                  z < -halfZ + blockSize * 2f;
                    
                    if (isEdge || _random.NextDouble() < config.BlockComplexity * 0.2f)
                    {
                        // Use varied shapes for irregular ships to enhance cobbled-together look
                        ship.Structure.AddBlock(CreateBlockWithShapeVariety(
                            new Vector3(x, y, z),
                            new Vector3(blockSize, blockSize, blockSize),
                            config.Material, BlockType.Hull, 0.5f)); // High shape variety for irregular ships
                    }
                }
            }
        }
        
        // Add bridge on top
        float bridgeZ = halfZ * 0.5f;
        for (float x = -dimensions.X * 0.2f; x < dimensions.X * 0.2f; x += blockSize)
        {
            for (float z = bridgeZ - blockSize * 2; z < bridgeZ + blockSize * 2; z += blockSize)
            {
                ship.Structure.AddBlock(new VoxelBlock(
                    new Vector3(x, halfY + blockSize, z),
                    new Vector3(blockSize, blockSize, blockSize),
                    config.Material, BlockType.Hull));
            }
        }
        
        // Add angled blocks at edges if enabled (for solid hull)
        if (config.UseAngledBlocks)
        {
            AddAngledEdgeBlocks(ship, Vector3.Zero, dimensions, config, blockSize);
        }
    }
    
    /// <summary>
    /// Generate angular/wedge-shaped hull (military style fighter/interceptor)
    /// Creates aggressive wedge with pronounced wings and engine nacelles
    /// IMPROVED: Denser block placement with proper connectivity
    /// </summary>
    private void GenerateAngularHull(GeneratedShip ship, Vector3 dimensions, ShipGenerationConfig config)
    {
        // Create aggressive military wedge hull - sharp and angular
        // IMPROVED: Denser block placement for more solid appearance
        float blockSize = 2f;  // Standard block size
        
        // Main wedge body - SOLID core with tapered shell
        for (float z = -dimensions.Z / 2; z < dimensions.Z / 2; z += blockSize)
        {
            float normalizedZ = (z + dimensions.Z / 2) / dimensions.Z;
            float taperFactor = 1.0f - normalizedZ * 0.7f;  // Aggressive taper
            float currentWidth = Math.Max(blockSize * 2, dimensions.X * taperFactor);
            float currentHeight = Math.Max(blockSize * 2, dimensions.Y * taperFactor * 0.6f);  // Flatter profile
            
            // Create solid cross-section for denser hull
            // Top surface
            for (float x = -currentWidth / 2; x < currentWidth / 2; x += blockSize)
            {
                ship.Structure.AddBlock(new VoxelBlock(
                    new Vector3(x, currentHeight / 2, z), 
                    new Vector3(blockSize, blockSize, blockSize), 
                    config.Material, BlockType.Hull));
            }
            
            // Bottom surface
            for (float x = -currentWidth / 2; x < currentWidth / 2; x += blockSize)
            {
                ship.Structure.AddBlock(new VoxelBlock(
                    new Vector3(x, -currentHeight / 2, z), 
                    new Vector3(blockSize, blockSize, blockSize), 
                    config.Material, BlockType.Hull));
            }
            
            // Side edges (left and right)
            ship.Structure.AddBlock(new VoxelBlock(
                new Vector3(-currentWidth / 2, 0, z), 
                new Vector3(blockSize, blockSize, blockSize), 
                config.Material, BlockType.Hull));
            ship.Structure.AddBlock(new VoxelBlock(
                new Vector3(currentWidth / 2 - blockSize, 0, z), 
                new Vector3(blockSize, blockSize, blockSize), 
                config.Material, BlockType.Hull));
            
            // Center spine for structural integrity - always present
            ship.Structure.AddBlock(new VoxelBlock(
                new Vector3(0, 0, z), 
                new Vector3(blockSize, blockSize, blockSize), 
                config.Material, BlockType.Hull));
        }
        
        // Add aggressive angular wings - swept back design with wedge shapes
        float wingLength = dimensions.Z * 0.4f;
        float wingStart = -dimensions.Z / 4;
        float wingSpan = dimensions.X * 0.4f;  // Slightly smaller wings
        
        for (float z = wingStart; z < wingStart + wingLength; z += blockSize)
        {
            float wingProgress = (z - wingStart) / wingLength;
            float currentWingSpan = wingSpan * (1.0f - wingProgress * 0.3f);
            float wingAngle = -wingProgress * 2;  // Slight downward angle
            
            // Fill wing from body edge outward - use wedges at the tips
            for (float x = 0; x < currentWingSpan; x += blockSize)
            {
                bool isWingTip = x >= currentWingSpan - blockSize;
                
                // Left wing
                if (isWingTip)
                {
                    // Wing tip wedge - pointed outward
                    ship.Structure.AddBlock(new VoxelBlock(
                        new Vector3(-dimensions.X / 2 - x - blockSize, wingAngle, z),
                        new Vector3(blockSize, blockSize * 0.5f, blockSize), 
                        config.Material, BlockType.Hull, BlockShape.Wedge, BlockOrientation.NegX));
                }
                else
                {
                    ship.Structure.AddBlock(new VoxelBlock(
                        new Vector3(-dimensions.X / 2 - x - blockSize, wingAngle, z),
                        new Vector3(blockSize, blockSize * 0.5f, blockSize), 
                        config.Material, BlockType.Hull));
                }
                
                // Right wing (symmetric)
                if (isWingTip)
                {
                    ship.Structure.AddBlock(new VoxelBlock(
                        new Vector3(dimensions.X / 2 + x, wingAngle, z),
                        new Vector3(blockSize, blockSize * 0.5f, blockSize), 
                        config.Material, BlockType.Hull, BlockShape.Wedge, BlockOrientation.PosX));
                }
                else
                {
                    ship.Structure.AddBlock(new VoxelBlock(
                        new Vector3(dimensions.X / 2 + x, wingAngle, z),
                        new Vector3(blockSize, blockSize * 0.5f, blockSize), 
                        config.Material, BlockType.Hull));
                }
            }
        }
        
        // Add engine nacelles at rear sides - denser pods
        float nacelleLength = dimensions.Z * 0.35f;
        float nacelleStart = -dimensions.Z / 2;
        float nacelleOffset = dimensions.X * 0.35f;  // Slightly closer
        
        for (float z = nacelleStart; z < nacelleStart + nacelleLength; z += blockSize)
        {
            // Solid nacelle cross-section (2x2 pattern)
            ship.Structure.AddBlock(new VoxelBlock(new Vector3(-nacelleOffset, 0, z), new Vector3(blockSize, blockSize, blockSize), config.Material, BlockType.Hull));
            ship.Structure.AddBlock(new VoxelBlock(new Vector3(-nacelleOffset + blockSize, 0, z), new Vector3(blockSize, blockSize, blockSize), config.Material, BlockType.Hull));
            ship.Structure.AddBlock(new VoxelBlock(new Vector3(-nacelleOffset, blockSize, z), new Vector3(blockSize, blockSize, blockSize), config.Material, BlockType.Hull));
            ship.Structure.AddBlock(new VoxelBlock(new Vector3(-nacelleOffset + blockSize, blockSize, z), new Vector3(blockSize, blockSize, blockSize), config.Material, BlockType.Hull));
            
            ship.Structure.AddBlock(new VoxelBlock(new Vector3(nacelleOffset - blockSize, 0, z), new Vector3(blockSize, blockSize, blockSize), config.Material, BlockType.Hull));
            ship.Structure.AddBlock(new VoxelBlock(new Vector3(nacelleOffset, 0, z), new Vector3(blockSize, blockSize, blockSize), config.Material, BlockType.Hull));
            ship.Structure.AddBlock(new VoxelBlock(new Vector3(nacelleOffset - blockSize, blockSize, z), new Vector3(blockSize, blockSize, blockSize), config.Material, BlockType.Hull));
            ship.Structure.AddBlock(new VoxelBlock(new Vector3(nacelleOffset, blockSize, z), new Vector3(blockSize, blockSize, blockSize), config.Material, BlockType.Hull));
            
            // Continuous connecting struts to main body
            for (float x = -dimensions.X / 2 + blockSize; x > -nacelleOffset; x -= blockSize)
            {
                ship.Structure.AddBlock(new VoxelBlock(new Vector3(x, blockSize * 0.5f, z), new Vector3(blockSize, blockSize * 0.5f, blockSize), config.Material, BlockType.Hull));
            }
            for (float x = dimensions.X / 2 - blockSize; x < nacelleOffset - blockSize; x += blockSize)
            {
                ship.Structure.AddBlock(new VoxelBlock(new Vector3(x, blockSize * 0.5f, z), new Vector3(blockSize, blockSize * 0.5f, blockSize), config.Material, BlockType.Hull));
            }
        }
        
        // Add angular armor plates on top - continuous coverage
        for (float z = -dimensions.Z / 4; z < dimensions.Z / 3; z += blockSize)
        {
            float normalizedZ = (z + dimensions.Z / 4) / (dimensions.Z / 3 + dimensions.Z / 4);
            float plateWidth = dimensions.X * (1.0f - normalizedZ * 0.5f);
            
            // Cover the top with armor plates
            for (float x = -plateWidth / 2; x < plateWidth / 2; x += blockSize)
            {
                ship.Structure.AddBlock(new VoxelBlock(
                    new Vector3(x, dimensions.Y / 3, z), 
                    new Vector3(blockSize, blockSize * 0.5f, blockSize), 
                    config.Material, BlockType.Armor));
            }
        }
        
        // Add front nose cone - pointed section with wedge shapes at the tip
        for (float z = dimensions.Z / 2 - blockSize * 3; z < dimensions.Z / 2; z += blockSize)
        {
            float progress = (z - (dimensions.Z / 2 - blockSize * 3)) / (blockSize * 3);
            float noseWidth = Math.Max(blockSize, (1.0f - progress) * dimensions.X * 0.3f);
            bool isNoseTip = z >= dimensions.Z / 2 - blockSize;
            
            for (float x = -noseWidth / 2; x < noseWidth / 2; x += blockSize)
            {
                if (isNoseTip)
                {
                    // Nose tip uses wedge shape pointing forward
                    ship.Structure.AddBlock(new VoxelBlock(
                        new Vector3(x, 0, z), 
                        new Vector3(blockSize, blockSize, blockSize), 
                        config.Material, BlockType.Hull, BlockShape.Wedge, BlockOrientation.PosZ));
                }
                else
                {
                    ship.Structure.AddBlock(new VoxelBlock(
                        new Vector3(x, 0, z), 
                        new Vector3(blockSize, blockSize, blockSize), 
                        config.Material, BlockType.Hull));
                }
            }
        }
    }
    
    /// <summary>
    /// Generate cylindrical hull (cargo/trading ships)
    /// Enhanced with cargo sections and distinctive industrial appearance
    /// FIXED: Now includes interior fill and varied block shapes for solidity
    /// </summary>
    private void GenerateCylindricalHull(GeneratedShip ship, Vector3 dimensions, ShipGenerationConfig config)
    {
        float radius = Math.Min(dimensions.X, dimensions.Y) / 2;
        float blockSize = 2f;
        
        // Main cylindrical hull section - Shell with INTERIOR FILL for solidity
        for (float z = -dimensions.Z / 2; z < dimensions.Z / 2; z += 2)  // Dense Z spacing for solid look
        {
            // Create circular shell using angle sampling with varied shapes
            for (float angle = 0; angle < 360; angle += 20)  // 18 blocks per ring (360/20)
            {
                float rad = angle * MathF.PI / 180f;
                float x = radius * MathF.Cos(rad);
                float y = radius * MathF.Sin(rad);
                
                // Use varied shapes for more visual interest
                BlockShape shape = GetRandomShape(0.2f, 0.1f);
                BlockOrientation orientation = shape != BlockShape.Cube ? GetRandomOrientation() : BlockOrientation.PosY;
                
                ship.Structure.AddBlock(new VoxelBlock(
                    new Vector3(x, y, z),
                    new Vector3(blockSize, blockSize, blockSize),
                    config.Material,
                    BlockType.Hull,
                    shape,
                    orientation
                ));
            }
            
            // CRITICAL FIX: Add interior fill rings to prevent hollow/flat appearance
            // This ensures the ship looks solid from all viewing angles
            if (_random.NextDouble() < 0.4) // Don't fill every layer, keep some space
            {
                float innerRadius = radius * 0.5f; // Interior ring
                for (float angle = 0; angle < 360; angle += 45) // 8 blocks per inner ring
                {
                    float rad = angle * MathF.PI / 180f;
                    float x = innerRadius * MathF.Cos(rad);
                    float y = innerRadius * MathF.Sin(rad);
                    
                    ship.Structure.AddBlock(new VoxelBlock(
                        new Vector3(x, y, z),
                        new Vector3(blockSize, blockSize, blockSize),
                        config.Material,
                        BlockType.Hull
                    ));
                }
            }
        }
        
        // Add cargo container-like sections along the length (smaller, less dense)
        int numContainers = 2;  // Reduced from 3
        float containerSpacing = dimensions.Z / (numContainers + 1);
        
        for (int i = 1; i <= numContainers; i++)
        {
            float zPos = -dimensions.Z / 2 + i * containerSpacing;
            float containerRadius = radius * 1.2f;
            
            // Create bulging cargo sections - denser for cohesion
            for (float z = zPos - 4; z <= zPos + 4; z += 2)  // Denser rings
            {
                for (float angle = 0; angle < 360; angle += 30)  // 12 blocks per ring
                {
                    float rad = angle * MathF.PI / 180f;
                    float x = containerRadius * MathF.Cos(rad);
                    float y = containerRadius * MathF.Sin(rad);
                    
                    ship.Structure.AddBlock(new VoxelBlock(
                        new Vector3(x, y, z),
                        new Vector3(blockSize, blockSize, blockSize),
                        config.Material,
                        BlockType.Hull
                    ));
                }
            }
        }
        
        // Add end caps - SPARSE circular ends for connectivity
        for (float angle = 0; angle < 360; angle += 30)
        {
            float rad = angle * MathF.PI / 180f;
            
            // Front cap - tapered
            for (float r = 0; r < radius; r += radius / 2)
            {
                float x = r * 0.8f * MathF.Cos(rad);
                float y = r * 0.8f * MathF.Sin(rad);
                ship.Structure.AddBlock(new VoxelBlock(
                    new Vector3(x, y, dimensions.Z / 2 - 2),
                    new Vector3(blockSize, blockSize, blockSize),
                    config.Material,
                    BlockType.Hull
                ));
            }
            
            // Back cap - full size
            for (float r = 0; r < radius; r += radius / 2)
            {
                float x = r * MathF.Cos(rad);
                float y = r * MathF.Sin(rad);
                ship.Structure.AddBlock(new VoxelBlock(
                    new Vector3(x, y, -dimensions.Z / 2),
                    new Vector3(blockSize, blockSize, blockSize),
                    config.Material,
                    BlockType.Hull
                ));
            }
        }
        
        // Add longitudinal support struts - 4 main beams
        if (dimensions.Z > 20)
        {
            for (float angle = 0; angle < 360; angle += 90)  // 4 struts at 90 degrees
            {
                float rad = angle * MathF.PI / 180f;
                float x = (radius + 2) * MathF.Cos(rad);
                float y = (radius + 2) * MathF.Sin(rad);
                
                for (float z = -dimensions.Z / 2 + 4; z < dimensions.Z / 2 - 4; z += 2)
                {
                    ship.Structure.AddBlock(new VoxelBlock(
                        new Vector3(x, y, z),
                        GetStretchedBlockSize("z"),
                        config.Material,
                        BlockType.Hull
                    ));
                }
            }
        }
    }
    
    /// <summary>
    /// Generate sleek hull (exploration/science ships - needle/teardrop design)
    /// Creates streamlined, elongated design with minimal cross-section
    /// IMPROVED: Denser body structure for more cohesive appearance
    /// </summary>
    private void GenerateSleekHull(GeneratedShip ship, Vector3 dimensions, ShipGenerationConfig config)
    {
        float blockSize = 2f;
        
        // Main spine - central structural beam (continuous)
        for (float z = -dimensions.Z / 2; z < dimensions.Z / 2; z += blockSize)
        {
            ship.Structure.AddBlock(new VoxelBlock(
                new Vector3(0, 0, z), 
                new Vector3(blockSize, blockSize, blockSize), 
                config.Material, BlockType.Hull));
        }
        
        // Streamlined body - flat, wide profile with smooth taper
        for (float z = -dimensions.Z / 3; z < dimensions.Z / 3; z += blockSize)
        {
            float normalizedZ = Math.Abs(z) / (dimensions.Z / 3);
            float widthFactor = 1.0f - normalizedZ * 0.7f;
            float currentWidth = Math.Max(blockSize * 2, dimensions.X * widthFactor * 0.7f);
            float currentHeight = Math.Max(blockSize, dimensions.Y * widthFactor * 0.35f);
            
            // Create continuous hull surface (filled, not just edges)
            for (float x = -currentWidth / 2; x < currentWidth / 2; x += blockSize)
            {
                // Top surface
                ship.Structure.AddBlock(new VoxelBlock(
                    new Vector3(x, currentHeight / 2, z),
                    new Vector3(blockSize, blockSize * 0.5f, blockSize), 
                    config.Material, BlockType.Hull));
                
                // Bottom surface
                ship.Structure.AddBlock(new VoxelBlock(
                    new Vector3(x, -currentHeight / 2, z),
                    new Vector3(blockSize, blockSize * 0.5f, blockSize), 
                    config.Material, BlockType.Hull));
            }
            
            // Side edges
            ship.Structure.AddBlock(new VoxelBlock(
                new Vector3(-currentWidth / 2, 0, z),
                new Vector3(blockSize * 0.5f, blockSize, blockSize), 
                config.Material, BlockType.Hull));
            ship.Structure.AddBlock(new VoxelBlock(
                new Vector3(currentWidth / 2 - blockSize * 0.5f, 0, z),
                new Vector3(blockSize * 0.5f, blockSize, blockSize), 
                config.Material, BlockType.Hull));
        }
        
        // Sharp pointed nose - continuous taper with wedge tip
        for (float z = dimensions.Z / 3; z < dimensions.Z / 2; z += blockSize)
        {
            float progress = (z - dimensions.Z / 3) / (dimensions.Z / 2 - dimensions.Z / 3);
            float noseWidth = Math.Max(blockSize, (1.0f - progress * 0.8f) * dimensions.X * 0.3f);
            bool isNoseTip = z >= dimensions.Z / 2 - blockSize;
            
            for (float x = -noseWidth / 2; x < noseWidth / 2; x += blockSize)
            {
                if (isNoseTip)
                {
                    // Sharp wedge at the very tip
                    ship.Structure.AddBlock(new VoxelBlock(
                        new Vector3(x, 0, z), 
                        new Vector3(blockSize, blockSize, blockSize), 
                        config.Material, BlockType.Hull, BlockShape.Wedge, BlockOrientation.PosZ));
                }
                else
                {
                    ship.Structure.AddBlock(new VoxelBlock(
                        new Vector3(x, 0, z), 
                        new Vector3(blockSize, blockSize, blockSize), 
                        config.Material, BlockType.Hull));
                }
            }
        }
        
        // Sleek vertical stabilizer fin at rear top - with wedge top
        float finHeight = dimensions.Y * 1.0f;
        float finLength = dimensions.Z * 0.25f;
        float finStart = -dimensions.Z / 2;
        
        for (float z = finStart; z < finStart + finLength; z += blockSize)
        {
            float finProgress = (z - finStart) / finLength;
            float currentFinHeight = finHeight * (1.0f - finProgress * 0.3f);
            
            // Continuous vertical fin
            for (float y = 0; y < currentFinHeight; y += blockSize)
            {
                bool isFinTop = y >= currentFinHeight - blockSize;
                
                if (isFinTop)
                {
                    // Top of fin uses wedge pointing up
                    ship.Structure.AddBlock(new VoxelBlock(
                        new Vector3(0, dimensions.Y / 4 + y, z),
                        new Vector3(blockSize * 0.5f, blockSize, blockSize * 0.5f), 
                        config.Material, BlockType.Hull, BlockShape.Wedge, BlockOrientation.PosY));
                }
                else
                {
                    ship.Structure.AddBlock(new VoxelBlock(
                        new Vector3(0, dimensions.Y / 4 + y, z),
                        new Vector3(blockSize * 0.5f, blockSize, blockSize * 0.5f), 
                        config.Material, BlockType.Hull));
                }
            }
        }
        
        // Engine pods on sides - denser for better visibility
        float podLength = dimensions.Z * 0.3f;
        float podStart = -dimensions.Z / 2;
        float podOffset = dimensions.X * 0.25f;
        
        for (float z = podStart; z < podStart + podLength; z += blockSize)
        {
            // Pod structure - 2 blocks tall
            ship.Structure.AddBlock(new VoxelBlock(
                new Vector3(-podOffset, 0, z), 
                new Vector3(blockSize, blockSize, blockSize), 
                config.Material, BlockType.Hull));
            ship.Structure.AddBlock(new VoxelBlock(
                new Vector3(-podOffset, -blockSize, z), 
                new Vector3(blockSize, blockSize, blockSize), 
                config.Material, BlockType.Hull));
            
            ship.Structure.AddBlock(new VoxelBlock(
                new Vector3(podOffset - blockSize, 0, z), 
                new Vector3(blockSize, blockSize, blockSize), 
                config.Material, BlockType.Hull));
            ship.Structure.AddBlock(new VoxelBlock(
                new Vector3(podOffset - blockSize, -blockSize, z), 
                new Vector3(blockSize, blockSize, blockSize), 
                config.Material, BlockType.Hull));
            
            // Connecting struts (continuous)
            for (float x = -podOffset + blockSize; x < 0; x += blockSize)
            {
                ship.Structure.AddBlock(new VoxelBlock(
                    new Vector3(x, -blockSize * 0.5f, z), 
                    new Vector3(blockSize, blockSize * 0.5f, blockSize), 
                    config.Material, BlockType.Hull));
            }
            for (float x = blockSize; x < podOffset - blockSize; x += blockSize)
            {
                ship.Structure.AddBlock(new VoxelBlock(
                    new Vector3(x, -blockSize * 0.5f, z), 
                    new Vector3(blockSize, blockSize * 0.5f, blockSize), 
                    config.Material, BlockType.Hull));
            }
        }
    }
    
    /// <summary>
    /// Generate irregular hull (pirate/cobbled together ships)
    /// ENHANCED: Add connected protrusions while maintaining structural integrity
    /// </summary>
    private void GenerateIrregularHull(GeneratedShip ship, Vector3 dimensions, ShipGenerationConfig config)
    {
        // Start with a slightly smaller core hull for base connectivity
        GenerateBlockyHull(ship, dimensions * 0.85f, config);
        
        // Get existing hull blocks to build from
        var existingHullBlocks = ship.Structure.Blocks
            .Where(b => b.BlockType == BlockType.Hull || b.BlockType == BlockType.Armor)
            .ToList();
        
        if (existingHullBlocks.Count == 0) return;
        
        // Add asymmetric connected protrusions for "cobbled together" look
        int protrusionCount = 3 + _random.Next(5); // 3-7 protrusions
        float blockSize = 2f;
        
        for (int i = 0; i < protrusionCount; i++)
        {
            // Pick a random existing hull block to grow from (ensures connectivity)
            var baseBlock = existingHullBlocks[_random.Next(existingHullBlocks.Count)];
            
            // Determine protrusion direction (outward from center)
            Vector3 protrusionDir = Vector3.Normalize(baseBlock.Position);
            if (protrusionDir.Length() < 0.1f) protrusionDir = new Vector3(1, 0, 0);
            
            // Vary the direction slightly for irregularity
            protrusionDir += new Vector3(
                ((float)_random.NextDouble() - 0.5f) * 0.6f,
                ((float)_random.NextDouble() - 0.5f) * 0.6f,
                ((float)_random.NextDouble() - 0.5f) * 0.6f
            );
            protrusionDir = Vector3.Normalize(protrusionDir);
            
            // Build protrusion block by block (guarantees connectivity)
            int protrusionLength = 2 + _random.Next(4); // 2-5 blocks
            Vector3 currentPos = baseBlock.Position;
            
            for (int j = 0; j < protrusionLength; j++)
            {
                // Move in the protrusion direction
                currentPos += protrusionDir * blockSize * 1.2f; // Slightly overlapping
                
                // Vary block size for irregular look
                Vector3 irregularSize = new Vector3(
                    blockSize * (0.8f + (float)_random.NextDouble() * 0.6f),
                    blockSize * (0.8f + (float)_random.NextDouble() * 0.6f),
                    blockSize * (0.8f + (float)_random.NextDouble() * 0.6f)
                );
                
                var protrusionBlock = new VoxelBlock(
                    currentPos,
                    irregularSize,
                    config.Material,
                    BlockType.Hull
                );
                ship.Structure.AddBlock(protrusionBlock);
                existingHullBlocks.Add(protrusionBlock); // Add to pool for potential branching
                
                // Occasionally branch
                if (_random.NextDouble() < 0.3 && j > 0)
                {
                    Vector3 branchDir = Vector3.Normalize(protrusionDir + new Vector3(
                        ((float)_random.NextDouble() - 0.5f),
                        ((float)_random.NextDouble() - 0.5f),
                        ((float)_random.NextDouble() - 0.5f)
                    ));
                    
                    int branchLength = 1 + _random.Next(3);
                    Vector3 branchPos = currentPos;
                    
                    for (int k = 0; k < branchLength; k++)
                    {
                        branchPos += branchDir * blockSize * 1.2f;
                        var branchBlock = new VoxelBlock(
                            branchPos,
                            new Vector3(blockSize * 0.9f, blockSize * 0.9f, blockSize * 0.9f),
                            config.Material,
                            BlockType.Hull
                        );
                        ship.Structure.AddBlock(branchBlock);
                    }
                }
            }
        }
        
        // Add random asymmetric armor plates for "patched up" appearance
        int armorPatchCount = 4 + _random.Next(6);
        for (int i = 0; i < armorPatchCount; i++)
        {
            var baseBlock = existingHullBlocks[_random.Next(existingHullBlocks.Count)];
            
            // Add irregular armor patch
            var patchSize = new Vector3(
                blockSize * (1.2f + (float)_random.NextDouble() * 0.8f),
                blockSize * (0.6f + (float)_random.NextDouble() * 0.4f),
                blockSize * (1.2f + (float)_random.NextDouble() * 0.8f)
            );
            
            Vector3 patchOffset = new Vector3(
                ((float)_random.NextDouble() - 0.5f) * blockSize,
                ((float)_random.NextDouble() - 0.5f) * blockSize,
                ((float)_random.NextDouble() - 0.5f) * blockSize
            );
            
            var armorPatch = new VoxelBlock(
                baseBlock.Position + patchOffset,
                patchSize,
                config.Material,
                BlockType.Armor
            );
            ship.Structure.AddBlock(armorPatch);
        }
    }
    
    /// <summary>
    /// Find the nearest hull block to a target position, optionally with an offset
    /// </summary>
    private VoxelBlock? FindNearestHullBlock(GeneratedShip ship, Vector3 targetPosition, bool findAdjacent = false)
    {
        var hullBlocks = ship.Structure.Blocks.Where(b => 
            b.BlockType == BlockType.Hull || b.BlockType == BlockType.Armor).ToList();
        
        if (hullBlocks.Count == 0) return null;
        
        float minDistance = float.MaxValue;
        VoxelBlock? nearest = null;
        
        foreach (var block in hullBlocks)
        {
            float distance = Vector3.Distance(block.Position, targetPosition);
            if (distance < minDistance)
            {
                minDistance = distance;
                nearest = block;
            }
        }
        
        return nearest;
    }
    
    /// <summary>
    /// Find an adjacent position to a hull block for placing components
    /// Ensures the new block will be touching or overlapping the hull block
    /// </summary>
    private Vector3 FindAdjacentPosition(GeneratedShip ship, VoxelBlock hullBlock, Vector3 preferredDirection, Vector3 componentSize)
    {
        // Calculate offsets that will make blocks touch (considering block sizes)
        // For a 2x2x2 hull block and 3x3x3 component, they need to be 2.5 units apart (touching edges)
        float hullHalfSize = hullBlock.Size.X / 2; // Assuming cubic blocks
        float componentHalfSize = componentSize.X / 2;
        float touchingDistance = hullHalfSize + componentHalfSize;
        
        Vector3[] directions = new[]
        {
            new Vector3(touchingDistance, 0, 0),
            new Vector3(-touchingDistance, 0, 0),
            new Vector3(0, touchingDistance, 0),
            new Vector3(0, -touchingDistance, 0),
            new Vector3(0, 0, touchingDistance),
            new Vector3(0, 0, -touchingDistance)
        };
        
        // Sort directions by similarity to preferred direction
        var sortedDirections = directions.OrderByDescending(dir => 
        {
            if (preferredDirection == Vector3.Zero) return 0;
            return Vector3.Dot(Vector3.Normalize(dir), Vector3.Normalize(preferredDirection));
        });
        
        foreach (var direction in sortedDirections)
        {
            Vector3 candidatePos = hullBlock.Position + direction;
            
            // Check if this position is already occupied (considering both positions and sizes)
            // Blocks should not overlap significantly - allow small overlap for better connectivity
            bool occupied = ship.Structure.Blocks.Any(b => 
            {
                // Calculate minimum distance considering block sizes (AABB distance)
                float dx = Math.Abs(b.Position.X - candidatePos.X);
                float dy = Math.Abs(b.Position.Y - candidatePos.Y);
                float dz = Math.Abs(b.Position.Z - candidatePos.Z);
                float minDist = (b.Size.X + componentSize.X) / 2 * 0.8f;  // 80% to allow slight overlap
                return dx < minDist && dy < minDist && dz < minDist;
            });
            
            if (!occupied)
            {
                return candidatePos;
            }
        }
        
        // Fallback: try diagonal positions
        Vector3[] diagonals = new[]
        {
            Vector3.Normalize(new Vector3(touchingDistance, touchingDistance, 0)) * touchingDistance,
            Vector3.Normalize(new Vector3(touchingDistance, -touchingDistance, 0)) * touchingDistance,
            Vector3.Normalize(new Vector3(-touchingDistance, touchingDistance, 0)) * touchingDistance,
            Vector3.Normalize(new Vector3(-touchingDistance, -touchingDistance, 0)) * touchingDistance
        };
        
        foreach (var diagonal in diagonals)
        {
            Vector3 candidatePos = hullBlock.Position + diagonal;
            
            // Check if this position is already occupied (considering both positions and sizes)
            bool occupied = ship.Structure.Blocks.Any(b => 
            {
                // Calculate minimum distance considering block sizes (AABB distance)
                float dx = Math.Abs(b.Position.X - candidatePos.X);
                float dy = Math.Abs(b.Position.Y - candidatePos.Y);
                float dz = Math.Abs(b.Position.Z - candidatePos.Z);
                float minDist = (b.Size.X + componentSize.X) / 2 * 0.8f;  // 80% to allow slight overlap
                return dx < minDist && dy < minDist && dz < minDist;
            });
            
            if (!occupied)
            {
                return candidatePos;
            }
        }
        
        // Last resort: place on the hull block itself (will overlap but be connected)
        return hullBlock.Position;
    }
    
    /// <summary>
    /// Snap a position to near the hull, finding an adjacent free spot that ensures connectivity
    /// </summary>
    private Vector3 SnapToNearestHull(GeneratedShip ship, Vector3 targetPosition, Vector3 preferredDirection, Vector3 componentSize)
    {
        var nearest = FindNearestHullBlock(ship, targetPosition);
        if (nearest == null) return targetPosition;
        
        // If no preferred direction, use direction from hull to target
        if (preferredDirection == default || preferredDirection == Vector3.Zero)
        {
            preferredDirection = targetPosition - nearest.Position;
            if (preferredDirection.Length() < 0.1f)
            {
                preferredDirection = new Vector3(0, 0, 1); // Default forward
            }
        }
        
        // Find an adjacent position that ensures blocks will be connected
        return FindAdjacentPosition(ship, nearest, preferredDirection, componentSize);
    }
    
    /// <summary>
    /// Place Avorion-style core components: power core, integrity field, computer blocks
    /// These are essential for ship functionality and upgrade slots
    /// </summary>
    private void PlaceCoreComponents(GeneratedShip ship, Vector3 dimensions, ShipGenerationConfig config)
    {
        float blockSize = 2f;
        var style = config.Style;
        
        // Calculate ship center for core component placement
        Vector3 shipCenter = Vector3.Zero;
        
        // 1. Place Power Core (main generator) at ship center
        // In Avorion, the generator is the heart of the ship
        if (config.RequirePowerCore)
        {
            float powerCoreSize = Math.Max(3f, dimensions.X * 0.15f);
            var powerCore = new VoxelBlock(
                shipCenter,
                new Vector3(powerCoreSize, powerCoreSize, powerCoreSize),
                config.Material,
                BlockType.Generator
            );
            ship.Structure.AddBlock(powerCore);
            
            // Add visible ventilation blocks around generator (Avorion aesthetic)
            if (config.BlockComplexity > 0.4f)
            {
                // Ventilation panels on sides
                ship.Structure.AddBlock(new VoxelBlock(
                    new Vector3(powerCoreSize / 2 + blockSize / 2, 0, 0),
                    new Vector3(blockSize * 0.5f, powerCoreSize * 0.8f, powerCoreSize * 0.8f),
                    config.Material, BlockType.Hull));
                ship.Structure.AddBlock(new VoxelBlock(
                    new Vector3(-powerCoreSize / 2 - blockSize / 2, 0, 0),
                    new Vector3(blockSize * 0.5f, powerCoreSize * 0.8f, powerCoreSize * 0.8f),
                    config.Material, BlockType.Hull));
            }
        }
        
        // 2. Place Integrity Field Generator (if required)
        // Positioned near power core for protection
        if (config.RequireIntegrityField)
        {
            float fieldSize = Math.Max(2f, dimensions.X * 0.1f);
            var integrityField = new VoxelBlock(
                new Vector3(0, dimensions.Y * 0.2f, dimensions.Z * 0.1f),
                new Vector3(fieldSize, fieldSize, fieldSize),
                config.Material,
                BlockType.ShieldGenerator
            );
            ship.Structure.AddBlock(integrityField);
        }
        
        // 3. Place Computer Blocks for upgrade slots
        // More computer blocks = more upgrade slots (Avorion mechanic)
        int computerBlockCount = GetRequiredComputerBlocks(config);
        float computerSpacing = dimensions.X * 0.3f;
        
        for (int i = 0; i < computerBlockCount; i++)
        {
            float xOffset = (i % 2 == 0 ? -1 : 1) * computerSpacing * ((i / 2) + 1) * 0.3f;
            float zOffset = dimensions.Z * 0.15f * (i / 2);
            
            var computerBlock = new VoxelBlock(
                new Vector3(xOffset, dimensions.Y * 0.1f, zOffset),
                new Vector3(blockSize, blockSize, blockSize),
                config.Material,
                BlockType.Computer // Use proper Computer block type for upgrade slots
            );
            computerBlock.ColorRGB = config.Style.AccentColor; // Distinguish visually
            ship.Structure.AddBlock(computerBlock);
        }
        
        ship.ComputerBlockCount = computerBlockCount;
        ship.UpgradeSlots = config.TargetUpgradeSlots + (computerBlockCount / 2);
        
        // 4. Place Crew Quarters near generator (Avorion placement rule)
        if (config.Size >= ShipSize.Corvette)
        {
            int crewQuarterCount = GetRequiredCrewQuarters(config.Size);
            float quarterSize = blockSize * 1.5f;
            
            for (int i = 0; i < crewQuarterCount; i++)
            {
                float angle = (360f / crewQuarterCount) * i * MathF.PI / 180f;
                float radius = dimensions.X * 0.25f;
                
                var quarters = new VoxelBlock(
                    new Vector3(
                        radius * MathF.Cos(angle),
                        dimensions.Y * 0.15f,
                        radius * MathF.Sin(angle) + dimensions.Z * 0.1f
                    ),
                    new Vector3(quarterSize, quarterSize, quarterSize),
                    config.Material,
                    BlockType.CrewQuarters
                );
                ship.Structure.AddBlock(quarters);
            }
        }
        
        // 5. Place Battery blocks for energy storage
        int batteryCount = GetRequiredBatteries(config.Size, config.Role);
        for (int i = 0; i < batteryCount; i++)
        {
            float xOffset = (i % 2 == 0 ? -1 : 1) * dimensions.X * 0.2f;
            float zOffset = -dimensions.Z * 0.15f - (i / 2) * blockSize * 1.5f;
            
            var battery = new VoxelBlock(
                new Vector3(xOffset, -dimensions.Y * 0.1f, zOffset),
                new Vector3(blockSize * 1.5f, blockSize, blockSize * 1.5f),
                config.Material,
                BlockType.Generator // Batteries are power storage
            );
            ship.Structure.AddBlock(battery);
        }
    }
    
    /// <summary>
    /// Get required computer blocks based on target upgrade slots and ship size
    /// </summary>
    private int GetRequiredComputerBlocks(ShipGenerationConfig config)
    {
        int baseCount = config.Size switch
        {
            ShipSize.Fighter => 1,
            ShipSize.Corvette => 2,
            ShipSize.Frigate => 3,
            ShipSize.Destroyer => 4,
            ShipSize.Cruiser => 6,
            ShipSize.Battleship => 8,
            ShipSize.Carrier => 10,
            _ => 2
        };
        
        // Add more for high upgrade slot requirements
        if (config.TargetUpgradeSlots > 5) baseCount += (config.TargetUpgradeSlots - 5) / 2;
        
        return baseCount;
    }
    
    /// <summary>
    /// Get required crew quarters based on ship size
    /// </summary>
    private int GetRequiredCrewQuarters(ShipSize size)
    {
        return size switch
        {
            ShipSize.Fighter => 0,
            ShipSize.Corvette => 1,
            ShipSize.Frigate => 2,
            ShipSize.Destroyer => 3,
            ShipSize.Cruiser => 4,
            ShipSize.Battleship => 6,
            ShipSize.Carrier => 8,
            _ => 1
        };
    }
    
    /// <summary>
    /// Get required batteries based on ship size and role
    /// </summary>
    private int GetRequiredBatteries(ShipSize size, ShipRole role)
    {
        int baseCount = size switch
        {
            ShipSize.Fighter => 1,
            ShipSize.Corvette => 1,
            ShipSize.Frigate => 2,
            ShipSize.Destroyer => 3,
            ShipSize.Cruiser => 4,
            ShipSize.Battleship => 5,
            ShipSize.Carrier => 6,
            _ => 2
        };
        
        // Combat ships need more power
        if (role == ShipRole.Combat) baseCount += 2;
        
        return baseCount;
    }
    
    /// <summary>
    /// Place functional components (engines, generators, shields)
    /// </summary>
    private void PlaceFunctionalComponents(GeneratedShip ship, ShipGenerationConfig config)
    {
        var dimensions = GetShipDimensions(config.Size);
        
        // Calculate required component counts based on ship size and role
        int engineCount = GetRequiredEngineCount(config.Size, config.Role);
        int generatorCount = GetRequiredGeneratorCount(config.Size, config.Role);
        int shieldCount = GetRequiredShieldCount(config.Size, config.Role);
        int thrusterCount = GetRequiredThrusterCount(config.Size, config.Role);
        int gyroCount = GetRequiredGyroCount(config.Size, config.Role);
        
        // Place engines at the back for thrust
        PlaceEngines(ship, dimensions, config, engineCount);
        
        // Place generators in the core (internal, protected)
        PlaceGenerators(ship, dimensions, config, generatorCount);
        
        // Place shields (can be distributed or centralized based on style)
        PlaceShields(ship, dimensions, config, shieldCount);
        
        // Place thrusters for omnidirectional movement
        PlaceThrusters(ship, dimensions, config, thrusterCount);
        
        // Place gyros for rotation
        PlaceGyros(ship, dimensions, config, gyroCount);
    }
    
    private int GetRequiredEngineCount(ShipSize size, ShipRole role)
    {
        int baseCount = size switch
        {
            ShipSize.Fighter => 1,
            ShipSize.Corvette => 2,
            ShipSize.Frigate => 3,
            ShipSize.Destroyer => 4,
            ShipSize.Cruiser => 6,
            ShipSize.Battleship => 8,
            ShipSize.Carrier => 10,
            _ => 3
        };
        
        // Role modifiers
        if (role == ShipRole.Combat || role == ShipRole.Exploration) baseCount += 1;
        return baseCount;
    }
    
    private int GetRequiredGeneratorCount(ShipSize size, ShipRole role)
    {
        return size switch
        {
            ShipSize.Fighter => 1,
            ShipSize.Corvette => 1,
            ShipSize.Frigate => 2,
            ShipSize.Destroyer => 3,
            ShipSize.Cruiser => 4,
            ShipSize.Battleship => 5,
            ShipSize.Carrier => 6,
            _ => 2
        };
    }
    
    private int GetRequiredShieldCount(ShipSize size, ShipRole role)
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
        
        if (role == ShipRole.Combat) baseCount += 1;
        return baseCount;
    }
    
    private int GetRequiredThrusterCount(ShipSize size, ShipRole role)
    {
        return size switch
        {
            ShipSize.Fighter => 4,
            ShipSize.Corvette => 4,
            ShipSize.Frigate => 6,
            ShipSize.Destroyer => 8,
            ShipSize.Cruiser => 10,
            ShipSize.Battleship => 12,
            ShipSize.Carrier => 14,
            _ => 6
        };
    }
    
    private int GetRequiredGyroCount(ShipSize size, ShipRole role)
    {
        return size switch
        {
            ShipSize.Fighter => 2,
            ShipSize.Corvette => 2,
            ShipSize.Frigate => 3,
            ShipSize.Destroyer => 4,
            ShipSize.Cruiser => 5,
            ShipSize.Battleship => 6,
            ShipSize.Carrier => 7,
            _ => 3
        };
    }
    
    private void PlaceEngines(GeneratedShip ship, Vector3 dimensions, ShipGenerationConfig config, int count)
    {
        // Place engines at the rear of the ship, ensuring they connect to hull
        float engineZ = -dimensions.Z / 2 + 2; // Near the back
        Vector3 engineSize = new Vector3(4, 4, 4); // INCREASED: Engines are larger and more visible
        
        for (int i = 0; i < count; i++)
        {
            float spread = dimensions.X * 0.6f;
            float x = -spread / 2 + (spread / (count - 1)) * i;
            if (count == 1) x = 0; // Center single engine
            
            Vector3 targetPosition = new Vector3(x, -dimensions.Y / 4, engineZ);
            
            // Snap to nearest hull with preference to grow outward (rear direction)
            Vector3 placementPosition = SnapToNearestHull(ship, targetPosition, new Vector3(0, 0, -1), engineSize);
            
            var engine = new VoxelBlock(
                placementPosition,
                engineSize,
                config.Material,
                BlockType.Engine
            );
            
            // Add distinctive color to engines
            engine.ColorRGB = ENGINE_COLOR;
            
            ship.Structure.AddBlock(engine);
            
            // Add larger engine nozzle extending backwards for visual prominence
            var nozzleSize = new Vector3(engineSize.X * 0.8f, engineSize.Y * 0.8f, 2f);
            var nozzle = new VoxelBlock(
                placementPosition + new Vector3(0, 0, -(engineSize.Z / 2 + nozzleSize.Z / 2)),
                nozzleSize,
                config.Material,
                BlockType.Hull
            );
            nozzle.ColorRGB = NOZZLE_COLOR;
            ship.Structure.AddBlock(nozzle);
        }
    }
    
    private void PlaceGenerators(GeneratedShip ship, Vector3 dimensions, ShipGenerationConfig config, int count)
    {
        // Place generators in the core (center of ship, protected), ensuring connectivity
        Vector3 generatorSize = new Vector3(3, 3, 3);
        
        for (int i = 0; i < count; i++)
        {
            float z = -dimensions.Z / 4 + (dimensions.Z / 2) / count * i;
            Vector3 targetPosition = new Vector3(0, 0, z);
            
            // For generators, prefer internal placement (no strong directional preference)
            Vector3 placementPosition = SnapToNearestHull(ship, targetPosition, Vector3.Zero, generatorSize);
            
            var generator = new VoxelBlock(
                placementPosition,
                generatorSize,
                config.Material,
                BlockType.Generator
            );
            ship.Structure.AddBlock(generator);
        }
    }
    
    private void PlaceShields(GeneratedShip ship, Vector3 dimensions, ShipGenerationConfig config, int count)
    {
        // Distribute shield generators throughout the ship, ensuring connectivity
        Vector3 shieldSize = new Vector3(3, 3, 3);
        
        for (int i = 0; i < count; i++)
        {
            float z = -dimensions.Z / 3 + (dimensions.Z * 0.66f / count) * i;
            float y = i % 2 == 0 ? dimensions.Y / 4 : -dimensions.Y / 4;
            
            Vector3 targetPosition = new Vector3(0, y, z);
            Vector3 preferredDirection = new Vector3(0, y > 0 ? 1 : -1, 0);
            
            // Snap to nearest hull with vertical preference
            Vector3 placementPosition = SnapToNearestHull(ship, targetPosition, preferredDirection, shieldSize);
            
            var shield = new VoxelBlock(
                placementPosition,
                shieldSize,
                config.Material,
                BlockType.ShieldGenerator
            );
            ship.Structure.AddBlock(shield);
        }
    }
    
    private void PlaceThrusters(GeneratedShip ship, Vector3 dimensions, ShipGenerationConfig config, int count)
    {
        // Place thrusters on all sides for omnidirectional movement, ensuring connectivity
        int thrustersPerSide = count / 4;
        Vector3 thrusterSize = new Vector3(2, 2, 2);
        
        // Top thrusters
        for (int i = 0; i < thrustersPerSide; i++)
        {
            float z = -dimensions.Z / 3 + (dimensions.Z * 0.66f / thrustersPerSide) * i;
            Vector3 targetPosition = new Vector3(0, dimensions.Y / 2 - 2, z);
            
            // Snap to nearest hull with upward preference
            Vector3 placementPosition = SnapToNearestHull(ship, targetPosition, new Vector3(0, 1, 0), thrusterSize);
            
            var thruster = new VoxelBlock(
                placementPosition,
                thrusterSize,
                config.Material,
                BlockType.Thruster
            );
            ship.Structure.AddBlock(thruster);
        }
        
        // Bottom thrusters
        for (int i = 0; i < thrustersPerSide; i++)
        {
            float z = -dimensions.Z / 3 + (dimensions.Z * 0.66f / thrustersPerSide) * i;
            Vector3 targetPosition = new Vector3(0, -dimensions.Y / 2, z);
            
            // Snap to nearest hull with downward preference
            Vector3 placementPosition = SnapToNearestHull(ship, targetPosition, new Vector3(0, -1, 0), thrusterSize);
            
            var thruster = new VoxelBlock(
                placementPosition,
                thrusterSize,
                config.Material,
                BlockType.Thruster
            );
            ship.Structure.AddBlock(thruster);
        }
        
        // Left thrusters
        for (int i = 0; i < thrustersPerSide; i++)
        {
            float z = -dimensions.Z / 3 + (dimensions.Z * 0.66f / thrustersPerSide) * i;
            Vector3 targetPosition = new Vector3(-dimensions.X / 2, 0, z);
            
            // Snap to nearest hull with left preference
            Vector3 placementPosition = SnapToNearestHull(ship, targetPosition, new Vector3(-1, 0, 0), thrusterSize);
            
            var thruster = new VoxelBlock(
                placementPosition,
                thrusterSize,
                config.Material,
                BlockType.Thruster
            );
            ship.Structure.AddBlock(thruster);
        }
        
        // Right thrusters
        for (int i = 0; i < thrustersPerSide; i++)
        {
            float z = -dimensions.Z / 3 + (dimensions.Z * 0.66f / thrustersPerSide) * i;
            Vector3 targetPosition = new Vector3(dimensions.X / 2 - 2, 0, z);
            
            // Snap to nearest hull with right preference
            Vector3 placementPosition = SnapToNearestHull(ship, targetPosition, new Vector3(1, 0, 0), thrusterSize);
            
            var thruster = new VoxelBlock(
                placementPosition,
                thrusterSize,
                config.Material,
                BlockType.Thruster
            );
            ship.Structure.AddBlock(thruster);
        }
    }
    
    private void PlaceGyros(GeneratedShip ship, Vector3 dimensions, ShipGenerationConfig config, int count)
    {
        // Place gyro arrays for rotation control, ensuring connectivity
        Vector3 gyroSize = new Vector3(2, 2, 2);
        
        for (int i = 0; i < count; i++)
        {
            float z = -dimensions.Z / 4 + (dimensions.Z / 2 / count) * i;
            float x = i % 2 == 0 ? dimensions.X / 4 : -dimensions.X / 4;
            
            Vector3 targetPosition = new Vector3(x, 0, z);
            Vector3 preferredDirection = new Vector3(x > 0 ? 1 : -1, 0, 0);
            
            // Snap to nearest hull with horizontal preference
            Vector3 placementPosition = SnapToNearestHull(ship, targetPosition, preferredDirection, gyroSize);
            
            var gyro = new VoxelBlock(
                placementPosition,
                gyroSize,
                config.Material,
                BlockType.GyroArray
            );
            ship.Structure.AddBlock(gyro);
        }
    }
    
    /// <summary>
    /// Place weapon mounts based on role, ensuring connectivity
    /// </summary>
    private void PlaceWeaponMounts(GeneratedShip ship, ShipGenerationConfig config)
    {
        var dimensions = GetShipDimensions(config.Size);
        Vector3 turretSize = new Vector3(2, 2, 2);
        
        int weaponCount = config.Role switch
        {
            ShipRole.Combat => Math.Max(config.MinimumWeaponMounts, (int)(config.Size) * 2 + 4),
            ShipRole.Multipurpose => Math.Max(config.MinimumWeaponMounts, (int)(config.Size) + 2),
            ShipRole.Trading => config.MinimumWeaponMounts,
            ShipRole.Mining => config.MinimumWeaponMounts + 1,
            ShipRole.Exploration => config.MinimumWeaponMounts,
            ShipRole.Salvage => config.MinimumWeaponMounts + 1,
            _ => config.MinimumWeaponMounts
        };
        
        weaponCount = Math.Max(1, (int)(weaponCount * config.Style.WeaponDensity));
        
        // Place weapons around the hull, snapping to hull blocks
        for (int i = 0; i < weaponCount; i++)
        {
            float angle = (360f / weaponCount) * i;
            float rad = angle * MathF.PI / 180f;
            float radius = Math.Min(dimensions.X, dimensions.Y) / 2;
            
            float x = radius * MathF.Cos(rad) * 0.8f;
            float y = radius * MathF.Sin(rad) * 0.8f;
            float z = (float)(_random.NextDouble() * dimensions.Z * 0.66f - dimensions.Z / 3);
            
            Vector3 targetPosition = new Vector3(x, y, z);
            Vector3 preferredDirection = new Vector3(x, y, 0).Length() > 0.1f 
                ? Vector3.Normalize(new Vector3(x, y, 0)) 
                : new Vector3(1, 0, 0);
            
            // Snap to nearest hull with outward preference
            Vector3 placementPosition = SnapToNearestHull(ship, targetPosition, preferredDirection, turretSize);
            
            var turret = new VoxelBlock(
                placementPosition,
                turretSize,
                config.Material,
                BlockType.TurretMount
            );
            ship.Structure.AddBlock(turret);
        }
    }
    
    /// <summary>
    /// Place utility blocks (cargo, hyperdrive, etc.), ensuring connectivity
    /// </summary>
    private void PlaceUtilityBlocks(GeneratedShip ship, ShipGenerationConfig config)
    {
        var dimensions = GetShipDimensions(config.Size);
        
        // Add hyperdrive if required
        if (config.RequireHyperdrive)
        {
            Vector3 hyperdriveSize = new Vector3(4, 4, 4); // Hyperdrive is large
            Vector3 targetPosition = new Vector3(0, 0, dimensions.Z / 3);
            Vector3 placementPosition = SnapToNearestHull(ship, targetPosition, new Vector3(0, 0, 1), hyperdriveSize);
            
            var hyperdrive = new VoxelBlock(
                placementPosition,
                hyperdriveSize,
                config.Material,
                BlockType.HyperdriveCore
            );
            ship.Structure.AddBlock(hyperdrive);
        }
        
        // Add cargo based on role
        if (config.RequireCargo)
        {
            Vector3 cargoSize = new Vector3(3, 3, 3);
            
            int cargoCount = config.Role switch
            {
                ShipRole.Trading => (int)config.Size * 3,
                ShipRole.Mining => (int)config.Size * 2,
                ShipRole.Salvage => (int)config.Size * 2,
                ShipRole.Multipurpose => (int)config.Size,
                _ => Math.Max(1, (int)config.Size / 2)
            };
            
            for (int i = 0; i < cargoCount; i++)
            {
                float z = -dimensions.Z / 4 + (dimensions.Z / 2 / cargoCount) * i;
                float y = -dimensions.Y / 3;
                
                Vector3 targetPosition = new Vector3(0, y, z);
                Vector3 placementPosition = SnapToNearestHull(ship, targetPosition, new Vector3(0, -1, 0), cargoSize);
                
                var cargo = new VoxelBlock(
                    placementPosition,
                    cargoSize,
                    config.Material,
                    BlockType.Cargo
                );
                ship.Structure.AddBlock(cargo);
            }
        }
        
        // Add crew quarters for larger ships
        if (config.Size >= ShipSize.Frigate)
        {
            Vector3 quartersSize = new Vector3(3, 3, 3);
            Vector3 targetPosition = new Vector3(0, dimensions.Y / 3, 0);
            Vector3 placementPosition = SnapToNearestHull(ship, targetPosition, new Vector3(0, 1, 0), quartersSize);
            
            var quarters = new VoxelBlock(
                placementPosition,
                quartersSize,
                config.Material,
                BlockType.CrewQuarters
            );
            ship.Structure.AddBlock(quarters);
        }
        
        // Add pod docking for player
        Vector3 podSize = new Vector3(2, 2, 2);
        Vector3 podTargetPosition = new Vector3(0, -dimensions.Y / 3, dimensions.Z / 3);
        Vector3 podPlacementPosition = SnapToNearestHull(ship, podTargetPosition, new Vector3(0, -1, 1), podSize);
        
        var podDock = new VoxelBlock(
            podPlacementPosition,
            podSize,
            config.Material,
            BlockType.PodDocking
        );
        ship.Structure.AddBlock(podDock);
    }
    
    /// <summary>
    /// Add armor plating around critical systems
    /// </summary>
    private void AddArmorPlating(GeneratedShip ship, ShipGenerationConfig config)
    {
        var style = config.Style;
        int armorBlocksToAdd = (int)(ship.Structure.Blocks.Count * style.ArmorToHullRatio);
        
        // Get all hull blocks
        var hullBlocks = ship.Structure.Blocks.Where(b => b.BlockType == BlockType.Hull).ToList();
        
        // Convert some hull to armor (outer layers priority)
        int converted = 0;
        foreach (var block in hullBlocks.OrderByDescending(b => Math.Abs(b.Position.X) + Math.Abs(b.Position.Y)))
        {
            if (converted >= armorBlocksToAdd) break;
            
            // Replace with armor block
            ship.Structure.RemoveBlock(block);
            var armorBlock = new VoxelBlock(
                block.Position,
                block.Size,
                config.Material,
                BlockType.Armor
            );
            ship.Structure.AddBlock(armorBlock);
            converted++;
        }
    }
    
    /// <summary>
    /// Apply faction color scheme to blocks
    /// </summary>
    private void ApplyColorScheme(GeneratedShip ship, ShipGenerationConfig config)
    {
        var style = config.Style;
        
        foreach (var block in ship.Structure.Blocks)
        {
            // Apply colors based on block type
            if (block.BlockType == BlockType.Hull)
            {
                block.ColorRGB = style.PrimaryColor;
            }
            else if (block.BlockType == BlockType.Armor)
            {
                block.ColorRGB = style.SecondaryColor;
            }
            else if (block.BlockType == BlockType.Engine || block.BlockType == BlockType.Thruster)
            {
                block.ColorRGB = style.AccentColor;
            }
            // Functional blocks keep their material colors
        }
    }
    
    /// <summary>
    /// Add surface detailing to ships (greebles, antennas, panels, vents)
    /// Enhances visual distinctiveness and adds character to ships
    /// </summary>
    private void AddSurfaceDetailing(GeneratedShip ship, ShipGenerationConfig config)
    {
        var style = config.Style;
        var dimensions = GetShipDimensions(config.Size);
        
        // Calculate how many detail elements to add based on ship size
        int detailCount = (int)(ship.Structure.Blocks.Count * 0.15f); // 15% of blocks as details
        detailCount = Math.Max(5, Math.Min(detailCount, 50)); // Between 5 and 50 details
        
        // Add wing structures for Combat and Exploration ships
        if (config.Role == ShipRole.Combat || config.Role == ShipRole.Exploration)
        {
            AddWingStructures(ship, dimensions, config);
        }
        
        // Add antennas on top of ship
        AddAntennas(ship, dimensions, config, detailCount / 5);
        
        // Add surface panels and vents
        AddSurfacePanels(ship, dimensions, config, detailCount / 3);
        
        // Add sensor arrays or lights
        AddSensorArrays(ship, dimensions, config, detailCount / 4);
        
        // Add glowing engine effects
        AddEngineGlow(ship, config);
        
        // Add hull pattern variations (stripes, decals)
        AddHullPatterns(ship, dimensions, config);
    }
    
    /// <summary>
    /// Add wing structures to make ships look more like actual spacecraft
    /// Uses layered blocks and smooth transitions for professional appearance
    /// </summary>
    private void AddWingStructures(GeneratedShip ship, Vector3 dimensions, ShipGenerationConfig config)
    {
        // Add 2-4 wing structures depending on ship size
        int wingCount = config.Size switch
        {
            ShipSize.Fighter => 2,
            ShipSize.Corvette => 2,
            ShipSize.Frigate => 2,
            ShipSize.Destroyer => 3,
            ShipSize.Cruiser => 3,
            ShipSize.Battleship => 4,
            ShipSize.Carrier => 4,
            _ => 2
        };
        
        for (int i = 0; i < wingCount; i++)
        {
            // Wings extend from the sides
            float side = i % 2 == 0 ? 1 : -1; // Alternate left/right
            float zPosition = dimensions.Z * (0.2f - 0.4f * (i / 2)); // Stagger along length
            
            // Wing dimensions - larger for bigger ships, INCREASED thickness
            float wingSpan = dimensions.X * 0.4f + (float)config.Size * 0.5f;
            float wingLength = dimensions.Z * 0.3f;
            float baseThickness = 5.0f; // INCREASED from 2.5f for more substantial wings
            
            // Wing root position (where it connects to hull)
            float rootX = side * (dimensions.X / 2);
            
            // Add transition blocks at wing root for smooth connection to hull
            AddWingRootTransition(ship, new Vector3(rootX, 0, zPosition), side, baseThickness, wingLength, config);
            
            // Create wing with layered blocks for proper 3D structure
            int wingBlocks = 8 + (int)config.Size / 2; // More blocks for smoother wing
            for (int j = 0; j < wingBlocks; j++)
            {
                float progress = (float)j / wingBlocks;
                float xOffset = side * (dimensions.X / 2 + progress * wingSpan);
                
                // Smooth tapering - reduce both thickness and length
                float currentThickness = baseThickness * (1 - progress * 0.6f); // Less aggressive taper
                float currentLength = wingLength * (1 - progress * 0.3f);
                float blockWidth = 2.0f * (1 - progress * 0.4f); // Wider blocks
                
                // CORE WING STRUCTURE - Build from bottom to top for proper layering
                
                // 1. Bottom surface layer (inverted wedge for underside)
                var bottomSurface = new VoxelBlock(
                    new Vector3(xOffset, -currentThickness * 0.4f, zPosition),
                    new Vector3(blockWidth, currentThickness * 0.3f, currentLength),
                    config.Material,
                    BlockType.Hull,
                    BlockShape.Wedge,
                    BlockOrientation.NegY  // Point downward
                );
                bottomSurface.ColorRGB = LerpColor(config.Style.PrimaryColor, config.Style.AccentColor, 0.3f);
                ship.Structure.AddBlock(bottomSurface);
                
                // 2. Main wing core (thicker center section)
                var wingCore = new VoxelBlock(
                    new Vector3(xOffset, 0, zPosition),
                    new Vector3(blockWidth, currentThickness, currentLength),
                    config.Material,
                    BlockType.Hull,
                    BlockShape.Cube,  // Solid center
                    BlockOrientation.PosY
                );
                wingCore.ColorRGB = config.Style.AccentColor;
                ship.Structure.AddBlock(wingCore);
                
                // 3. Top surface layer (wedge for aerodynamic profile)
                var topSurface = new VoxelBlock(
                    new Vector3(xOffset, currentThickness * 0.4f, zPosition),
                    new Vector3(blockWidth * 0.9f, currentThickness * 0.4f, currentLength * 0.95f),
                    config.Material,
                    BlockType.Hull,
                    BlockShape.Wedge,
                    BlockOrientation.PosY
                );
                topSurface.ColorRGB = config.Style.AccentColor;
                ship.Structure.AddBlock(topSurface);
                
                // 4. Leading edge detail (forward-facing wedges for swept look)
                if (progress < 0.8f)
                {
                    var leadingEdge = new VoxelBlock(
                        new Vector3(xOffset, 0, zPosition + currentLength * 0.35f),
                        new Vector3(blockWidth * 0.7f, currentThickness * 0.8f, currentLength * 0.25f),
                        config.Material,
                        BlockType.Hull,
                        BlockShape.Wedge,
                        BlockOrientation.PosZ
                    );
                    leadingEdge.ColorRGB = config.Style.AccentColor;
                    ship.Structure.AddBlock(leadingEdge);
                }
                
                // 5. Trailing edge detail (rear-facing wedges)
                if (progress < 0.7f)
                {
                    var trailingEdge = new VoxelBlock(
                        new Vector3(xOffset, 0, zPosition - currentLength * 0.35f),
                        new Vector3(blockWidth * 0.6f, currentThickness * 0.7f, currentLength * 0.2f),
                        config.Material,
                        BlockType.Hull,
                        BlockShape.Wedge,
                        BlockOrientation.NegZ
                    );
                    trailingEdge.ColorRGB = LerpColor(config.Style.AccentColor, config.Style.SecondaryColor, 0.4f);
                    ship.Structure.AddBlock(trailingEdge);
                }
                
                // 6. Wing ribs/structure (every 2nd block for internal structure look)
                if (j % 2 == 0 && progress < 0.6f)
                {
                    // Internal rib running along wing length
                    var rib = new VoxelBlock(
                        new Vector3(xOffset, 0, zPosition),
                        new Vector3(blockWidth * 0.4f, currentThickness * 0.9f, currentLength * 1.1f),
                        config.Material,
                        BlockType.Hull,
                        BlockShape.HalfBlock,
                        side > 0 ? BlockOrientation.PosX : BlockOrientation.NegX
                    );
                    rib.ColorRGB = LerpColor(config.Style.PrimaryColor, config.Style.AccentColor, 0.5f);
                    ship.Structure.AddBlock(rib);
                }
                
                // 7. Wingtip details (for outer sections)
                if (progress > 0.7f)
                {
                    // Angled wingtip
                    var wingtip = new VoxelBlock(
                        new Vector3(xOffset, currentThickness * 0.2f, zPosition),
                        new Vector3(blockWidth * 0.6f, currentThickness * 0.6f, currentLength * 0.7f),
                        config.Material,
                        BlockType.Hull,
                        BlockShape.Corner,
                        side > 0 ? BlockOrientation.PosX : BlockOrientation.NegX
                    );
                    wingtip.ColorRGB = config.Style.AccentColor;
                    ship.Structure.AddBlock(wingtip);
                }
            }
        }
    }
    
    /// <summary>
    /// Add transition blocks where wing connects to hull for smooth appearance
    /// Enhanced with multi-layer transitions for professional blending
    /// </summary>
    private void AddWingRootTransition(GeneratedShip ship, Vector3 rootPosition, float side, float thickness, float length, ShipGenerationConfig config)
    {
        // Create 4-5 transition blocks that blend wing into hull with multiple layers
        for (int i = 0; i < 4; i++)
        {
            float transitionProgress = (float)i / 4;
            float transitionOffset = side * i * 0.8f;
            
            // Use varied blocks for smooth blending - InnerCorner -> Corner -> Wedge progression
            BlockShape transitionShape = i switch
            {
                0 => BlockShape.InnerCorner,  // Closest to hull - smoothest blend
                1 => BlockShape.Corner,        // Early transition
                2 => BlockShape.Wedge,         // Mid transition  
                _ => BlockShape.HalfBlock      // Final transition to wing
            };
            
            BlockOrientation orientation = side > 0 
                ? BlockOrientation.PosX
                : BlockOrientation.NegX;
            
            // Main transition block
            var transitionBlock = new VoxelBlock(
                rootPosition + new Vector3(transitionOffset, 0, 0),
                new Vector3(1.5f, thickness * (0.6f + transitionProgress * 0.4f), length * (0.75f + transitionProgress * 0.25f)),
                config.Material,
                BlockType.Hull,
                transitionShape,
                orientation
            );
            
            // Blend colors from hull primary to accent
            transitionBlock.ColorRGB = LerpColor(config.Style.PrimaryColor, config.Style.AccentColor, transitionProgress);
            ship.Structure.AddBlock(transitionBlock);
            
            // Add top/bottom reinforcement layers for first 2 transition blocks
            if (i < 2)
            {
                // Top layer
                var topLayer = new VoxelBlock(
                    rootPosition + new Vector3(transitionOffset, thickness * 0.3f, 0),
                    new Vector3(1.3f, thickness * 0.4f, length * 0.9f),
                    config.Material,
                    BlockType.Hull,
                    BlockShape.HalfBlock,
                    BlockOrientation.PosY
                );
                topLayer.ColorRGB = LerpColor(config.Style.PrimaryColor, config.Style.AccentColor, transitionProgress * 0.7f);
                ship.Structure.AddBlock(topLayer);
                
                // Bottom layer
                var bottomLayer = new VoxelBlock(
                    rootPosition + new Vector3(transitionOffset, -thickness * 0.3f, 0),
                    new Vector3(1.3f, thickness * 0.4f, length * 0.9f),
                    config.Material,
                    BlockType.Hull,
                    BlockShape.HalfBlock,
                    BlockOrientation.NegY
                );
                bottomLayer.ColorRGB = LerpColor(config.Style.PrimaryColor, config.Style.AccentColor, transitionProgress * 0.5f);
                ship.Structure.AddBlock(bottomLayer);
            }
        }
    }
    
    /// <summary>
    /// Linearly interpolate between two RGB colors represented as uint
    /// </summary>
    private uint LerpColor(uint color1, uint color2, float t)
    {
        t = Math.Clamp(t, 0f, 1f);
        
        // Extract RGB components from color1
        byte r1 = (byte)((color1 >> 16) & 0xFF);
        byte g1 = (byte)((color1 >> 8) & 0xFF);
        byte b1 = (byte)(color1 & 0xFF);
        
        // Extract RGB components from color2
        byte r2 = (byte)((color2 >> 16) & 0xFF);
        byte g2 = (byte)((color2 >> 8) & 0xFF);
        byte b2 = (byte)(color2 & 0xFF);
        
        // Interpolate each component
        byte r = (byte)(r1 + (r2 - r1) * t);
        byte g = (byte)(g1 + (g2 - g1) * t);
        byte b = (byte)(b1 + (b2 - b1) * t);
        
        // Recombine into uint
        return (uint)((r << 16) | (g << 8) | b);
    }
    
    /// <summary>
    /// Add antenna arrays to the ship
    /// </summary>
    private void AddAntennas(GeneratedShip ship, Vector3 dimensions, ShipGenerationConfig config, int count)
    {
        // Find top hull blocks to attach antennas
        var topBlocks = ship.Structure.Blocks
            .Where(b => b.BlockType == BlockType.Hull)
            .OrderByDescending(b => b.Position.Y)
            .Take(count * 2)
            .ToList();
        
        for (int i = 0; i < Math.Min(count, topBlocks.Count); i++)
        {
            var baseBlock = topBlocks[i * 2];
            
            // Add thin antenna extending upward
            float antennaHeight = 3 + (float)_random.NextDouble() * 3; // 3-6 units tall
            var antennaSize = new Vector3(0.5f, antennaHeight, 0.5f);
            
            var antenna = new VoxelBlock(
                baseBlock.Position + new Vector3(0, baseBlock.Size.Y / 2 + antennaHeight / 2, 0),
                antennaSize,
                config.Material,
                BlockType.Hull
            );
            antenna.ColorRGB = config.Style.AccentColor; // Use accent color for visibility
            ship.Structure.AddBlock(antenna);
        }
    }
    
    /// <summary>
    /// Add surface panels and vents for visual interest
    /// </summary>
    private void AddSurfacePanels(GeneratedShip ship, Vector3 dimensions, ShipGenerationConfig config, int count)
    {
        var hullBlocks = ship.Structure.Blocks
            .Where(b => b.BlockType == BlockType.Hull)
            .ToList();
        
        if (hullBlocks.Count == 0) return;
        
        for (int i = 0; i < count; i++)
        {
            var baseBlock = hullBlocks[_random.Next(hullBlocks.Count)];
            
            // Determine panel orientation based on block position
            float offsetDistance = 0.2f;
            Vector3 offset;
            Vector3 panelSize;
            
            // Panels stick out slightly from hull
            if (Math.Abs(baseBlock.Position.X) > Math.Abs(baseBlock.Position.Y))
            {
                // Side panel
                offset = new Vector3(Math.Sign(baseBlock.Position.X) * (baseBlock.Size.X / 2 + offsetDistance), 0, 0);
                panelSize = new Vector3(0.4f, 1.5f, 1.5f);
            }
            else
            {
                // Top/bottom panel
                offset = new Vector3(0, Math.Sign(baseBlock.Position.Y) * (baseBlock.Size.Y / 2 + offsetDistance), 0);
                panelSize = new Vector3(1.5f, 0.4f, 1.5f);
            }
            
            var panel = new VoxelBlock(
                baseBlock.Position + offset,
                panelSize,
                config.Material,
                BlockType.Hull
            );
            
            // Alternate between primary and secondary colors for variety
            panel.ColorRGB = i % 2 == 0 ? config.Style.PrimaryColor : config.Style.SecondaryColor;
            ship.Structure.AddBlock(panel);
        }
    }
    
    /// <summary>
    /// Add sensor arrays and lights to the ship
    /// </summary>
    private void AddSensorArrays(GeneratedShip ship, Vector3 dimensions, ShipGenerationConfig config, int count)
    {
        // Add sensors near the front of the ship
        var frontBlocks = ship.Structure.Blocks
            .Where(b => b.BlockType == BlockType.Hull && b.Position.Z > dimensions.Z / 4)
            .OrderByDescending(b => b.Position.Z)
            .Take(count * 2)
            .ToList();
        
        for (int i = 0; i < Math.Min(count, frontBlocks.Count); i++)
        {
            var baseBlock = frontBlocks[i];
            
            // Small sensor dish or light
            var sensorSize = new Vector3(1f, 1f, 1f);
            var offset = new Vector3(
                (float)_random.NextDouble() * 2 - 1,
                (float)_random.NextDouble() * 2 - 1,
                0.5f
            );
            
            var sensor = new VoxelBlock(
                baseBlock.Position + offset,
                sensorSize,
                config.Material,
                BlockType.TurretMount
            );
            sensor.ColorRGB = config.Style.AccentColor;
            ship.Structure.AddBlock(sensor);
        }
    }
    
    /// <summary>
    /// Add glowing effects to engines for visual distinction
    /// </summary>
    private void AddEngineGlow(GeneratedShip ship, ShipGenerationConfig config)
    {
        var engines = ship.Structure.Blocks
            .Where(b => b.BlockType == BlockType.Engine || b.BlockType == BlockType.Thruster)
            .ToList();
        
        foreach (var engine in engines)
        {
            // Add LARGER glowing block behind engine for better visibility
            var glowSize = new Vector3(1.5f, 1.5f, 1.5f); // INCREASED from 0.8
            var glowOffset = new Vector3(0, 0, -engine.Size.Z / 2 - glowSize.Z / 2 - 0.3f);
            
            var glow = new VoxelBlock(
                engine.Position + glowOffset,
                glowSize,
                "Energy", // Use energy material for glow effect
                BlockType.Hull
            );
            glow.ColorRGB = ENGINE_GLOW_COLOR;
            ship.Structure.AddBlock(glow);
            
            // Add secondary dimmer glow for depth effect
            var outerGlowSize = new Vector3(2f, 2f, 0.5f);
            var outerGlowOffset = new Vector3(0, 0, -engine.Size.Z / 2 - glowSize.Z - 0.8f);
            
            var outerGlow = new VoxelBlock(
                engine.Position + outerGlowOffset,
                outerGlowSize,
                "Energy",
                BlockType.Hull
            );
            outerGlow.ColorRGB = OUTER_GLOW_COLOR;
            ship.Structure.AddBlock(outerGlow);
        }
    }
    
    /// <summary>
    /// Add hull patterns (stripes, decals) for visual variety
    /// </summary>
    private void AddHullPatterns(GeneratedShip ship, Vector3 dimensions, ShipGenerationConfig config)
    {
        // Add racing stripes or faction markings
        if (config.Style.Philosophy == DesignPhilosophy.SpeedFocused ||
            config.Style.Philosophy == DesignPhilosophy.CombatFocused)
        {
            // Add stripes along the length
            var sideBlocks = ship.Structure.Blocks
                .Where(b => b.BlockType == BlockType.Hull || b.BlockType == BlockType.Armor)
                .Where(b => Math.Abs(b.Position.Y) < dimensions.Y / 4) // Middle height
                .OrderBy(b => b.Position.Z)
                .ToList();
            
            // Color every 3rd block with accent color for stripe effect
            for (int i = 0; i < sideBlocks.Count; i += 3)
            {
                if (_random.NextDouble() < 0.3) // 30% chance per stripe segment
                {
                    sideBlocks[i].ColorRGB = config.Style.AccentColor;
                }
            }
        }
    }
    
    /// <summary>
    /// Calculate ship statistics
    /// </summary>
    private void CalculateShipStats(GeneratedShip ship)
    {
        ship.TotalThrust = 0;
        ship.TotalPowerGeneration = 0;
        ship.TotalShieldCapacity = 0;
        ship.WeaponMountCount = 0;
        ship.CargoBlockCount = 0;
        
        foreach (var block in ship.Structure.Blocks)
        {
            ship.TotalThrust += block.ThrustPower;
            ship.TotalPowerGeneration += block.PowerGeneration;
            ship.TotalShieldCapacity += block.ShieldCapacity;
            
            if (block.BlockType == BlockType.TurretMount)
                ship.WeaponMountCount++;
            if (block.BlockType == BlockType.Cargo)
                ship.CargoBlockCount++;
        }
        
        ship.Stats["TotalMass"] = ship.TotalMass;
        ship.Stats["TotalThrust"] = ship.TotalThrust;
        ship.Stats["PowerGeneration"] = ship.TotalPowerGeneration;
        ship.Stats["ShieldCapacity"] = ship.TotalShieldCapacity;
        ship.Stats["ThrustToMass"] = ship.TotalMass > 0 ? ship.TotalThrust / ship.TotalMass : 0;
    }
    
    /// <summary>
    /// Validate that the ship is functional
    /// </summary>
    private void ValidateShip(GeneratedShip ship, ShipGenerationConfig config)
    {
        // Check for minimum requirements
        if (ship.TotalThrust <= 0)
        {
            ship.Warnings.Add("WARNING: Ship has no thrust! Cannot move.");
        }
        
        if (ship.TotalPowerGeneration <= 0)
        {
            ship.Warnings.Add("WARNING: Ship has no power generation! Systems will not function.");
        }
        
        if (ship.WeaponMountCount < config.MinimumWeaponMounts)
        {
            ship.Warnings.Add($"WARNING: Ship has only {ship.WeaponMountCount} weapon mounts (minimum: {config.MinimumWeaponMounts})");
        }
        
        // Check thrust-to-mass ratio
        float thrustToMass = ship.TotalMass > 0 ? ship.TotalThrust / ship.TotalMass : 0;
        if (thrustToMass < 0.5f)
        {
            ship.Warnings.Add($"WARNING: Low thrust-to-mass ratio ({thrustToMass:F2}). Ship may be sluggish.");
        }
        
        // Log warnings
        foreach (var warning in ship.Warnings)
        {
            _logger.Warning("ShipGenerator", warning);
        }
    }
    
    /// <summary>
    /// Validate structural integrity with connectivity graph
    /// </summary>
    private void ValidateStructuralIntegrity(GeneratedShip ship, ShipGenerationConfig config)
    {
        var integritySystem = new StructuralIntegritySystem();
        var result = integritySystem.ValidateStructure(ship.Structure);
        
        if (!result.IsValid)
        {
            foreach (var error in result.Errors)
            {
                ship.Warnings.Add($"STRUCTURAL INTEGRITY: {error}");
                _logger.Warning("ShipGenerator", error);
            }
            
            // Attempt to fix disconnected blocks
            if (result.DisconnectedBlocks.Count > 0)
            {
                _logger.Info("ShipGenerator", $"Attempting to connect {result.DisconnectedBlocks.Count} disconnected blocks");
                var connectingBlocks = integritySystem.SuggestConnectingBlocks(ship.Structure, result);
                
                foreach (var block in connectingBlocks.Take(10)) // Limit repairs to avoid bloat
                {
                    ship.Structure.AddBlock(block);
                }
                
                // Re-validate after repair
                var revalidation = integritySystem.ValidateStructure(ship.Structure);
                if (revalidation.IsValid)
                {
                    _logger.Info("ShipGenerator", "Structural integrity restored by adding connecting blocks");
                }
            }
        }
        else
        {
            _logger.Info("ShipGenerator", $"Structural integrity validated - all {ship.Structure.Blocks.Count} blocks connected");
        }
        
        // Store integrity percentage
        ship.Stats["StructuralIntegrity"] = integritySystem.CalculateStructuralIntegrityPercentage(ship.Structure, result);
    }
    
    /// <summary>
    /// Validate functional requirements and connectivity
    /// </summary>
    private void ValidateFunctionalRequirements(GeneratedShip ship, ShipGenerationConfig config)
    {
        var requirementsSystem = new FunctionalRequirementsSystem();
        var result = requirementsSystem.ValidateRequirements(ship.Structure);
        
        if (!result.IsValid)
        {
            foreach (var error in result.Errors)
            {
                ship.Warnings.Add($"FUNCTIONAL: {error}");
                _logger.Warning("ShipGenerator", error);
            }
        }
        
        foreach (var warning in result.Warnings)
        {
            ship.Warnings.Add($"FUNCTIONAL: {warning}");
            _logger.Warning("ShipGenerator", warning);
        }
        
        // Get suggestions for improvements
        var suggestions = requirementsSystem.GetComponentSuggestions(result);
        foreach (var suggestion in suggestions.Take(5)) // Limit suggestions
        {
            _logger.Info("ShipGenerator", $"Suggestion: {suggestion}");
        }
        
        // Store functional metrics
        ship.Stats["PowerMargin"] = result.TotalPowerConsumption > 0 
            ? result.TotalPowerGeneration / result.TotalPowerConsumption 
            : 0f;
        ship.Stats["EnginesConnected"] = result.EnginesConnectedToPower ? 1f : 0f;
    }
    
    /// <summary>
    /// Validate aesthetic guidelines
    /// </summary>
    private void ValidateAesthetics(GeneratedShip ship, ShipGenerationConfig config)
    {
        var aestheticsSystem = new AestheticGuidelinesSystem();
        var result = aestheticsSystem.ValidateAesthetics(ship.Structure, config.Style);
        
        // Log aesthetic analysis
        _logger.Info("ShipGenerator", 
            $"Aesthetics - Symmetry: {result.DetectedSymmetry} ({result.SymmetryScore:F2}), " +
            $"Balance: {result.BalanceScore:F2}, " +
            $"Design Language: {(result.HasConsistentDesignLanguage ? "Consistent" : "Inconsistent")}");
        
        // Add suggestions as warnings
        foreach (var suggestion in result.Suggestions.Take(3)) // Limit to top 3 suggestions
        {
            ship.Warnings.Add($"AESTHETIC: {suggestion}");
        }
        
        // Store aesthetic metrics
        ship.Stats["Symmetry"] = result.SymmetryScore;
        ship.Stats["Balance"] = result.BalanceScore;
        ship.Stats["DesignLanguage"] = result.HasConsistentDesignLanguage ? 1f : 0f;
    }
}
