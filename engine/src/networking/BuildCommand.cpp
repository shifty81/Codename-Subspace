#include "networking/BuildCommand.h"
#include "ships/BlockPlacement.h"

#include <algorithm>
#include <cstring>

namespace subspace {

// ---------------------------------------------------------------------------
// BuildCommand
// ---------------------------------------------------------------------------
bool BuildCommand::Validate(const Ship& ship) const {
    switch (type) {
        case CommandType::PlaceBlock:
            return BlockPlacement::CanPlace(ship, block);

        case CommandType::RemoveBlock:
            return ship.occupiedCells.find(targetCell) != ship.occupiedCells.end();

        case CommandType::PaintBlock:
            return ship.occupiedCells.find(targetCell) != ship.occupiedCells.end();
    }
    return false;
}

bool BuildCommand::Apply(Ship& ship) const {
    if (!Validate(ship)) return false;

    switch (type) {
        case CommandType::PlaceBlock: {
            auto newBlock = std::make_shared<Block>(block);
            BlockPlacement::PlaceWithSymmetry(ship, newBlock, symmetryFlags);
            return true;
        }

        case CommandType::RemoveBlock: {
            auto it = ship.occupiedCells.find(targetCell);
            if (it == ship.occupiedCells.end()) return false;
            BlockPlacement::Remove(ship, it->second);
            return true;
        }

        case CommandType::PaintBlock: {
            auto it = ship.occupiedCells.find(targetCell);
            if (it == ship.occupiedCells.end()) return false;
            it->second->material = paintMaterial;
            return true;
        }
    }
    return false;
}

// ---------------------------------------------------------------------------
// ShipReplication
// ---------------------------------------------------------------------------
BuildCommand ShipReplication::CreatePlaceCommand(const Block& block, uint8_t symmetry) {
    BuildCommand cmd{};
    cmd.type = CommandType::PlaceBlock;
    cmd.block = block;
    cmd.symmetryFlags = symmetry;
    return cmd;
}

BuildCommand ShipReplication::CreateRemoveCommand(const Vector3Int& cell) {
    BuildCommand cmd{};
    cmd.type = CommandType::RemoveBlock;
    cmd.targetCell = cell;
    return cmd;
}

BuildCommand ShipReplication::CreatePaintCommand(const Vector3Int& cell, MaterialType material) {
    BuildCommand cmd{};
    cmd.type = CommandType::PaintBlock;
    cmd.targetCell = cell;
    cmd.paintMaterial = material;
    return cmd;
}

std::vector<uint8_t> ShipReplication::Serialize(const BuildCommand& cmd) {
    std::vector<uint8_t> data;

    auto writeRaw = [&](const void* src, size_t len) {
        const auto* bytes = static_cast<const uint8_t*>(src);
        data.insert(data.end(), bytes, bytes + len);
    };

    auto cmdType = static_cast<int32_t>(cmd.type);
    writeRaw(&cmdType, sizeof(cmdType));

    // Block data
    writeRaw(&cmd.block.gridPos.x, sizeof(int));
    writeRaw(&cmd.block.gridPos.y, sizeof(int));
    writeRaw(&cmd.block.gridPos.z, sizeof(int));
    writeRaw(&cmd.block.size.x, sizeof(int));
    writeRaw(&cmd.block.size.y, sizeof(int));
    writeRaw(&cmd.block.size.z, sizeof(int));
    writeRaw(&cmd.block.rotationIndex, sizeof(int));

    auto shape = static_cast<int32_t>(cmd.block.shape);
    auto btype = static_cast<int32_t>(cmd.block.type);
    auto mat   = static_cast<int32_t>(cmd.block.material);
    writeRaw(&shape, sizeof(int32_t));
    writeRaw(&btype, sizeof(int32_t));
    writeRaw(&mat, sizeof(int32_t));

    // Target cell
    writeRaw(&cmd.targetCell.x, sizeof(int));
    writeRaw(&cmd.targetCell.y, sizeof(int));
    writeRaw(&cmd.targetCell.z, sizeof(int));

    // Paint material
    auto paintMat = static_cast<int32_t>(cmd.paintMaterial);
    writeRaw(&paintMat, sizeof(int32_t));

    // Symmetry
    writeRaw(&cmd.symmetryFlags, sizeof(uint8_t));

    return data;
}

BuildCommand ShipReplication::Deserialize(const std::vector<uint8_t>& data) {
    BuildCommand cmd{};
    size_t offset = 0;

    auto readRaw = [&](void* dst, size_t len) {
        if (offset + len <= data.size()) {
            std::memcpy(dst, data.data() + offset, len);
        }
        offset += len;
    };

    int32_t cmdType = 0;
    readRaw(&cmdType, sizeof(cmdType));
    cmd.type = static_cast<CommandType>(cmdType);

    readRaw(&cmd.block.gridPos.x, sizeof(int));
    readRaw(&cmd.block.gridPos.y, sizeof(int));
    readRaw(&cmd.block.gridPos.z, sizeof(int));
    readRaw(&cmd.block.size.x, sizeof(int));
    readRaw(&cmd.block.size.y, sizeof(int));
    readRaw(&cmd.block.size.z, sizeof(int));
    readRaw(&cmd.block.rotationIndex, sizeof(int));

    int32_t shape = 0, btype = 0, mat = 0;
    readRaw(&shape, sizeof(int32_t));
    readRaw(&btype, sizeof(int32_t));
    readRaw(&mat, sizeof(int32_t));
    cmd.block.shape    = static_cast<BlockShape>(shape);
    cmd.block.type     = static_cast<BlockType>(btype);
    cmd.block.material = static_cast<MaterialType>(mat);

    readRaw(&cmd.targetCell.x, sizeof(int));
    readRaw(&cmd.targetCell.y, sizeof(int));
    readRaw(&cmd.targetCell.z, sizeof(int));

    int32_t paintMat = 0;
    readRaw(&paintMat, sizeof(int32_t));
    cmd.paintMaterial = static_cast<MaterialType>(paintMat);

    readRaw(&cmd.symmetryFlags, sizeof(uint8_t));

    return cmd;
}

} // namespace subspace
