using System.Numerics;
using AvorionLike.Core.Logging;

namespace AvorionLike.Core.Voxel;

/// <summary>
/// Design goal for AI ship generation
/// </summary>
public enum ShipDesignGoal
{
    CargoHauler,        // Maximize cargo capacity
    Battleship,         // Heavy armor, weapons, shields
    Scout,              // High speed, maneuverability
    Miner,              // Mining equipment, cargo
    Interceptor,        // Fast, agile, light weapons
    Carrier,            // Large, pod docking, defenses
    Tanker,             // Maximum cargo, minimal combat
    Frigate             // Balanced multi-role
}

/// <summary>
/// Parameters for AI-driven ship generation
/// </summary>
public class AIShipGenerationParameters
{
    public ShipDesignGoal Goal { get; set; } = ShipDesignGoal.Frigate;
    public string Material { get; set; } = "Iron";
    public int TargetBlockCount { get; set; } = 100;
    public int Seed { get; set; } = 0;
    
    // Design constraints
    public int MinWeaponMounts { get; set; } = 2;
    public bool RequireHyperdrive { get; set; } = true;
    public bool RequireShields { get; set; } = true;
    public int MinCrewCapacity { get; set; } = 5;
    
    // Aesthetic preferences
    public bool AvoidSimpleBoxes { get; set; } = true;
    public float DesiredAspectRatio { get; set; } = 2.5f; // Length to width ratio
    public bool UseAngularDesign { get; set; } = true;
}

/// <summary>
/// Block placement plan created by AI
/// </summary>
public class BlockPlacementPlan
{
    public Vector3 Position { get; set; }
    public Vector3 Size { get; set; }
    public BlockType BlockType { get; set; }
    public string MaterialType { get; set; } = "Iron";
    public int Priority { get; set; }
    public bool IsInternal { get; set; }
    public string Reason { get; set; } = "";
}

/// <summary>
/// Result of AI ship generation
/// </summary>
public class AIGeneratedShip
{
    public VoxelStructureComponent Structure { get; set; } = new();
    public ShipAggregate Aggregate { get; set; }
    public AIShipGenerationParameters Parameters { get; set; } = new();
    public List<string> DesignDecisions { get; set; } = new();
    public List<string> Warnings { get; set; } = new();
    public float DesignQuality { get; set; } // 0-100 rating
    
    public AIGeneratedShip()
    {
        Aggregate = new ShipAggregate(Structure);
    }
}

/// <summary>
/// AI-driven procedural ship generator using smart design rules
/// Implements goal-oriented design with prioritization and aesthetic guidelines
/// </summary>
public class AIShipGenerator
{
    private Random _random;
    private readonly Logger _logger = Logger.Instance;
    
    // Block grid spacing used throughout generation
    private const float BLOCK_SIZE = 2f;
    // Blocks within this distance of X=0 are considered on the centerline (inherently symmetric)
    private const float SYMMETRY_AXIS_THRESHOLD = 0.5f;
    // Adjacency threshold for flood-fill connectivity check (slightly larger than block size for floating-point tolerance)
    private const float ADJACENCY_THRESHOLD = BLOCK_SIZE * 1.5f;
    
    public AIShipGenerator(int seed = 0)
    {
        _random = seed == 0 ? new Random() : new Random(seed);
    }
    
    /// <summary>
    /// Generate a ship using AI-driven design rules
    /// Enhanced with Avorion-style layered construction:
    /// 1. Framework skeleton defines shape
    /// 2. Internal functional blocks placed in protected core
    /// 3. Hull fills the body
    /// 4. Armor shell wraps exterior
    /// 5. Symmetry enforced via X-axis mirroring (like Avorion NPC ships)
    /// </summary>
    public AIGeneratedShip GenerateShip(AIShipGenerationParameters parameters)
    {
        _random = new Random(parameters.Seed == 0 ? Environment.TickCount : parameters.Seed);
        
        var result = new AIGeneratedShip { Parameters = parameters };
        
        _logger.Info("AIShipGenerator", $"Generating {parameters.Goal} ship with {parameters.TargetBlockCount} blocks");
        result.DesignDecisions.Add($"Starting generation of {parameters.Goal} ship");
        
        // Step 1: Determine ship dimensions based on goal and block count
        var dimensions = DetermineShipDimensions(parameters);
        result.DesignDecisions.Add($"Determined dimensions: {dimensions.X:F1} x {dimensions.Y:F1} x {dimensions.Z:F1}");
        
        // Step 2: Create block placement plan based on design goal
        var placementPlan = CreatePlacementPlan(parameters, dimensions);
        result.DesignDecisions.Add($"Created placement plan with {placementPlan.Count} blocks");
        
        // Step 3: Define ship outline using framework blocks with symmetry
        DefineShipOutline(result, dimensions, parameters);
        result.DesignDecisions.Add($"Defined ship outline with {result.Structure.Blocks.Count} framework blocks");
        
        // Step 4: Place critical internal components (generators, crew quarters)
        PlaceInternalComponents(result, placementPlan, parameters);
        result.DesignDecisions.Add($"Placed internal components, total blocks: {result.Structure.Blocks.Count}");
        
        // Step 5: Place functional systems based on priority
        PlaceFunctionalSystems(result, placementPlan, parameters);
        result.DesignDecisions.Add($"Placed functional systems, total blocks: {result.Structure.Blocks.Count}");
        
        // Step 6: Add hull fill between framework and armor
        AddHullFill(result, dimensions, parameters);
        result.DesignDecisions.Add($"Added hull fill, total blocks: {result.Structure.Blocks.Count}");
        
        // Step 7: Add external armor shell with contextual shapes
        AddArmorShell(result, parameters);
        result.DesignDecisions.Add($"Added armor shell, total blocks: {result.Structure.Blocks.Count}");
        
        // Step 8: Enforce symmetry across X-axis (Avorion-style mirroring for NPC ships)
        EnforceSymmetry(result);
        result.DesignDecisions.Add($"Enforced symmetry, total blocks: {result.Structure.Blocks.Count}");
        
        // Step 9: Optimize and validate
        OptimizeDesign(result, parameters);
        result.DesignDecisions.Add($"Optimized design, final blocks: {result.Structure.Blocks.Count}");
        
        // Step 10: Calculate final statistics
        result.Aggregate.Recalculate();
        
        // Step 11: Validate requirements
        ValidateDesign(result);
        
        // Step 12: Rate design quality
        result.DesignQuality = RateDesignQuality(result);
        
        _logger.Info("AIShipGenerator", $"Completed ship with {result.Structure.Blocks.Count} blocks, quality: {result.DesignQuality:F0}/100");
        
        return result;
    }
    
    /// <summary>
    /// Determine optimal ship dimensions based on goal and target block count
    /// </summary>
    private Vector3 DetermineShipDimensions(AIShipGenerationParameters parameters)
    {
        // Base size from block count (rough approximation)
        float volumeEstimate = parameters.TargetBlockCount * 8; // Assume average block volume of 8
        float baseScale = MathF.Pow(volumeEstimate, 1f / 3f);
        
        // Adjust aspect ratio based on goal
        Vector3 dimensions = parameters.Goal switch
        {
            ShipDesignGoal.CargoHauler => new Vector3(baseScale * 1.2f, baseScale * 1.0f, baseScale * 1.5f), // Wide and long
            ShipDesignGoal.Battleship => new Vector3(baseScale * 1.5f, baseScale * 1.0f, baseScale * 2.0f), // Wide, imposing
            ShipDesignGoal.Scout => new Vector3(baseScale * 0.7f, baseScale * 0.5f, baseScale * 2.5f), // Sleek and narrow
            ShipDesignGoal.Miner => new Vector3(baseScale * 1.3f, baseScale * 0.8f, baseScale * 1.5f), // Industrial look
            ShipDesignGoal.Interceptor => new Vector3(baseScale * 0.6f, baseScale * 0.4f, baseScale * 2.8f), // Very sleek
            ShipDesignGoal.Carrier => new Vector3(baseScale * 2.0f, baseScale * 1.2f, baseScale * 1.8f), // Large and wide
            ShipDesignGoal.Tanker => new Vector3(baseScale * 1.5f, baseScale * 1.3f, baseScale * 2.0f), // Bulky
            _ => new Vector3(baseScale, baseScale * 0.6f, baseScale * parameters.DesiredAspectRatio) // Balanced
        };
        
        return dimensions;
    }
    
    /// <summary>
    /// Create a prioritized placement plan for blocks
    /// </summary>
    private List<BlockPlacementPlan> CreatePlacementPlan(AIShipGenerationParameters parameters, Vector3 dimensions)
    {
        var plan = new List<BlockPlacementPlan>();
        
        // Determine block priorities based on goal
        var priorities = GetBlockPriorities(parameters.Goal);
        
        // Calculate how many of each block type we need
        foreach (var priority in priorities.OrderByDescending(p => p.Value))
        {
            int count = CalculateRequiredBlockCount(priority.Key, parameters);
            for (int i = 0; i < count; i++)
            {
                var definition = BlockDefinitionDatabase.GetDefinition(priority.Key);
                plan.Add(new BlockPlacementPlan
                {
                    BlockType = priority.Key,
                    MaterialType = parameters.Material,
                    Priority = priority.Value,
                    IsInternal = definition.RequiresInternalPlacement,
                    Size = new Vector3(2, 2, 2), // Standard block size
                    Reason = $"{priority.Key} required for {parameters.Goal} design"
                });
            }
        }
        
        return plan;
    }
    
    /// <summary>
    /// Get block type priorities based on design goal
    /// </summary>
    private Dictionary<BlockType, int> GetBlockPriorities(ShipDesignGoal goal)
    {
        return goal switch
        {
            ShipDesignGoal.CargoHauler => new Dictionary<BlockType, int>
            {
                [BlockType.Cargo] = 10,
                [BlockType.Engine] = 8,
                [BlockType.Generator] = 9,
                [BlockType.Thruster] = 6,
                [BlockType.GyroArray] = 5,
                [BlockType.HyperdriveCore] = 7,
                [BlockType.CrewQuarters] = 6,
                [BlockType.Armor] = 4,
                [BlockType.TurretMount] = 3
            },
            
            ShipDesignGoal.Battleship => new Dictionary<BlockType, int>
            {
                [BlockType.TurretMount] = 10,
                [BlockType.ShieldGenerator] = 9,
                [BlockType.Armor] = 10,
                [BlockType.Generator] = 10,
                [BlockType.Engine] = 7,
                [BlockType.Thruster] = 6,
                [BlockType.GyroArray] = 6,
                [BlockType.CrewQuarters] = 7,
                [BlockType.HyperdriveCore] = 5,
                [BlockType.Cargo] = 3
            },
            
            ShipDesignGoal.Scout => new Dictionary<BlockType, int>
            {
                [BlockType.Engine] = 10,
                [BlockType.Thruster] = 10,
                [BlockType.GyroArray] = 9,
                [BlockType.Generator] = 8,
                [BlockType.HyperdriveCore] = 10,
                [BlockType.ShieldGenerator] = 6,
                [BlockType.CrewQuarters] = 5,
                [BlockType.TurretMount] = 4,
                [BlockType.Armor] = 3,
                [BlockType.Cargo] = 4
            },
            
            ShipDesignGoal.Miner => new Dictionary<BlockType, int>
            {
                [BlockType.Cargo] = 10,
                [BlockType.Generator] = 9,
                [BlockType.Engine] = 7,
                [BlockType.Thruster] = 6,
                [BlockType.GyroArray] = 6,
                [BlockType.CrewQuarters] = 7,
                [BlockType.HyperdriveCore] = 8,
                [BlockType.Armor] = 5,
                [BlockType.TurretMount] = 4
            },
            
            _ => new Dictionary<BlockType, int>
            {
                [BlockType.Engine] = 8,
                [BlockType.Generator] = 8,
                [BlockType.Thruster] = 7,
                [BlockType.GyroArray] = 6,
                [BlockType.ShieldGenerator] = 7,
                [BlockType.Armor] = 6,
                [BlockType.TurretMount] = 7,
                [BlockType.Cargo] = 6,
                [BlockType.CrewQuarters] = 6,
                [BlockType.HyperdriveCore] = 7
            }
        };
    }
    
    /// <summary>
    /// Calculate how many blocks of a type are needed
    /// Uses Avorion-style volume-based scaling where larger ships have proportionally more blocks
    /// </summary>
    private int CalculateRequiredBlockCount(BlockType blockType, AIShipGenerationParameters parameters)
    {
        float blockBudget = parameters.TargetBlockCount;
        
        // Allocate percentage of blocks based on type and goal
        float percentage = blockType switch
        {
            BlockType.Hull => 0.25f,
            BlockType.Framework => 0.08f, // Avorion-style framework for shaping
            BlockType.Armor => parameters.Goal == ShipDesignGoal.Battleship ? 0.25f : 0.10f,
            BlockType.Engine => 0.08f,
            BlockType.Thruster => 0.05f,
            BlockType.GyroArray => 0.04f,
            BlockType.Generator => 0.06f,
            BlockType.ShieldGenerator => 0.03f,
            BlockType.Cargo => parameters.Goal == ShipDesignGoal.CargoHauler ? 0.20f : 0.05f,
            BlockType.CrewQuarters => 0.03f,
            BlockType.TurretMount => parameters.Goal == ShipDesignGoal.Battleship ? 0.10f : 0.03f,
            BlockType.HyperdriveCore => 0.01f,
            BlockType.PodDocking => 0.02f,
            _ => 0.01f
        };
        
        return Math.Max(1, (int)(blockBudget * percentage));
    }
    
    /// <summary>
    /// Define ship outline with hull/framework blocks using Avorion-style construction.
    /// Uses contextual block shapes: wedges at nose, corners at edges, cubes for body.
    /// Builds blocks on the positive-X side and centerline (X=0).
    /// Blocks at X &gt; SYMMETRY_AXIS_THRESHOLD are mirrored to -X by EnforceSymmetry.
    /// Blocks at X ≤ SYMMETRY_AXIS_THRESHOLD (centerline) are inherently symmetric and not mirrored.
    /// </summary>
    private void DefineShipOutline(AIGeneratedShip ship, Vector3 dimensions, AIShipGenerationParameters parameters)
    {
        float halfX = dimensions.X / 2;
        float halfY = dimensions.Y / 2;
        float halfZ = dimensions.Z / 2;
        
        // Avorion-style layered outline: build +X side and centerline, mirror later
        // Front nose section (tapered with wedges)
        float noseStart = halfZ * 0.6f;
        for (float z = noseStart; z < halfZ; z += BLOCK_SIZE)
        {
            float noseProgress = (z - noseStart) / (halfZ - noseStart);
            float taperFactor = 1.0f - noseProgress * 0.85f;
            float currentWidth = Math.Max(BLOCK_SIZE, halfX * taperFactor);
            float currentHeight = Math.Max(BLOCK_SIZE, halfY * taperFactor);
            
            for (float x = 0; x < currentWidth; x += BLOCK_SIZE)
            {
                for (float y = -currentHeight; y < currentHeight; y += BLOCK_SIZE)
                {
                    bool isEdge = x > currentWidth - BLOCK_SIZE * 1.5f ||
                                  Math.Abs(y) > currentHeight - BLOCK_SIZE * 1.5f;
                    
                    // Contextual shapes: wedges at nose tip, corners at edges
                    BlockShape shape = BlockShape.Cube;
                    BlockOrientation orient = BlockOrientation.PosY;
                    BlockType bType = BlockType.Hull;
                    
                    if (noseProgress > 0.7f)
                    {
                        shape = BlockShape.Wedge;
                        orient = BlockOrientation.PosZ;
                    }
                    else if (isEdge && noseProgress > 0.3f)
                    {
                        shape = BlockShape.Wedge;
                        orient = y > 0 ? BlockOrientation.PosY : BlockOrientation.NegY;
                    }
                    
                    if (isEdge)
                    {
                        AddBlock(ship.Structure, new Vector3(x, y, z), new Vector3(BLOCK_SIZE),
                            parameters.Material, bType, shape, orient);
                    }
                }
            }
        }
        
        // Main body section (solid shell with framework interior)
        for (float z = -halfZ * 0.6f; z < noseStart; z += BLOCK_SIZE)
        {
            for (float x = 0; x < halfX; x += BLOCK_SIZE)
            {
                for (float y = -halfY; y < halfY; y += BLOCK_SIZE)
                {
                    bool isEdgeX = x > halfX - BLOCK_SIZE * 1.5f;
                    bool isEdgeY = Math.Abs(y) > halfY - BLOCK_SIZE * 1.5f;
                    bool isEdge = isEdgeX || isEdgeY;
                    
                    if (isEdge)
                    {
                        // Shell blocks
                        AddBlock(ship.Structure, new Vector3(x, y, z), new Vector3(BLOCK_SIZE),
                            parameters.Material, BlockType.Hull);
                    }
                    else if (_random.NextDouble() < 0.15f)
                    {
                        // Sparse framework interior for structure
                        AddBlock(ship.Structure, new Vector3(x, y, z), new Vector3(BLOCK_SIZE),
                            parameters.Material, BlockType.Framework);
                    }
                }
            }
        }
        
        // Rear engine section (slightly wider, flat back for engine mounts)
        for (float z = -halfZ; z < -halfZ * 0.6f; z += BLOCK_SIZE)
        {
            float rearProgress = Math.Abs(z + halfZ * 0.6f) / (halfZ * 0.4f);
            float widthFactor = 1.0f + rearProgress * 0.15f;
            float rearWidth = Math.Min(halfX * widthFactor, halfX * 1.15f);
            
            for (float x = 0; x < rearWidth; x += BLOCK_SIZE)
            {
                for (float y = -halfY; y < halfY; y += BLOCK_SIZE)
                {
                    bool isEdge = x > rearWidth - BLOCK_SIZE * 1.5f ||
                                  Math.Abs(y) > halfY - BLOCK_SIZE * 1.5f ||
                                  z < -halfZ + BLOCK_SIZE;
                    
                    if (isEdge)
                    {
                        AddBlock(ship.Structure, new Vector3(x, y, z), new Vector3(BLOCK_SIZE),
                            parameters.Material, BlockType.Hull);
                    }
                }
            }
        }
        
        // Central spine along Z axis for structural integrity (inherently on centerline)
        for (float z = -halfZ; z < halfZ; z += BLOCK_SIZE)
        {
            AddBlock(ship.Structure, new Vector3(0, 0, z), new Vector3(BLOCK_SIZE),
                parameters.Material, BlockType.Hull);
        }
    }
    
    /// <summary>
    /// Place critical internal components
    /// </summary>
    private void PlaceInternalComponents(AIGeneratedShip ship, List<BlockPlacementPlan> plan, AIShipGenerationParameters parameters)
    {
        // Place internal blocks in protected central area
        var internalBlocks = plan.Where(p => p.IsInternal).OrderByDescending(p => p.Priority);
        
        float blockSize = 2f;
        Vector3 internalStart = new Vector3(-5, -3, -5); // Central protected area
        Vector3 currentPos = internalStart;
        
        foreach (var blockPlan in internalBlocks)
        {
            // Check if block already exists at this position
            if (!IsPositionOccupied(ship.Structure, currentPos, blockSize))
            {
                AddBlock(ship.Structure, currentPos, blockPlan.Size, blockPlan.MaterialType, blockPlan.BlockType);
                
                // Move to next position
                currentPos.X += blockSize;
                if (currentPos.X > 5)
                {
                    currentPos.X = internalStart.X;
                    currentPos.Y += blockSize;
                    if (currentPos.Y > 3)
                    {
                        currentPos.Y = internalStart.Y;
                        currentPos.Z += blockSize;
                    }
                }
            }
        }
    }
    
    /// <summary>
    /// Place functional systems (engines, weapons, etc.)
    /// Avorion-style: engines at rear with unobstructed faces, turrets on top/sides,
    /// thrusters distributed around hull for maneuverability
    /// </summary>
    private void PlaceFunctionalSystems(AIGeneratedShip ship, List<BlockPlacementPlan> plan, AIShipGenerationParameters parameters)
    {
        var functionalBlocks = plan.Where(p => !p.IsInternal).OrderByDescending(p => p.Priority);
        
        foreach (var blockPlan in functionalBlocks)
        {
            Vector3 optimalPosition = FindOptimalPosition(ship, blockPlan.BlockType, parameters);
            
            if (!IsPositionOccupied(ship.Structure, optimalPosition, 2f))
            {
                // Engines face backward (-Z), so orient them accordingly
                BlockShape shape = BlockShape.Cube;
                BlockOrientation orient = BlockOrientation.PosY;
                if (blockPlan.BlockType == BlockType.Engine)
                {
                    orient = BlockOrientation.NegZ; // Rear-facing
                }
                
                AddBlock(ship.Structure, optimalPosition, blockPlan.Size, blockPlan.MaterialType, blockPlan.BlockType, shape, orient);
            }
        }
    }
    
    /// <summary>
    /// Find optimal position for a block type following Avorion placement rules:
    /// - Engines: rear of ship, must have unobstructed rear face
    /// - Thrusters: distributed around hull edges for maneuverability
    /// - Turrets: top surface and sides for firing arcs
    /// - Gyros: far from center of mass for maximum rotational effect
    /// </summary>
    private Vector3 FindOptimalPosition(AIGeneratedShip ship, BlockType blockType, AIShipGenerationParameters parameters)
    {
        var dims = DetermineShipDimensions(parameters);
        float halfX = dims.X / 2;
        float halfY = dims.Y / 2;
        float halfZ = dims.Z / 2;
        
        return blockType switch
        {
            // Engines at rear, spread across rear face
            BlockType.Engine => new Vector3(
                _random.Next(-(int)(halfX * 0.6f), (int)(halfX * 0.6f)),
                _random.Next(-(int)(halfY * 0.5f), (int)(halfY * 0.5f)),
                -halfZ + 1),
            
            // Thrusters distributed around hull for omnidirectional thrust
            BlockType.Thruster => GetThrusterPosition(halfX, halfY, halfZ),
            
            // Turrets on top and sides for clear firing arcs
            BlockType.TurretMount => new Vector3(
                _random.Next(-(int)(halfX * 0.7f), (int)(halfX * 0.7f)),
                halfY - 1,
                _random.Next(-(int)(halfZ * 0.5f), (int)(halfZ * 0.5f))),
            
            // Gyros at extremities for maximum rotational leverage
            BlockType.GyroArray => new Vector3(
                _random.NextDouble() < 0.5 ? -halfX * 0.8f : halfX * 0.8f,
                0,
                _random.Next(-(int)(halfZ * 0.3f), (int)(halfZ * 0.3f))),
            
            _ => new Vector3(
                _random.Next(-(int)(halfX * 0.5f), (int)(halfX * 0.5f)),
                _random.Next(-(int)(halfY * 0.5f), (int)(halfY * 0.5f)),
                _random.Next(-(int)(halfZ * 0.5f), (int)(halfZ * 0.5f)))
        };
    }
    
    /// <summary>
    /// Get a distributed thruster position for good maneuverability coverage
    /// </summary>
    private Vector3 GetThrusterPosition(float halfX, float halfY, float halfZ)
    {
        // Place thrusters on 6 faces for omnidirectional control
        int face = _random.Next(6);
        return face switch
        {
            0 => new Vector3(halfX - 1, _random.Next(-(int)halfY, (int)halfY), _random.Next(-(int)halfZ, (int)halfZ)),
            1 => new Vector3(-halfX + 1, _random.Next(-(int)halfY, (int)halfY), _random.Next(-(int)halfZ, (int)halfZ)),
            2 => new Vector3(_random.Next(-(int)halfX, (int)halfX), halfY - 1, _random.Next(-(int)halfZ, (int)halfZ)),
            3 => new Vector3(_random.Next(-(int)halfX, (int)halfX), -halfY + 1, _random.Next(-(int)halfZ, (int)halfZ)),
            4 => new Vector3(_random.Next(-(int)halfX, (int)halfX), _random.Next(-(int)halfY, (int)halfY), halfZ - 1),
            _ => new Vector3(_random.Next(-(int)halfX, (int)halfX), _random.Next(-(int)halfY, (int)halfY), -halfZ + 1),
        };
    }
    
    /// <summary>
    /// Add hull fill between framework skeleton and armor exterior.
    /// Avorion-style: fills gaps in hull to create solid appearance.
    /// </summary>
    private void AddHullFill(AIGeneratedShip ship, Vector3 dimensions, AIShipGenerationParameters parameters)
    {
        float blockSize = 2f;
        float halfX = dimensions.X / 2;
        float halfY = dimensions.Y / 2;
        float halfZ = dimensions.Z / 2;
        
        // Fill hull gaps between existing blocks to create solid body
        for (float z = -halfZ; z < halfZ; z += blockSize)
        {
            float zProgress = (z + halfZ) / dimensions.Z;
            // Taper toward nose
            float taperFactor = zProgress > 0.7f ? 1.0f - (zProgress - 0.7f) * 2.5f : 1.0f;
            taperFactor = Math.Max(0.15f, taperFactor);
            
            float currentHalfX = halfX * taperFactor;
            float currentHalfY = halfY * taperFactor;
            
            for (float x = -currentHalfX; x < currentHalfX; x += blockSize)
            {
                for (float y = -currentHalfY; y < currentHalfY; y += blockSize)
                {
                    Vector3 pos = new Vector3(x, y, z);
                    if (!IsPositionOccupied(ship.Structure, pos, blockSize * 0.9f))
                    {
                        bool isShell = Math.Abs(x) > currentHalfX - blockSize * 1.5f ||
                                       Math.Abs(y) > currentHalfY - blockSize * 1.5f;
                        
                        if (isShell)
                        {
                            AddBlock(ship.Structure, pos, new Vector3(blockSize), parameters.Material, BlockType.Hull);
                        }
                    }
                }
            }
        }
    }
    
    /// <summary>
    /// Add external armor shell with contextual shapes (Avorion style).
    /// Uses wedge/corner blocks at edges instead of random placement.
    /// </summary>
    private void AddArmorShell(AIGeneratedShip ship, AIShipGenerationParameters parameters)
    {
        // Add armor blocks around exterior hull with contextual shapes
        var existingBlocks = ship.Structure.Blocks.ToList();
        
        // Find bounding box for contextual shape selection
        float minZ = existingBlocks.Min(b => b.Position.Z);
        float maxZ = existingBlocks.Max(b => b.Position.Z);
        float minY = existingBlocks.Min(b => b.Position.Y);
        float maxY = existingBlocks.Max(b => b.Position.Y);
        
        foreach (var block in existingBlocks)
        {
            if (block.BlockType == BlockType.Hull || block.BlockType == BlockType.Framework)
            {
                Vector3[] armorOffsets = new[]
                {
                    new Vector3(2, 0, 0), new Vector3(-2, 0, 0),
                    new Vector3(0, 2, 0), new Vector3(0, -2, 0),
                    new Vector3(0, 0, 2), new Vector3(0, 0, -2)
                };
                
                foreach (var offset in armorOffsets)
                {
                    Vector3 armorPos = block.Position + offset;
                    if (!IsPositionOccupied(ship.Structure, armorPos, 1f) && _random.NextDouble() < 0.35)
                    {
                        // Contextual shape: wedges at front, corners at edges
                        BlockShape shape = BlockShape.Cube;
                        BlockOrientation orient = BlockOrientation.PosY;
                        
                        float zProgress = (armorPos.Z - minZ) / Math.Max(1f, maxZ - minZ);
                        if (zProgress > 0.8f) // Near front
                        {
                            shape = BlockShape.Wedge;
                            orient = BlockOrientation.PosZ;
                        }
                        else if (armorPos.Y > maxY - 2) // Top edge
                        {
                            shape = BlockShape.Wedge;
                            orient = BlockOrientation.PosY;
                        }
                        else if (armorPos.Y < minY + 2) // Bottom edge
                        {
                            shape = BlockShape.Wedge;
                            orient = BlockOrientation.NegY;
                        }
                        
                        AddBlock(ship.Structure, armorPos, new Vector3(2, 2, 2), parameters.Material, BlockType.Armor, shape, orient);
                    }
                }
            }
        }
    }
    
    /// <summary>
    /// Enforce left-right (X-axis) symmetry by mirroring blocks.
    /// Avorion NPC ships always have mirrored symmetry for balanced appearance.
    /// </summary>
    private void EnforceSymmetry(AIGeneratedShip ship)
    {
        var blocksToMirror = ship.Structure.Blocks
            .Where(b => b.Position.X > SYMMETRY_AXIS_THRESHOLD)
            .ToList();
        
        foreach (var block in blocksToMirror)
        {
            Vector3 mirroredPos = new Vector3(-block.Position.X, block.Position.Y, block.Position.Z);
            
            if (!IsPositionOccupied(ship.Structure, mirroredPos, 1f))
            {
                // Mirror orientation for shaped blocks
                BlockOrientation mirroredOrient = block.Orientation;
                if (block.Orientation == BlockOrientation.PosX)
                    mirroredOrient = BlockOrientation.NegX;
                else if (block.Orientation == BlockOrientation.NegX)
                    mirroredOrient = BlockOrientation.PosX;
                
                var mirroredBlock = new VoxelBlock(mirroredPos, block.Size, block.MaterialType, 
                    block.BlockType, block.Shape, mirroredOrient);
                mirroredBlock.ColorRGB = block.ColorRGB;
                ship.Structure.AddBlock(mirroredBlock);
            }
        }
    }
    
    /// <summary>
    /// Optimize the design
    /// </summary>
    private void OptimizeDesign(AIGeneratedShip ship, AIShipGenerationParameters parameters)
    {
        // Remove any isolated/disconnected blocks
        // Ensure all blocks are connected to main structure
        // This is a simplified check - real implementation would use flood fill
        
        var centerBlock = ship.Structure.Blocks
            .OrderBy(b => Vector3.Distance(b.Position, Vector3.Zero))
            .FirstOrDefault();
        
        if (centerBlock != null)
        {
            // Keep only connected blocks (simplified)
            var blocksToRemove = ship.Structure.Blocks
                .Where(b => !IsConnectedToStructure(b, ship.Structure.Blocks, centerBlock))
                .ToList();
            
            foreach (var block in blocksToRemove)
            {
                ship.Structure.Blocks.Remove(block);
            }
        }
    }
    
    /// <summary>
    /// Check if a block is connected to the main structure using flood-fill.
    /// Avorion requires all blocks to be structurally connected.
    /// </summary>
    private bool IsConnectedToStructure(VoxelBlock block, List<VoxelBlock> allBlocks, VoxelBlock centerBlock)
    {
        if (block == centerBlock) return true;
        
        // Flood-fill from centerBlock to find all reachable blocks
        var visited = new HashSet<Guid>();
        var queue = new Queue<VoxelBlock>();
        queue.Enqueue(centerBlock);
        visited.Add(centerBlock.Id);
        
        while (queue.Count > 0)
        {
            var current = queue.Dequeue();
            
            if (current.Id == block.Id)
                return true;
            
            // Find adjacent blocks
            foreach (var neighbor in allBlocks)
            {
                if (!visited.Contains(neighbor.Id) && 
                    Vector3.Distance(current.Position, neighbor.Position) < ADJACENCY_THRESHOLD)
                {
                    visited.Add(neighbor.Id);
                    queue.Enqueue(neighbor);
                }
            }
        }
        
        return false;
    }
    
    /// <summary>
    /// Validate the design meets requirements
    /// </summary>
    private void ValidateDesign(AIGeneratedShip ship)
    {
        var warnings = ship.Aggregate.ValidateRequirements();
        ship.Warnings.AddRange(warnings);
        
        if (ship.Structure.Blocks.Count == 0)
        {
            ship.Warnings.Add("CRITICAL: Ship has no blocks");
        }
        
        if (ship.Aggregate.TotalMass == 0)
        {
            ship.Warnings.Add("CRITICAL: Ship has no mass");
        }
    }
    
    /// <summary>
    /// Rate the design quality
    /// </summary>
    private float RateDesignQuality(AIGeneratedShip ship)
    {
        float score = 100f;
        
        // Deduct for warnings
        score -= ship.Warnings.Count * 5f;
        
        // Deduct if power is insufficient
        if (ship.Aggregate.AvailablePower < 0)
        {
            score -= 20f;
        }
        
        // Bonus for good ratings
        score += ship.Aggregate.ManeuverabilityRating * 0.1f;
        score += ship.Aggregate.CombatEffectivenessRating * 0.1f;
        
        // Bonus for meeting goals
        if (ship.Parameters.Goal == ShipDesignGoal.CargoHauler && ship.Aggregate.CargoEfficiencyRating > 50)
        {
            score += 10f;
        }
        
        return Math.Clamp(score, 0f, 100f);
    }
    
    /// <summary>
    /// Check if a position is occupied
    /// </summary>
    private bool IsPositionOccupied(VoxelStructureComponent structure, Vector3 position, float tolerance)
    {
        return structure.Blocks.Any(b => Vector3.Distance(b.Position, position) < tolerance);
    }
    
    /// <summary>
    /// Add a block to the structure with optional shape and orientation
    /// </summary>
    private void AddBlock(VoxelStructureComponent structure, Vector3 position, Vector3 size, string material, BlockType blockType, BlockShape shape = BlockShape.Cube, BlockOrientation orientation = BlockOrientation.PosY)
    {
        var block = new VoxelBlock(position, size, material, blockType, shape, orientation);
        structure.AddBlock(block);
    }
}
