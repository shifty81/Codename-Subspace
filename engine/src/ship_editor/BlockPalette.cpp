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
        {"Hull Half",     "Structure", BlockShape::HalfBlock, BlockType::Hull,  MaterialType::Iron},
        {"Framework",     "Structure", BlockShape::Cube,   BlockType::Framework, MaterialType::Iron},
        {"Armor Block",   "Structure", BlockShape::Cube,   BlockType::Armor,    MaterialType::Iron},
        {"Armor Wedge",   "Structure", BlockShape::Wedge,  BlockType::Armor,    MaterialType::Iron},

        // Functional
        {"Engine",        "Functional", BlockShape::Cube,  BlockType::Engine,      MaterialType::Iron},
        {"Thruster",      "Functional", BlockShape::Cube,  BlockType::Thruster,    MaterialType::Iron},
        {"Generator",     "Functional", BlockShape::Cube,  BlockType::Generator,   MaterialType::Iron},
        {"Battery",       "Functional", BlockShape::Cube,  BlockType::Battery,     MaterialType::Iron},
        {"Gyroscope",     "Functional", BlockShape::Cube,  BlockType::Gyro,        MaterialType::Iron},
        {"Shield Gen",    "Functional", BlockShape::Cube,  BlockType::ShieldGenerator, MaterialType::Titanium},
        {"Integrity Field", "Functional", BlockShape::Cube, BlockType::IntegrityField, MaterialType::Titanium},
        {"Computer",      "Functional", BlockShape::Cube,  BlockType::Computer,    MaterialType::Titanium},
        {"Cargo Bay",     "Functional", BlockShape::Rect,  BlockType::Cargo,       MaterialType::Iron},
        {"Crew Quarters", "Functional", BlockShape::Rect,  BlockType::CrewQuarters, MaterialType::Iron},
        {"Hyperdrive",    "Functional", BlockShape::Cube,  BlockType::HyperdriveCore, MaterialType::Naonite},
        {"Pod Dock",      "Functional", BlockShape::Cube,  BlockType::PodDocking, MaterialType::Titanium},

        // Weapons
        {"Weapon Mount",  "Weapons",   BlockShape::Cube,  BlockType::WeaponMount, MaterialType::Iron},
    };
}

} // namespace subspace
