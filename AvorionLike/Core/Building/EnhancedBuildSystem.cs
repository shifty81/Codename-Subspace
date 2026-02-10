using System.Numerics;
using AvorionLike.Core.ECS;
using AvorionLike.Core.Voxel;
using AvorionLike.Core.Resources;
using AvorionLike.Core.Physics;

namespace AvorionLike.Core.Building;

/// <summary>
/// Build mode tool types
/// </summary>
public enum BuildTool
{
    Add,           // Place new blocks
    Remove,        // Delete blocks
    Paint,         // Change block color
    Select,        // Select multiple blocks
    Transform,     // Change block type without changing shape
    Repair,        // Repair damaged blocks
    Merge,         // Combine adjacent identical blocks
    Scale,         // Scale selected blocks
    Rotate,        // Rotate selected blocks
    Move           // Move selected blocks
}

/// <summary>
/// Mirror mode for symmetrical building
/// </summary>
[Flags]
public enum MirrorAxis
{
    None = 0,
    X = 1,
    Y = 2,
    Z = 4,
    XY = X | Y,
    XZ = X | Z,
    YZ = Y | Z,
    XYZ = X | Y | Z
}

/// <summary>
/// Grid space mode
/// </summary>
public enum GridSpace
{
    Local,        // Relative to ship
    Global,       // World coordinates
    BlockCenter   // Snap to block centers
}

/// <summary>
/// Ship builder state and settings
/// </summary>
public class ShipBuilderState
{
    // Current tool and mode
    public BuildTool CurrentTool { get; set; } = BuildTool.Add;
    public BlockType SelectedBlockType { get; set; } = BlockType.Hull;
    public BlockShape SelectedShape { get; set; } = BlockShape.Cube;
    public BlockOrientation SelectedOrientation { get; set; } = BlockOrientation.PosY;
    public string SelectedMaterial { get; set; } = "Iron";
    public Vector3 SelectedColor { get; set; } = new Vector3(0.5f, 0.5f, 0.5f); // RGB
    
    // Grid and placement
    public float GridSize { get; set; } = 1f; // Meters
    public float ScaleStep { get; set; } = 0.1f;
    public GridSpace GridSpaceMode { get; set; } = GridSpace.Local;
    
    // Mirror settings
    public MirrorAxis MirrorMode { get; set; } = MirrorAxis.None;
    public Vector3 MirrorOrigin { get; set; } = Vector3.Zero;
    public bool UseCenterAsMirrorOrigin { get; set; } = true;
    
    // Selection
    public List<Guid> SelectedBlocks { get; set; } = new();
    public Vector3 SelectionStart { get; set; }
    public Vector3 SelectionEnd { get; set; }
    public bool IsSelecting { get; set; } = false;
    
    // Clipboard for copy/paste
    public List<VoxelBlock> ClipboardBlocks { get; set; } = new();
    public Vector3 ClipboardScale { get; set; } = Vector3.One;
    
    // Highlighting
    public bool HighlightDamage { get; set; } = false;
    public bool HighlightTurretPlacement { get; set; } = false;
    
    // Camera
    public Vector3 CameraPosition { get; set; } = new Vector3(0, 0, 50);
    public Vector3 CameraTarget { get; set; } = Vector3.Zero;
    public float CameraDistance { get; set; } = 50f;
}

/// <summary>
/// Real-time ship statistics calculated during building
/// </summary>
public class ShipStatistics
{
    // Structure
    public int TotalBlocks { get; set; }
    public Dictionary<BlockType, int> BlocksByType { get; set; } = new();
    public float TotalMass { get; set; }
    public float TotalVolume { get; set; }
    
    // Durability
    public float MaxHull { get; set; }
    public float CurrentHull { get; set; }
    public float MaxShields { get; set; }
    public float ArmorRating { get; set; }
    
    // Power and Energy
    public float PowerGeneration { get; set; }
    public float PowerConsumption { get; set; }
    public float PowerBalance => PowerGeneration - PowerConsumption;
    public float EnergyCapacity { get; set; }
    
    // Propulsion
    public float TotalThrust { get; set; }
    public float TotalTorque { get; set; }
    public float MaxSpeed { get; set; }
    public float Acceleration { get; set; }
    
    // Maneuverability (degrees per second)
    public float PitchSpeed { get; set; }
    public float YawSpeed { get; set; }
    public float RollSpeed { get; set; }
    
    // Storage
    public float CargoCapacity { get; set; }
    public float CurrentCargo { get; set; }
    
    // Crew
    public int RequiredCrew { get; set; }
    public int CurrentCrew { get; set; }
    public int MaxCrew { get; set; }
    public Dictionary<string, int> CrewByRole { get; set; } = new();
    
    // Weapons
    public int TurretSlots { get; set; }
    public int OccupiedTurretSlots { get; set; }
    public float TotalDPS { get; set; }
    
    // Cost
    public Dictionary<ResourceType, int> ResourceCost { get; set; } = new();
    public int TotalCredits { get; set; }
    
    // Special
    public float HyperdriveRange { get; set; }
    public int HangarCapacity { get; set; }
}

/// <summary>
/// Enhanced build system with full building interface capabilities
/// NOTE: This is the data/logic layer. UI rendering would be in a separate BuildModeUI class
/// </summary>
public class EnhancedBuildSystem : SystemBase
{
    private readonly EntityManager _entityManager;
    private ShipBuilderState? _currentBuildState;
    private Guid? _entityBeingEdited;
    
    public bool IsBuildModeActive => _currentBuildState != null;
    public ShipBuilderState? BuildState => _currentBuildState;
    public Guid? CurrentEntity => _entityBeingEdited;
    
    public EnhancedBuildSystem(EntityManager entityManager) : base("EnhancedBuildSystem")
    {
        _entityManager = entityManager;
    }
    
    /// <summary>
    /// Enter build mode for an entity
    /// </summary>
    public ShipBuilderState EnterBuildMode(Guid entityId)
    {
        _entityBeingEdited = entityId;
        _currentBuildState = new ShipBuilderState();
        
        // Initialize camera to look at the entity
        var physics = _entityManager.GetComponent<PhysicsComponent>(entityId);
        if (physics != null)
        {
            _currentBuildState.CameraTarget = physics.Position;
            _currentBuildState.CameraPosition = physics.Position + new Vector3(0, 0, 50);
        }
        
        return _currentBuildState;
    }
    
    /// <summary>
    /// Exit build mode
    /// </summary>
    public void ExitBuildMode()
    {
        _currentBuildState = null;
        _entityBeingEdited = null;
    }
    
    /// <summary>
    /// Add a block at the specified position
    /// </summary>
    public bool AddBlock(Vector3 position, Vector3 size, BlockType blockType, string material)
    {
        if (!_entityBeingEdited.HasValue || _currentBuildState == null) return false;
        
        var voxelComponent = _entityManager.GetComponent<VoxelStructureComponent>(_entityBeingEdited.Value);
        if (voxelComponent == null) return false;
        
        // Snap position to grid
        position = BuildSystem.SnapToGrid(position);
        
        // Check overlap
        var newBlock = new VoxelBlock(position, size, material, blockType, 
            _currentBuildState.SelectedShape, _currentBuildState.SelectedOrientation);
        if (BuildSystem.OverlapsExistingBlocks(newBlock, voxelComponent))
            return false;
        
        // Check adjacency (must touch existing block, unless first block)
        if (voxelComponent.Blocks.Count > 0 && !BuildSystem.TouchesAtLeastOneBlock(newBlock, voxelComponent))
            return false;
        
        // Check if we have resources
        var inventory = _entityManager.GetComponent<InventoryComponent>(_entityBeingEdited.Value);
        if (inventory != null)
        {
            var cost = CalculateBlockCost(size, material);
            if (!HasResources(inventory, cost)) return false;
            ConsumeResources(inventory, cost);
        }
        
        // Add the block
        voxelComponent.AddBlock(newBlock);
        
        // Apply mirror mode if active
        if (_currentBuildState.MirrorMode != MirrorAxis.None)
        {
            AddMirroredBlocks(voxelComponent, newBlock, _currentBuildState.MirrorMode, _currentBuildState.MirrorOrigin);
        }
        
        return true;
    }
    
    /// <summary>
    /// Calculate real-time ship statistics
    /// </summary>
    public ShipStatistics CalculateShipStatistics(Guid entityId)
    {
        var stats = new ShipStatistics();
        
        var voxelComponent = _entityManager.GetComponent<VoxelStructureComponent>(entityId);
        if (voxelComponent == null) return stats;
        
        stats.TotalBlocks = voxelComponent.Blocks.Count;
        stats.TotalMass = voxelComponent.TotalMass;
        stats.TotalVolume = stats.TotalMass / 100f; // Rough estimate
        
        // Count blocks by type
        foreach (var block in voxelComponent.Blocks)
        {
            if (!stats.BlocksByType.ContainsKey(block.BlockType))
                stats.BlocksByType[block.BlockType] = 0;
            stats.BlocksByType[block.BlockType]++;
        }
        
        // Durability - use structural integrity as proxy for hull
        stats.MaxHull = 100f;
        stats.CurrentHull = voxelComponent.StructuralIntegrity;
        stats.MaxShields = voxelComponent.ShieldCapacity;
        stats.ArmorRating = voxelComponent.GetBlocksByType(BlockType.Armor).Count() * 10f;
        
        // Power
        stats.PowerGeneration = voxelComponent.PowerGeneration;
        stats.PowerConsumption = 50f; // Estimated based on ship size
        stats.EnergyCapacity = 1000f; // Default
        
        // Propulsion
        stats.TotalThrust = voxelComponent.TotalThrust;
        stats.TotalTorque = voxelComponent.TotalTorque;
        if (stats.TotalMass > 0)
        {
            stats.Acceleration = stats.TotalThrust / stats.TotalMass;
            stats.MaxSpeed = stats.Acceleration * 10f;
            
            // Maneuverability
            float momentOfInertia = voxelComponent.MomentOfInertia;
            if (momentOfInertia > 0)
            {
                float angularAccel = stats.TotalTorque / momentOfInertia;
                stats.PitchSpeed = angularAccel * 57.3f;
                stats.YawSpeed = angularAccel * 57.3f;
                stats.RollSpeed = angularAccel * 57.3f;
            }
        }
        
        // Storage - estimate based on cargo blocks
        stats.CargoCapacity = voxelComponent.GetBlocksByType(BlockType.Cargo).Count() * 100f;
        
        // Crew
        stats.RequiredCrew = CalculateRequiredCrew(voxelComponent);
        stats.MaxCrew = voxelComponent.GetBlocksByType(BlockType.CrewQuarters).Count() * 10;
        stats.CrewByRole["Engineers"] = voxelComponent.GetBlocksByType(BlockType.Engine).Count() * 2;
        stats.CrewByRole["Technicians"] = voxelComponent.GetBlocksByType(BlockType.Generator).Count();
        stats.CrewByRole["General"] = Math.Max(1, voxelComponent.Blocks.Count / 100);
        
        // Calculate resource cost
        stats.ResourceCost = CalculateTotalResourceCost(voxelComponent);
        stats.TotalCredits = stats.ResourceCost
            .Where(kvp => kvp.Key != ResourceType.Credits)
            .Sum(kvp => GetResourceValue(kvp.Key) * kvp.Value);
        
        return stats;
    }
    
    private void AddMirroredBlocks(VoxelStructureComponent voxel, VoxelBlock original, MirrorAxis axis, Vector3 origin)
    {
        if (axis.HasFlag(MirrorAxis.X))
        {
            var mirroredPos = new Vector3(-original.Position.X + 2 * origin.X, original.Position.Y, original.Position.Z);
            voxel.AddBlock(new VoxelBlock(mirroredPos, original.Size, original.MaterialType, original.BlockType));
        }
        
        if (axis.HasFlag(MirrorAxis.Y))
        {
            var mirroredPos = new Vector3(original.Position.X, -original.Position.Y + 2 * origin.Y, original.Position.Z);
            voxel.AddBlock(new VoxelBlock(mirroredPos, original.Size, original.MaterialType, original.BlockType));
        }
        
        if (axis.HasFlag(MirrorAxis.Z))
        {
            var mirroredPos = new Vector3(original.Position.X, original.Position.Y, -original.Position.Z + 2 * origin.Z);
            voxel.AddBlock(new VoxelBlock(mirroredPos, original.Size, original.MaterialType, original.BlockType));
        }
    }
    
    private Dictionary<ResourceType, int> CalculateBlockCost(Vector3 size, string material)
    {
        var cost = new Dictionary<ResourceType, int>();
        float volume = size.X * size.Y * size.Z;
        int amount = (int)(volume * 10);
        
        var resourceType = material switch
        {
            "Iron" => ResourceType.Iron,
            "Titanium" => ResourceType.Titanium,
            "Naonite" => ResourceType.Naonite,
            "Trinium" => ResourceType.Trinium,
            "Xanion" => ResourceType.Xanion,
            "Ogonite" => ResourceType.Ogonite,
            "Avorion" => ResourceType.Avorion,
            _ => ResourceType.Iron
        };
        
        cost[resourceType] = amount;
        
        return cost;
    }
    
    private Dictionary<ResourceType, int> CalculateTotalResourceCost(VoxelStructureComponent voxel)
    {
        var totalCost = new Dictionary<ResourceType, int>();
        
        foreach (var block in voxel.Blocks)
        {
            var blockCost = CalculateBlockCost(block.Size, block.MaterialType);
            foreach (var kvp in blockCost)
            {
                if (!totalCost.ContainsKey(kvp.Key))
                    totalCost[kvp.Key] = 0;
                totalCost[kvp.Key] += kvp.Value;
            }
        }
        
        return totalCost;
    }
    
    private bool HasResources(InventoryComponent inventory, Dictionary<ResourceType, int> cost)
    {
        foreach (var kvp in cost)
        {
            if (inventory.Inventory.GetResourceAmount(kvp.Key) < kvp.Value)
                return false;
        }
        return true;
    }
    
    private void ConsumeResources(InventoryComponent inventory, Dictionary<ResourceType, int> cost)
    {
        foreach (var kvp in cost)
        {
            inventory.Inventory.RemoveResource(kvp.Key, kvp.Value);
        }
    }
    
    private int GetResourceValue(ResourceType type)
    {
        return type switch
        {
            ResourceType.Iron => 1,
            ResourceType.Titanium => 3,
            ResourceType.Naonite => 10,
            ResourceType.Trinium => 30,
            ResourceType.Xanion => 100,
            ResourceType.Ogonite => 300,
            ResourceType.Avorion => 1000,
            _ => 1
        };
    }
    
    private int CalculateRequiredCrew(VoxelStructureComponent voxel)
    {
        int crew = 0;
        crew += voxel.GetBlocksByType(BlockType.Engine).Count() * 2;
        crew += voxel.GetBlocksByType(BlockType.Generator).Count();
        crew += voxel.GetBlocksByType(BlockType.ShieldGenerator).Count();
        crew += Math.Max(1, voxel.Blocks.Count / 100);
        return crew;
    }
    
    public override void Update(float deltaTime)
    {
        // Build system operations are triggered by user input, not regular updates
    }
}
