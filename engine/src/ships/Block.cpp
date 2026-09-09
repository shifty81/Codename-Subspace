#include "ships/Block.h"

#include <algorithm>
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

    // density, hpMul, energy, RGBA, massMultiplier, shieldMultiplier, tech, RGB
    s_materials[MaterialType::Iron]     = {7.87f, 1.0f, 0.8f, {0.72f, 0.72f, 0.75f, 1.0f}, 1.0f, 0.5f, 1, 0xB8B8C0};
    s_materials[MaterialType::Titanium] = {4.51f, 1.5f, 1.0f, {0.82f, 0.87f, 0.95f, 1.0f}, 0.9f, 0.8f, 2, 0xD0DEF2};
    s_materials[MaterialType::Naonite]  = {3.80f, 2.0f, 1.2f, {0.15f, 0.92f, 0.35f, 1.0f}, 0.8f, 1.2f, 3, 0x26EB59};
    s_materials[MaterialType::Trinium]  = {2.70f, 2.5f, 1.5f, {0.25f, 0.65f, 1.00f, 1.0f}, 0.6f, 1.5f, 4, 0x40A6FF};
    s_materials[MaterialType::Xanion]   = {2.20f, 3.0f, 1.8f, {1.00f, 0.82f, 0.15f, 1.0f}, 0.5f, 2.0f, 5, 0xFFD126};
    s_materials[MaterialType::Ogonite]  = {1.80f, 4.0f, 2.2f, {1.00f, 0.40f, 0.15f, 1.0f}, 0.4f, 2.5f, 6, 0xFF6626};
    s_materials[MaterialType::Avorion]  = {1.20f, 5.0f, 3.0f, {0.85f, 0.20f, 1.00f, 1.0f}, 0.3f, 3.5f, 7, 0xD933FF};

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

MaterialType MaterialDatabase::FromName(const std::string& name, MaterialType fallback) {
    if (name == "Iron" || name == "iron") return MaterialType::Iron;
    if (name == "Titanium" || name == "titanium") return MaterialType::Titanium;
    if (name == "Naonite" || name == "naonite") return MaterialType::Naonite;
    if (name == "Trinium" || name == "trinium") return MaterialType::Trinium;
    if (name == "Xanion" || name == "xanion") return MaterialType::Xanion;
    if (name == "Ogonite" || name == "ogonite") return MaterialType::Ogonite;
    if (name == "Avorion" || name == "avorion") return MaterialType::Avorion;
    return fallback;
}

std::string MaterialDatabase::ToName(MaterialType type) {
    switch (type) {
        case MaterialType::Iron: return "Iron";
        case MaterialType::Titanium: return "Titanium";
        case MaterialType::Naonite: return "Naonite";
        case MaterialType::Trinium: return "Trinium";
        case MaterialType::Xanion: return "Xanion";
        case MaterialType::Ogonite: return "Ogonite";
        case MaterialType::Avorion: return "Avorion";
    }
    return "Iron";
}

// ---------------------------------------------------------------------------
// Block helpers
// ---------------------------------------------------------------------------
float GetBlockBaseHP(BlockType type) {
    switch (type) {
        case BlockType::Hull:             return 100.0f;
        case BlockType::Armor:            return 500.0f;
        case BlockType::Engine:           return  80.0f;
        case BlockType::Thruster:         return  70.0f;
        case BlockType::Generator:        return  90.0f;
        case BlockType::Gyro:             return  70.0f;
        case BlockType::ShieldGenerator:  return  95.0f;
        case BlockType::Cargo:            return  60.0f;
        case BlockType::WeaponMount:      return  75.0f;
        case BlockType::HyperdriveCore:   return 150.0f;
        case BlockType::CrewQuarters:     return  80.0f;
        case BlockType::PodDocking:       return 120.0f;
        case BlockType::Computer:         return  65.0f;
        case BlockType::Battery:          return  85.0f;
        case BlockType::IntegrityField:   return  90.0f;
        case BlockType::Framework:        return  20.0f;
    }
    return 100.0f;
}

bool IsValidBlockShape(int value) {
    switch (static_cast<BlockShape>(value)) {
        case BlockShape::Cube:
        case BlockShape::Rect:
        case BlockShape::Wedge:
        case BlockShape::Corner:
        case BlockShape::Slope:
        case BlockShape::InnerCorner:
        case BlockShape::Tetrahedron:
        case BlockShape::HalfBlock:
        case BlockShape::SlopedPlate:
            return value >= 0;
    }
    return false;
}

bool IsValidBlockType(int value) {
    switch (static_cast<BlockType>(value)) {
        case BlockType::Hull:
        case BlockType::Armor:
        case BlockType::Engine:
        case BlockType::Generator:
        case BlockType::Gyro:
        case BlockType::Cargo:
        case BlockType::WeaponMount:
        case BlockType::Thruster:
        case BlockType::ShieldGenerator:
        case BlockType::HyperdriveCore:
        case BlockType::CrewQuarters:
        case BlockType::PodDocking:
        case BlockType::Computer:
        case BlockType::Battery:
        case BlockType::IntegrityField:
        case BlockType::Framework:
            return value >= 0;
    }
    return false;
}

std::string BlockTypeName(BlockType type) {
    switch (type) {
        case BlockType::Hull: return "Hull";
        case BlockType::Armor: return "Armor";
        case BlockType::Engine: return "Engine";
        case BlockType::Generator: return "Generator";
        case BlockType::Gyro: return "Gyro";
        case BlockType::Cargo: return "Cargo";
        case BlockType::WeaponMount: return "WeaponMount";
        case BlockType::Thruster: return "Thruster";
        case BlockType::ShieldGenerator: return "ShieldGenerator";
        case BlockType::HyperdriveCore: return "HyperdriveCore";
        case BlockType::CrewQuarters: return "CrewQuarters";
        case BlockType::PodDocking: return "PodDocking";
        case BlockType::Computer: return "Computer";
        case BlockType::Battery: return "Battery";
        case BlockType::IntegrityField: return "IntegrityField";
        case BlockType::Framework: return "Framework";
    }
    return "Unknown";
}

std::string BlockShapeName(BlockShape shape) {
    switch (shape) {
        case BlockShape::Cube: return "Cube";
        case BlockShape::Rect: return "Rect";
        case BlockShape::Wedge: return "Wedge";
        case BlockShape::Corner: return "Corner";
        case BlockShape::Slope: return "Slope";
        case BlockShape::InnerCorner: return "InnerCorner";
        case BlockShape::Tetrahedron: return "Tetrahedron";
        case BlockShape::HalfBlock: return "HalfBlock";
        case BlockShape::SlopedPlate: return "SlopedPlate";
    }
    return "Unknown";
}

float BlockShapeVolumeMultiplier(BlockShape shape) {
    switch (shape) {
        case BlockShape::Wedge:
        case BlockShape::HalfBlock:
            return 0.5f;
        case BlockShape::Corner:
        case BlockShape::Tetrahedron:
            return 0.25f;
        case BlockShape::InnerCorner:
            return 0.75f;
        case BlockShape::SlopedPlate:
        case BlockShape::Slope:
            return 0.3f;
        case BlockShape::Cube:
        case BlockShape::Rect:
        default:
            return 1.0f;
    }
}

float Block::Volume() const {
    return static_cast<float>(std::abs(size.x) * std::abs(size.y) * std::abs(size.z));
}

float Block::EffectiveVolume() const {
    return Volume() * BlockShapeVolumeMultiplier(shape);
}

float Block::Mass() const {
    const MaterialStats& stats = MaterialDatabase::Get(material);
    float base = EffectiveVolume() * stats.density * stats.massMultiplier;
    if (type == BlockType::Armor) base *= 1.5f;
    if (type == BlockType::Framework) base *= 0.1f;
    return base;
}

bool Block::IsAlive() const {
    return !isDestroyed && currentHP > 0.0f;
}

void Block::TakeDamage(float damage) {
    if (damage <= 0.0f || isDestroyed) return;
    currentHP = std::max(0.0f, currentHP - damage);
    if (currentHP <= 0.0f) {
        isDestroyed = true;
    }
}

bool Block::Repair(float amount) {
    if (amount <= 0.0f || maxHP <= 0.0f) return false;
    currentHP = std::min(maxHP, currentHP + amount);
    if (currentHP > 0.0f) {
        isDestroyed = false;
    }
    return true;
}

} // namespace subspace
