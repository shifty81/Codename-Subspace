#pragma once

#include "ships/Block.h"
#include "ships/Ship.h"

#include <cstdint>
#include <vector>

namespace subspace {

enum class CommandType { PlaceBlock, RemoveBlock, PaintBlock };

struct BuildCommand {
    CommandType type;
    Block block;
    Vector3Int targetCell;
    MaterialType paintMaterial;
    uint8_t symmetryFlags;

    // Validate this command against a ship
    bool Validate(const Ship& ship) const;

    // Apply this command to a ship
    bool Apply(Ship& ship) const;
};

class ShipReplication {
public:
    // Create a build command for placing a block
    static BuildCommand CreatePlaceCommand(const Block& block, uint8_t symmetry);

    // Create a remove command
    static BuildCommand CreateRemoveCommand(const Vector3Int& cell);

    // Create a paint command
    static BuildCommand CreatePaintCommand(const Vector3Int& cell, MaterialType material);

    // Serialize/Deserialize commands (simple binary format)
    static std::vector<uint8_t> Serialize(const BuildCommand& cmd);
    static BuildCommand Deserialize(const std::vector<uint8_t>& data);
};

} // namespace subspace
