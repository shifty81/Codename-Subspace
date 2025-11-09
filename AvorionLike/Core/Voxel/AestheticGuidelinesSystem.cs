using System.Numerics;
using AvorionLike.Core.Logging;
using AvorionLike.Core.Procedural;

namespace AvorionLike.Core.Voxel;

/// <summary>
/// System for enforcing aesthetic guidelines on ship designs.
/// Implements symmetry, balance, scale, proportion, and design language rules.
/// </summary>
public class AestheticGuidelinesSystem
{
    private readonly Logger _logger = Logger.Instance;

    /// <summary>
    /// Result of aesthetic validation
    /// </summary>
    public class AestheticResult
    {
        public bool MeetsGuidelines { get; set; } = true;
        public List<string> Suggestions { get; set; } = new();
        
        // Symmetry
        public float SymmetryScore { get; set; } // 0-1, where 1 is perfect symmetry
        public SymmetryType DetectedSymmetry { get; set; } = SymmetryType.None;
        
        // Balance
        public Vector3 CenterOfMass { get; set; }
        public Vector3 GeometricCenter { get; set; }
        public float BalanceScore { get; set; } // 0-1, where 1 is perfectly balanced
        
        // Scale and Proportion
        public Vector3 Dimensions { get; set; }
        public float AspectRatioXY { get; set; }
        public float AspectRatioYZ { get; set; }
        public float AspectRatioXZ { get; set; }
        public bool HasReasonableProportions { get; set; } = true;
        
        // Design Language
        public Dictionary<BlockType, uint> ColorsByType { get; set; } = new();
        public bool HasConsistentDesignLanguage { get; set; } = true;
        public int FunctionalColorVariety { get; set; }
    }

    public enum SymmetryType
    {
        None,
        MirrorX,    // Left-right mirror
        MirrorY,    // Top-bottom mirror
        MirrorZ,    // Front-back mirror
        Bilateral,  // X and Z symmetry
        Radial      // Rotational symmetry
    }

    /// <summary>
    /// Validate aesthetic guidelines for a ship
    /// </summary>
    public AestheticResult ValidateAesthetics(VoxelStructureComponent structure, FactionShipStyle? style = null)
    {
        var result = new AestheticResult();

        if (structure.Blocks.Count == 0)
        {
            result.MeetsGuidelines = false;
            result.Suggestions.Add("Structure has no blocks");
            return result;
        }

        // Step 1: Analyze symmetry
        AnalyzeSymmetry(structure, result, style);

        // Step 2: Check balance
        CheckBalance(structure, result);

        // Step 3: Validate proportions
        ValidateProportions(structure, result);

        // Step 4: Check design language consistency
        ValidateDesignLanguage(structure, result, style);

        // Step 5: Generate suggestions
        GenerateSuggestions(result, style);

        _logger.Info("AestheticGuidelines", 
            $"Validation complete - Symmetry: {result.SymmetryScore:F2}, Balance: {result.BalanceScore:F2}, Design Language: {result.HasConsistentDesignLanguage}");

        return result;
    }

    /// <summary>
    /// Analyze ship symmetry
    /// </summary>
    private void AnalyzeSymmetry(VoxelStructureComponent structure, AestheticResult result, FactionShipStyle? style)
    {
        if (structure.Blocks.Count == 0)
            return;

        // Calculate geometric center
        result.GeometricCenter = new Vector3(
            structure.Blocks.Average(b => b.Position.X),
            structure.Blocks.Average(b => b.Position.Y),
            structure.Blocks.Average(b => b.Position.Z)
        );

        // Check X-axis (left-right) symmetry
        float symmetryX = CalculateAxisSymmetry(structure.Blocks, result.GeometricCenter, Axis.X);
        float symmetryY = CalculateAxisSymmetry(structure.Blocks, result.GeometricCenter, Axis.Y);
        float symmetryZ = CalculateAxisSymmetry(structure.Blocks, result.GeometricCenter, Axis.Z);

        // Determine dominant symmetry type
        if (symmetryX > 0.8f && symmetryZ > 0.8f)
        {
            result.DetectedSymmetry = SymmetryType.Bilateral;
            result.SymmetryScore = (symmetryX + symmetryZ) / 2f;
        }
        else if (symmetryX > 0.8f)
        {
            result.DetectedSymmetry = SymmetryType.MirrorX;
            result.SymmetryScore = symmetryX;
        }
        else if (symmetryY > 0.8f)
        {
            result.DetectedSymmetry = SymmetryType.MirrorY;
            result.SymmetryScore = symmetryY;
        }
        else if (symmetryZ > 0.8f)
        {
            result.DetectedSymmetry = SymmetryType.MirrorZ;
            result.SymmetryScore = symmetryZ;
        }
        else
        {
            result.DetectedSymmetry = SymmetryType.None;
            result.SymmetryScore = Math.Max(symmetryX, Math.Max(symmetryY, symmetryZ));
        }

        // Check if symmetry meets style requirements
        if (style != null && style.SymmetryLevel > 0.5f && result.SymmetryScore < style.SymmetryLevel)
        {
            result.MeetsGuidelines = false;
        }
    }

    private enum Axis { X, Y, Z }

    /// <summary>
    /// Calculate symmetry score for a given axis
    /// </summary>
    private float CalculateAxisSymmetry(List<VoxelBlock> blocks, Vector3 center, Axis axis)
    {
        int matchedPairs = 0;
        int totalBlocks = blocks.Count;
        var checkedBlocks = new HashSet<Guid>();

        foreach (var block in blocks)
        {
            if (checkedBlocks.Contains(block.Id))
                continue;

            // Create mirrored position
            Vector3 mirroredPos = block.Position;
            switch (axis)
            {
                case Axis.X:
                    mirroredPos.X = 2 * center.X - block.Position.X;
                    break;
                case Axis.Y:
                    mirroredPos.Y = 2 * center.Y - block.Position.Y;
                    break;
                case Axis.Z:
                    mirroredPos.Z = 2 * center.Z - block.Position.Z;
                    break;
            }

            // Find matching block at mirrored position
            var mirroredBlock = blocks.FirstOrDefault(b => 
                !checkedBlocks.Contains(b.Id) &&
                Vector3.Distance(b.Position, mirroredPos) < 1.0f &&
                b.BlockType == block.BlockType);

            if (mirroredBlock != null)
            {
                matchedPairs++;
                checkedBlocks.Add(block.Id);
                checkedBlocks.Add(mirroredBlock.Id);
            }
        }

        // Blocks on the center plane count as symmetric
        int centerBlocks = blocks.Count(b =>
        {
            return axis switch
            {
                Axis.X => Math.Abs(b.Position.X - center.X) < 0.5f,
                Axis.Y => Math.Abs(b.Position.Y - center.Y) < 0.5f,
                Axis.Z => Math.Abs(b.Position.Z - center.Z) < 0.5f,
                _ => false
            };
        });

        float symmetryScore = (matchedPairs * 2 + centerBlocks) / (float)totalBlocks;
        return Math.Clamp(symmetryScore, 0f, 1f);
    }

    /// <summary>
    /// Check if the ship is balanced (center of mass near geometric center)
    /// </summary>
    private void CheckBalance(VoxelStructureComponent structure, AestheticResult result)
    {
        result.CenterOfMass = structure.CenterOfMass;
        
        if (result.GeometricCenter == Vector3.Zero)
        {
            result.GeometricCenter = new Vector3(
                structure.Blocks.Average(b => b.Position.X),
                structure.Blocks.Average(b => b.Position.Y),
                structure.Blocks.Average(b => b.Position.Z)
            );
        }

        // Calculate how far center of mass is from geometric center
        float distance = Vector3.Distance(result.CenterOfMass, result.GeometricCenter);
        
        // Calculate ship size for normalization
        var minPos = new Vector3(
            structure.Blocks.Min(b => b.Position.X),
            structure.Blocks.Min(b => b.Position.Y),
            structure.Blocks.Min(b => b.Position.Z)
        );
        var maxPos = new Vector3(
            structure.Blocks.Max(b => b.Position.X),
            structure.Blocks.Max(b => b.Position.Y),
            structure.Blocks.Max(b => b.Position.Z)
        );
        result.Dimensions = maxPos - minPos;
        float shipSize = result.Dimensions.Length();

        // Balance score: closer to 1 means better balance
        float normalizedDistance = distance / Math.Max(1f, shipSize);
        result.BalanceScore = Math.Max(0f, 1f - normalizedDistance * 5f); // Distance more than 20% of ship size is poor balance

        if (result.BalanceScore < 0.6f)
        {
            result.MeetsGuidelines = false;
        }
    }

    /// <summary>
    /// Validate ship proportions
    /// </summary>
    private void ValidateProportions(VoxelStructureComponent structure, AestheticResult result)
    {
        if (result.Dimensions == Vector3.Zero)
        {
            var minPos = new Vector3(
                structure.Blocks.Min(b => b.Position.X),
                structure.Blocks.Min(b => b.Position.Y),
                structure.Blocks.Min(b => b.Position.Z)
            );
            var maxPos = new Vector3(
                structure.Blocks.Max(b => b.Position.X),
                structure.Blocks.Max(b => b.Position.Y),
                structure.Blocks.Max(b => b.Position.Z)
            );
            result.Dimensions = maxPos - minPos;
        }

        // Calculate aspect ratios
        result.AspectRatioXY = result.Dimensions.X / Math.Max(1f, result.Dimensions.Y);
        result.AspectRatioYZ = result.Dimensions.Y / Math.Max(1f, result.Dimensions.Z);
        result.AspectRatioXZ = result.Dimensions.X / Math.Max(1f, result.Dimensions.Z);

        // Check for unreasonable proportions
        // Ships should not be too flat or too elongated
        const float minRatio = 0.2f;  // Not thinner than 1:5
        const float maxRatio = 5.0f;  // Not longer than 5:1

        if (result.AspectRatioXY < minRatio || result.AspectRatioXY > maxRatio ||
            result.AspectRatioYZ < minRatio || result.AspectRatioYZ > maxRatio ||
            result.AspectRatioXZ < minRatio || result.AspectRatioXZ > maxRatio)
        {
            result.HasReasonableProportions = false;
            result.MeetsGuidelines = false;
        }
    }

    /// <summary>
    /// Validate design language consistency (colors by function, etc.)
    /// </summary>
    private void ValidateDesignLanguage(VoxelStructureComponent structure, AestheticResult result, FactionShipStyle? style)
    {
        // Group blocks by type and check color consistency
        var blockTypeGroups = structure.Blocks.GroupBy(b => b.BlockType);

        foreach (var group in blockTypeGroups)
        {
            var colors = group.Select(b => b.ColorRGB).Distinct().ToList();
            
            // Store the most common color for this block type
            if (colors.Count > 0)
            {
                result.ColorsByType[group.Key] = colors
                    .GroupBy(c => c)
                    .OrderByDescending(g => g.Count())
                    .First()
                    .Key;
            }

            // If a functional block type has too many different colors, design language is inconsistent
            if (group.Key != BlockType.Hull && group.Key != BlockType.Armor && colors.Count > 2)
            {
                result.HasConsistentDesignLanguage = false;
            }
        }

        result.FunctionalColorVariety = result.ColorsByType.Count;

        // Check if functional blocks have distinct colors
        var functionalBlocks = new[] 
        { 
            BlockType.Engine, BlockType.Thruster, BlockType.Generator, 
            BlockType.ShieldGenerator, BlockType.TurretMount 
        };

        var functionalColors = result.ColorsByType
            .Where(kvp => functionalBlocks.Contains(kvp.Key))
            .Select(kvp => kvp.Value)
            .Distinct()
            .Count();

        // Good design language has distinct colors for different functional block types
        if (functionalColors < Math.Min(3, result.FunctionalColorVariety))
        {
            result.HasConsistentDesignLanguage = false;
        }

        // If style is provided, check if colors match style requirements
        if (style != null)
        {
            bool primaryColorUsed = result.ColorsByType.Values.Any(c => c == style.PrimaryColor);
            bool secondaryColorUsed = result.ColorsByType.Values.Any(c => c == style.SecondaryColor);

            if (!primaryColorUsed || !secondaryColorUsed)
            {
                result.HasConsistentDesignLanguage = false;
            }
        }
    }

    /// <summary>
    /// Generate aesthetic improvement suggestions
    /// </summary>
    private void GenerateSuggestions(AestheticResult result, FactionShipStyle? style)
    {
        // Symmetry suggestions
        if (result.SymmetryScore < 0.7f)
        {
            result.Suggestions.Add($"Consider adding symmetry to the design (current score: {result.SymmetryScore:F2})");
            
            if (style != null && style.SymmetryLevel > 0.5f)
            {
                result.Suggestions.Add($"This faction style prefers symmetrical designs (target: {style.SymmetryLevel:F2})");
            }
        }

        // Balance suggestions
        if (result.BalanceScore < 0.7f)
        {
            result.Suggestions.Add($"Ship is unbalanced - center of mass is far from geometric center (balance score: {result.BalanceScore:F2})");
            result.Suggestions.Add("Consider redistributing heavy blocks (armor, engines) more evenly");
        }

        // Proportion suggestions
        if (!result.HasReasonableProportions)
        {
            if (result.AspectRatioXY > 5f || result.AspectRatioXY < 0.2f)
            {
                result.Suggestions.Add($"Ship width/height ratio is extreme ({result.AspectRatioXY:F2}). Consider adjusting proportions.");
            }
            if (result.AspectRatioXZ > 5f || result.AspectRatioXZ < 0.2f)
            {
                result.Suggestions.Add($"Ship width/length ratio is extreme ({result.AspectRatioXZ:F2}). Consider adjusting proportions.");
            }
            if (result.AspectRatioYZ > 5f || result.AspectRatioYZ < 0.2f)
            {
                result.Suggestions.Add($"Ship height/length ratio is extreme ({result.AspectRatioYZ:F2}). Consider adjusting proportions.");
            }
        }

        // Design language suggestions
        if (!result.HasConsistentDesignLanguage)
        {
            result.Suggestions.Add("Design language is inconsistent - use distinct colors for different functional block types");
            
            if (result.FunctionalColorVariety < 3)
            {
                result.Suggestions.Add("Use more color variety to distinguish functional systems (engines, shields, weapons)");
            }
        }

        // Style-specific suggestions
        if (style != null)
        {
            if (result.DetectedSymmetry == SymmetryType.None && style.SymmetryLevel > 0.7f)
            {
                result.Suggestions.Add($"{style.FactionName} ships traditionally have {GetSymmetryDescription(style)} symmetry");
            }

            if (!result.ColorsByType.Values.Contains(style.PrimaryColor))
            {
                result.Suggestions.Add($"Consider using {style.FactionName}'s primary color scheme");
            }
        }
    }

    private string GetSymmetryDescription(FactionShipStyle style)
    {
        if (style.PreferredHullShape == ShipHullShape.Cylindrical)
            return "radial";
        else if (style.SymmetryLevel > 0.8f)
            return "bilateral";
        else if (style.SymmetryLevel > 0.5f)
            return "left-right";
        else
            return "asymmetric";
    }

    /// <summary>
    /// Apply aesthetic guidelines to improve a ship design
    /// </summary>
    public List<VoxelBlock> SuggestAestheticImprovements(VoxelStructureComponent structure, AestheticResult result)
    {
        var improvements = new List<VoxelBlock>();

        // If symmetry is low, suggest mirrored blocks
        if (result.SymmetryScore < 0.7f && result.DetectedSymmetry == SymmetryType.None)
        {
            improvements.AddRange(SuggestSymmetricBlocks(structure, result.GeometricCenter));
        }

        return improvements;
    }

    /// <summary>
    /// Suggest blocks to improve symmetry
    /// </summary>
    private List<VoxelBlock> SuggestSymmetricBlocks(VoxelStructureComponent structure, Vector3 center)
    {
        var suggestions = new List<VoxelBlock>();
        var existingPositions = new HashSet<Vector3>(structure.Blocks.Select(b => b.Position));

        // Mirror blocks across X axis
        foreach (var block in structure.Blocks.Take(10)) // Limit suggestions to avoid overwhelming
        {
            Vector3 mirroredPos = block.Position;
            mirroredPos.X = 2 * center.X - block.Position.X;

            // Only suggest if no block exists at mirrored position
            if (!existingPositions.Any(p => Vector3.Distance(p, mirroredPos) < 1.0f))
            {
                var mirroredBlock = new VoxelBlock(
                    mirroredPos,
                    block.Size,
                    block.MaterialType,
                    block.BlockType
                );
                mirroredBlock.ColorRGB = block.ColorRGB;
                suggestions.Add(mirroredBlock);
            }
        }

        return suggestions;
    }
}
