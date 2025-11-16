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
    
    public AIShipGenerator(int seed = 0)
    {
        _random = seed == 0 ? new Random() : new Random(seed);
    }
    
    /// <summary>
    /// Generate a ship using AI-driven design rules
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
        
        // Step 3: Define ship outline using framework blocks
        DefineShipOutline(result, dimensions, parameters);
        result.DesignDecisions.Add($"Defined ship outline with {result.Structure.Blocks.Count} framework blocks");
        
        // Step 4: Place critical internal components (generators, crew quarters)
        PlaceInternalComponents(result, placementPlan, parameters);
        result.DesignDecisions.Add($"Placed internal components, total blocks: {result.Structure.Blocks.Count}");
        
        // Step 5: Place functional systems based on priority
        PlaceFunctionalSystems(result, placementPlan, parameters);
        result.DesignDecisions.Add($"Placed functional systems, total blocks: {result.Structure.Blocks.Count}");
        
        // Step 6: Add external armor shell
        AddArmorShell(result, parameters);
        result.DesignDecisions.Add($"Added armor shell, total blocks: {result.Structure.Blocks.Count}");
        
        // Step 7: Optimize and validate
        OptimizeDesign(result, parameters);
        result.DesignDecisions.Add($"Optimized design, final blocks: {result.Structure.Blocks.Count}");
        
        // Step 8: Calculate final statistics
        result.Aggregate.Recalculate();
        
        // Step 9: Validate requirements
        ValidateDesign(result);
        
        // Step 10: Rate design quality
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
    /// </summary>
    private int CalculateRequiredBlockCount(BlockType blockType, AIShipGenerationParameters parameters)
    {
        float blockBudget = parameters.TargetBlockCount;
        
        // Allocate percentage of blocks based on type and goal
        float percentage = blockType switch
        {
            BlockType.Hull => 0.30f, // 30% for structure
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
    /// Define ship outline with hull blocks (block-out method)
    /// </summary>
    private void DefineShipOutline(AIGeneratedShip ship, Vector3 dimensions, AIShipGenerationParameters parameters)
    {
        // Create a frame/skeleton using hull blocks
        // This defines the shape without filling it completely
        
        float blockSize = 2f;
        int layers = 3; // Multiple layers for better shape definition
        
        for (int layer = 0; layer < layers; layer++)
        {
            float layerOffset = layer * blockSize;
            
            // Front section (nose)
            for (float x = -dimensions.X / 2 + layerOffset; x <= dimensions.X / 2 - layerOffset; x += blockSize)
            {
                for (float y = -dimensions.Y / 2 + layerOffset; y <= dimensions.Y / 2 - layerOffset; y += blockSize)
                {
                    float z = dimensions.Z / 2 - layerOffset;
                    if (ShouldPlaceFrameBlock(x, y, z, dimensions, layer))
                    {
                        AddBlock(ship.Structure, new Vector3(x, y, z), new Vector3(blockSize), 
                            parameters.Material, BlockType.Hull);
                    }
                }
            }
            
            // Rear section (engines)
            for (float x = -dimensions.X / 2 + layerOffset; x <= dimensions.X / 2 - layerOffset; x += blockSize)
            {
                for (float y = -dimensions.Y / 2 + layerOffset; y <= dimensions.Y / 2 - layerOffset; y += blockSize)
                {
                    float z = -dimensions.Z / 2 + layerOffset;
                    if (ShouldPlaceFrameBlock(x, y, z, dimensions, layer))
                    {
                        AddBlock(ship.Structure, new Vector3(x, y, z), new Vector3(blockSize),
                            parameters.Material, BlockType.Hull);
                    }
                }
            }
            
            // Side edges
            for (float z = -dimensions.Z / 2; z <= dimensions.Z / 2; z += blockSize)
            {
                // Left side
                AddBlock(ship.Structure, new Vector3(-dimensions.X / 2 + layerOffset, 0, z), new Vector3(blockSize),
                    parameters.Material, BlockType.Hull);
                // Right side
                AddBlock(ship.Structure, new Vector3(dimensions.X / 2 - layerOffset, 0, z), new Vector3(blockSize),
                    parameters.Material, BlockType.Hull);
            }
        }
    }
    
    /// <summary>
    /// Determine if a frame block should be placed at this position
    /// </summary>
    private bool ShouldPlaceFrameBlock(float x, float y, float z, Vector3 dimensions, int layer)
    {
        // Only place on edges/corners, not interior
        bool onEdgeX = MathF.Abs(x) > dimensions.X / 3;
        bool onEdgeY = MathF.Abs(y) > dimensions.Y / 3;
        
        // Outer layer is complete frame, inner layers are sparse
        return layer == 0 || (onEdgeX || onEdgeY);
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
    /// </summary>
    private void PlaceFunctionalSystems(AIGeneratedShip ship, List<BlockPlacementPlan> plan, AIShipGenerationParameters parameters)
    {
        var functionalBlocks = plan.Where(p => !p.IsInternal).OrderByDescending(p => p.Priority);
        
        foreach (var blockPlan in functionalBlocks)
        {
            Vector3 optimalPosition = FindOptimalPosition(ship, blockPlan.BlockType, parameters);
            
            if (!IsPositionOccupied(ship.Structure, optimalPosition, 2f))
            {
                AddBlock(ship.Structure, optimalPosition, blockPlan.Size, blockPlan.MaterialType, blockPlan.BlockType);
            }
        }
    }
    
    /// <summary>
    /// Find optimal position for a block type
    /// </summary>
    private Vector3 FindOptimalPosition(AIGeneratedShip ship, BlockType blockType, AIShipGenerationParameters parameters)
    {
        // Strategic placement based on block type
        return blockType switch
        {
            BlockType.Engine => new Vector3(0, 0, -15), // Rear
            BlockType.Thruster => new Vector3(_random.Next(-8, 8), _random.Next(-4, 4), _random.Next(-10, 10)),
            BlockType.TurretMount => new Vector3(_random.Next(-10, 10), 5, _random.Next(-5, 10)), // Top/sides
            BlockType.GyroArray => new Vector3(0, 0, 0), // Center of mass
            _ => new Vector3(_random.Next(-8, 8), _random.Next(-4, 4), _random.Next(-10, 10))
        };
    }
    
    /// <summary>
    /// Add external armor shell
    /// </summary>
    private void AddArmorShell(AIGeneratedShip ship, AIShipGenerationParameters parameters)
    {
        // Add thin layer of armor blocks around external surface
        var existingBlocks = ship.Structure.Blocks.ToList();
        
        foreach (var block in existingBlocks)
        {
            // Add armor adjacent to hull blocks on exterior
            if (block.BlockType == BlockType.Hull)
            {
                // Add armor around exterior hull
                Vector3[] armorOffsets = new[]
                {
                    new Vector3(2, 0, 0), new Vector3(-2, 0, 0),
                    new Vector3(0, 2, 0), new Vector3(0, -2, 0),
                    new Vector3(0, 0, 2), new Vector3(0, 0, -2)
                };
                
                foreach (var offset in armorOffsets)
                {
                    Vector3 armorPos = block.Position + offset;
                    if (!IsPositionOccupied(ship.Structure, armorPos, 1f) && _random.NextDouble() < 0.3)
                    {
                        AddBlock(ship.Structure, armorPos, new Vector3(2, 2, 2), parameters.Material, BlockType.Armor);
                    }
                }
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
    /// Check if a block is connected to the main structure
    /// </summary>
    private bool IsConnectedToStructure(VoxelBlock block, List<VoxelBlock> allBlocks, VoxelBlock centerBlock)
    {
        // Simplified connectivity check
        if (block == centerBlock) return true;
        
        float maxDistance = 5f; // Adjacent blocks
        return allBlocks.Any(b => b != block && Vector3.Distance(b.Position, block.Position) < maxDistance);
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
    /// Add a block to the structure
    /// </summary>
    private void AddBlock(VoxelStructureComponent structure, Vector3 position, Vector3 size, string material, BlockType blockType)
    {
        var block = new VoxelBlock(position, size, material, blockType);
        structure.AddBlock(block);
    }
}
