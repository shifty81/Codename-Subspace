#include "ship_editor/ShipEditorState.h"

namespace subspace {

static constexpr int kShapeCount = 5;     // Cube, Rect, Wedge, Corner, Slope
static constexpr int kBlockTypeCount = 7; // Hull, Armor, Engine, Generator, Gyro, Cargo, WeaponMount

void ShipEditorState::ToggleSymmetryX() {
    symmetry ^= SymmetryMirrorX;
}

void ShipEditorState::ToggleSymmetryY() {
    symmetry ^= SymmetryMirrorY;
}

void ShipEditorState::ToggleSymmetryZ() {
    symmetry ^= SymmetryMirrorZ;
}

void ShipEditorState::Rotate90() {
    rotationIndex = (rotationIndex + 1) % 4;
}

void ShipEditorState::NextShape() {
    int idx = static_cast<int>(selectedShape);
    selectedShape = static_cast<BlockShape>((idx + 1) % kShapeCount);
}

void ShipEditorState::PrevShape() {
    int idx = static_cast<int>(selectedShape);
    selectedShape = static_cast<BlockShape>((idx + kShapeCount - 1) % kShapeCount);
}

void ShipEditorState::NextBlockType() {
    int idx = static_cast<int>(selectedType);
    selectedType = static_cast<BlockType>((idx + 1) % kBlockTypeCount);
}

void ShipEditorState::PrevBlockType() {
    int idx = static_cast<int>(selectedType);
    selectedType = static_cast<BlockType>((idx + kBlockTypeCount - 1) % kBlockTypeCount);
}

} // namespace subspace
