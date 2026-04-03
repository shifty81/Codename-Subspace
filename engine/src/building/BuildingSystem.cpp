#include "building/BuildingSystem.h"

#include <algorithm>
#include <cmath>

namespace subspace {

// ---------------------------------------------------------------------------
// EnhancedBuildSystem
// ---------------------------------------------------------------------------

EnhancedBuildSystem::EnhancedBuildSystem() : SystemBase("EnhancedBuildSystem") {}
EnhancedBuildSystem::EnhancedBuildSystem(EntityManager& em)
    : SystemBase("EnhancedBuildSystem"), _em(&em) {}

void EnhancedBuildSystem::SetEntityManager(EntityManager* em) { _em = em; }

void EnhancedBuildSystem::Update(float /*deltaTime*/) {}

PlacementResult EnhancedBuildSystem::PlaceBlock(uint64_t entityId,
                                                 float px, float py, float pz,
                                                 float sx, float sy, float sz) {
    if (!_em) return { false, 0, 0, "No EntityManager" };

    auto* structure = _em->GetComponent<VoxelStructureComponent>(entityId);
    if (!structure) return { false, 0, 0, "Entity has no VoxelStructureComponent" };

    auto* state = _em->GetComponent<BuilderStateComponent>(entityId);

    const std::string mat    = state ? state->selectedMaterial : "Iron";
    const BlockType   btype  = state ? state->selectedBlockType : BlockType::Hull;
    const BlockShape  bshape = state ? state->selectedShape     : BlockShape::Cube;
    const BlockOrientation orient = state ? state->selectedOrient : BlockOrientation::PosY;

    VoxelBlock primary = VoxelBlock::Make(px, py, pz, sx, sy, sz, mat, btype, bshape, orient);
    if (state && state->selectedColor)
        primary.colorRGB = state->selectedColor;

    int added = 1;
    structure->AddBlock(primary);

    // Mirror copies
    if (state) {
        for (auto& copy : MirroredCopies(*state, primary)) {
            structure->AddBlock(copy);
            ++added;
        }
    }

    return { true, added, 0, "" };
}

PlacementResult EnhancedBuildSystem::RemoveBlock(uint64_t entityId,
                                                  size_t blockIndex) {
    if (!_em) return { false, 0, 0, "No EntityManager" };
    auto* structure = _em->GetComponent<VoxelStructureComponent>(entityId);
    if (!structure) return { false, 0, 0, "Entity has no VoxelStructureComponent" };
    if (!structure->RemoveBlock(blockIndex))
        return { false, 0, 0, "Block index out of range" };
    return { true, 0, 1, "" };
}

PlacementResult EnhancedBuildSystem::PaintBlock(uint64_t entityId,
                                                  size_t blockIndex,
                                                  uint32_t newColorRGB) {
    if (!_em) return { false, 0, 0, "No EntityManager" };
    auto* structure = _em->GetComponent<VoxelStructureComponent>(entityId);
    if (!structure || blockIndex >= structure->blocks.size())
        return { false, 0, 0, "Invalid block index" };
    structure->blocks[blockIndex].colorRGB = newColorRGB;
    return { true, 0, 0, "" };
}

PlacementResult EnhancedBuildSystem::RepairBlock(uint64_t entityId,
                                                  size_t blockIndex) {
    if (!_em) return { false, 0, 0, "No EntityManager" };
    auto* structure = _em->GetComponent<VoxelStructureComponent>(entityId);
    if (!structure || blockIndex >= structure->blocks.size())
        return { false, 0, 0, "Invalid block index" };
    auto& b = structure->blocks[blockIndex];
    b.durability  = b.maxDurability;
    b.isDestroyed = false;
    return { true, 0, 0, "" };
}

void EnhancedBuildSystem::SelectBlocksInBox(uint64_t entityId,
                                             float minX, float minY, float minZ,
                                             float maxX, float maxY, float maxZ) {
    if (!_em) return;
    auto* structure = _em->GetComponent<VoxelStructureComponent>(entityId);
    auto* state     = _em->GetComponent<BuilderStateComponent>(entityId);
    if (!structure || !state) return;

    state->selectedBlocks.clear();
    for (size_t i = 0; i < structure->blocks.size(); ++i) {
        const auto& b = structure->blocks[i];
        float cx = b.px + b.sx * 0.5f;
        float cy = b.py + b.sy * 0.5f;
        float cz = b.pz + b.sz * 0.5f;
        if (cx >= minX && cx <= maxX &&
            cy >= minY && cy <= maxY &&
            cz >= minZ && cz <= maxZ)
            state->selectedBlocks.push_back(i);
    }
}

void EnhancedBuildSystem::ClearSelection(uint64_t entityId) {
    if (!_em) return;
    auto* state = _em->GetComponent<BuilderStateComponent>(entityId);
    if (state) state->selectedBlocks.clear();
}

void EnhancedBuildSystem::CopySelection(uint64_t entityId) {
    if (!_em) return;
    auto* structure = _em->GetComponent<VoxelStructureComponent>(entityId);
    auto* state     = _em->GetComponent<BuilderStateComponent>(entityId);
    if (!structure || !state) return;

    state->clipboard.clear();
    for (size_t idx : state->selectedBlocks) {
        if (idx < structure->blocks.size())
            state->clipboard.push_back(structure->blocks[idx]);
    }
}

PlacementResult EnhancedBuildSystem::PasteClipboard(uint64_t entityId,
                                                      float offsetX, float offsetY,
                                                      float offsetZ) {
    if (!_em) return { false, 0, 0, "No EntityManager" };
    auto* structure = _em->GetComponent<VoxelStructureComponent>(entityId);
    auto* state     = _em->GetComponent<BuilderStateComponent>(entityId);
    if (!structure || !state) return { false, 0, 0, "Missing components" };
    if (state->clipboard.empty()) return { false, 0, 0, "Clipboard empty" };

    int added = 0;
    for (auto block : state->clipboard) {
        block.px += offsetX;
        block.py += offsetY;
        block.pz += offsetZ;
        structure->AddBlock(block);
        ++added;
    }
    return { true, added, 0, "" };
}

std::vector<VoxelBlock>
EnhancedBuildSystem::MirroredCopies(const BuilderStateComponent& state,
                                     const VoxelBlock& orig) const {
    std::vector<VoxelBlock> copies;
    if (state.mirrorMode == MirrorAxis::None) return copies;

    auto addMirror = [&](float mx, float my, float mz) {
        VoxelBlock b = orig;
        b.px = mx; b.py = my; b.pz = mz;
        copies.push_back(b);
    };

    float ox = state.mirrorOriginX;
    float oy = state.mirrorOriginY;
    float oz = state.mirrorOriginZ;

    bool x = HasFlag(state.mirrorMode, MirrorAxis::X);
    bool y = HasFlag(state.mirrorMode, MirrorAxis::Y);
    bool z = HasFlag(state.mirrorMode, MirrorAxis::Z);

    float rx = 2.0f * ox - orig.px - orig.sx;
    float ry = 2.0f * oy - orig.py - orig.sy;
    float rz = 2.0f * oz - orig.pz - orig.sz;

    if (x)              addMirror(rx,       orig.py, orig.pz);
    if (y)              addMirror(orig.px,  ry,      orig.pz);
    if (z)              addMirror(orig.px,  orig.py, rz);
    if (x && y)         addMirror(rx,       ry,      orig.pz);
    if (x && z)         addMirror(rx,       orig.py, rz);
    if (y && z)         addMirror(orig.px,  ry,      rz);
    if (x && y && z)    addMirror(rx,       ry,      rz);

    return copies;
}

// ---------------------------------------------------------------------------
// ShipInteriorComponent
// ---------------------------------------------------------------------------

void ShipInteriorComponent::AddRoom(ShipInteriorRoom room) {
    rooms.push_back(std::move(room));
}

bool ShipInteriorComponent::RemoveRoom(const std::string& name) {
    auto it = std::find_if(rooms.begin(), rooms.end(),
        [&](const ShipInteriorRoom& r){ return r.name == name; });
    if (it == rooms.end()) return false;
    rooms.erase(it);
    return true;
}

ShipInteriorRoom* ShipInteriorComponent::FindRoom(const std::string& name) {
    for (auto& r : rooms)
        if (r.name == name) return &r;
    return nullptr;
}

int ShipInteriorComponent::TotalCrewCapacity() const {
    int total = 0;
    for (const auto& r : rooms) total += r.capacity;
    return total;
}

ComponentData ShipInteriorComponent::Serialize() const {
    ComponentData cd;
    cd.componentType = "ShipInteriorComponent";
    cd.data["roomCount"] = std::to_string(rooms.size());
    for (size_t i = 0; i < rooms.size(); ++i) {
        const auto& r = rooms[i];
        std::string p = "r" + std::to_string(i) + "_";
        cd.data[p + "name"]     = r.name;
        cd.data[p + "type"]     = r.roomType;
        cd.data[p + "cap"]      = std::to_string(r.capacity);
        cd.data[p + "functional"] = r.functional ? "1" : "0";
    }
    return cd;
}

void ShipInteriorComponent::Deserialize(const ComponentData& data) {
    rooms.clear();
    int count = 0;
    auto it = data.data.find("roomCount");
    if (it != data.data.end()) count = std::stoi(it->second);
    for (int i = 0; i < count; ++i) {
        ShipInteriorRoom r;
        std::string p = "r" + std::to_string(i) + "_";
        auto f = [&](const std::string& k, const std::string& def = "") -> std::string {
            auto jt = data.data.find(p + k);
            return (jt != data.data.end()) ? jt->second : def;
        };
        r.name       = f("name");
        r.roomType   = f("type");
        r.capacity   = std::stoi(f("cap", "0"));
        r.functional = (f("functional", "1") == "1");
        rooms.push_back(r);
    }
}

// ---------------------------------------------------------------------------
// ShipInteriorSystem
// ---------------------------------------------------------------------------

ShipInteriorSystem::ShipInteriorSystem() : SystemBase("ShipInteriorSystem") {}
ShipInteriorSystem::ShipInteriorSystem(EntityManager& em)
    : SystemBase("ShipInteriorSystem"), _em(&em) {}

void ShipInteriorSystem::SetEntityManager(EntityManager* em) { _em = em; }
void ShipInteriorSystem::Update(float /*deltaTime*/) {}

void ShipInteriorSystem::GenerateDefaultInterior(uint64_t entityId,
                                                  float shipLength,
                                                  float shipWidth,
                                                  float shipHeight) {
    if (!_em) return;

    auto* interior = _em->GetComponent<ShipInteriorComponent>(entityId);
    if (!interior) return;

    interior->rooms.clear();

    // Bridge at the front
    interior->AddRoom({ "Bridge", "Bridge",
        shipLength * 0.4f, 0.0f, 0.0f,
        4.0f, std::max(2.5f, shipHeight * 0.3f), shipWidth * 0.4f, 2, true });

    // Engine Room at the rear
    interior->AddRoom({ "Engine Room", "Engine",
        -shipLength * 0.4f, 0.0f, 0.0f,
        shipLength * 0.2f, shipHeight * 0.5f, shipWidth * 0.5f, 0, true });

    // Cargo Hold in the middle
    interior->AddRoom({ "Cargo Hold", "Cargo",
        0.0f, 0.0f, 0.0f,
        shipLength * 0.3f, shipHeight * 0.6f, shipWidth * 0.7f, 0, true });

    // Crew Quarters (small ships may omit)
    if (shipLength > 15.0f) {
        interior->AddRoom({ "Crew Quarters", "Crew",
            shipLength * 0.2f, 0.0f, 0.0f,
            shipLength * 0.15f, shipHeight * 0.4f, shipWidth * 0.5f, 4, true });
    }
}

bool ShipInteriorSystem::ValidateInterior(uint64_t entityId) const {
    if (!_em) return false;
    const auto* interior = _em->GetComponent<ShipInteriorComponent>(entityId);
    if (!interior) return false;

    bool hasBridge = false, hasEngine = false;
    for (const auto& r : interior->rooms) {
        if (r.roomType == "Bridge" && r.functional) hasBridge = true;
        if (r.roomType == "Engine" && r.functional) hasEngine = true;
    }
    return hasBridge && hasEngine;
}

} // namespace subspace
