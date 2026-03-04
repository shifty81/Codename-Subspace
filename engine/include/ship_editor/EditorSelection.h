#pragma once

#include "ships/Block.h"
#include "ships/Ship.h"

#include <memory>
#include <unordered_set>
#include <vector>

namespace subspace {

// Manages a set of selected block positions inside the editor.
class EditorSelection {
public:
    void Add(const Vector3Int& cell);
    void Remove(const Vector3Int& cell);
    void Toggle(const Vector3Int& cell);
    bool Contains(const Vector3Int& cell) const;

    // Select all cells in the axis-aligned box from min to max (inclusive).
    void SelectBox(const Vector3Int& min, const Vector3Int& max);

    void Clear();
    bool IsEmpty() const;
    size_t Count() const;

    // Gather the Block objects that match the selected cells.
    std::vector<Block> GatherBlocks(const Ship& ship) const;

    // Return all selected positions.
    std::vector<Vector3Int> GetPositions() const;

    // Compute the axis-aligned bounding box of the selection.
    // Returns false when the selection is empty.
    bool GetBounds(Vector3Int& outMin, Vector3Int& outMax) const;

private:
    std::unordered_set<Vector3Int> m_cells;
};

} // namespace subspace

// Hash for unordered_set<Vector3Int> is already provided via core/Math.h
