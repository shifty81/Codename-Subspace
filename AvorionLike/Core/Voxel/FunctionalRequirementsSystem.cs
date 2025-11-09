using System.Numerics;
using AvorionLike.Core.Logging;

namespace AvorionLike.Core.Voxel;

/// <summary>
/// System for validating functional requirements of ships.
/// Ensures essential systems are present, connected, and properly positioned.
/// </summary>
public class FunctionalRequirementsSystem
{
    private readonly Logger _logger = Logger.Instance;

    /// <summary>
    /// Result of functional requirements validation
    /// </summary>
    public class RequirementsResult
    {
        public bool IsValid { get; set; } = true;
        public List<string> Errors { get; set; } = new();
        public List<string> Warnings { get; set; } = new();
        
        // Component counts
        public int EngineCount { get; set; }
        public int GeneratorCount { get; set; }
        public int ThrusterCount { get; set; }
        public int ShieldGeneratorCount { get; set; }
        public int GyroCount { get; set; }
        public int CoreSystemCount { get; set; }
        
        // Connectivity checks
        public bool EnginesConnectedToPower { get; set; }
        public bool ThrustersConnectedToPower { get; set; }
        public bool ShieldsConnectedToPower { get; set; }
        
        // Positioning checks
        public bool EnginesAtRear { get; set; } = true;
        public bool ThrustersDistributed { get; set; } = true;
        public bool GeneratorsInternal { get; set; } = true;
        
        // Power balance
        public float TotalPowerGeneration { get; set; }
        public float TotalPowerConsumption { get; set; }
        public bool HasAdequatePower { get; set; }
    }

    /// <summary>
    /// Validate functional requirements of a ship
    /// </summary>
    public RequirementsResult ValidateRequirements(VoxelStructureComponent structure)
    {
        var result = new RequirementsResult();

        if (structure.Blocks.Count == 0)
        {
            result.IsValid = false;
            result.Errors.Add("Structure has no blocks");
            return result;
        }

        // Step 1: Count essential components
        CountComponents(structure, result);

        // Step 2: Validate minimum requirements
        ValidateMinimumComponents(result);

        // Step 3: Check component positioning
        ValidateComponentPositioning(structure, result);

        // Step 4: Validate power system connectivity
        ValidatePowerConnectivity(structure, result);

        // Step 5: Calculate power balance
        ValidatePowerBalance(structure, result);

        // Step 6: Validate component placement logic
        ValidateComponentPlacementLogic(structure, result);

        // Log results
        if (result.IsValid)
        {
            _logger.Info("FunctionalRequirements", "All functional requirements validated successfully");
        }
        else
        {
            _logger.Warning("FunctionalRequirements", $"Validation failed with {result.Errors.Count} errors");
        }

        return result;
    }

    /// <summary>
    /// Count essential components in the structure
    /// </summary>
    private void CountComponents(VoxelStructureComponent structure, RequirementsResult result)
    {
        foreach (var block in structure.Blocks)
        {
            switch (block.BlockType)
            {
                case BlockType.Engine:
                    result.EngineCount++;
                    break;
                case BlockType.Generator:
                    result.GeneratorCount++;
                    break;
                case BlockType.Thruster:
                    result.ThrusterCount++;
                    break;
                case BlockType.ShieldGenerator:
                    result.ShieldGeneratorCount++;
                    break;
                case BlockType.GyroArray:
                    result.GyroCount++;
                    break;
                case BlockType.HyperdriveCore:
                case BlockType.CrewQuarters:
                case BlockType.PodDocking:
                    result.CoreSystemCount++;
                    break;
            }
        }
    }

    /// <summary>
    /// Validate that minimum components are present
    /// </summary>
    private void ValidateMinimumComponents(RequirementsResult result)
    {
        if (result.EngineCount == 0)
        {
            result.Errors.Add("Ship has no engines - cannot provide forward thrust");
            result.IsValid = false;
        }

        if (result.GeneratorCount == 0)
        {
            result.Errors.Add("Ship has no power generators - systems cannot function");
            result.IsValid = false;
        }

        if (result.CoreSystemCount == 0)
        {
            result.Warnings.Add("Ship has no core system (Hyperdrive, Crew Quarters, or Pod Docking)");
        }

        if (result.ThrusterCount < 4)
        {
            result.Warnings.Add($"Ship has only {result.ThrusterCount} thrusters - recommend at least 4 for omnidirectional movement");
        }

        if (result.GyroCount == 0)
        {
            result.Warnings.Add("Ship has no gyro arrays - rotation will be limited");
        }
    }

    /// <summary>
    /// Validate component positioning (engines at rear, etc.)
    /// </summary>
    private void ValidateComponentPositioning(VoxelStructureComponent structure, RequirementsResult result)
    {
        if (structure.Blocks.Count == 0)
            return;

        // Calculate ship bounds
        var minZ = structure.Blocks.Min(b => b.Position.Z);
        var maxZ = structure.Blocks.Max(b => b.Position.Z);
        var shipLength = maxZ - minZ;

        // Check if engines are at the rear (negative Z is rear in this coordinate system)
        var engines = structure.Blocks.Where(b => b.BlockType == BlockType.Engine).ToList();
        if (engines.Count > 0)
        {
            float avgEngineZ = engines.Average(e => e.Position.Z);
            float rearThreshold = minZ + shipLength * 0.3f; // Rear 30% of ship

            if (avgEngineZ > rearThreshold)
            {
                result.EnginesAtRear = false;
                result.Warnings.Add("Engines should be positioned at the rear of the ship for optimal thrust");
            }
        }

        // Check if thrusters are distributed around the ship
        var thrusters = structure.Blocks.Where(b => b.BlockType == BlockType.Thruster).ToList();
        if (thrusters.Count >= 4)
        {
            var thrusterPositions = thrusters.Select(t => t.Position).ToList();
            float avgX = thrusterPositions.Average(p => p.X);
            float avgY = thrusterPositions.Average(p => p.Y);
            float avgZ = thrusterPositions.Average(p => p.Z);

            // Calculate variance - well distributed thrusters should have high variance
            float varianceX = thrusterPositions.Average(p => MathF.Pow(p.X - avgX, 2));
            float varianceY = thrusterPositions.Average(p => MathF.Pow(p.Y - avgY, 2));

            if (varianceX < 1.0f || varianceY < 1.0f)
            {
                result.ThrustersDistributed = false;
                result.Warnings.Add("Thrusters should be distributed around the ship for balanced movement");
            }
        }

        // Check if generators are internal (protected)
        var generators = structure.Blocks.Where(b => b.BlockType == BlockType.Generator).ToList();
        if (generators.Count > 0)
        {
            var minX = structure.Blocks.Min(b => b.Position.X);
            var maxX = structure.Blocks.Max(b => b.Position.X);
            var minY = structure.Blocks.Min(b => b.Position.Y);
            var maxY = structure.Blocks.Max(b => b.Position.Y);

            var shipWidth = maxX - minX;
            var shipHeight = maxY - minY;

            // Generators should be in the inner 60% of the ship
            var innerMarginX = shipWidth * 0.2f;
            var innerMarginY = shipHeight * 0.2f;

            foreach (var gen in generators)
            {
                if (gen.Position.X < minX + innerMarginX || gen.Position.X > maxX - innerMarginX ||
                    gen.Position.Y < minY + innerMarginY || gen.Position.Y > maxY - innerMarginY)
                {
                    result.GeneratorsInternal = false;
                    result.Warnings.Add("Generators should be placed internally for protection");
                    break;
                }
            }
        }
    }

    /// <summary>
    /// Validate that power-consuming components are connected to generators
    /// This is a logical connection check based on structural integrity
    /// </summary>
    private void ValidatePowerConnectivity(VoxelStructureComponent structure, RequirementsResult result)
    {
        // Get all generators and power-consuming blocks
        var generators = structure.Blocks.Where(b => b.BlockType == BlockType.Generator).ToList();
        var engines = structure.Blocks.Where(b => b.BlockType == BlockType.Engine).ToList();
        var thrusters = structure.Blocks.Where(b => b.BlockType == BlockType.Thruster).ToList();
        var shields = structure.Blocks.Where(b => b.BlockType == BlockType.ShieldGenerator).ToList();

        if (generators.Count == 0)
        {
            result.EnginesConnectedToPower = false;
            result.ThrustersConnectedToPower = false;
            result.ShieldsConnectedToPower = false;
            return;
        }

        // Use structural integrity to check if components are in the same connected graph
        var integrity = new StructuralIntegritySystem();
        var integrityResult = integrity.ValidateStructure(structure);

        if (!integrityResult.IsValid)
        {
            result.EnginesConnectedToPower = false;
            result.ThrustersConnectedToPower = false;
            result.ShieldsConnectedToPower = false;
            result.Errors.Add("Cannot validate power connectivity - structure has integrity issues");
            return;
        }

        // If all blocks are connected, then power can flow to all components
        result.EnginesConnectedToPower = engines.All(e => integrityResult.ConnectedBlocks.Contains(e.Id));
        result.ThrustersConnectedToPower = thrusters.All(t => integrityResult.ConnectedBlocks.Contains(t.Id));
        result.ShieldsConnectedToPower = shields.All(s => integrityResult.ConnectedBlocks.Contains(s.Id));

        if (!result.EnginesConnectedToPower)
        {
            result.Errors.Add("Some engines are not connected to the power grid");
            result.IsValid = false;
        }

        if (!result.ThrustersConnectedToPower)
        {
            result.Warnings.Add("Some thrusters are not connected to the power grid");
        }

        if (!result.ShieldsConnectedToPower)
        {
            result.Warnings.Add("Some shield generators are not connected to the power grid");
        }
    }

    /// <summary>
    /// Validate power generation vs consumption
    /// </summary>
    private void ValidatePowerBalance(VoxelStructureComponent structure, RequirementsResult result)
    {
        result.TotalPowerGeneration = structure.Blocks
            .Where(b => b.BlockType == BlockType.Generator)
            .Sum(b => b.PowerGeneration);

        // Estimate power consumption (simplified)
        float engineConsumption = structure.Blocks
            .Where(b => b.BlockType == BlockType.Engine)
            .Sum(b => b.ThrustPower * 0.5f); // Engines consume 50% of thrust as power

        float thrusterConsumption = structure.Blocks
            .Where(b => b.BlockType == BlockType.Thruster)
            .Sum(b => b.ThrustPower * 0.3f); // Thrusters consume 30% of thrust as power

        float shieldConsumption = structure.Blocks
            .Where(b => b.BlockType == BlockType.ShieldGenerator)
            .Sum(b => b.ShieldCapacity * 0.2f); // Shields consume 20% of capacity as power

        float gyroConsumption = structure.Blocks
            .Where(b => b.BlockType == BlockType.GyroArray)
            .Count() * 10f; // Gyros consume 10 power each

        result.TotalPowerConsumption = engineConsumption + thrusterConsumption + shieldConsumption + gyroConsumption;

        // Power generation should exceed consumption by at least 20%
        float powerMargin = result.TotalPowerGeneration / Math.Max(1f, result.TotalPowerConsumption);
        result.HasAdequatePower = powerMargin >= 1.2f;

        if (!result.HasAdequatePower)
        {
            if (powerMargin < 1.0f)
            {
                result.Errors.Add($"Insufficient power generation: {result.TotalPowerGeneration:F0}W generated, {result.TotalPowerConsumption:F0}W required");
                result.IsValid = false;
            }
            else
            {
                result.Warnings.Add($"Low power margin: {powerMargin:F2}x (recommend at least 1.2x)");
            }
        }
    }

    /// <summary>
    /// Validate logical component placement (engines near fuel/power, etc.)
    /// </summary>
    private void ValidateComponentPlacementLogic(VoxelStructureComponent structure, RequirementsResult result)
    {
        var engines = structure.Blocks.Where(b => b.BlockType == BlockType.Engine).ToList();
        var generators = structure.Blocks.Where(b => b.BlockType == BlockType.Generator).ToList();

        if (engines.Count == 0 || generators.Count == 0)
            return;

        // Check if engines are relatively close to at least one generator
        foreach (var engine in engines)
        {
            float minDistance = generators.Min(g => Vector3.Distance(engine.Position, g.Position));
            
            // Engines should be within 30 units of a generator
            if (minDistance > 30f)
            {
                result.Warnings.Add($"Engine at {engine.Position} is far from power source (distance: {minDistance:F1})");
            }
        }

        // Check if shield generators are distributed or centralized
        var shields = structure.Blocks.Where(b => b.BlockType == BlockType.ShieldGenerator).ToList();
        if (shields.Count >= 2)
        {
            var positions = shields.Select(s => s.Position).ToList();
            float avgDistance = 0f;
            int comparisons = 0;

            for (int i = 0; i < positions.Count; i++)
            {
                for (int j = i + 1; j < positions.Count; j++)
                {
                    avgDistance += Vector3.Distance(positions[i], positions[j]);
                    comparisons++;
                }
            }

            if (comparisons > 0)
            {
                avgDistance /= comparisons;
                
                // Well-distributed shields should be at least 10 units apart
                if (avgDistance < 10f)
                {
                    result.Warnings.Add("Shield generators are clustered - consider distributing them for better coverage");
                }
            }
        }
    }

    /// <summary>
    /// Get suggestions for missing components
    /// </summary>
    public List<string> GetComponentSuggestions(RequirementsResult result)
    {
        var suggestions = new List<string>();

        if (result.EngineCount == 0)
            suggestions.Add("Add main engines at the rear of the ship for forward thrust");

        if (result.GeneratorCount == 0)
            suggestions.Add("Add power generators in the core of the ship");

        if (result.ThrusterCount < 4)
            suggestions.Add($"Add {4 - result.ThrusterCount} more thrusters distributed around the ship");

        if (result.GyroCount == 0)
            suggestions.Add("Add gyro arrays for improved rotation control");

        if (result.ShieldGeneratorCount == 0)
            suggestions.Add("Consider adding shield generators for protection");

        if (!result.HasAdequatePower)
            suggestions.Add("Add more generators or reduce power-consuming components");

        if (!result.EnginesAtRear)
            suggestions.Add("Reposition engines closer to the rear of the ship");

        if (!result.ThrustersDistributed)
            suggestions.Add("Distribute thrusters more evenly around the ship");

        if (!result.GeneratorsInternal)
            suggestions.Add("Move generators to the interior of the ship for protection");

        return suggestions;
    }
}
