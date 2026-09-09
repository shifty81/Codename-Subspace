#include "ship_editor/ShipValidator.h"

#include <queue>
#include <unordered_set>

namespace subspace {

// ---------- ValidationResult helpers ----------

void ValidationResult::AddError(const std::string& msg) {
    valid = false;
    errors.push_back(msg);
}

void ValidationResult::AddWarning(const std::string& msg) {
    warnings.push_back(msg);
}

// ---------- ShipValidator ----------

ValidationResult ShipValidator::Validate(const Ship& ship) {
    ValidationResult result;

    if (!HasBlocks(ship)) {
        result.AddError("Ship has no blocks");
        return result;  // all other checks are meaningless
    }

    if (!IsConnected(ship)) {
        result.AddError("Ship contains disconnected blocks");
    }

    if (!HasEngine(ship)) {
        result.AddWarning("Ship has no engine block — it will not move");
    }

    if (!HasGenerator(ship)) {
        result.AddWarning("Ship has no generator — it will not produce power");
    }

    return result;
}

bool ShipValidator::HasBlocks(const Ship& ship) {
    return !ship.IsEmpty();
}

bool ShipValidator::IsConnected(const Ship& ship) {
    if (ship.blocks.empty()) return true;

    // BFS from the first block; if we visit all blocks, the ship is connected.
    std::unordered_set<Vector3Int> visited;
    std::queue<Vector3Int> frontier;

    const Vector3Int start = ship.blocks.front()->gridPos;
    frontier.push(start);
    visited.insert(start);

    const Vector3Int directions[6] = {
        { 1, 0, 0}, {-1, 0, 0},
        { 0, 1, 0}, { 0,-1, 0},
        { 0, 0, 1}, { 0, 0,-1}
    };

    while (!frontier.empty()) {
        Vector3Int pos = frontier.front();
        frontier.pop();

        for (const auto& d : directions) {
            Vector3Int neighbor = pos + d;
            if (visited.count(neighbor) == 0 &&
                ship.occupiedCells.count(neighbor) > 0) {
                visited.insert(neighbor);
                frontier.push(neighbor);
            }
        }
    }

    return visited.size() == ship.occupiedCells.size();
}

bool ShipValidator::HasEngine(const Ship& ship) {
    for (const auto& b : ship.blocks) {
        if (b->type == BlockType::Engine) return true;
    }
    return false;
}

bool ShipValidator::HasGenerator(const Ship& ship) {
    for (const auto& b : ship.blocks) {
        if (b->type == BlockType::Generator) return true;
    }
    return false;
}

bool ShipValidator::MassWithinLimit(const Ship& ship, float maxMass) {
    return ship.totalMass <= maxMass;
}

bool ShipValidator::BlockCountWithinLimit(const Ship& ship, size_t maxBlocks) {
    return ship.BlockCount() <= maxBlocks;
}

} // namespace subspace
