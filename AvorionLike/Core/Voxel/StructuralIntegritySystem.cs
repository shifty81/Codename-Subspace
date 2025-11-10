using System.Numerics;
using AvorionLike.Core.Logging;

namespace AvorionLike.Core.Voxel;

/// <summary>
/// System for validating and ensuring structural integrity of voxel ships.
/// Implements connectivity graphs, core block validation, and path checking.
/// </summary>
public class StructuralIntegritySystem
{
    private readonly Logger _logger = Logger.Instance;

    /// <summary>
    /// Result of structural integrity validation
    /// </summary>
    public class IntegrityResult
    {
        public bool IsValid { get; set; } = true;
        public List<string> Errors { get; set; } = new();
        public List<string> Warnings { get; set; } = new();
        public Dictionary<Guid, int> BlockDistancesFromCore { get; set; } = new();
        public HashSet<Guid> ConnectedBlocks { get; set; } = new();
        public HashSet<Guid> DisconnectedBlocks { get; set; } = new();
        public Guid? CoreBlockId { get; set; }
        public int MaxDistanceFromCore { get; set; }
    }

    /// <summary>
    /// Validate the structural integrity of a ship structure
    /// </summary>
    public IntegrityResult ValidateStructure(VoxelStructureComponent structure, int maxDistanceFromCore = 50)
    {
        var result = new IntegrityResult
        {
            MaxDistanceFromCore = maxDistanceFromCore
        };

        if (structure.Blocks.Count == 0)
        {
            result.IsValid = false;
            result.Errors.Add("Structure has no blocks");
            return result;
        }

        // Step 1: Identify core block(s)
        var coreBlock = IdentifyCoreBlock(structure);
        if (coreBlock == null)
        {
            result.Errors.Add("No core block found (requires HyperdriveCore, CrewQuarters, or PodDocking)");
            result.IsValid = false;
            return result;
        }

        result.CoreBlockId = coreBlock.Id;
        _logger.Debug("StructuralIntegrity", $"Core block identified: {coreBlock.BlockType} at {coreBlock.Position}");

        // Step 2: Build connectivity graph
        var adjacencyMap = BuildConnectivityGraph(structure.Blocks);

        // Step 3: Perform BFS to find all connected blocks and their distances
        var (connectedBlocks, distances) = PerformConnectivitySearch(coreBlock, structure.Blocks, adjacencyMap);

        result.ConnectedBlocks = connectedBlocks;
        result.BlockDistancesFromCore = distances;

        // Step 4: Identify disconnected blocks
        foreach (var block in structure.Blocks)
        {
            if (!connectedBlocks.Contains(block.Id))
            {
                result.DisconnectedBlocks.Add(block.Id);
                result.Errors.Add($"Block {block.BlockType} at {block.Position} is disconnected from core");
            }
        }

        // Step 5: Check distance constraints
        foreach (var kvp in distances)
        {
            if (kvp.Value > maxDistanceFromCore)
            {
                var block = structure.Blocks.FirstOrDefault(b => b.Id == kvp.Key);
                if (block != null)
                {
                    result.Warnings.Add($"Block {block.BlockType} at {block.Position} is {kvp.Value} blocks from core (max: {maxDistanceFromCore})");
                }
            }
        }

        // Step 6: Final validation
        if (result.DisconnectedBlocks.Count > 0)
        {
            result.IsValid = false;
            _logger.Warning("StructuralIntegrity", $"Structure has {result.DisconnectedBlocks.Count} disconnected blocks");
        }
        else
        {
            _logger.Info("StructuralIntegrity", $"Structure validated: {connectedBlocks.Count} blocks connected to core");
        }

        return result;
    }

    /// <summary>
    /// Identify the core block of the ship (cockpit, engine room, or hyperdrive)
    /// </summary>
    private VoxelBlock? IdentifyCoreBlock(VoxelStructureComponent structure)
    {
        // Priority: HyperdriveCore > CrewQuarters > PodDocking > Generator
        var core = structure.Blocks.FirstOrDefault(b => b.BlockType == BlockType.HyperdriveCore);
        if (core != null) return core;

        core = structure.Blocks.FirstOrDefault(b => b.BlockType == BlockType.CrewQuarters);
        if (core != null) return core;

        core = structure.Blocks.FirstOrDefault(b => b.BlockType == BlockType.PodDocking);
        if (core != null) return core;

        // Fallback to the first generator as core
        core = structure.Blocks.FirstOrDefault(b => b.BlockType == BlockType.Generator);
        return core;
    }

    /// <summary>
    /// Build connectivity graph based on adjacent blocks
    /// </summary>
    private Dictionary<Guid, List<Guid>> BuildConnectivityGraph(List<VoxelBlock> blocks)
    {
        var adjacencyMap = new Dictionary<Guid, List<Guid>>();

        foreach (var block in blocks)
        {
            adjacencyMap[block.Id] = new List<Guid>();
        }

        // Find all adjacent blocks (blocks that touch or overlap)
        for (int i = 0; i < blocks.Count; i++)
        {
            for (int j = i + 1; j < blocks.Count; j++)
            {
                if (AreBlocksAdjacent(blocks[i], blocks[j]))
                {
                    adjacencyMap[blocks[i].Id].Add(blocks[j].Id);
                    adjacencyMap[blocks[j].Id].Add(blocks[i].Id);
                }
            }
        }

        return adjacencyMap;
    }

    /// <summary>
    /// Check if two blocks are adjacent (touching or close enough)
    /// Increased tolerance for procedurally generated ships with spacing
    /// </summary>
    private bool AreBlocksAdjacent(VoxelBlock a, VoxelBlock b)
    {
        // Blocks are adjacent if they intersect or are within an acceptable distance
        // Increased tolerance to account for block spacing in procedural generation
        const float adjacencyTolerance = 4.5f;  // Increased from 0.1f to handle block spacing

        // Calculate the minimum distance between the block boundaries
        float dx = Math.Max(0, Math.Max(
            a.Position.X - b.Size.X / 2 - (b.Position.X + b.Size.X / 2),
            b.Position.X - a.Size.X / 2 - (a.Position.X + a.Size.X / 2)));
        
        float dy = Math.Max(0, Math.Max(
            a.Position.Y - b.Size.Y / 2 - (b.Position.Y + b.Size.Y / 2),
            b.Position.Y - a.Size.Y / 2 - (a.Position.Y + a.Size.Y / 2)));
        
        float dz = Math.Max(0, Math.Max(
            a.Position.Z - b.Size.Z / 2 - (b.Position.Z + b.Size.Z / 2),
            b.Position.Z - a.Size.Z / 2 - (a.Position.Z + a.Size.Z / 2)));

        float distance = MathF.Sqrt(dx * dx + dy * dy + dz * dz);

        return distance <= adjacencyTolerance;
    }

    /// <summary>
    /// Perform BFS to find all connected blocks and calculate distances from core
    /// </summary>
    private (HashSet<Guid> connectedBlocks, Dictionary<Guid, int> distances) PerformConnectivitySearch(
        VoxelBlock coreBlock,
        List<VoxelBlock> allBlocks,
        Dictionary<Guid, List<Guid>> adjacencyMap)
    {
        var connectedBlocks = new HashSet<Guid>();
        var distances = new Dictionary<Guid, int>();
        var queue = new Queue<(Guid blockId, int distance)>();

        // Start BFS from core block
        queue.Enqueue((coreBlock.Id, 0));
        connectedBlocks.Add(coreBlock.Id);
        distances[coreBlock.Id] = 0;

        while (queue.Count > 0)
        {
            var (currentId, currentDistance) = queue.Dequeue();

            if (!adjacencyMap.ContainsKey(currentId))
                continue;

            foreach (var neighborId in adjacencyMap[currentId])
            {
                if (!connectedBlocks.Contains(neighborId))
                {
                    connectedBlocks.Add(neighborId);
                    distances[neighborId] = currentDistance + 1;
                    queue.Enqueue((neighborId, currentDistance + 1));
                }
            }
        }

        return (connectedBlocks, distances);
    }

    /// <summary>
    /// Attempt to repair disconnected blocks by adding connecting blocks
    /// </summary>
    public List<VoxelBlock> SuggestConnectingBlocks(VoxelStructureComponent structure, IntegrityResult integrityResult)
    {
        var suggestions = new List<VoxelBlock>();

        if (integrityResult.DisconnectedBlocks.Count == 0)
            return suggestions;

        var coreBlock = structure.Blocks.FirstOrDefault(b => b.Id == integrityResult.CoreBlockId);
        if (coreBlock == null)
            return suggestions;

        foreach (var disconnectedId in integrityResult.DisconnectedBlocks)
        {
            var disconnectedBlock = structure.Blocks.FirstOrDefault(b => b.Id == disconnectedId);
            if (disconnectedBlock == null)
                continue;

            // Find nearest connected block
            VoxelBlock? nearestConnected = null;
            float minDistance = float.MaxValue;

            foreach (var connectedId in integrityResult.ConnectedBlocks)
            {
                var connectedBlock = structure.Blocks.FirstOrDefault(b => b.Id == connectedId);
                if (connectedBlock == null)
                    continue;

                float distance = Vector3.Distance(disconnectedBlock.Position, connectedBlock.Position);
                if (distance < minDistance)
                {
                    minDistance = distance;
                    nearestConnected = connectedBlock;
                }
            }

            if (nearestConnected != null)
            {
                // Suggest hull blocks to bridge the gap
                var bridgeBlocks = CreateBridgePath(nearestConnected, disconnectedBlock, structure.Blocks[0].MaterialType);
                suggestions.AddRange(bridgeBlocks);
            }
        }

        return suggestions;
    }

    /// <summary>
    /// Create a simple path of hull blocks between two blocks
    /// </summary>
    private List<VoxelBlock> CreateBridgePath(VoxelBlock from, VoxelBlock to, string material)
    {
        var path = new List<VoxelBlock>();

        // Simple linear interpolation path
        Vector3 direction = to.Position - from.Position;
        float distance = direction.Length();
        if (distance < 0.01f)
            return path;

        direction = Vector3.Normalize(direction);

        // Place blocks along the path
        int steps = (int)(distance / 2.0f); // Assuming block size of 2
        for (int i = 1; i < steps; i++)
        {
            Vector3 position = from.Position + direction * (i * 2.0f);
            var bridgeBlock = new VoxelBlock(position, new Vector3(2, 2, 2), material, BlockType.Hull);
            path.Add(bridgeBlock);
        }

        return path;
    }

    /// <summary>
    /// Calculate structural integrity percentage based on connectivity
    /// </summary>
    public float CalculateStructuralIntegrityPercentage(VoxelStructureComponent structure, IntegrityResult result)
    {
        if (structure.Blocks.Count == 0)
            return 0f;

        float connectedRatio = (float)result.ConnectedBlocks.Count / structure.Blocks.Count;
        float distancePenalty = 0f;

        // Apply penalty for blocks far from core
        foreach (var distance in result.BlockDistancesFromCore.Values)
        {
            if (distance > result.MaxDistanceFromCore)
            {
                distancePenalty += 0.01f; // 1% penalty per block over limit
            }
        }

        float integrity = Math.Max(0f, Math.Min(100f, connectedRatio * 100f - distancePenalty));
        return integrity;
    }
}
