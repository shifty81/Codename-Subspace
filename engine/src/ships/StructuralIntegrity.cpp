#include "ships/StructuralIntegrity.h"
#include "ships/BlockPlacement.h"

#include <queue>
#include <unordered_map>
#include <unordered_set>

namespace subspace {

// ---------------------------------------------------------------------------
// BFS flood-fill through the occupiedCells adjacency map.
// When a cell is visited, ALL cells belonging to the same block are visited
// so that multi-cell blocks are treated as single graph nodes.
// ---------------------------------------------------------------------------

static const Vector3Int kDirections[6] = {
    { 1,  0,  0}, {-1,  0,  0},
    { 0,  1,  0}, { 0, -1,  0},
    { 0,  0,  1}, { 0,  0, -1}
};

/// Flood-fill from a starting cell. Populates `visitedCells` and returns the
/// set of unique Block pointers reached.
static std::vector<std::shared_ptr<Block>> FloodFill(
    const std::unordered_map<Vector3Int, std::shared_ptr<Block>>& cells,
    const Vector3Int& start,
    std::unordered_set<Vector3Int>& visitedCells)
{
    std::vector<std::shared_ptr<Block>> group;
    std::unordered_set<Block*> visitedBlocks;
    std::queue<Vector3Int> frontier;

    frontier.push(start);
    visitedCells.insert(start);

    while (!frontier.empty()) {
        Vector3Int cur = frontier.front();
        frontier.pop();

        auto cellIt = cells.find(cur);
        if (cellIt == cells.end()) continue;

        auto& block = cellIt->second;
        if (visitedBlocks.insert(block.get()).second) {
            group.push_back(block);

            // Enqueue every cell this multi-cell block occupies
            auto occupied = BlockPlacement::GetOccupiedCells(*block);
            for (const auto& oc : occupied) {
                if (visitedCells.insert(oc).second) {
                    frontier.push(oc);
                }
            }
        }

        // Explore 6-directional neighbors
        for (const auto& dir : kDirections) {
            Vector3Int neighbor = cur + dir;
            if (visitedCells.count(neighbor) == 0 && cells.count(neighbor) > 0) {
                visitedCells.insert(neighbor);
                frontier.push(neighbor);
            }
        }
    }

    return group;
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

bool StructuralIntegrity::IsFullyConnected(const Ship& ship) {
    if (ship.occupiedCells.empty()) return true;

    std::unordered_set<Vector3Int> visited;
    FloodFill(ship.occupiedCells, ship.occupiedCells.begin()->first, visited);

    return visited.size() == ship.occupiedCells.size();
}

std::vector<std::vector<std::shared_ptr<Block>>> StructuralIntegrity::FindDisconnectedGroups(const Ship& ship) {
    std::vector<std::vector<std::shared_ptr<Block>>> groups;
    std::unordered_set<Vector3Int> visited;

    for (const auto& [pos, block] : ship.occupiedCells) {
        if (visited.count(pos) == 0) {
            groups.push_back(FloodFill(ship.occupiedCells, pos, visited));
        }
    }

    // Sort groups so the largest (main hull) comes first
    std::sort(groups.begin(), groups.end(),
              [](const auto& a, const auto& b) { return a.size() > b.size(); });

    return groups;
}

bool StructuralIntegrity::WouldDisconnect(const Ship& ship, std::shared_ptr<Block> block) {
    if (!block) return false;

    // Build a temporary cell map without the target block's cells
    auto removedCells = BlockPlacement::GetOccupiedCells(*block);
    std::unordered_set<Vector3Int> removedSet(removedCells.begin(), removedCells.end());

    std::unordered_map<Vector3Int, std::shared_ptr<Block>> tempCells;
    for (const auto& [pos, b] : ship.occupiedCells) {
        if (removedSet.count(pos) == 0) {
            tempCells[pos] = b;
        }
    }

    if (tempCells.empty()) return false;

    // Flood-fill from the first remaining cell
    std::unordered_set<Vector3Int> visited;
    FloodFill(tempCells, tempCells.begin()->first, visited);

    return visited.size() != tempCells.size();
}

} // namespace subspace
