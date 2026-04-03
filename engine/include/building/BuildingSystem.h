#pragma once

#include "core/ecs/IComponent.h"
#include "core/ecs/SystemBase.h"
#include "core/ecs/EntityManager.h"
#include "voxel/VoxelSystem.h"

#include <cstdint>
#include <string>
#include <vector>

namespace subspace {

// ---------------------------------------------------------------------------
// Enumerations
// ---------------------------------------------------------------------------

/// Active build-mode tool.
enum class BuildTool {
    Add,       ///< Place new blocks
    Remove,    ///< Delete existing blocks
    Paint,     ///< Repaint block colour
    Select,    ///< Box-select blocks
    Transform, ///< Change block type in-place
    Repair,    ///< Restore block durability
    Merge,     ///< Combine adjacent identical blocks
    Scale,     ///< Resize selected blocks
    Rotate,    ///< Rotate selected blocks
    Move       ///< Translate selected blocks
};

/// Axis-aligned mirror planes (bit flags).
enum class MirrorAxis : uint8_t {
    None = 0,
    X    = 1,
    Y    = 2,
    Z    = 4,
    XY   = X | Y,
    XZ   = X | Z,
    YZ   = Y | Z,
    XYZ  = X | Y | Z
};
inline MirrorAxis operator|(MirrorAxis a, MirrorAxis b) {
    return static_cast<MirrorAxis>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}
inline bool HasFlag(MirrorAxis val, MirrorAxis flag) {
    return (static_cast<uint8_t>(val) & static_cast<uint8_t>(flag)) != 0;
}

/// Coordinate space used for grid snapping.
enum class GridSpace { Local, Global, BlockCenter };

// ---------------------------------------------------------------------------
// BuilderStateComponent
// ---------------------------------------------------------------------------

/// Per-entity builder state (active tool, selection, clipboard).
struct BuilderStateComponent : public IComponent {
    BuildTool    currentTool        = BuildTool::Add;
    BlockType    selectedBlockType  = BlockType::Hull;
    BlockShape   selectedShape      = BlockShape::Cube;
    BlockOrientation selectedOrient = BlockOrientation::PosY;
    std::string  selectedMaterial   = "Iron";
    uint32_t     selectedColor      = 0x808080;

    float       gridSize            = 1.0f;   ///< Snap grid size in metres
    float       scaleStep           = 0.1f;
    GridSpace   gridSpace           = GridSpace::Local;

    MirrorAxis  mirrorMode          = MirrorAxis::None;
    float       mirrorOriginX       = 0.0f;
    float       mirrorOriginY       = 0.0f;
    float       mirrorOriginZ       = 0.0f;

    std::vector<size_t> selectedBlocks;    ///< Block indices inside structure
    std::vector<VoxelBlock> clipboard;     ///< Copy/paste buffer
};

// ---------------------------------------------------------------------------
// PlacementResult — returned by build actions
// ---------------------------------------------------------------------------

struct PlacementResult {
    bool  success    = false;
    int   blocksAdded   = 0;
    int   blocksRemoved = 0;
    std::string message;
};

// ---------------------------------------------------------------------------
// EnhancedBuildSystem
// ---------------------------------------------------------------------------

/// Full ship-editor build system: place, remove, paint, mirror, repair.
class EnhancedBuildSystem : public SystemBase {
public:
    EnhancedBuildSystem();
    explicit EnhancedBuildSystem(EntityManager& em);

    void Update(float deltaTime) override;
    void SetEntityManager(EntityManager* em);

    /// Place a block on entity's VoxelStructureComponent.
    PlacementResult PlaceBlock(uint64_t entityId,
                               float px, float py, float pz,
                               float sx, float sy, float sz);

    /// Remove block at index from entity's VoxelStructureComponent.
    PlacementResult RemoveBlock(uint64_t entityId, size_t blockIndex);

    /// Paint (recolour) a block at blockIndex.
    PlacementResult PaintBlock(uint64_t entityId, size_t blockIndex,
                               uint32_t newColorRGB);

    /// Repair a block to full durability.
    PlacementResult RepairBlock(uint64_t entityId, size_t blockIndex);

    /// Select all blocks within an AABB.
    void SelectBlocksInBox(uint64_t entityId,
                           float minX, float minY, float minZ,
                           float maxX, float maxY, float maxZ);

    /// Clear selection for an entity.
    void ClearSelection(uint64_t entityId);

    /// Copy selected blocks to clipboard.
    void CopySelection(uint64_t entityId);

    /// Paste clipboard blocks at an offset.
    PlacementResult PasteClipboard(uint64_t entityId,
                                   float offsetX, float offsetY, float offsetZ);

private:
    EntityManager* _em = nullptr;

    /// Generate mirror placements for a block depending on MirrorAxis flags.
    std::vector<VoxelBlock> MirroredCopies(const BuilderStateComponent& state,
                                           const VoxelBlock& original) const;
};

// ---------------------------------------------------------------------------
// ShipInteriorSystem
// ---------------------------------------------------------------------------

/// Manages interior room / module layout within a ship hull.
class ShipInteriorRoom {
public:
    std::string name;
    std::string roomType;          ///< "Bridge", "Engine", "Cargo", "Crew", …
    float px = 0, py = 0, pz = 0; ///< Position inside ship (local)
    float sx = 5, sy = 3, sz = 5; ///< Dimensions
    int   capacity = 0;            ///< e.g. crew capacity
    bool  functional = true;
};

/// Component that holds a ship's interior room layout.
class ShipInteriorComponent : public IComponent {
public:
    std::vector<ShipInteriorRoom> rooms;

    void AddRoom(ShipInteriorRoom room);
    bool RemoveRoom(const std::string& name);
    ShipInteriorRoom* FindRoom(const std::string& name);
    int TotalCrewCapacity() const;

    ComponentData Serialize()   const;
    void Deserialize(const ComponentData& data);
};

/// System that initialises and validates ship interior layouts.
class ShipInteriorSystem : public SystemBase {
public:
    ShipInteriorSystem();
    explicit ShipInteriorSystem(EntityManager& em);

    void Update(float deltaTime) override;
    void SetEntityManager(EntityManager* em);

    /// Generate a default interior for a freshly created ship hull.
    void GenerateDefaultInterior(uint64_t entityId, float shipLength,
                                 float shipWidth, float shipHeight);

    /// Validate that required rooms (Bridge, Engine Room) are present.
    bool ValidateInterior(uint64_t entityId) const;

private:
    EntityManager* _em = nullptr;
};

} // namespace subspace
