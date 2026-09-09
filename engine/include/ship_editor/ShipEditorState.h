#pragma once

#include "core/Math.h"
#include "ships/Block.h"
#include "ship_editor/SymmetrySystem.h"

#include <cstdint>

namespace subspace {

enum class BuildMode {
    Place,
    Remove,
    Paint,
    Select
};

class ShipEditorState {
public:
    BuildMode mode = BuildMode::Place;
    BlockShape selectedShape = BlockShape::Cube;
    BlockType selectedType = BlockType::Hull;
    MaterialType selectedMaterial = MaterialType::Iron;
    uint8_t symmetry = SymmetryNone;
    int rotationIndex = 0;  // 0-3, 90-degree increments
    Vector3Int hoverCell = Vector3Int::Zero();
    Vector3Int blockSize = Vector3Int::One();

    void ToggleSymmetryX();
    void ToggleSymmetryY();
    void ToggleSymmetryZ();

    void Rotate90();

    void NextShape();
    void PrevShape();

    void NextBlockType();
    void PrevBlockType();
};

} // namespace subspace
