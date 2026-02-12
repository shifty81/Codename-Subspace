#pragma once

#include "core/Math.h"
#include "ships/Block.h"
#include "ships/Ship.h"
#include "ship_editor/ShipEditorState.h"

namespace subspace {

class ShipEditorController {
public:
    explicit ShipEditorController(Ship& ship);

    ShipEditorState& GetState();
    const ShipEditorState& GetState() const;

    // Build a preview block from the current editor state
    Block BuildGhostBlock() const;

    // Check if the ghost block can be placed
    bool CanPlaceGhost() const;

    // Place the current ghost block (with symmetry)
    bool Place();

    // Remove block at current hover cell
    bool RemoveAtHover();

    // Paint block at hover cell with current material
    bool PaintAtHover();

    void SetHoverCell(const Vector3Int& cell);

    Ship& GetShip();
    const Ship& GetShip() const;

private:
    Ship& m_ship;
    ShipEditorState m_state;
};

} // namespace subspace
