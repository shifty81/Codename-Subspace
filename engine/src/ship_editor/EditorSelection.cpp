#include "ship_editor/EditorSelection.h"

#include <algorithm>
#include <limits>

namespace subspace {

void EditorSelection::Add(const Vector3Int& cell) {
    m_cells.insert(cell);
}

void EditorSelection::Remove(const Vector3Int& cell) {
    m_cells.erase(cell);
}

void EditorSelection::Toggle(const Vector3Int& cell) {
    auto it = m_cells.find(cell);
    if (it != m_cells.end()) {
        m_cells.erase(it);
    } else {
        m_cells.insert(cell);
    }
}

bool EditorSelection::Contains(const Vector3Int& cell) const {
    return m_cells.count(cell) > 0;
}

void EditorSelection::SelectBox(const Vector3Int& min, const Vector3Int& max) {
    int x0 = std::min(min.x, max.x);
    int x1 = std::max(min.x, max.x);
    int y0 = std::min(min.y, max.y);
    int y1 = std::max(min.y, max.y);
    int z0 = std::min(min.z, max.z);
    int z1 = std::max(min.z, max.z);

    for (int x = x0; x <= x1; ++x) {
        for (int y = y0; y <= y1; ++y) {
            for (int z = z0; z <= z1; ++z) {
                m_cells.insert({x, y, z});
            }
        }
    }
}

void EditorSelection::Clear() {
    m_cells.clear();
}

bool EditorSelection::IsEmpty() const {
    return m_cells.empty();
}

size_t EditorSelection::Count() const {
    return m_cells.size();
}

std::vector<Block> EditorSelection::GatherBlocks(const Ship& ship) const {
    std::vector<Block> result;
    for (const auto& cell : m_cells) {
        auto it = ship.occupiedCells.find(cell);
        if (it != ship.occupiedCells.end()) {
            result.push_back(*(it->second));
        }
    }
    return result;
}

std::vector<Vector3Int> EditorSelection::GetPositions() const {
    return {m_cells.begin(), m_cells.end()};
}

bool EditorSelection::GetBounds(Vector3Int& outMin, Vector3Int& outMax) const {
    if (m_cells.empty()) {
        return false;
    }

    int minX = std::numeric_limits<int>::max();
    int minY = std::numeric_limits<int>::max();
    int minZ = std::numeric_limits<int>::max();
    int maxX = std::numeric_limits<int>::min();
    int maxY = std::numeric_limits<int>::min();
    int maxZ = std::numeric_limits<int>::min();

    for (const auto& c : m_cells) {
        minX = std::min(minX, c.x);
        minY = std::min(minY, c.y);
        minZ = std::min(minZ, c.z);
        maxX = std::max(maxX, c.x);
        maxY = std::max(maxY, c.y);
        maxZ = std::max(maxZ, c.z);
    }

    outMin = {minX, minY, minZ};
    outMax = {maxX, maxY, maxZ};
    return true;
}

} // namespace subspace
