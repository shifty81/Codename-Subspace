#pragma once

#include "ships/Block.h"

#include <vector>

namespace subspace {

// Stores a set of blocks relative to an anchor position for copy/paste.
class EditorClipboard {
public:
    // Copy blocks into the clipboard, recentering them around the anchor.
    void Copy(const std::vector<Block>& blocks, const Vector3Int& anchor);

    // Paste blocks at a target position. Returns new blocks with positions
    // offset from the target.
    std::vector<Block> Paste(const Vector3Int& target) const;

    bool IsEmpty() const;
    void Clear();

    size_t BlockCount() const;

    const std::vector<Block>& GetBlocks() const;

private:
    std::vector<Block> m_blocks;  // stored relative to (0,0,0)
};

} // namespace subspace
