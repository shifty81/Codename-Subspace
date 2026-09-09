#pragma once

#include "core/Math.h"

namespace subspace {

// Provides grid snapping utilities for the ship editor.
class EditorGrid {
public:
    EditorGrid();
    explicit EditorGrid(int cellSize);

    // Snap a world-space position to the nearest grid cell.
    Vector3Int SnapToGrid(float worldX, float worldY, float worldZ) const;
    Vector3Int SnapToGrid(const Vector3& worldPos) const;

    // Convert a grid cell to its world-space center.
    Vector3 CellToWorld(const Vector3Int& cell) const;

    // Return the world-space size of one cell.
    int GetCellSize() const;
    void SetCellSize(int size);

    bool IsVisible() const;
    void SetVisible(bool visible);

    // Grid extent (half-width in cells) used for rendering.
    int GetExtent() const;
    void SetExtent(int extent);

private:
    int m_cellSize = 1;
    bool m_visible = true;
    int m_extent = 50;
};

} // namespace subspace
