#pragma once

#include "core/Math.h"
#include "core/ecs/Entity.h"

#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace subspace {

/// Grid-based spatial hash for broad-phase collision detection.
/// Divides 3D space into uniform cells and tracks which entities
/// occupy each cell, enabling efficient neighbor queries in O(1)
/// average case instead of O(n²) brute-force.
class SpatialHash {
public:
    /// Construct a spatial hash with the given cell size.
    /// Cell size should be >= the largest collision radius * 2.
    explicit SpatialHash(float cellSize = 50.0f);

    /// Remove all entries from the grid.
    void Clear();

    /// Insert an entity at the given position with a collision radius.
    void Insert(EntityId id, const Vector3& position, float radius);

    /// Remove an entity from the grid.
    void Remove(EntityId id);

    /// Query all entity IDs within range of the given position.
    /// Returns entities whose cells overlap the query sphere.
    std::vector<EntityId> QueryNearby(const Vector3& position, float radius) const;

    /// Get the cell size used by this hash.
    float GetCellSize() const { return _cellSize; }

    /// Get the number of occupied cells.
    size_t GetCellCount() const { return _cells.size(); }

    /// Get total number of tracked entities.
    size_t GetEntityCount() const { return _entityCells.size(); }

private:
    struct CellKey {
        int x, y, z;
        bool operator==(const CellKey& o) const {
            return x == o.x && y == o.y && z == o.z;
        }
    };

    struct CellKeyHash {
        std::size_t operator()(const CellKey& k) const noexcept {
            std::size_t h = 0;
            h ^= std::hash<int>{}(k.x) + 0x9e3779b9 + (h << 6) + (h >> 2);
            h ^= std::hash<int>{}(k.y) + 0x9e3779b9 + (h << 6) + (h >> 2);
            h ^= std::hash<int>{}(k.z) + 0x9e3779b9 + (h << 6) + (h >> 2);
            return h;
        }
    };

    CellKey PositionToCell(const Vector3& pos) const;

    float _cellSize;
    float _inverseCellSize;

    // cell -> set of entity IDs in that cell
    std::unordered_map<CellKey, std::unordered_set<EntityId>, CellKeyHash> _cells;

    // entity -> list of cells it occupies (for removal)
    std::unordered_map<EntityId, std::vector<CellKey>> _entityCells;
};

} // namespace subspace
