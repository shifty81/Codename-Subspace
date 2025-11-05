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
        
        // Check if player has materials
        var material = MaterialProperties.GetMaterial(session.SelectedMaterial);
        float volume = session.SelectedSize.X * session.SelectedSize.Y * session.SelectedSize.Z;
        int materialCost = (int)(volume * 10); // 10 units per cubic unit
        
        // Try to parse material to ResourceType, fallback to Iron
        if (!Enum.TryParse<ResourceType>(session.SelectedMaterial, out var materialType))
        {
            result.Message = $"Invalid material type: {session.SelectedMaterial}";
            return result;
        }
        
        if (!inventory.HasResource(materialType, materialCost))
        {
            result.Message = $"Not enough {session.SelectedMaterial}. Need {materialCost}";
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
        result.Message = "Block placed successfully";
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
}
