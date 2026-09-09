#include "core/physics/SpatialHash.h"

#include <cmath>

namespace subspace {

SpatialHash::SpatialHash(float cellSize)
    : _cellSize(cellSize)
    , _inverseCellSize(1.0f / cellSize)
{
}

void SpatialHash::Clear()
{
    _cells.clear();
    _entityCells.clear();
}

SpatialHash::CellKey SpatialHash::PositionToCell(const Vector3& pos) const
{
    return {
        static_cast<int>(std::floor(pos.x * _inverseCellSize)),
        static_cast<int>(std::floor(pos.y * _inverseCellSize)),
        static_cast<int>(std::floor(pos.z * _inverseCellSize))
    };
}

void SpatialHash::Insert(EntityId id, const Vector3& position, float radius)
{
    // Remove old entry if present
    Remove(id);

    // Determine the range of cells the entity overlaps
    Vector3 minCorner(position.x - radius, position.y - radius, position.z - radius);
    Vector3 maxCorner(position.x + radius, position.y + radius, position.z + radius);

    CellKey minCell = PositionToCell(minCorner);
    CellKey maxCell = PositionToCell(maxCorner);

    std::vector<CellKey> occupiedCells;

    for (int cx = minCell.x; cx <= maxCell.x; ++cx) {
        for (int cy = minCell.y; cy <= maxCell.y; ++cy) {
            for (int cz = minCell.z; cz <= maxCell.z; ++cz) {
                CellKey key{cx, cy, cz};
                _cells[key].insert(id);
                occupiedCells.push_back(key);
            }
        }
    }

    _entityCells[id] = std::move(occupiedCells);
}

void SpatialHash::Remove(EntityId id)
{
    auto it = _entityCells.find(id);
    if (it == _entityCells.end()) return;

    for (const auto& key : it->second) {
        auto cellIt = _cells.find(key);
        if (cellIt != _cells.end()) {
            cellIt->second.erase(id);
            if (cellIt->second.empty()) {
                _cells.erase(cellIt);
            }
        }
    }

    _entityCells.erase(it);
}

std::vector<EntityId> SpatialHash::QueryNearby(const Vector3& position, float radius) const
{
    Vector3 minCorner(position.x - radius, position.y - radius, position.z - radius);
    Vector3 maxCorner(position.x + radius, position.y + radius, position.z + radius);

    CellKey minCell = PositionToCell(minCorner);
    CellKey maxCell = PositionToCell(maxCorner);

    std::unordered_set<EntityId> found;

    for (int cx = minCell.x; cx <= maxCell.x; ++cx) {
        for (int cy = minCell.y; cy <= maxCell.y; ++cy) {
            for (int cz = minCell.z; cz <= maxCell.z; ++cz) {
                CellKey key{cx, cy, cz};
                auto it = _cells.find(key);
                if (it != _cells.end()) {
                    found.insert(it->second.begin(), it->second.end());
                }
            }
        }
    }

    return {found.begin(), found.end()};
}

} // namespace subspace
