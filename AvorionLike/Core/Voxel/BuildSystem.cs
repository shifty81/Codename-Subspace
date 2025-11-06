using System.Numerics;
using AvorionLike.Core.ECS;
using AvorionLike.Core.Resources;

namespace AvorionLike.Core.Voxel;

/// <summary>
/// Represents a build mode session for ship construction
/// </summary>
public class BuildSession
{
    public Guid ShipId { get; set; }
    public bool IsActive { get; set; }
    public string SelectedMaterial { get; set; } = "Iron";
    public BlockType SelectedBlockType { get; set; } = BlockType.Hull;
    public Vector3 SelectedSize { get; set; } = new(2, 2, 2);
    public List<VoxelBlock> UndoStack { get; set; } = new();
    public int MaxUndoSteps { get; set; } = 50;
    
    // Block stretching properties
    public bool IsStretching { get; set; } = false;
    public Vector3? StretchStartPosition { get; set; }
    public BlockStretchAxis StretchAxis { get; set; } = BlockStretchAxis.X;
    public Vector3 StretchStartSize { get; set; } = new(2, 2, 2);
}

/// <summary>
/// Axes along which blocks can be stretched
/// </summary>
public enum BlockStretchAxis
{
    X,  // Stretch along X axis (width)
    Y,  // Stretch along Y axis (height)
    Z,  // Stretch along Z axis (depth)
    XY, // Stretch diagonally on XY plane
    XZ, // Stretch diagonally on XZ plane
    YZ, // Stretch diagonally on YZ plane
    XYZ // Stretch in all three dimensions
}

/// <summary>
/// Component for build mode capabilities
/// </summary>
public class BuildModeComponent : IComponent
{
    public Guid EntityId { get; set; }
    public bool CanBuild { get; set; } = true;
    public BuildSession? CurrentSession { get; set; }
}

/// <summary>
/// Result of a block placement operation
/// </summary>
public class PlacementResult
{
    public bool Success { get; set; }
    public string Message { get; set; } = "";
    public VoxelBlock? PlacedBlock { get; set; }
}

/// <summary>
/// System for managing ship building
/// </summary>
public class BuildSystem : SystemBase
{
    private readonly EntityManager _entityManager;

    public BuildSystem(EntityManager entityManager) : base("BuildSystem")
    {
        _entityManager = entityManager;
    }

    public override void Update(float deltaTime)
    {
        // Build system is mostly event-driven, not time-based
        // Could add auto-save functionality here
    }
    
    /// <summary>
    /// Start a build session for a ship
    /// </summary>
    public bool StartBuildSession(Guid shipId)
    {
        var buildMode = _entityManager.GetComponent<BuildModeComponent>(shipId);
        if (buildMode == null)
        {
            buildMode = new BuildModeComponent { EntityId = shipId };
            _entityManager.AddComponent(shipId, buildMode);
        }
        
        if (!buildMode.CanBuild)
        {
            return false;
        }
        
        buildMode.CurrentSession = new BuildSession
        {
            ShipId = shipId,
            IsActive = true
        };
        
        return true;
    }
    
    /// <summary>
    /// End the current build session
    /// </summary>
    public void EndBuildSession(Guid shipId)
    {
        var buildMode = _entityManager.GetComponent<BuildModeComponent>(shipId);
        if (buildMode?.CurrentSession != null)
        {
            buildMode.CurrentSession.IsActive = false;
            buildMode.CurrentSession = null;
        }
    }
    
    /// <summary>
    /// Place a block in build mode
    /// </summary>
    public PlacementResult PlaceBlock(Guid shipId, Vector3 position, Inventory inventory)
    {
        var result = new PlacementResult();
        
        var buildMode = _entityManager.GetComponent<BuildModeComponent>(shipId);
        if (buildMode?.CurrentSession == null || !buildMode.CurrentSession.IsActive)
        {
            result.Message = "No active build session";
            return result;
        }
        
        var structure = _entityManager.GetComponent<VoxelStructureComponent>(shipId);
        if (structure == null)
        {
            result.Message = "Ship has no structure component";
            return result;
        }
        
        var session = buildMode.CurrentSession;
        
        // Calculate cost using new system
        int materialCost = CalculateBlockCost(session.SelectedSize, session.SelectedMaterial, session.SelectedBlockType);
        
        // Try to parse material to ResourceType
        if (!Enum.TryParse<ResourceType>(session.SelectedMaterial, out var materialType))
        {
            result.Message = $"Invalid material type: {session.SelectedMaterial}";
            return result;
        }
        
        if (!inventory.HasResource(materialType, materialCost))
        {
            result.Message = $"Not enough {session.SelectedMaterial}. Need {materialCost}, have {inventory.GetResourceAmount(materialType)}";
            return result;
        }
        
        // Check for overlaps
        var newBlock = new VoxelBlock(position, session.SelectedSize, session.SelectedMaterial, session.SelectedBlockType);
        
        foreach (var existingBlock in structure.Blocks)
        {
            if (newBlock.Intersects(existingBlock))
            {
                result.Message = "Block overlaps with existing block";
                return result;
            }
        }
        
        // Place the block
        structure.AddBlock(newBlock);
        inventory.RemoveResource(materialType, materialCost);
        
        // Add to undo stack
        if (session.UndoStack.Count >= session.MaxUndoSteps)
        {
            session.UndoStack.RemoveAt(0);
        }
        session.UndoStack.Add(newBlock);
        
        result.Success = true;
        result.Message = $"Block placed successfully (Cost: {materialCost} {session.SelectedMaterial})";
        result.PlacedBlock = newBlock;
        
        return result;
    }
    
    /// <summary>
    /// Remove a block in build mode
    /// </summary>
    public PlacementResult RemoveBlock(Guid shipId, Vector3 position, float tolerance, Inventory inventory)
    {
        var result = new PlacementResult();
        
        var buildMode = _entityManager.GetComponent<BuildModeComponent>(shipId);
        if (buildMode?.CurrentSession == null || !buildMode.CurrentSession.IsActive)
        {
            result.Message = "No active build session";
            return result;
        }
        
        var structure = _entityManager.GetComponent<VoxelStructureComponent>(shipId);
        if (structure == null)
        {
            result.Message = "Ship has no structure component";
            return result;
        }
        
        // Find block at position
        var blocks = structure.GetBlocksAt(position, tolerance).ToList();
        if (blocks.Count == 0)
        {
            result.Message = "No block found at position";
            return result;
        }
        
        var blockToRemove = blocks[0];
        
        // Refund materials (50% of cost)
        float volume = blockToRemove.Size.X * blockToRemove.Size.Y * blockToRemove.Size.Z;
        int refund = (int)(volume * 5); // Half of placement cost
        
        // Try to parse material to ResourceType, fallback to Iron
        if (Enum.TryParse<ResourceType>(blockToRemove.MaterialType, out var materialType))
        {
            inventory.AddResource(materialType, refund);
        }
        
        // Remove the block
        structure.RemoveBlock(blockToRemove);
        
        result.Success = true;
        result.Message = $"Block removed. Refunded {refund} {blockToRemove.MaterialType}";
        
        return result;
    }
    
    /// <summary>
    /// Set the selected material for building
    /// </summary>
    public bool SetSelectedMaterial(Guid shipId, string material)
    {
        var buildMode = _entityManager.GetComponent<BuildModeComponent>(shipId);
        if (buildMode?.CurrentSession == null)
        {
            return false;
        }
        
        // Validate material exists
        if (!MaterialProperties.Materials.ContainsKey(material))
        {
            return false;
        }
        
        buildMode.CurrentSession.SelectedMaterial = material;
        return true;
    }
    
    /// <summary>
    /// Set the selected block type for building
    /// </summary>
    public bool SetSelectedBlockType(Guid shipId, BlockType blockType)
    {
        var buildMode = _entityManager.GetComponent<BuildModeComponent>(shipId);
        if (buildMode?.CurrentSession == null)
        {
            return false;
        }
        
        buildMode.CurrentSession.SelectedBlockType = blockType;
        return true;
    }
    
    /// <summary>
    /// Set the selected block size
    /// </summary>
    public bool SetSelectedSize(Guid shipId, Vector3 size)
    {
        var buildMode = _entityManager.GetComponent<BuildModeComponent>(shipId);
        if (buildMode?.CurrentSession == null)
        {
            return false;
        }
        
        // Validate size (must be positive)
        if (size.X <= 0 || size.Y <= 0 || size.Z <= 0)
        {
            return false;
        }
        
        buildMode.CurrentSession.SelectedSize = size;
        return true;
    }
    
    /// <summary>
    /// Undo the last block placement
    /// </summary>
    public bool UndoLastPlacement(Guid shipId, Inventory inventory)
    {
        var buildMode = _entityManager.GetComponent<BuildModeComponent>(shipId);
        if (buildMode?.CurrentSession == null || buildMode.CurrentSession.UndoStack.Count == 0)
        {
            return false;
        }
        
        var structure = _entityManager.GetComponent<VoxelStructureComponent>(shipId);
        if (structure == null)
        {
            return false;
        }
        
        // Get last placed block
        var lastBlock = buildMode.CurrentSession.UndoStack[^1];
        buildMode.CurrentSession.UndoStack.RemoveAt(buildMode.CurrentSession.UndoStack.Count - 1);
        
        // Remove it and refund materials
        structure.RemoveBlock(lastBlock);
        
        float volume = lastBlock.Size.X * lastBlock.Size.Y * lastBlock.Size.Z;
        int refund = (int)(volume * 10); // Full refund for undo
        
        // Try to parse material to ResourceType, fallback to Iron
        if (Enum.TryParse<ResourceType>(lastBlock.MaterialType, out var materialType))
        {
            inventory.AddResource(materialType, refund);
        }
        
        return true;
    }
    
    /// <summary>
    /// Start stretching a block from a position
    /// </summary>
    public bool StartBlockStretch(Guid shipId, Vector3 startPosition, BlockStretchAxis axis)
    {
        var buildMode = _entityManager.GetComponent<BuildModeComponent>(shipId);
        if (buildMode?.CurrentSession == null || !buildMode.CurrentSession.IsActive)
        {
            return false;
        }
        
        var session = buildMode.CurrentSession;
        session.IsStretching = true;
        session.StretchStartPosition = startPosition;
        session.StretchAxis = axis;
        session.StretchStartSize = session.SelectedSize;
        
        return true;
    }
    
    /// <summary>
    /// Update block size during stretching
    /// </summary>
    public Vector3 UpdateBlockStretch(Guid shipId, Vector3 currentPosition)
    {
        var buildMode = _entityManager.GetComponent<BuildModeComponent>(shipId);
        if (buildMode?.CurrentSession == null || !buildMode.CurrentSession.IsStretching)
        {
            return Vector3.Zero;
        }
        
        var session = buildMode.CurrentSession;
        if (!session.StretchStartPosition.HasValue)
        {
            return session.SelectedSize;
        }
        
        Vector3 delta = currentPosition - session.StretchStartPosition.Value;
        Vector3 newSize = session.StretchStartSize;
        
        // Apply stretch based on axis
        switch (session.StretchAxis)
        {
            case BlockStretchAxis.X:
                newSize.X = Math.Max(1f, session.StretchStartSize.X + Math.Abs(delta.X));
                break;
            case BlockStretchAxis.Y:
                newSize.Y = Math.Max(1f, session.StretchStartSize.Y + Math.Abs(delta.Y));
                break;
            case BlockStretchAxis.Z:
                newSize.Z = Math.Max(1f, session.StretchStartSize.Z + Math.Abs(delta.Z));
                break;
            case BlockStretchAxis.XY:
                newSize.X = Math.Max(1f, session.StretchStartSize.X + Math.Abs(delta.X));
                newSize.Y = Math.Max(1f, session.StretchStartSize.Y + Math.Abs(delta.Y));
                break;
            case BlockStretchAxis.XZ:
                newSize.X = Math.Max(1f, session.StretchStartSize.X + Math.Abs(delta.X));
                newSize.Z = Math.Max(1f, session.StretchStartSize.Z + Math.Abs(delta.Z));
                break;
            case BlockStretchAxis.YZ:
                newSize.Y = Math.Max(1f, session.StretchStartSize.Y + Math.Abs(delta.Y));
                newSize.Z = Math.Max(1f, session.StretchStartSize.Z + Math.Abs(delta.Z));
                break;
            case BlockStretchAxis.XYZ:
                newSize.X = Math.Max(1f, session.StretchStartSize.X + Math.Abs(delta.X));
                newSize.Y = Math.Max(1f, session.StretchStartSize.Y + Math.Abs(delta.Y));
                newSize.Z = Math.Max(1f, session.StretchStartSize.Z + Math.Abs(delta.Z));
                break;
        }
        
        session.SelectedSize = newSize;
        return newSize;
    }
    
    /// <summary>
    /// Finish stretching and place the block
    /// </summary>
    public PlacementResult FinishBlockStretch(Guid shipId, Inventory inventory)
    {
        var buildMode = _entityManager.GetComponent<BuildModeComponent>(shipId);
        if (buildMode?.CurrentSession == null || !buildMode.CurrentSession.IsStretching)
        {
            return new PlacementResult { Message = "Not currently stretching" };
        }
        
        var session = buildMode.CurrentSession;
        
        // Place the block at the start position with the stretched size
        var result = PlaceBlock(shipId, session.StretchStartPosition!.Value, inventory);
        
        // End stretching mode
        session.IsStretching = false;
        session.StretchStartPosition = null;
        
        return result;
    }
    
    /// <summary>
    /// Cancel current block stretching operation
    /// </summary>
    public void CancelBlockStretch(Guid shipId)
    {
        var buildMode = _entityManager.GetComponent<BuildModeComponent>(shipId);
        if (buildMode?.CurrentSession == null)
        {
            return;
        }
        
        var session = buildMode.CurrentSession;
        session.IsStretching = false;
        session.StretchStartPosition = null;
        session.SelectedSize = session.StretchStartSize;
    }
    
    /// <summary>
    /// Calculate the cost of placing a block based on its size and type
    /// </summary>
    public int CalculateBlockCost(Vector3 size, string materialType, BlockType blockType)
    {
        float volume = size.X * size.Y * size.Z;
        
        // Base cost per cubic unit
        int baseCost = 10;
        
        // Block type multipliers (specialized blocks cost more)
        float typeMultiplier = blockType switch
        {
            BlockType.Hull => 1.0f,
            BlockType.Armor => 1.5f,
            BlockType.Engine => 2.0f,
            BlockType.Thruster => 1.8f,
            BlockType.Generator => 2.5f,
            BlockType.ShieldGenerator => 3.0f,
            BlockType.GyroArray => 1.5f,
            BlockType.Cargo => 1.2f,
            BlockType.TurretMount => 2.0f,
            BlockType.HyperdriveCore => 3.5f,
            BlockType.CrewQuarters => 1.3f,
            BlockType.PodDocking => 1.5f,
            _ => 1.0f
        };
        
        // Material quality affects cost
        var material = MaterialProperties.GetMaterial(materialType);
        float materialMultiplier = material.DurabilityMultiplier;
        
        return (int)(volume * baseCost * typeMultiplier * materialMultiplier);
    }
    
    /// <summary>
    /// Get preview stats for a block before placing it
    /// </summary>
    public BlockPreviewStats GetBlockPreviewStats(Vector3 size, string materialType, BlockType blockType)
    {
        var material = MaterialProperties.GetMaterial(materialType);
        float volume = size.X * size.Y * size.Z;
        
        var stats = new BlockPreviewStats
        {
            Size = size,
            Volume = volume,
            MaterialType = materialType,
            BlockType = blockType,
            Mass = volume * material.MassMultiplier,
            MaxDurability = 100f * material.DurabilityMultiplier * volume,
            BuildCost = CalculateBlockCost(size, materialType, blockType)
        };
        
        // Calculate block-specific stats based on size
        switch (blockType)
        {
            case BlockType.Engine:
                stats.ThrustPower = 50f * volume * material.EnergyEfficiency;
                stats.PowerConsumption = 5f * volume;
                break;
            case BlockType.Thruster:
                stats.ThrustPower = 30f * volume * material.EnergyEfficiency;
                stats.PowerConsumption = 3f * volume;
                break;
            case BlockType.GyroArray:
                stats.TorquePower = 20f * volume * material.EnergyEfficiency;
                stats.PowerConsumption = 2f * volume;
                break;
            case BlockType.Generator:
                stats.PowerGeneration = 100f * volume * material.EnergyEfficiency;
                stats.EnergyStorage = 50f * volume;
                break;
            case BlockType.ShieldGenerator:
                stats.ShieldCapacity = 200f * volume * material.ShieldMultiplier;
                stats.PowerConsumption = 10f * volume;
                break;
            case BlockType.Cargo:
                stats.CargoCapacity = 100f * volume;
                break;
            case BlockType.Armor:
                // Armor has enhanced durability
                stats.MaxDurability *= 5.0f;
                stats.Mass *= 1.5f;
                break;
        }
        
        return stats;
    }
}

/// <summary>
/// Preview statistics for a block before placement
/// </summary>
public class BlockPreviewStats
{
    public Vector3 Size { get; set; }
    public float Volume { get; set; }
    public string MaterialType { get; set; } = "";
    public BlockType BlockType { get; set; }
    public float Mass { get; set; }
    public float MaxDurability { get; set; }
    public int BuildCost { get; set; }
    
    // Block-specific stats
    public float ThrustPower { get; set; }
    public float TorquePower { get; set; }
    public float PowerGeneration { get; set; }
    public float PowerConsumption { get; set; }
    public float EnergyStorage { get; set; }
    public float ShieldCapacity { get; set; }
    public float CargoCapacity { get; set; }
}
