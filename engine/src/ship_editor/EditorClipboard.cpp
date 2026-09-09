#include "ship_editor/EditorClipboard.h"

namespace subspace {

void EditorClipboard::Copy(const std::vector<Block>& blocks, const Vector3Int& anchor) {
    m_blocks.clear();
    m_blocks.reserve(blocks.size());
    for (const auto& b : blocks) {
        Block relative = b;
        relative.gridPos = b.gridPos - anchor;
        m_blocks.push_back(relative);
    }
}

std::vector<Block> EditorClipboard::Paste(const Vector3Int& target) const {
    std::vector<Block> result;
    result.reserve(m_blocks.size());
    for (const auto& b : m_blocks) {
        Block placed = b;
        placed.gridPos = b.gridPos + target;
        result.push_back(placed);
    }
    return result;
}

bool EditorClipboard::IsEmpty() const {
    return m_blocks.empty();
}

void EditorClipboard::Clear() {
    m_blocks.clear();
}

size_t EditorClipboard::BlockCount() const {
    return m_blocks.size();
}

const std::vector<Block>& EditorClipboard::GetBlocks() const {
    return m_blocks;
}

} // namespace subspace
