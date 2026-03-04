#include "ship_editor/BlockPalette.h"

#include <algorithm>

namespace subspace {

BlockPalette::BlockPalette() {
    PopulateDefaults();
}

const std::vector<BlockPaletteEntry>& BlockPalette::GetAll() const {
    return m_entries;
}

std::vector<BlockPaletteEntry> BlockPalette::GetByCategory(const std::string& category) const {
    std::vector<BlockPaletteEntry> result;
    for (const auto& e : m_entries) {
        if (e.category == category) {
            result.push_back(e);
        }
    }
    return result;
}

std::vector<std::string> BlockPalette::GetCategories() const {
    std::vector<std::string> cats;
    for (const auto& e : m_entries) {
        if (std::find(cats.begin(), cats.end(), e.category) == cats.end()) {
            cats.push_back(e.category);
        }
    }
    return cats;
}

const BlockPaletteEntry* BlockPalette::FindByType(BlockType type) const {
    for (const auto& e : m_entries) {
        if (e.type == type) {
            return &e;
        }
    }
    return nullptr;
}

size_t BlockPalette::Count() const {
    return m_entries.size();
}

void BlockPalette::PopulateDefaults() {
    m_entries = {
        // Structure
        {"Hull Block",    "Structure", BlockShape::Cube,   BlockType::Hull,     MaterialType::Iron},
        {"Hull Wedge",    "Structure", BlockShape::Wedge,  BlockType::Hull,     MaterialType::Iron},
        {"Hull Corner",   "Structure", BlockShape::Corner, BlockType::Hull,     MaterialType::Iron},
        {"Hull Slope",    "Structure", BlockShape::Slope,  BlockType::Hull,     MaterialType::Iron},
        {"Armor Block",   "Structure", BlockShape::Cube,   BlockType::Armor,    MaterialType::Iron},
        {"Armor Wedge",   "Structure", BlockShape::Wedge,  BlockType::Armor,    MaterialType::Iron},

        // Functional
        {"Engine",        "Functional", BlockShape::Cube,  BlockType::Engine,      MaterialType::Iron},
        {"Generator",     "Functional", BlockShape::Cube,  BlockType::Generator,   MaterialType::Iron},
        {"Gyroscope",     "Functional", BlockShape::Cube,  BlockType::Gyro,        MaterialType::Iron},
        {"Cargo Bay",     "Functional", BlockShape::Rect,  BlockType::Cargo,       MaterialType::Iron},

        // Weapons
        {"Weapon Mount",  "Weapons",   BlockShape::Cube,  BlockType::WeaponMount, MaterialType::Iron},
    };
}

} // namespace subspace
