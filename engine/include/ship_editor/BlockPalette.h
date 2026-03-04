#pragma once

#include "ships/Block.h"

#include <string>
#include <vector>

namespace subspace {

// Describes one entry in the block palette.
struct BlockPaletteEntry {
    std::string name;
    std::string category;      // e.g. "Structure", "Functional", "Weapons"
    BlockShape shape;
    BlockType type;
    MaterialType defaultMaterial;
};

// Provides a browsable catalog of available block types for the editor UI.
class BlockPalette {
public:
    BlockPalette();

    const std::vector<BlockPaletteEntry>& GetAll() const;

    // Filter by category name (case-sensitive).
    std::vector<BlockPaletteEntry> GetByCategory(const std::string& category) const;

    // Return all distinct category names.
    std::vector<std::string> GetCategories() const;

    // Lookup by type (first match).
    const BlockPaletteEntry* FindByType(BlockType type) const;

    size_t Count() const;

private:
    std::vector<BlockPaletteEntry> m_entries;
    void PopulateDefaults();
};

} // namespace subspace
