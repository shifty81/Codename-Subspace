#pragma once

#include "core/ecs/IComponent.h"
#include "core/ecs/SystemBase.h"
#include "core/ecs/EntityManager.h"
#include "core/persistence/SaveGameManager.h"
#include "ships/Block.h"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace subspace {

// ---------------------------------------------------------------------------
// BlockOrientation — which face points "up" (unique to voxel system)
// ---------------------------------------------------------------------------

enum class BlockOrientation {
    PosY,
    NegY,
    PosX,
    NegX,
    PosZ,
    NegZ
};

// ---------------------------------------------------------------------------
// Per-material property table
// ---------------------------------------------------------------------------

/// Lookup table for material-based modifiers.
struct MaterialProperties {
    float massMultiplier      = 1.0f;
    float durabilityMultiplier = 1.0f;
    float energyEfficiency    = 1.0f;
    float shieldMultiplier    = 1.0f;
    uint32_t color            = 0xB2B2B2; // default grey

    /// Get properties for a named material (Iron, Titanium, Naonite, …).
    static const MaterialProperties& Get(const std::string& material);
};

// ---------------------------------------------------------------------------
// VoxelBlock
// ---------------------------------------------------------------------------

/// Represents a single voxel block within a VoxelStructureComponent.
struct VoxelBlock {
    // Position and geometry
    float px = 0.0f, py = 0.0f, pz = 0.0f; // position (local space)
    float sx = 1.0f, sy = 1.0f, sz = 1.0f; // size
    BlockType    blockType   = BlockType::Hull;
    BlockShape   shape       = BlockShape::Cube;
    BlockOrientation orientation = BlockOrientation::PosY;

    // Material
    std::string material = "Iron";
    uint32_t    colorRGB  = 0xB2B2B2;

    // Stats
    float durability    = 100.0f;
    float maxDurability = 100.0f;
    float mass          = 1.0f;
    float thrustPower   = 0.0f;
    float powerGen      = 0.0f;
    float shieldCap     = 0.0f;
    bool  isDestroyed   = false;

    /// Default-construct and compute stats from material + blockType.
    static VoxelBlock Make(float px, float py, float pz,
                           float sx, float sy, float sz,
                           const std::string& material = "Iron",
                           BlockType type = BlockType::Hull,
                           BlockShape shape = BlockShape::Cube,
                           BlockOrientation orient = BlockOrientation::PosY);

    /// Apply damage; sets isDestroyed when durability reaches zero.
    void TakeDamage(float damage);

    /// Returns true when this block's AABB overlaps another block's AABB.
    bool Intersects(const VoxelBlock& other) const;

    /// Serialize to ComponentData map entry.
    void Serialize(ComponentData& out) const;

    /// Deserialize from ComponentData map entry (must share the same key prefix).
    static VoxelBlock Deserialize(const ComponentData& in);
};

// ---------------------------------------------------------------------------
// VoxelStructureComponent
// ---------------------------------------------------------------------------

/// ECS component that holds the voxel grid for a ship, station, or asteroid.
class VoxelStructureComponent : public IComponent {
public:
    std::vector<VoxelBlock> blocks;

    /// Add a block and return a reference to it (may reallocate).
    VoxelBlock& AddBlock(VoxelBlock block);

    /// Remove a block by index. Returns false if index is out of range.
    bool RemoveBlock(size_t index);

    /// Count of non-destroyed blocks.
    size_t ActiveBlockCount() const;

    /// Total mass of all non-destroyed blocks.
    float TotalMass() const;

    /// Sum of all block ThrustPower values.
    float TotalThrust() const;

    /// Sum of all block PowerGen values.
    float TotalPowerGen() const;

    /// Sum of all block ShieldCap values.
    float TotalShieldCap() const;

    ComponentData Serialize()   const;
    void Deserialize(const ComponentData& data);
};

// ---------------------------------------------------------------------------
// VoxelDamageComponent
// ---------------------------------------------------------------------------

/// Stores damage-visualization voxel overlays per module.
class VoxelDamageComponent : public IComponent {
public:
    /// All damage voxels (union of per-module lists).
    std::vector<VoxelBlock> damageVoxels;

    /// Per-module damage voxels indexed by module uint64 id.
    std::unordered_map<uint64_t, std::vector<VoxelBlock>> moduleDamageMap;

    /// Whether the visual overlay is active.
    bool showDamage = true;

    ComponentData Serialize()   const;
    void Deserialize(const ComponentData& data);
};

// ---------------------------------------------------------------------------
// VoxelDamageSystem
// ---------------------------------------------------------------------------

/// Updates damage-visualization overlays for voxel structures each frame.
class VoxelDamageSystem : public SystemBase {
public:
    VoxelDamageSystem();
    explicit VoxelDamageSystem(EntityManager& em);

    void Update(float deltaTime) override;
    void SetEntityManager(EntityManager* em);

    /// Apply damage to a specific block inside an entity's VoxelStructureComponent.
    void ApplyDamageToBlock(uint64_t entityId, size_t blockIndex, float damage);

    /// Repair a specific block by blockIndex.
    void RepairBlock(uint64_t entityId, size_t blockIndex, float repairAmount);

    /// Clear all damage overlays for an entity.
    void ClearDamageVisualization(uint64_t entityId);

private:
    EntityManager* _em = nullptr;

    void RebuildDamageOverlay(uint64_t entityId,
                              VoxelStructureComponent& structure,
                              VoxelDamageComponent& damage);
};

} // namespace subspace
