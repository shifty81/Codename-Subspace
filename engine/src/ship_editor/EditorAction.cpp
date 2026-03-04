#include "ship_editor/EditorAction.h"

#include <algorithm>

namespace subspace {

EditorAction EditorAction::MakePlaceAction(const Block& placed) {
    EditorAction a{};
    a.type = EditorActionType::PlaceBlock;
    a.blockData = placed;
    return a;
}

EditorAction EditorAction::MakeRemoveAction(const Block& removed) {
    EditorAction a{};
    a.type = EditorActionType::RemoveBlock;
    a.blockData = removed;
    return a;
}

EditorAction EditorAction::MakePaintAction(const Block& target, MaterialType oldMat) {
    EditorAction a{};
    a.type = EditorActionType::PaintBlock;
    a.blockData = target;
    a.previousMaterial = oldMat;
    return a;
}

EditorAction EditorAction::MakeMultiPlace(const std::vector<Block>& blocks) {
    EditorAction a{};
    a.type = EditorActionType::MultiPlace;
    a.multiBlocks = blocks;
    return a;
}

EditorAction EditorAction::MakeMultiRemove(const std::vector<Block>& blocks) {
    EditorAction a{};
    a.type = EditorActionType::MultiRemove;
    a.multiBlocks = blocks;
    return a;
}

// -----------------------------------------------------------------------

EditorHistory::EditorHistory(size_t maxSize)
    : m_maxSize(maxSize) {
    m_actions.reserve(maxSize);
}

void EditorHistory::PushAction(const EditorAction& action) {
    // Discard any redo actions beyond the cursor
    if (m_cursor < m_actions.size()) {
        m_actions.erase(m_actions.begin() + static_cast<std::ptrdiff_t>(m_cursor),
                        m_actions.end());
    }

    m_actions.push_back(action);

    // Enforce max size by removing oldest
    if (m_actions.size() > m_maxSize) {
        m_actions.erase(m_actions.begin());
    } else {
        ++m_cursor;
    }
}

bool EditorHistory::CanUndo() const {
    return m_cursor > 0;
}

bool EditorHistory::CanRedo() const {
    return m_cursor < m_actions.size();
}

EditorAction EditorHistory::Undo() {
    --m_cursor;
    return m_actions[m_cursor];
}

EditorAction EditorHistory::Redo() {
    EditorAction action = m_actions[m_cursor];
    ++m_cursor;
    return action;
}

void EditorHistory::Clear() {
    m_actions.clear();
    m_cursor = 0;
}

size_t EditorHistory::UndoCount() const {
    return m_cursor;
}

size_t EditorHistory::RedoCount() const {
    return m_actions.size() - m_cursor;
}

size_t EditorHistory::MaxSize() const {
    return m_maxSize;
}

} // namespace subspace
