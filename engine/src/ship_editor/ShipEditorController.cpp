#include "ship_editor/ShipEditorController.h"
#include "ships/BlockPlacement.h"
#include "ships/ShipDamage.h"
#include "ships/ShipStats.h"

#include <memory>

namespace subspace {

ShipEditorController::ShipEditorController(Ship& ship)
    : m_ship(ship) {}

ShipEditorState& ShipEditorController::GetState() {
    return m_state;
}

const ShipEditorState& ShipEditorController::GetState() const {
    return m_state;
}

Block ShipEditorController::BuildGhostBlock() const {
    Block ghost{};
    ghost.gridPos = m_state.hoverCell;
    ghost.size = m_state.blockSize;
    ghost.rotationIndex = m_state.rotationIndex;
    ghost.shape = m_state.selectedShape;
    ghost.type = m_state.selectedType;
    ghost.material = m_state.selectedMaterial;
    ghost.maxHP = GetBlockBaseHP(m_state.selectedType);
    ghost.currentHP = ghost.maxHP;
    return ghost;
}

bool ShipEditorController::CanPlaceGhost() const {
    Block ghost = BuildGhostBlock();
    return BlockPlacement::CanPlace(m_ship, ghost);
}

bool ShipEditorController::Place() {
    Block ghost = BuildGhostBlock();
    if (!BlockPlacement::CanPlace(m_ship, ghost)) {
        return false;
    }

    auto blockPtr = std::make_shared<Block>(ghost);
    BlockPlacement::PlaceWithSymmetry(m_ship, blockPtr, m_state.symmetry);
    m_history.PushAction(EditorAction::MakePlaceAction(ghost));
    return true;
}

bool ShipEditorController::RemoveAtHover() {
    auto it = m_ship.occupiedCells.find(m_state.hoverCell);
    if (it == m_ship.occupiedCells.end()) {
        return false;
    }

    Block removed = *(it->second);
    ShipDamage::RemoveBlock(m_ship, it->second);
    m_history.PushAction(EditorAction::MakeRemoveAction(removed));
    return true;
}

bool ShipEditorController::PaintAtHover() {
    auto it = m_ship.occupiedCells.find(m_state.hoverCell);
    if (it == m_ship.occupiedCells.end()) {
        return false;
    }

    MaterialType oldMat = it->second->material;
    it->second->material = m_state.selectedMaterial;
    m_history.PushAction(EditorAction::MakePaintAction(*(it->second), oldMat));
    return true;
}

void ShipEditorController::SetHoverCell(const Vector3Int& cell) {
    m_state.hoverCell = cell;
}

// ---- Undo / Redo ----

bool ShipEditorController::CanUndo() const {
    return m_history.CanUndo();
}

bool ShipEditorController::CanRedo() const {
    return m_history.CanRedo();
}

bool ShipEditorController::Undo() {
    if (!m_history.CanUndo()) return false;

    EditorAction action = m_history.Undo();
    switch (action.type) {
        case EditorActionType::PlaceBlock: {
            // Undo a placement → remove the block
            auto it = m_ship.occupiedCells.find(action.blockData.gridPos);
            if (it != m_ship.occupiedCells.end()) {
                ShipDamage::RemoveBlock(m_ship, it->second);
            }
            break;
        }
        case EditorActionType::RemoveBlock: {
            // Undo a removal → re-place the block
            auto ptr = std::make_shared<Block>(action.blockData);
            BlockPlacement::Place(m_ship, ptr);
            break;
        }
        case EditorActionType::PaintBlock: {
            // Undo a paint → restore previous material
            auto it = m_ship.occupiedCells.find(action.blockData.gridPos);
            if (it != m_ship.occupiedCells.end()) {
                it->second->material = action.previousMaterial;
            }
            break;
        }
        case EditorActionType::MultiPlace: {
            for (const auto& b : action.multiBlocks) {
                auto it = m_ship.occupiedCells.find(b.gridPos);
                if (it != m_ship.occupiedCells.end()) {
                    ShipDamage::RemoveBlock(m_ship, it->second);
                }
            }
            break;
        }
        case EditorActionType::MultiRemove: {
            for (const auto& b : action.multiBlocks) {
                auto ptr = std::make_shared<Block>(b);
                BlockPlacement::Place(m_ship, ptr);
            }
            break;
        }
    }
    return true;
}

bool ShipEditorController::Redo() {
    if (!m_history.CanRedo()) return false;

    EditorAction action = m_history.Redo();
    switch (action.type) {
        case EditorActionType::PlaceBlock: {
            auto ptr = std::make_shared<Block>(action.blockData);
            BlockPlacement::Place(m_ship, ptr);
            break;
        }
        case EditorActionType::RemoveBlock: {
            auto it = m_ship.occupiedCells.find(action.blockData.gridPos);
            if (it != m_ship.occupiedCells.end()) {
                ShipDamage::RemoveBlock(m_ship, it->second);
            }
            break;
        }
        case EditorActionType::PaintBlock: {
            auto it = m_ship.occupiedCells.find(action.blockData.gridPos);
            if (it != m_ship.occupiedCells.end()) {
                it->second->material = action.blockData.material;
            }
            break;
        }
        case EditorActionType::MultiPlace: {
            for (const auto& b : action.multiBlocks) {
                auto ptr = std::make_shared<Block>(b);
                BlockPlacement::Place(m_ship, ptr);
            }
            break;
        }
        case EditorActionType::MultiRemove: {
            for (const auto& b : action.multiBlocks) {
                auto it = m_ship.occupiedCells.find(b.gridPos);
                if (it != m_ship.occupiedCells.end()) {
                    ShipDamage::RemoveBlock(m_ship, it->second);
                }
            }
            break;
        }
    }
    return true;
}

EditorHistory& ShipEditorController::GetHistory() {
    return m_history;
}

// ---- Selection ----

EditorSelection& ShipEditorController::GetSelection() {
    return m_selection;
}

const EditorSelection& ShipEditorController::GetSelection() const {
    return m_selection;
}

// ---- Clipboard ----

void ShipEditorController::CopySelection() {
    auto blocks = m_selection.GatherBlocks(m_ship);
    if (blocks.empty()) return;

    Vector3Int bMin, bMax;
    m_selection.GetBounds(bMin, bMax);
    m_clipboard.Copy(blocks, bMin);
}

void ShipEditorController::CutSelection() {
    auto blocks = m_selection.GatherBlocks(m_ship);
    if (blocks.empty()) return;

    Vector3Int bMin, bMax;
    m_selection.GetBounds(bMin, bMax);
    m_clipboard.Copy(blocks, bMin);

    // Remove selected blocks and record as multi-remove
    for (const auto& b : blocks) {
        auto it = m_ship.occupiedCells.find(b.gridPos);
        if (it != m_ship.occupiedCells.end()) {
            ShipDamage::RemoveBlock(m_ship, it->second);
        }
    }
    m_history.PushAction(EditorAction::MakeMultiRemove(blocks));
    m_selection.Clear();
}

bool ShipEditorController::PasteAtHover() {
    if (m_clipboard.IsEmpty()) return false;

    auto blocks = m_clipboard.Paste(m_state.hoverCell);

    // Check no overlap with existing blocks
    for (const auto& b : blocks) {
        auto cells = BlockPlacement::GetOccupiedCells(b);
        for (const auto& c : cells) {
            if (m_ship.occupiedCells.count(c) > 0) {
                return false;
            }
        }
    }

    // If ship is not empty, at least one pasted block must be adjacent to the
    // existing structure.
    if (!m_ship.IsEmpty()) {
        bool anyAdjacent = false;
        for (const auto& b : blocks) {
            auto adj = BlockPlacement::GetAdjacentCells(b);
            for (const auto& a : adj) {
                if (m_ship.occupiedCells.count(a) > 0) {
                    anyAdjacent = true;
                    break;
                }
            }
            if (anyAdjacent) break;
        }
        if (!anyAdjacent) return false;
    }

    // Place all blocks directly (we already validated as a group)
    for (const auto& b : blocks) {
        auto ptr = std::make_shared<Block>(b);
        auto cells = BlockPlacement::GetOccupiedCells(*ptr);
        for (const auto& c : cells) {
            m_ship.occupiedCells[c] = ptr;
        }
        m_ship.blocks.push_back(ptr);
    }
    ShipStats::Recalculate(m_ship);

    m_history.PushAction(EditorAction::MakeMultiPlace(blocks));
    return true;
}

EditorClipboard& ShipEditorController::GetClipboard() {
    return m_clipboard;
}

// ---- Validation ----

ValidationResult ShipEditorController::ValidateShip() const {
    return ShipValidator::Validate(m_ship);
}

// ---- Palette ----

const BlockPalette& ShipEditorController::GetPalette() const {
    return m_palette;
}

// ---- Grid ----

EditorGrid& ShipEditorController::GetGrid() {
    return m_grid;
}

const EditorGrid& ShipEditorController::GetGrid() const {
    return m_grid;
}

// ---- Remove selection ----

bool ShipEditorController::RemoveSelected() {
    auto blocks = m_selection.GatherBlocks(m_ship);
    if (blocks.empty()) return false;

    for (const auto& b : blocks) {
        auto it = m_ship.occupiedCells.find(b.gridPos);
        if (it != m_ship.occupiedCells.end()) {
            ShipDamage::RemoveBlock(m_ship, it->second);
        }
    }
    m_history.PushAction(EditorAction::MakeMultiRemove(blocks));
    m_selection.Clear();
    return true;
}

Ship& ShipEditorController::GetShip() {
    return m_ship;
}

const Ship& ShipEditorController::GetShip() const {
    return m_ship;
}

} // namespace subspace
