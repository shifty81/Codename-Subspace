using System.Numerics;
using AvorionLike.Core.Voxel;
using AvorionLike.Core.Logging;

namespace AvorionLike.Core.Procedural;

/// <summary>
/// Configuration for industrial mining ship generation
/// </summary>
public class IndustrialMiningShipConfig
{
    public ShipSize Size { get; set; } = ShipSize.Frigate;
    public string Material { get; set; } = "Iron";
    public int Seed { get; set; } = 0;
    
    // Mining-specific configuration
    public int MiningLaserCount { get; set; } = 4;
    public int OreProcessorCount { get; set; } = 2;
    public int CargoModuleCount { get; set; } = 6;
    
    // Industrial style options
    public float IndustrialComplexity { get; set; } = 0.7f;
    public bool UseAsymmetricMiningArms { get; set; } = true;
    public bool UseExposedFramework { get; set; } = true;
    public bool UseAngularPanels { get; set; } = true;
    
    // Color scheme
    public uint PrimaryColor { get; set; } = 0x4A4A50;     // Industrial dark gray
    public uint SecondaryColor { get; set; } = 0x8B7355;   // Industrial tan/rust
    public uint AccentColor { get; set; } = 0xFFA500;      // Safety orange
    public uint HighlightColor { get; set; } = 0xFFD700;   // Warning yellow
}

/// <summary>
/// Result of industrial mining ship generation
/// </summary>
public class GeneratedMiningShip
{
    public VoxelStructureComponent Structure { get; set; } = new();
    public IndustrialMiningShipConfig Config { get; set; } = new();
    public Dictionary<string, float> Stats { get; set; } = new();
    public List<string> Warnings { get; set; } = new();
    public string ShipName { get; set; } = "Industrial Mining Vessel";
    
    public float TotalMass => Structure.TotalMass;
    public float TotalThrust { get; set; }
    public float TotalPowerGeneration { get; set; }
    public float MiningCapacity { get; set; }
    public int CargoCapacity { get; set; }
    public int MiningLaserCount { get; set; }
}

/// <summary>
/// Procedurally generates industrial mining ships with angular and blocky shapes
/// Inspired by voxel-based mining ships from games like Avorion, Space Engineers, and Empyrion
/// 
/// Design Philosophy:
/// - Angular, blocky industrial aesthetic
/// - Exposed framework and machinery
/// - Asymmetric mining arms and equipment
/// - Heavy cargo capacity
/// - Reinforced hull structure
/// - Visible ore processing equipment
/// </summary>
public class IndustrialMiningShipGenerator
{
    private Random _random;
    private readonly Logger _logger = Logger.Instance;
    
    public IndustrialMiningShipGenerator(int seed = 0)
    {
        _random = seed == 0 ? new Random() : new Random(seed);
    }
    
    /// <summary>
    /// Generate an industrial mining ship based on configuration
    /// </summary>
    public GeneratedMiningShip GenerateMiningShip(IndustrialMiningShipConfig config)
    {
        _random = new Random(config.Seed == 0 ? Environment.TickCount : config.Seed);
        
        var result = new GeneratedMiningShip { Config = config };
        
        _logger.Info("MiningShipGenerator", $"Generating {config.Size} industrial mining ship (Seed: {config.Seed})");
        
        // Step 1: Calculate ship dimensions
        var dimensions = GetMiningShipDimensions(config.Size);
        
        _logger.Info("MiningShipGenerator", $"Ship dimensions: {dimensions.X:F1}x{dimensions.Y:F1}x{dimensions.Z:F1}");
        
        // Step 2: Generate main hull structure (blocky, industrial)
        GenerateBlockyIndustrialHull(result, dimensions, config);
        
        // Step 3: Add exposed framework/gantry structure
        if (config.UseExposedFramework)
        {
            GenerateExposedFramework(result, dimensions, config);
        }
        
        // Step 4: Add cargo modules (large boxy containers)
        GenerateCargoModules(result, dimensions, config);
        
        // Step 5: Add mining equipment (forward-facing lasers/drills)
        GenerateMiningEquipment(result, dimensions, config);
        
        // Step 6: Add ore processing modules
        GenerateOreProcessors(result, dimensions, config);
        
        // Step 7: Add engine block (rear industrial engines)
        GenerateIndustrialEngines(result, dimensions, config);
        
        // Step 8: Add industrial details (pipes, vents, panels)
        GenerateIndustrialDetails(result, dimensions, config);
        
        // Step 9: Add functional components (generators, shields)
        GenerateFunctionalComponents(result, dimensions, config);
        
        // Step 10: Apply color scheme
        ApplyIndustrialColorScheme(result, config);
        
        // Step 11: Calculate statistics
        CalculateShipStats(result);
        
        // Step 12: Validate ship
        ValidateMiningShip(result);
        
        _logger.Info("MiningShipGenerator", 
            $"Generated mining ship with {result.Structure.Blocks.Count} blocks, " +
            $"{result.CargoCapacity} cargo bays, {result.MiningLaserCount} mining lasers");
        
        return result;
    }
    
    /// <summary>
    /// Get dimensions for mining ships - wider and bulkier than combat vessels
    /// </summary>
    private Vector3 GetMiningShipDimensions(ShipSize size)
    {
        // Mining ships are wider and bulkier than combat ships
        return size switch
        {
            ShipSize.Fighter => new Vector3(12, 8, 16),       // Small mining drone
            ShipSize.Corvette => new Vector3(18, 12, 24),     // Light mining vessel
            ShipSize.Frigate => new Vector3(28, 16, 38),      // Standard mining ship
            ShipSize.Destroyer => new Vector3(38, 22, 52),    // Heavy mining ship
            ShipSize.Cruiser => new Vector3(52, 30, 70),      // Mining cruiser
            ShipSize.Battleship => new Vector3(70, 40, 95),   // Industrial behemoth
            ShipSize.Carrier => new Vector3(90, 50, 120),     // Massive mining operation vessel
            _ => new Vector3(28, 16, 38)
        };
    }
    
    /// <summary>
    /// Generate the main blocky industrial hull
    /// Creates a rugged, angular appearance typical of mining vessels
    /// </summary>
    private void GenerateBlockyIndustrialHull(GeneratedMiningShip ship, Vector3 dimensions, IndustrialMiningShipConfig config)
    {
        float blockSize = 2f;
        
        // Main body - thick central rectangular hull
        float mainWidth = dimensions.X * 0.6f;
        float mainHeight = dimensions.Y * 0.7f;
        float mainLength = dimensions.Z * 0.7f;
        
        // Generate main hull block section
        GenerateBlockySectionFilled(ship, Vector3.Zero, new Vector3(mainWidth, mainHeight, mainLength), config, blockSize);
        
        // Add angular front section (mining head mounting area)
        float frontWidth = mainWidth * 0.8f;
        float frontHeight = mainHeight * 0.6f;
        float frontLength = dimensions.Z * 0.2f;
        float frontZ = mainLength / 2 + frontLength / 2 - blockSize;
        
        GenerateAngularFrontSection(ship, new Vector3(0, -mainHeight * 0.1f, frontZ), 
            new Vector3(frontWidth, frontHeight, frontLength), config, blockSize);
        
        // Add rear engine mounting block
        float rearWidth = mainWidth * 0.9f;
        float rearHeight = mainHeight * 0.8f;
        float rearLength = dimensions.Z * 0.15f;
        float rearZ = -mainLength / 2 - rearLength / 2 + blockSize;
        
        GenerateBlockySectionFilled(ship, new Vector3(0, 0, rearZ), 
            new Vector3(rearWidth, rearHeight, rearLength), config, blockSize);
        
        // Add bridge/command module on top
        float bridgeWidth = mainWidth * 0.4f;
        float bridgeHeight = dimensions.Y * 0.25f;
        float bridgeLength = mainLength * 0.3f;
        float bridgeY = mainHeight / 2 + bridgeHeight / 2 - blockSize;
        float bridgeZ = mainLength * 0.2f;
        
        GenerateBlockySectionFilled(ship, new Vector3(0, bridgeY, bridgeZ), 
            new Vector3(bridgeWidth, bridgeHeight, bridgeLength), config, blockSize);
    }
    
    /// <summary>
    /// Generate a filled blocky section for the hull
    /// </summary>
    private void GenerateBlockySectionFilled(GeneratedMiningShip ship, Vector3 center, Vector3 size, 
        IndustrialMiningShipConfig config, float blockSize)
    {
        float halfX = size.X / 2;
        float halfY = size.Y / 2;
        float halfZ = size.Z / 2;
        
        // Pre-calculate edge thresholds to avoid repeated arithmetic in loops
        float edgeThresholdX = halfX - blockSize * 1.5f;
        float edgeThresholdY = halfY - blockSize * 1.5f;
        float edgeThresholdZ = halfZ - blockSize * 1.5f;
        double interiorChance = config.IndustrialComplexity * 0.2;
        
        // Generate shell with some interior structure
        for (float x = -halfX; x < halfX; x += blockSize)
        {
            for (float y = -halfY; y < halfY; y += blockSize)
            {
                for (float z = -halfZ; z < halfZ; z += blockSize)
                {
                    // Create shell (blocks on edges)
                    bool isEdge = Math.Abs(x) > edgeThresholdX ||
                                  Math.Abs(y) > edgeThresholdY ||
                                  Math.Abs(z) > edgeThresholdZ;
                    
                    // Add some interior structure based on complexity
                    bool isInterior = !isEdge && _random.NextDouble() < interiorChance;
                    
                    if (isEdge || isInterior)
                    {
                        ship.Structure.AddBlock(new VoxelBlock(
                            center + new Vector3(x, y, z),
                            new Vector3(blockSize, blockSize, blockSize),
                            config.Material, BlockType.Hull));
                    }
                }
            }
        }
    }
    
    /// <summary>
    /// Generate angular front section for mining equipment mounting
    /// </summary>
    private void GenerateAngularFrontSection(GeneratedMiningShip ship, Vector3 center, Vector3 size, 
        IndustrialMiningShipConfig config, float blockSize)
    {
        float halfX = size.X / 2;
        float halfY = size.Y / 2;
        float halfZ = size.Z / 2;
        
        // Create angular tapered section
        for (float z = -halfZ; z < halfZ; z += blockSize)
        {
            float zProgress = (z + halfZ) / size.Z;
            float taper = 1.0f - zProgress * 0.4f; // Taper toward front
            float currentWidth = size.X * taper;
            float currentHeight = size.Y * taper;
            
            // Generate cross-section
            for (float x = -currentWidth / 2; x < currentWidth / 2; x += blockSize)
            {
                for (float y = -currentHeight / 2; y < currentHeight / 2; y += blockSize)
                {
                    bool isEdge = Math.Abs(x) > currentWidth / 2 - blockSize * 1.2f ||
                                  Math.Abs(y) > currentHeight / 2 - blockSize * 1.2f;
                    
                    if (isEdge)
                    {
                        // Use wedge shapes at the very front for angular look
                        bool isFrontEdge = z > halfZ - blockSize * 2;
                        
                        if (isFrontEdge)
                        {
                            ship.Structure.AddBlock(new VoxelBlock(
                                center + new Vector3(x, y, z),
                                new Vector3(blockSize, blockSize, blockSize),
                                config.Material, BlockType.Hull, BlockShape.Wedge, BlockOrientation.PosZ));
                        }
                        else
                        {
                            ship.Structure.AddBlock(new VoxelBlock(
                                center + new Vector3(x, y, z),
                                new Vector3(blockSize, blockSize, blockSize),
                                config.Material, BlockType.Hull));
                        }
                    }
                }
            }
        }
    }
    
    /// <summary>
    /// Generate exposed framework/gantry structure typical of industrial vessels
    /// </summary>
    private void GenerateExposedFramework(GeneratedMiningShip ship, Vector3 dimensions, IndustrialMiningShipConfig config)
    {
        float beamSize = 1.5f;
        
        // Vertical support pillars on sides
        float pillarSpacing = dimensions.Z / 5;
        for (float z = -dimensions.Z / 3; z < dimensions.Z / 3; z += pillarSpacing)
        {
            // Left side pillars
            GenerateVerticalBeam(ship, new Vector3(-dimensions.X / 2 - beamSize, 0, z), 
                dimensions.Y * 0.6f, beamSize, config);
            
            // Right side pillars
            GenerateVerticalBeam(ship, new Vector3(dimensions.X / 2 + beamSize, 0, z), 
                dimensions.Y * 0.6f, beamSize, config);
        }
        
        // Horizontal cross-beams on top
        for (float z = -dimensions.Z / 3; z < dimensions.Z / 3; z += pillarSpacing)
        {
            GenerateHorizontalBeam(ship, new Vector3(0, dimensions.Y / 2 + beamSize, z), 
                dimensions.X * 0.8f, beamSize, config);
        }
        
        // Gantry crane structure on top (for mining operations)
        float gantryZ = dimensions.Z * 0.3f;
        float gantryWidth = dimensions.X * 0.5f;
        float gantryHeight = dimensions.Y * 0.2f;
        
        // Gantry rails
        ship.Structure.AddBlock(new VoxelBlock(
            new Vector3(-gantryWidth / 2, dimensions.Y / 2 + gantryHeight, gantryZ),
            new Vector3(beamSize, beamSize, dimensions.Z * 0.3f),
            config.Material, BlockType.Hull));
        ship.Structure.AddBlock(new VoxelBlock(
            new Vector3(gantryWidth / 2, dimensions.Y / 2 + gantryHeight, gantryZ),
            new Vector3(beamSize, beamSize, dimensions.Z * 0.3f),
            config.Material, BlockType.Hull));
        
        // Gantry cross beam
        ship.Structure.AddBlock(new VoxelBlock(
            new Vector3(0, dimensions.Y / 2 + gantryHeight, gantryZ),
            new Vector3(gantryWidth, beamSize, beamSize),
            config.Material, BlockType.Hull));
    }
    
    /// <summary>
    /// Generate a vertical support beam
    /// </summary>
    private void GenerateVerticalBeam(GeneratedMiningShip ship, Vector3 position, float height, float beamSize, 
        IndustrialMiningShipConfig config)
    {
        float halfHeight = height / 2;
        for (float y = -halfHeight; y < halfHeight; y += beamSize * 1.5f)
        {
            ship.Structure.AddBlock(new VoxelBlock(
                position + new Vector3(0, y, 0),
                new Vector3(beamSize, beamSize * 1.5f, beamSize),
                config.Material, BlockType.Hull));
        }
    }
    
    /// <summary>
    /// Generate a horizontal cross beam
    /// </summary>
    private void GenerateHorizontalBeam(GeneratedMiningShip ship, Vector3 position, float width, float beamSize, 
        IndustrialMiningShipConfig config)
    {
        ship.Structure.AddBlock(new VoxelBlock(
            position,
            new Vector3(width, beamSize, beamSize),
            config.Material, BlockType.Hull));
    }
    
    /// <summary>
    /// Generate cargo modules (large boxy containers)
    /// </summary>
    private void GenerateCargoModules(GeneratedMiningShip ship, Vector3 dimensions, IndustrialMiningShipConfig config)
    {
        float cargoWidth = dimensions.X * 0.25f;
        float cargoHeight = dimensions.Y * 0.35f;
        float cargoLength = dimensions.Z * 0.15f;
        float blockSize = 2f;
        
        int modulesPlaced = 0;
        int modulesPerSide = (config.CargoModuleCount + 1) / 2;
        
        // Place cargo modules along the sides of the ship
        for (int i = 0; i < modulesPerSide && modulesPlaced < config.CargoModuleCount; i++)
        {
            float zOffset = -dimensions.Z * 0.25f + (i * cargoLength * 1.5f);
            
            // Left side cargo bay
            if (modulesPlaced < config.CargoModuleCount)
            {
                Vector3 leftPos = new Vector3(-dimensions.X / 2 - cargoWidth / 2 + blockSize, 
                    -dimensions.Y * 0.1f, zOffset);
                GenerateCargoModule(ship, leftPos, new Vector3(cargoWidth, cargoHeight, cargoLength), config, blockSize);
                modulesPlaced++;
            }
            
            // Right side cargo bay
            if (modulesPlaced < config.CargoModuleCount)
            {
                Vector3 rightPos = new Vector3(dimensions.X / 2 + cargoWidth / 2 - blockSize, 
                    -dimensions.Y * 0.1f, zOffset);
                GenerateCargoModule(ship, rightPos, new Vector3(cargoWidth, cargoHeight, cargoLength), config, blockSize);
                modulesPlaced++;
            }
        }
        
        // Bottom cargo holds
        if (modulesPlaced < config.CargoModuleCount)
        {
            Vector3 bottomPos = new Vector3(0, -dimensions.Y / 2 - cargoHeight / 2 + blockSize, 0);
            GenerateCargoModule(ship, bottomPos, new Vector3(cargoWidth * 1.5f, cargoHeight, cargoLength * 2), config, blockSize);
            modulesPlaced++;
        }
        
        ship.CargoCapacity = modulesPlaced;
    }
    
    /// <summary>
    /// Generate a single cargo module (blocky container)
    /// </summary>
    private void GenerateCargoModule(GeneratedMiningShip ship, Vector3 center, Vector3 size, 
        IndustrialMiningShipConfig config, float blockSize)
    {
        // Generate solid blocky cargo container
        float halfX = size.X / 2;
        float halfY = size.Y / 2;
        float halfZ = size.Z / 2;
        
        for (float x = -halfX; x < halfX; x += blockSize)
        {
            for (float y = -halfY; y < halfY; y += blockSize)
            {
                for (float z = -halfZ; z < halfZ; z += blockSize)
                {
                    // Create hollow cargo container (shell only)
                    bool isShell = Math.Abs(x) > halfX - blockSize * 1.2f ||
                                   Math.Abs(y) > halfY - blockSize * 1.2f ||
                                   Math.Abs(z) > halfZ - blockSize * 1.2f;
                    
                    if (isShell)
                    {
                        ship.Structure.AddBlock(new VoxelBlock(
                            center + new Vector3(x, y, z),
                            new Vector3(blockSize, blockSize, blockSize),
                            config.Material, BlockType.Cargo));
                    }
                }
            }
        }
        
        // Add support struts connecting to main hull
        float strutSize = 1.5f;
        float strutX = center.X > 0 ? center.X - size.X / 2 - strutSize : center.X + size.X / 2 + strutSize;
        ship.Structure.AddBlock(new VoxelBlock(
            new Vector3(strutX, center.Y, center.Z),
            new Vector3(strutSize * 2, strutSize, strutSize),
            config.Material, BlockType.Hull));
    }
    
    /// <summary>
    /// Generate mining equipment (forward-facing lasers and drills)
    /// </summary>
    private void GenerateMiningEquipment(GeneratedMiningShip ship, Vector3 dimensions, IndustrialMiningShipConfig config)
    {
        float blockSize = 2f;
        float miningArmLength = dimensions.Z * 0.15f;
        float miningArmWidth = dimensions.X * 0.08f;
        
        int laserMountCount = config.MiningLaserCount;
        ship.MiningLaserCount = laserMountCount;
        
        // Front-mounted mining arms with lasers
        float armSpacing = dimensions.X / (laserMountCount + 1);
        
        for (int i = 0; i < laserMountCount; i++)
        {
            float xOffset = -dimensions.X / 2 + (i + 1) * armSpacing;
            float zPos = dimensions.Z / 2 + miningArmLength / 2;
            
            // Alternate arm height for visual interest
            float yOffset = (i % 2 == 0) ? dimensions.Y * 0.1f : -dimensions.Y * 0.1f;
            
            // Mining arm structure
            Vector3 armCenter = new Vector3(xOffset, yOffset, zPos);
            
            // Arm beam
            ship.Structure.AddBlock(new VoxelBlock(
                armCenter,
                new Vector3(miningArmWidth, miningArmWidth, miningArmLength),
                config.Material, BlockType.Hull));
            
            // Mining laser mount at the end
            Vector3 laserPos = armCenter + new Vector3(0, 0, miningArmLength / 2 + blockSize / 2);
            ship.Structure.AddBlock(new VoxelBlock(
                laserPos,
                new Vector3(blockSize * 1.5f, blockSize * 1.5f, blockSize),
                config.Material, BlockType.TurretMount));
            
            // Joint connector to main hull
            Vector3 jointPos = new Vector3(xOffset, yOffset, dimensions.Z / 2 - blockSize);
            ship.Structure.AddBlock(new VoxelBlock(
                jointPos,
                new Vector3(miningArmWidth * 1.5f, miningArmWidth * 1.5f, blockSize * 2),
                config.Material, BlockType.Hull));
        }
        
        // Add asymmetric extended mining arm if enabled
        if (config.UseAsymmetricMiningArms && laserMountCount >= 2)
        {
            // Extended boom arm on one side
            float boomX = dimensions.X * 0.3f;
            float boomY = dimensions.Y * 0.15f;
            float boomZ = dimensions.Z / 2 + miningArmLength * 1.5f;
            
            // Boom structure
            for (float z = dimensions.Z / 2; z < boomZ; z += blockSize)
            {
                ship.Structure.AddBlock(new VoxelBlock(
                    new Vector3(boomX, boomY, z),
                    new Vector3(blockSize, blockSize, blockSize),
                    config.Material, BlockType.Hull));
            }
            
            // Boom tip with heavy duty mining laser
            ship.Structure.AddBlock(new VoxelBlock(
                new Vector3(boomX, boomY, boomZ),
                new Vector3(blockSize * 2, blockSize * 2, blockSize * 2),
                config.Material, BlockType.TurretMount));
            
            ship.MiningLaserCount++;
        }
    }
    
    /// <summary>
    /// Generate ore processing modules
    /// </summary>
    private void GenerateOreProcessors(GeneratedMiningShip ship, Vector3 dimensions, IndustrialMiningShipConfig config)
    {
        float blockSize = 2f;
        float processorWidth = dimensions.X * 0.2f;
        float processorHeight = dimensions.Y * 0.25f;
        float processorLength = dimensions.Z * 0.12f;
        
        // Place ore processors along the dorsal spine of the ship
        for (int i = 0; i < config.OreProcessorCount; i++)
        {
            float zOffset = -dimensions.Z * 0.1f + i * processorLength * 1.8f;
            Vector3 center = new Vector3(0, dimensions.Y / 2 + processorHeight / 2 - blockSize, zOffset);
            
            // Generate blocky processor module
            float halfX = processorWidth / 2;
            float halfY = processorHeight / 2;
            float halfZ = processorLength / 2;
            
            for (float x = -halfX; x < halfX; x += blockSize)
            {
                for (float y = -halfY; y < halfY; y += blockSize)
                {
                    for (float z = -halfZ; z < halfZ; z += blockSize)
                    {
                        bool isShell = Math.Abs(x) > halfX - blockSize * 1.2f ||
                                       Math.Abs(y) > halfY - blockSize * 1.2f ||
                                       Math.Abs(z) > halfZ - blockSize * 1.2f;
                        
                        if (isShell)
                        {
                            ship.Structure.AddBlock(new VoxelBlock(
                                center + new Vector3(x, y, z),
                                new Vector3(blockSize, blockSize, blockSize),
                                config.Material, BlockType.Hull));
                        }
                    }
                }
            }
            
            // Add industrial pipe connections
            ship.Structure.AddBlock(new VoxelBlock(
                center + new Vector3(-processorWidth / 2 - blockSize / 2, 0, 0),
                new Vector3(blockSize, blockSize * 0.5f, blockSize * 0.5f),
                config.Material, BlockType.Hull));
            ship.Structure.AddBlock(new VoxelBlock(
                center + new Vector3(processorWidth / 2 + blockSize / 2, 0, 0),
                new Vector3(blockSize, blockSize * 0.5f, blockSize * 0.5f),
                config.Material, BlockType.Hull));
        }
    }
    
    /// <summary>
    /// Generate industrial engine block
    /// </summary>
    private void GenerateIndustrialEngines(GeneratedMiningShip ship, Vector3 dimensions, IndustrialMiningShipConfig config)
    {
        float blockSize = 2f;
        float engineZ = -dimensions.Z / 2 - blockSize;
        
        // Engine count based on ship size
        int engineCount = config.Size switch
        {
            ShipSize.Fighter => 1,
            ShipSize.Corvette => 2,
            ShipSize.Frigate => 3,
            ShipSize.Destroyer => 4,
            ShipSize.Cruiser => 5,
            ShipSize.Battleship => 6,
            ShipSize.Carrier => 8,
            _ => 3
        };
        
        // Handle single engine case to avoid division by zero
        float engineSpacing = engineCount > 1 ? dimensions.X * 0.7f / (engineCount - 1) : 0;
        float startX = -dimensions.X * 0.35f;
        
        for (int i = 0; i < engineCount; i++)
        {
            float xPos = engineCount == 1 ? 0 : startX + i * engineSpacing;
            
            float yPos = -dimensions.Y * 0.1f; // Slightly below center
            
            // Engine nacelle (blocky)
            float engineWidth = dimensions.X * 0.12f;
            float engineHeight = dimensions.Y * 0.2f;
            float engineLength = dimensions.Z * 0.08f;
            
            // Engine block
            ship.Structure.AddBlock(new VoxelBlock(
                new Vector3(xPos, yPos, engineZ - engineLength / 2),
                new Vector3(engineWidth, engineHeight, engineLength),
                config.Material, BlockType.Engine));
            
            // Engine intake (front of engine)
            ship.Structure.AddBlock(new VoxelBlock(
                new Vector3(xPos, yPos, engineZ + engineLength / 2 + blockSize / 2),
                new Vector3(engineWidth * 0.8f, engineHeight * 0.8f, blockSize),
                config.Material, BlockType.Hull));
            
            // Engine exhaust nozzle (rear)
            ship.Structure.AddBlock(new VoxelBlock(
                new Vector3(xPos, yPos, engineZ - engineLength - blockSize / 2),
                new Vector3(engineWidth * 1.1f, engineHeight * 1.1f, blockSize),
                config.Material, BlockType.Hull));
        }
        
        // Add maneuvering thrusters on corners
        float thrusterSize = blockSize * 1.5f;
        Vector3[] thrusterPositions = new[]
        {
            new Vector3(-dimensions.X / 2 - thrusterSize / 2, dimensions.Y / 2, -dimensions.Z / 3),
            new Vector3(dimensions.X / 2 + thrusterSize / 2, dimensions.Y / 2, -dimensions.Z / 3),
            new Vector3(-dimensions.X / 2 - thrusterSize / 2, -dimensions.Y / 2, -dimensions.Z / 3),
            new Vector3(dimensions.X / 2 + thrusterSize / 2, -dimensions.Y / 2, -dimensions.Z / 3),
        };
        
        foreach (var pos in thrusterPositions)
        {
            ship.Structure.AddBlock(new VoxelBlock(pos, new Vector3(thrusterSize), config.Material, BlockType.Thruster));
        }
    }
    
    /// <summary>
    /// Generate industrial details (pipes, vents, panels, greebles)
    /// </summary>
    private void GenerateIndustrialDetails(GeneratedMiningShip ship, Vector3 dimensions, IndustrialMiningShipConfig config)
    {
        float blockSize = 2f;
        float pipeSize = 0.8f;
        
        // Add pipes running along the hull
        int pipeCount = (int)(config.IndustrialComplexity * 8);
        for (int i = 0; i < pipeCount; i++)
        {
            float xSide = _random.NextDouble() < 0.5 ? -1 : 1;
            float xPos = xSide * (dimensions.X / 2 + pipeSize);
            float yPos = (float)(_random.NextDouble() * dimensions.Y - dimensions.Y / 2) * 0.6f;
            float zStart = -dimensions.Z / 3;
            float zEnd = dimensions.Z / 3;
            
            // Pipe segments
            for (float z = zStart; z < zEnd; z += blockSize * 2)
            {
                ship.Structure.AddBlock(new VoxelBlock(
                    new Vector3(xPos, yPos, z),
                    new Vector3(pipeSize, pipeSize, blockSize * 2),
                    config.Material, BlockType.Hull));
            }
        }
        
        // Add ventilation panels
        int ventCount = (int)(config.IndustrialComplexity * 6);
        for (int i = 0; i < ventCount; i++)
        {
            float x = (float)(_random.NextDouble() * dimensions.X * 0.5f - dimensions.X * 0.25f);
            float z = (float)(_random.NextDouble() * dimensions.Z * 0.6f - dimensions.Z * 0.3f);
            
            // Top vents
            ship.Structure.AddBlock(new VoxelBlock(
                new Vector3(x, dimensions.Y / 2 + blockSize / 2, z),
                new Vector3(blockSize * 1.5f, blockSize * 0.3f, blockSize),
                config.Material, BlockType.Hull));
        }
        
        // Add angular armor panels if enabled
        if (config.UseAngularPanels)
        {
            // Side armor panels with angular shape
            for (float z = -dimensions.Z / 4; z < dimensions.Z / 4; z += blockSize * 4)
            {
                // Left side panel
                ship.Structure.AddBlock(new VoxelBlock(
                    new Vector3(-dimensions.X / 2 - blockSize / 2, 0, z),
                    new Vector3(blockSize, blockSize * 2, blockSize * 3),
                    config.Material, BlockType.Armor, BlockShape.Wedge, BlockOrientation.NegX));
                
                // Right side panel
                ship.Structure.AddBlock(new VoxelBlock(
                    new Vector3(dimensions.X / 2 + blockSize / 2, 0, z),
                    new Vector3(blockSize, blockSize * 2, blockSize * 3),
                    config.Material, BlockType.Armor, BlockShape.Wedge, BlockOrientation.PosX));
            }
        }
        
        // Add warning lights/sensors
        Vector3[] sensorPositions = new[]
        {
            new Vector3(0, dimensions.Y / 2 + blockSize, dimensions.Z / 2 - blockSize * 2),  // Front top
            new Vector3(-dimensions.X / 2, 0, dimensions.Z / 2 - blockSize),                 // Front left
            new Vector3(dimensions.X / 2, 0, dimensions.Z / 2 - blockSize),                  // Front right
        };
        
        foreach (var pos in sensorPositions)
        {
            ship.Structure.AddBlock(new VoxelBlock(pos, new Vector3(blockSize * 0.5f), config.Material, BlockType.Hull));
        }
    }
    
    /// <summary>
    /// Generate functional components (generators, shields, etc.)
    /// </summary>
    private void GenerateFunctionalComponents(GeneratedMiningShip ship, Vector3 dimensions, IndustrialMiningShipConfig config)
    {
        float blockSize = 2f;
        
        // Generator (core of the ship)
        ship.Structure.AddBlock(new VoxelBlock(
            Vector3.Zero,
            new Vector3(blockSize * 2, blockSize * 2, blockSize * 2),
            config.Material, BlockType.Generator));
        
        // Additional generators for larger ships
        if (config.Size >= ShipSize.Destroyer)
        {
            ship.Structure.AddBlock(new VoxelBlock(
                new Vector3(-dimensions.X * 0.15f, 0, -dimensions.Z * 0.1f),
                new Vector3(blockSize * 1.5f, blockSize * 1.5f, blockSize * 1.5f),
                config.Material, BlockType.Generator));
            ship.Structure.AddBlock(new VoxelBlock(
                new Vector3(dimensions.X * 0.15f, 0, -dimensions.Z * 0.1f),
                new Vector3(blockSize * 1.5f, blockSize * 1.5f, blockSize * 1.5f),
                config.Material, BlockType.Generator));
        }
        
        // Shield generators (distributed)
        int shieldCount = config.Size >= ShipSize.Destroyer ? 2 : 1;
        for (int i = 0; i < shieldCount; i++)
        {
            float z = i == 0 ? dimensions.Z * 0.2f : -dimensions.Z * 0.2f;
            ship.Structure.AddBlock(new VoxelBlock(
                new Vector3(0, dimensions.Y * 0.2f, z),
                new Vector3(blockSize * 1.5f, blockSize * 1.5f, blockSize * 1.5f),
                config.Material, BlockType.ShieldGenerator));
        }
        
        // Gyro arrays for rotation
        int gyroCount = config.Size >= ShipSize.Frigate ? 3 : 2;
        for (int i = 0; i < gyroCount; i++)
        {
            float xOffset = (i - (gyroCount - 1) / 2f) * dimensions.X * 0.2f;
            ship.Structure.AddBlock(new VoxelBlock(
                new Vector3(xOffset, -dimensions.Y * 0.2f, 0),
                new Vector3(blockSize, blockSize, blockSize),
                config.Material, BlockType.GyroArray));
        }
        
        // Crew quarters
        ship.Structure.AddBlock(new VoxelBlock(
            new Vector3(0, dimensions.Y * 0.3f, dimensions.Z * 0.15f),
            new Vector3(blockSize * 2, blockSize * 1.5f, blockSize * 2),
            config.Material, BlockType.CrewQuarters));
        
        // Hyperdrive (if ship is large enough)
        if (config.Size >= ShipSize.Frigate)
        {
            ship.Structure.AddBlock(new VoxelBlock(
                new Vector3(0, 0, -dimensions.Z * 0.25f),
                new Vector3(blockSize * 2, blockSize * 2, blockSize * 3),
                config.Material, BlockType.HyperdriveCore));
        }
    }
    
    /// <summary>
    /// Apply industrial color scheme
    /// </summary>
    private void ApplyIndustrialColorScheme(GeneratedMiningShip ship, IndustrialMiningShipConfig config)
    {
        foreach (var block in ship.Structure.Blocks)
        {
            switch (block.BlockType)
            {
                case BlockType.Hull:
                    block.ColorRGB = config.PrimaryColor;
                    break;
                case BlockType.Armor:
                    block.ColorRGB = config.SecondaryColor;
                    break;
                case BlockType.Cargo:
                    block.ColorRGB = config.SecondaryColor;
                    break;
                case BlockType.Engine:
                case BlockType.Thruster:
                    block.ColorRGB = config.AccentColor;
                    break;
                case BlockType.TurretMount:
                    block.ColorRGB = config.HighlightColor; // Mining lasers in yellow
                    break;
                default:
                    // Keep material default color
                    break;
            }
        }
    }
    
    /// <summary>
    /// Calculate ship statistics
    /// </summary>
    private void CalculateShipStats(GeneratedMiningShip ship)
    {
        ship.TotalThrust = 0;
        ship.TotalPowerGeneration = 0;
        
        foreach (var block in ship.Structure.Blocks)
        {
            ship.TotalThrust += block.ThrustPower;
            ship.TotalPowerGeneration += block.PowerGeneration;
        }
        
        // Mining capacity based on mining laser count and cargo capacity
        ship.MiningCapacity = ship.MiningLaserCount * 100f + ship.CargoCapacity * 50f;
        
        ship.Stats["TotalMass"] = ship.TotalMass;
        ship.Stats["TotalThrust"] = ship.TotalThrust;
        ship.Stats["PowerGeneration"] = ship.TotalPowerGeneration;
        ship.Stats["MiningCapacity"] = ship.MiningCapacity;
        ship.Stats["CargoCapacity"] = ship.CargoCapacity;
        ship.Stats["ThrustToMass"] = ship.TotalMass > 0 ? ship.TotalThrust / ship.TotalMass : 0;
    }
    
    /// <summary>
    /// Validate the mining ship
    /// </summary>
    private void ValidateMiningShip(GeneratedMiningShip ship)
    {
        if (ship.Structure.Blocks.Count == 0)
        {
            ship.Warnings.Add("CRITICAL: Ship has no blocks!");
        }
        
        if (ship.MiningLaserCount == 0)
        {
            ship.Warnings.Add("WARNING: Ship has no mining lasers!");
        }
        
        if (ship.CargoCapacity == 0)
        {
            ship.Warnings.Add("WARNING: Ship has no cargo capacity!");
        }
        
        if (ship.TotalThrust <= 0)
        {
            ship.Warnings.Add("WARNING: Ship has no thrust!");
        }
        
        if (ship.TotalPowerGeneration <= 0)
        {
            ship.Warnings.Add("WARNING: Ship has no power generation!");
        }
        
        float thrustToMass = ship.TotalMass > 0 ? ship.TotalThrust / ship.TotalMass : 0;
        if (thrustToMass < 0.3f)
        {
            ship.Warnings.Add($"WARNING: Low thrust-to-mass ratio ({thrustToMass:F2}). Ship will be sluggish.");
        }
        
        foreach (var warning in ship.Warnings)
        {
            _logger.Warning("MiningShipGenerator", warning);
        }
    }
}
