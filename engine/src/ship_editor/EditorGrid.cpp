#include "ship_editor/EditorGrid.h"

#include <cmath>
#include <algorithm>

namespace subspace {

EditorGrid::EditorGrid() = default;

EditorGrid::EditorGrid(int cellSize)
    : m_cellSize(std::max(1, cellSize)) {}

Vector3Int EditorGrid::SnapToGrid(float worldX, float worldY, float worldZ) const {
    auto snap = [&](float v) -> int {
        return static_cast<int>(std::floor(v / static_cast<float>(m_cellSize)));
    };
    return {snap(worldX), snap(worldY), snap(worldZ)};
}

Vector3Int EditorGrid::SnapToGrid(const Vector3& worldPos) const {
    return SnapToGrid(worldPos.x, worldPos.y, worldPos.z);
}

Vector3 EditorGrid::CellToWorld(const Vector3Int& cell) const {
    float half = static_cast<float>(m_cellSize) * 0.5f;
    return {
        static_cast<float>(cell.x * m_cellSize) + half,
        static_cast<float>(cell.y * m_cellSize) + half,
        static_cast<float>(cell.z * m_cellSize) + half
    };
}

int EditorGrid::GetCellSize() const {
    return m_cellSize;
}

void EditorGrid::SetCellSize(int size) {
    m_cellSize = std::max(1, size);
}

bool EditorGrid::IsVisible() const {
    return m_visible;
}

void EditorGrid::SetVisible(bool visible) {
    m_visible = visible;
}

int EditorGrid::GetExtent() const {
    return m_extent;
}

void EditorGrid::SetExtent(int extent) {
    m_extent = std::max(1, extent);
}

} // namespace subspace
