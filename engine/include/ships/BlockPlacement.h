#pragma once

#include "ships/Block.h"
#include "ships/Ship.h"

#include <cstdint>
#include <memory>
#include <vector>

namespace subspace {

class BlockPlacement {
public:
    static constexpr uint8_t MirrorX = 1;
    static constexpr uint8_t MirrorY = 2;
    static constexpr uint8_t MirrorZ = 4;

    // Get all grid cells occupied by a block (considering its size)
    static std::vector<Vector3Int> GetOccupiedCells(const Block& block);

    // Get all adjacent cells (neighbors not occupied by this block)
    static std::vector<Vector3Int> GetAdjacentCells(const Block& block);

    // Check if a block can be placed
    static bool CanPlace(const Ship& ship, const Block& block);

    // Place a block (returns false if placement invalid)
    static bool Place(Ship& ship, std::shared_ptr<Block> block);

    // Place with symmetry (MirrorX=1, MirrorY=2, MirrorZ=4)
    static void PlaceWithSymmetry(Ship& ship, std::shared_ptr<Block> block, uint8_t symmetryFlags);

    // Remove a block
    static void Remove(Ship& ship, std::shared_ptr<Block> block);
};

} // namespace subspace
