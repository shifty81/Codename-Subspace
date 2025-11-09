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
/// Configuration for ship generation
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
}

/// <summary>
/// Procedurally generates functional ships based on faction styles and requirements
/// </summary>
public class ProceduralShipGenerator
{
    private Random _random;
    private readonly Logger _logger = Logger.Instance;
    
    public ProceduralShipGenerator(int seed = 0)
    {
        _random = seed == 0 ? new Random() : new Random(seed);
    }
    
    /// <summary>
    /// Generate a complete ship based on configuration
    /// </summary>
    public GeneratedShip GenerateShip(ShipGenerationConfig config)
    {
        _random = new Random(config.Seed == 0 ? Environment.TickCount : config.Seed);
        
        var result = new GeneratedShip { Config = config };
        
        _logger.Info("ShipGenerator", $"Generating {config.Size} {config.Role} ship for {config.Style.FactionName}");
        
        // Step 1: Determine ship dimensions based on size
        var dimensions = GetShipDimensions(config.Size);
        
        // Step 2: Generate core hull structure
        GenerateHullStructure(result, dimensions, config);
        
        // Step 3: Place functional components (generators, engines, etc.)
        PlaceFunctionalComponents(result, config);
        
        // Step 4: Add weapons based on role
        PlaceWeaponMounts(result, config);
        
        // Step 5: Add utility blocks (cargo, hyperdrive, etc.)
        PlaceUtilityBlocks(result, config);
        
        // Step 6: Add armor plating based on faction style
        AddArmorPlating(result, config);
        
        // Step 7: Apply faction color scheme
        ApplyColorScheme(result, config);
        
        // Step 8: Calculate final statistics
        CalculateShipStats(result);
        
        // Step 9: Validate ship is functional
        ValidateShip(result, config);
        
        // Step 10: Validate structural integrity
        ValidateStructuralIntegrity(result, config);
        
        // Step 11: Validate functional requirements
        ValidateFunctionalRequirements(result, config);
        
        // Step 12: Validate aesthetic guidelines
        ValidateAesthetics(result, config);
        
        _logger.Info("ShipGenerator", $"Generated ship with {result.Structure.Blocks.Count} blocks, " +
            $"{result.TotalThrust:F0} thrust, {result.TotalPowerGeneration:F0} power, {result.WeaponMountCount} weapons");
        
        return result;
    }
    
    /// <summary>
    /// Get dimensions for a ship based on size category
    /// </summary>
    private Vector3 GetShipDimensions(ShipSize size)
    {
        return size switch
        {
            ShipSize.Fighter => new Vector3(8, 4, 12),
            ShipSize.Corvette => new Vector3(12, 6, 18),
            ShipSize.Frigate => new Vector3(18, 10, 26),
            ShipSize.Destroyer => new Vector3(26, 14, 38),
            ShipSize.Cruiser => new Vector3(36, 20, 54),
            ShipSize.Battleship => new Vector3(50, 28, 74),
            ShipSize.Carrier => new Vector3(70, 40, 100),
            _ => new Vector3(18, 10, 26)
        };
    }
    
    /// <summary>
    /// Generate the basic hull structure based on faction style
    /// </summary>
    private void GenerateHullStructure(GeneratedShip ship, Vector3 dimensions, ShipGenerationConfig config)
    {
        var style = config.Style;
        var shape = style.PreferredHullShape;
        
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
    /// Generate a simple blocky hull (brick-like, functional over form)
    /// </summary>
    private void GenerateBlockyHull(GeneratedShip ship, Vector3 dimensions, ShipGenerationConfig config)
    {
        // Create a basic rectangular hull with some internal structure
        // Bottom layer
        for (float x = -dimensions.X / 2; x < dimensions.X / 2; x += 2)
        {
            for (float z = -dimensions.Z / 2; z < dimensions.Z / 2; z += 2)
            {
                var block = new VoxelBlock(
                    new Vector3(x, -dimensions.Y / 2, z),
                    new Vector3(2, 2, 2),
                    config.Material,
                    BlockType.Hull
                );
                ship.Structure.AddBlock(block);
            }
        }
        
        // Outer walls
        for (float y = -dimensions.Y / 2; y < dimensions.Y / 2; y += 2)
        {
            for (float z = -dimensions.Z / 2; z < dimensions.Z / 2; z += 2)
            {
                // Left wall
                var leftBlock = new VoxelBlock(
                    new Vector3(-dimensions.X / 2, y, z),
                    new Vector3(2, 2, 2),
                    config.Material,
                    BlockType.Hull
                );
                ship.Structure.AddBlock(leftBlock);
                
                // Right wall (if symmetric)
                if (config.Style.SymmetryLevel > _random.NextDouble())
                {
                    var rightBlock = new VoxelBlock(
                        new Vector3(dimensions.X / 2 - 2, y, z),
                        new Vector3(2, 2, 2),
                        config.Material,
                        BlockType.Hull
                    );
                    ship.Structure.AddBlock(rightBlock);
                }
            }
        }
        
        // Top layer
        for (float x = -dimensions.X / 2; x < dimensions.X / 2; x += 2)
        {
            for (float z = -dimensions.Z / 2; z < dimensions.Z / 2; z += 2)
            {
                var block = new VoxelBlock(
                    new Vector3(x, dimensions.Y / 2 - 2, z),
                    new Vector3(2, 2, 2),
                    config.Material,
                    BlockType.Hull
                );
                ship.Structure.AddBlock(block);
            }
        }
        
        // Front and back faces
        for (float x = -dimensions.X / 2; x < dimensions.X / 2; x += 2)
        {
            for (float y = -dimensions.Y / 2; y < dimensions.Y / 2; y += 2)
            {
                // Back face
                var backBlock = new VoxelBlock(
                    new Vector3(x, y, -dimensions.Z / 2),
                    new Vector3(2, 2, 2),
                    config.Material,
                    BlockType.Hull
                );
                ship.Structure.AddBlock(backBlock);
                
                // Front face (nose)
                var frontBlock = new VoxelBlock(
                    new Vector3(x, y, dimensions.Z / 2 - 2),
                    new Vector3(2, 2, 2),
                    config.Material,
                    BlockType.Hull
                );
                ship.Structure.AddBlock(frontBlock);
            }
        }
    }
    
    /// <summary>
    /// Generate angular/wedge-shaped hull (military style)
    /// </summary>
    private void GenerateAngularHull(GeneratedShip ship, Vector3 dimensions, ShipGenerationConfig config)
    {
        // Similar to blocky but with tapered front
        for (float z = -dimensions.Z / 2; z < dimensions.Z / 2; z += 2)
        {
            // Taper factor increases toward the front
            float taperFactor = 1.0f - Math.Max(0, (z + dimensions.Z / 2) / dimensions.Z) * 0.3f;
            float currentWidth = dimensions.X * taperFactor;
            float currentHeight = dimensions.Y * taperFactor;
            
            for (float x = -currentWidth / 2; x < currentWidth / 2; x += 2)
            {
                for (float y = -currentHeight / 2; y < currentHeight / 2; y += 2)
                {
                    // Only place blocks on the hull surface, not filling the entire volume
                    bool isOnSurface = Math.Abs(x) >= currentWidth / 2 - 2 ||
                                      Math.Abs(y) >= currentHeight / 2 - 2 ||
                                      Math.Abs(z) >= dimensions.Z / 2 - 2;
                    
                    if (isOnSurface)
                    {
                        var block = new VoxelBlock(
                            new Vector3(x, y, z),
                            new Vector3(2, 2, 2),
                            config.Material,
                            BlockType.Hull
                        );
                        ship.Structure.AddBlock(block);
                    }
                }
            }
        }
    }
    
    /// <summary>
    /// Generate cylindrical hull (cargo/trading ships)
    /// </summary>
    private void GenerateCylindricalHull(GeneratedShip ship, Vector3 dimensions, ShipGenerationConfig config)
    {
        float radius = Math.Min(dimensions.X, dimensions.Y) / 2;
        
        for (float z = -dimensions.Z / 2; z < dimensions.Z / 2; z += 2)
        {
            for (float angle = 0; angle < 360; angle += 30)
            {
                float rad = angle * MathF.PI / 180f;
                float x = radius * MathF.Cos(rad);
                float y = radius * MathF.Sin(rad);
                
                var block = new VoxelBlock(
                    new Vector3(x, y, z),
                    new Vector3(2, 2, 2),
                    config.Material,
                    BlockType.Hull
                );
                ship.Structure.AddBlock(block);
            }
        }
        
        // Add end caps
        for (float x = -radius; x <= radius; x += 2)
        {
            for (float y = -radius; y <= radius; y += 2)
            {
                if (x * x + y * y <= radius * radius)
                {
                    // Front cap
                    var frontBlock = new VoxelBlock(
                        new Vector3(x, y, dimensions.Z / 2 - 2),
                        new Vector3(2, 2, 2),
                        config.Material,
                        BlockType.Hull
                    );
                    ship.Structure.AddBlock(frontBlock);
                    
                    // Back cap
                    var backBlock = new VoxelBlock(
                        new Vector3(x, y, -dimensions.Z / 2),
                        new Vector3(2, 2, 2),
                        config.Material,
                        BlockType.Hull
                    );
                    ship.Structure.AddBlock(backBlock);
                }
            }
        }
    }
    
    /// <summary>
    /// Generate sleek hull (exploration/science ships)
    /// </summary>
    private void GenerateSleekHull(GeneratedShip ship, Vector3 dimensions, ShipGenerationConfig config)
    {
        // Similar to angular but more streamlined
        for (float z = -dimensions.Z / 2; z < dimensions.Z / 2; z += 2)
        {
            // More aggressive taper for sleek look
            float taperFactor = 1.0f - Math.Max(0, (z + dimensions.Z / 2) / dimensions.Z) * 0.5f;
            float currentWidth = dimensions.X * taperFactor;
            float currentHeight = dimensions.Y * taperFactor * 0.7f; // Flatter profile
            
            for (float x = -currentWidth / 2; x < currentWidth / 2; x += 2)
            {
                // Top and bottom surfaces
                var topBlock = new VoxelBlock(
                    new Vector3(x, currentHeight / 2, z),
                    new Vector3(2, 2, 2),
                    config.Material,
                    BlockType.Hull
                );
                ship.Structure.AddBlock(topBlock);
                
                var bottomBlock = new VoxelBlock(
                    new Vector3(x, -currentHeight / 2, z),
                    new Vector3(2, 2, 2),
                    config.Material,
                    BlockType.Hull
                );
                ship.Structure.AddBlock(bottomBlock);
            }
            
            // Side surfaces
            for (float y = -currentHeight / 2; y < currentHeight / 2; y += 2)
            {
                var leftBlock = new VoxelBlock(
                    new Vector3(-currentWidth / 2, y, z),
                    new Vector3(2, 2, 2),
                    config.Material,
                    BlockType.Hull
                );
                ship.Structure.AddBlock(leftBlock);
                
                var rightBlock = new VoxelBlock(
                    new Vector3(currentWidth / 2 - 2, y, z),
                    new Vector3(2, 2, 2),
                    config.Material,
                    BlockType.Hull
                );
                ship.Structure.AddBlock(rightBlock);
            }
        }
    }
    
    /// <summary>
    /// Generate irregular hull (pirate/cobbled together ships)
    /// </summary>
    private void GenerateIrregularHull(GeneratedShip ship, Vector3 dimensions, ShipGenerationConfig config)
    {
        // Start with a basic blocky hull but add asymmetric sections
        GenerateBlockyHull(ship, dimensions * 0.8f, config);
        
        // Add random protrusions and asymmetric sections
        int protrusions = _random.Next(3, 8);
        for (int i = 0; i < protrusions; i++)
        {
            float x = (float)(_random.NextDouble() * dimensions.X - dimensions.X / 2);
            float y = (float)(_random.NextDouble() * dimensions.Y - dimensions.Y / 2);
            float z = (float)(_random.NextDouble() * dimensions.Z - dimensions.Z / 2);
            
            Vector3 protrusionSize = new Vector3(
                (float)(_random.NextDouble() * 6 + 2),
                (float)(_random.NextDouble() * 6 + 2),
                (float)(_random.NextDouble() * 6 + 2)
            );
            
            // Add a small section
            for (float px = 0; px < protrusionSize.X; px += 2)
            {
                for (float py = 0; py < protrusionSize.Y; py += 2)
                {
                    for (float pz = 0; pz < protrusionSize.Z; pz += 2)
                    {
                        var block = new VoxelBlock(
                            new Vector3(x + px, y + py, z + pz),
                            new Vector3(2, 2, 2),
                            config.Material,
                            BlockType.Hull
                        );
                        ship.Structure.AddBlock(block);
                    }
                }
            }
        }
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
        // Place engines at the rear of the ship
        float engineZ = -dimensions.Z / 2 + 2; // Near the back
        
        for (int i = 0; i < count; i++)
        {
            float spread = dimensions.X * 0.6f;
            float x = -spread / 2 + (spread / (count - 1)) * i;
            if (count == 1) x = 0; // Center single engine
            
            var engine = new VoxelBlock(
                new Vector3(x, -dimensions.Y / 4, engineZ),
                new Vector3(3, 3, 3), // Engines are larger
                config.Material,
                BlockType.Engine
            );
            ship.Structure.AddBlock(engine);
        }
    }
    
    private void PlaceGenerators(GeneratedShip ship, Vector3 dimensions, ShipGenerationConfig config, int count)
    {
        // Place generators in the core (center of ship, protected)
        for (int i = 0; i < count; i++)
        {
            float z = -dimensions.Z / 4 + (dimensions.Z / 2) / count * i;
            
            var generator = new VoxelBlock(
                new Vector3(0, 0, z),
                new Vector3(3, 3, 3),
                config.Material,
                BlockType.Generator
            );
            ship.Structure.AddBlock(generator);
        }
    }
    
    private void PlaceShields(GeneratedShip ship, Vector3 dimensions, ShipGenerationConfig config, int count)
    {
        // Distribute shield generators throughout the ship
        for (int i = 0; i < count; i++)
        {
            float z = -dimensions.Z / 3 + (dimensions.Z * 0.66f / count) * i;
            float y = i % 2 == 0 ? dimensions.Y / 4 : -dimensions.Y / 4;
            
            var shield = new VoxelBlock(
                new Vector3(0, y, z),
                new Vector3(3, 3, 3),
                config.Material,
                BlockType.ShieldGenerator
            );
            ship.Structure.AddBlock(shield);
        }
    }
    
    private void PlaceThrusters(GeneratedShip ship, Vector3 dimensions, ShipGenerationConfig config, int count)
    {
        // Place thrusters on all sides for omnidirectional movement
        int thrustersPerSide = count / 4;
        
        // Top thrusters
        for (int i = 0; i < thrustersPerSide; i++)
        {
            float z = -dimensions.Z / 3 + (dimensions.Z * 0.66f / thrustersPerSide) * i;
            var thruster = new VoxelBlock(
                new Vector3(0, dimensions.Y / 2 - 2, z),
                new Vector3(2, 2, 2),
                config.Material,
                BlockType.Thruster
            );
            ship.Structure.AddBlock(thruster);
        }
        
        // Bottom thrusters
        for (int i = 0; i < thrustersPerSide; i++)
        {
            float z = -dimensions.Z / 3 + (dimensions.Z * 0.66f / thrustersPerSide) * i;
            var thruster = new VoxelBlock(
                new Vector3(0, -dimensions.Y / 2, z),
                new Vector3(2, 2, 2),
                config.Material,
                BlockType.Thruster
            );
            ship.Structure.AddBlock(thruster);
        }
        
        // Left thrusters
        for (int i = 0; i < thrustersPerSide; i++)
        {
            float z = -dimensions.Z / 3 + (dimensions.Z * 0.66f / thrustersPerSide) * i;
            var thruster = new VoxelBlock(
                new Vector3(-dimensions.X / 2, 0, z),
                new Vector3(2, 2, 2),
                config.Material,
                BlockType.Thruster
            );
            ship.Structure.AddBlock(thruster);
        }
        
        // Right thrusters
        for (int i = 0; i < thrustersPerSide; i++)
        {
            float z = -dimensions.Z / 3 + (dimensions.Z * 0.66f / thrustersPerSide) * i;
            var thruster = new VoxelBlock(
                new Vector3(dimensions.X / 2 - 2, 0, z),
                new Vector3(2, 2, 2),
                config.Material,
                BlockType.Thruster
            );
            ship.Structure.AddBlock(thruster);
        }
    }
    
    private void PlaceGyros(GeneratedShip ship, Vector3 dimensions, ShipGenerationConfig config, int count)
    {
        // Place gyro arrays for rotation control
        for (int i = 0; i < count; i++)
        {
            float z = -dimensions.Z / 4 + (dimensions.Z / 2 / count) * i;
            float x = i % 2 == 0 ? dimensions.X / 4 : -dimensions.X / 4;
            
            var gyro = new VoxelBlock(
                new Vector3(x, 0, z),
                new Vector3(2, 2, 2),
                config.Material,
                BlockType.GyroArray
            );
            ship.Structure.AddBlock(gyro);
        }
    }
    
    /// <summary>
    /// Place weapon mounts based on role
    /// </summary>
    private void PlaceWeaponMounts(GeneratedShip ship, ShipGenerationConfig config)
    {
        var dimensions = GetShipDimensions(config.Size);
        
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
        
        weaponCount = (int)(weaponCount * config.Style.WeaponDensity);
        
        // Place weapons around the hull
        for (int i = 0; i < weaponCount; i++)
        {
            float angle = (360f / weaponCount) * i;
            float rad = angle * MathF.PI / 180f;
            float radius = Math.Min(dimensions.X, dimensions.Y) / 2;
            
            float x = radius * MathF.Cos(rad) * 0.8f;
            float y = radius * MathF.Sin(rad) * 0.8f;
            float z = _random.Next((int)(-dimensions.Z / 3), (int)(dimensions.Z / 3));
            
            var turret = new VoxelBlock(
                new Vector3(x, y, z),
                new Vector3(2, 2, 2),
                config.Material,
                BlockType.TurretMount
            );
            ship.Structure.AddBlock(turret);
        }
    }
    
    /// <summary>
    /// Place utility blocks (cargo, hyperdrive, etc.)
    /// </summary>
    private void PlaceUtilityBlocks(GeneratedShip ship, ShipGenerationConfig config)
    {
        var dimensions = GetShipDimensions(config.Size);
        
        // Add hyperdrive if required
        if (config.RequireHyperdrive)
        {
            var hyperdrive = new VoxelBlock(
                new Vector3(0, 0, dimensions.Z / 3),
                new Vector3(4, 4, 4), // Hyperdrive is large
                config.Material,
                BlockType.HyperdriveCore
            );
            ship.Structure.AddBlock(hyperdrive);
        }
        
        // Add cargo based on role
        if (config.RequireCargo)
        {
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
                
                var cargo = new VoxelBlock(
                    new Vector3(0, y, z),
                    new Vector3(3, 3, 3),
                    config.Material,
                    BlockType.Cargo
                );
                ship.Structure.AddBlock(cargo);
            }
        }
        
        // Add crew quarters for larger ships
        if (config.Size >= ShipSize.Frigate)
        {
            var quarters = new VoxelBlock(
                new Vector3(0, dimensions.Y / 3, 0),
                new Vector3(3, 3, 3),
                config.Material,
                BlockType.CrewQuarters
            );
            ship.Structure.AddBlock(quarters);
        }
        
        // Add pod docking for player
        var podDock = new VoxelBlock(
            new Vector3(0, -dimensions.Y / 3, dimensions.Z / 3),
            new Vector3(2, 2, 2),
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
