#include "ships/BlockPlacement.h"
#include "ships/ShipStats.h"

#include <unordered_set>

namespace subspace {

std::vector<Vector3Int> BlockPlacement::GetOccupiedCells(const Block& block) {
    std::vector<Vector3Int> cells;
    cells.reserve(static_cast<size_t>(block.size.x * block.size.y * block.size.z));

    for (int x = 0; x < block.size.x; ++x) {
        for (int y = 0; y < block.size.y; ++y) {
            for (int z = 0; z < block.size.z; ++z) {
                cells.push_back({block.gridPos.x + x,
                                 block.gridPos.y + y,
                                 block.gridPos.z + z});
            }
        }
    }
    return cells;
}

std::vector<Vector3Int> BlockPlacement::GetAdjacentCells(const Block& block) {
    static constexpr Vector3Int directions[6] = {
        { 1, 0, 0}, {-1, 0, 0},
        { 0, 1, 0}, { 0,-1, 0},
        { 0, 0, 1}, { 0, 0,-1}
    };

    auto occupied = GetOccupiedCells(block);
    std::unordered_set<Vector3Int> occupiedSet(occupied.begin(), occupied.end());
    std::unordered_set<Vector3Int> adjacentSet;

    for (const auto& cell : occupied) {
        for (const auto& dir : directions) {
            Vector3Int neighbor = cell + dir;
            if (occupiedSet.find(neighbor) == occupiedSet.end()) {
                adjacentSet.insert(neighbor);
            }
        }
    }

    return {adjacentSet.begin(), adjacentSet.end()};
}

bool BlockPlacement::CanPlace(const Ship& ship, const Block& block) {
    auto cells = GetOccupiedCells(block);

    // Check for overlap
    for (const auto& cell : cells) {
        if (ship.occupiedCells.find(cell) != ship.occupiedCells.end()) {
            return false;
        }
    }

    // First block always allowed
    if (ship.IsEmpty()) {
        return true;
    }

    // Must be adjacent to at least one existing block
    auto adjacent = GetAdjacentCells(block);
    for (const auto& adj : adjacent) {
        if (ship.occupiedCells.find(adj) != ship.occupiedCells.end()) {
            return true;
        }
    }

    return false;
}

bool BlockPlacement::Place(Ship& ship, std::shared_ptr<Block> block) {
    if (!block || !CanPlace(ship, *block)) {
        return false;
    }

    auto cells = GetOccupiedCells(*block);
    for (const auto& cell : cells) {
        ship.occupiedCells[cell] = block;
    }
    ship.blocks.push_back(block);

    ShipStats::Recalculate(ship);
    return true;
}

// Internal helper: place without triggering stat recalculation
static bool PlaceNoRecalc(Ship& ship, std::shared_ptr<Block> block) {
    if (!block || !BlockPlacement::CanPlace(ship, *block)) {
        return false;
    }

    auto cells = BlockPlacement::GetOccupiedCells(*block);
    for (const auto& cell : cells) {
        ship.occupiedCells[cell] = block;
    }
    ship.blocks.push_back(block);
    return true;
}

void BlockPlacement::PlaceWithSymmetry(Ship& ship, std::shared_ptr<Block> block,
                                       uint8_t symmetryFlags) {
    PlaceNoRecalc(ship, block);

    // Collect the set of blocks to mirror (starts with just the original)
    std::vector<std::shared_ptr<Block>> toMirror;
    toMirror.push_back(block);

    if (symmetryFlags & MirrorX) {
        std::vector<std::shared_ptr<Block>> newBlocks;
        for (const auto& src : toMirror) {
            auto mirrored = std::make_shared<Block>(*src);
            mirrored->gridPos.x = -src->gridPos.x - src->size.x;
            newBlocks.push_back(mirrored);
        }
        for (auto& b : newBlocks) {
            PlaceNoRecalc(ship, b);
            toMirror.push_back(b);
        }
    }

    if (symmetryFlags & MirrorY) {
        std::vector<std::shared_ptr<Block>> newBlocks;
        for (const auto& src : toMirror) {
            auto mirrored = std::make_shared<Block>(*src);
            mirrored->gridPos.y = -src->gridPos.y - src->size.y;
            newBlocks.push_back(mirrored);
        }
        for (auto& b : newBlocks) {
            PlaceNoRecalc(ship, b);
            toMirror.push_back(b);
        }
    }

    if (symmetryFlags & MirrorZ) {
        std::vector<std::shared_ptr<Block>> newBlocks;
        for (const auto& src : toMirror) {
            auto mirrored = std::make_shared<Block>(*src);
            mirrored->gridPos.z = -src->gridPos.z - src->size.z;
            newBlocks.push_back(mirrored);
        }
        for (auto& b : newBlocks) {
            PlaceNoRecalc(ship, b);
        }
    }

    // Single recalculation after all placements
    ShipStats::Recalculate(ship);
}

void BlockPlacement::Remove(Ship& ship, std::shared_ptr<Block> block) {
    if (!block) return;

    auto cells = GetOccupiedCells(*block);
    for (const auto& cell : cells) {
        ship.occupiedCells.erase(cell);
    }

    auto it = std::find(ship.blocks.begin(), ship.blocks.end(), block);
    if (it != ship.blocks.end()) {
        ship.blocks.erase(it);
    }

    ShipStats::Recalculate(ship);
}

} // namespace subspace
