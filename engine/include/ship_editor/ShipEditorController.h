#pragma once

#include "core/Math.h"
#include "ships/Block.h"
#include "ships/Ship.h"
#include "ship_editor/ShipEditorState.h"
#include "ship_editor/EditorAction.h"
#include "ship_editor/EditorClipboard.h"
#include "ship_editor/EditorSelection.h"
#include "ship_editor/ShipValidator.h"
#include "ship_editor/BlockPalette.h"
#include "ship_editor/EditorGrid.h"

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

    // ---- Undo / Redo ----
    bool Undo();
    bool Redo();
    bool CanUndo() const;
    bool CanRedo() const;
    EditorHistory& GetHistory();

    // ---- Selection ----
    EditorSelection& GetSelection();
    const EditorSelection& GetSelection() const;

    // ---- Clipboard ----
    void CopySelection();
    void CutSelection();
    bool PasteAtHover();
    EditorClipboard& GetClipboard();

    // ---- Validation ----
    ValidationResult ValidateShip() const;

    // ---- Block palette ----
    const BlockPalette& GetPalette() const;

    // ---- Grid ----
    EditorGrid& GetGrid();
    const EditorGrid& GetGrid() const;

    // ---- Remove selection ----
    bool RemoveSelected();

    Ship& GetShip();
    const Ship& GetShip() const;

private:
    Ship& m_ship;
    ShipEditorState m_state;
    EditorHistory m_history;
    EditorSelection m_selection;
    EditorClipboard m_clipboard;
    BlockPalette m_palette;
    EditorGrid m_grid;
};

} // namespace subspace
