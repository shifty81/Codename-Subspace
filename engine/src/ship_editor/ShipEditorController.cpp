#include "ship_editor/ShipEditorController.h"
#include "ships/BlockPlacement.h"
#include "ships/ShipDamage.h"

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
    return true;
}

bool ShipEditorController::RemoveAtHover() {
    auto it = m_ship.occupiedCells.find(m_state.hoverCell);
    if (it == m_ship.occupiedCells.end()) {
        return false;
    }

    ShipDamage::RemoveBlock(m_ship, it->second);
    return true;
}

bool ShipEditorController::PaintAtHover() {
    auto it = m_ship.occupiedCells.find(m_state.hoverCell);
    if (it == m_ship.occupiedCells.end()) {
        return false;
    }

    it->second->material = m_state.selectedMaterial;
    return true;
}

void ShipEditorController::SetHoverCell(const Vector3Int& cell) {
    m_state.hoverCell = cell;
}

Ship& ShipEditorController::GetShip() {
    return m_ship;
}

const Ship& ShipEditorController::GetShip() const {
    return m_ship;
}

} // namespace subspace
