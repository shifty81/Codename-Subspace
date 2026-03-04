#pragma once

#include "ships/Block.h"
#include "ships/Ship.h"

#include <memory>
#include <string>
#include <vector>

namespace subspace {

enum class EditorActionType {
    PlaceBlock,
    RemoveBlock,
    PaintBlock,
    MultiPlace,
    MultiRemove
};

// Represents a single undoable editor action.
struct EditorAction {
    EditorActionType type;

    // For PlaceBlock / RemoveBlock / PaintBlock
    Block blockData;

    // For PaintBlock – the material before the paint
    MaterialType previousMaterial = MaterialType::Iron;

    // For MultiPlace / MultiRemove – stores multiple blocks
    std::vector<Block> multiBlocks;

    // For MultiRemove – stores the materials prior to removal (unused for place)
    std::vector<MaterialType> previousMaterials;

    static EditorAction MakePlaceAction(const Block& placed);
    static EditorAction MakeRemoveAction(const Block& removed);
    static EditorAction MakePaintAction(const Block& target, MaterialType oldMat);
    static EditorAction MakeMultiPlace(const std::vector<Block>& blocks);
    static EditorAction MakeMultiRemove(const std::vector<Block>& blocks);
};

// Manages a linear undo/redo history of editor actions.
class EditorHistory {
public:
    explicit EditorHistory(size_t maxSize = 100);

    void PushAction(const EditorAction& action);

    bool CanUndo() const;
    bool CanRedo() const;

    // Returns the action that was undone (caller applies the inverse).
    EditorAction Undo();

    // Returns the action that was redone (caller applies it again).
    EditorAction Redo();

    void Clear();

    size_t UndoCount() const;
    size_t RedoCount() const;

    size_t MaxSize() const;

private:
    std::vector<EditorAction> m_actions;
    size_t m_cursor = 0;   // points past the last applied action
    size_t m_maxSize;
};

} // namespace subspace
