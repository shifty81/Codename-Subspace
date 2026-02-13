#include "ships/Block.h"

#include <cmath>
#include <stdexcept>
#include <string>

namespace subspace {

// ---------------------------------------------------------------------------
// MaterialDatabase
// ---------------------------------------------------------------------------
bool MaterialDatabase::s_initialized = false;
std::unordered_map<MaterialType, MaterialStats> MaterialDatabase::s_materials;

void MaterialDatabase::Initialize() {
    if (s_initialized) return;

    //                          density  hpMul  energy   R      G      B      A
    s_materials[MaterialType::Iron]     = { 7.8f, 1.0f, 0.0f, {0.56f, 0.57f, 0.58f, 1.0f} };
    s_materials[MaterialType::Titanium] = { 4.5f, 1.3f, 0.0f, {0.75f, 0.76f, 0.78f, 1.0f} };
    s_materials[MaterialType::Naonite]  = { 5.0f, 1.5f, 0.1f, {0.20f, 0.80f, 0.30f, 1.0f} };
    s_materials[MaterialType::Trinium]  = { 3.0f, 1.8f, 0.2f, {0.30f, 0.50f, 0.90f, 1.0f} };
    s_materials[MaterialType::Xanion]   = { 6.0f, 2.2f, 0.4f, {0.70f, 0.30f, 0.80f, 1.0f} };
    s_materials[MaterialType::Ogonite]  = { 9.0f, 3.0f, 0.3f, {0.85f, 0.55f, 0.15f, 1.0f} };
    s_materials[MaterialType::Avorion]  = { 2.0f, 4.0f, 1.0f, {0.95f, 0.90f, 0.40f, 1.0f} };

    s_initialized = true;
}

const MaterialStats& MaterialDatabase::Get(MaterialType type) {
    Initialize();
    auto it = s_materials.find(type);
    if (it == s_materials.end()) {
        throw std::runtime_error(
            "Unknown MaterialType: " + std::to_string(static_cast<int>(type)));
    }
    return it->second;
}

// ---------------------------------------------------------------------------
// Block helpers
// ---------------------------------------------------------------------------
float GetBlockBaseHP(BlockType type) {
    switch (type) {
        case BlockType::Hull:        return 100.0f;
        case BlockType::Armor:       return 200.0f;
        case BlockType::Engine:      return  80.0f;
        case BlockType::Generator:   return  90.0f;
        case BlockType::Gyro:        return  70.0f;
        case BlockType::Cargo:       return  60.0f;
        case BlockType::WeaponMount: return  75.0f;
    }
    return 100.0f; // fallback
}

float Block::Volume() const {
    return static_cast<float>(std::abs(size.x) * std::abs(size.y) * std::abs(size.z));
}

float Block::Mass() const {
    const MaterialStats& stats = MaterialDatabase::Get(material);
    return Volume() * stats.density;
}

} // namespace subspace
