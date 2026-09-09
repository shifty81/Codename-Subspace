#pragma once

#include "core/Math.h"

#include <cstdint>
#include <string>
#include <unordered_map>

namespace subspace {

// C++ block taxonomy ported from the AvorionLike/Core/Voxel C# lane.
// Existing C++ enum ordinals are preserved for saved blueprints/network commands.
enum class BlockShape {
    Cube = 0,
    Rect = 1,
    Wedge = 2,
    Corner = 3,
    Slope = 4,
    InnerCorner = 5,
    Tetrahedron = 6,
    HalfBlock = 7,
    SlopedPlate = 8
};

enum class BlockOrientation {
    PosX = 0,
    NegX = 1,
    PosY = 2,
    NegY = 3,
    PosZ = 4,
    NegZ = 5
};

enum class BlockType {
    // Existing C++ values retained.
    Hull = 0,
    Armor = 1,
    Engine = 2,
    Generator = 3,
    Gyro = 4,
    Cargo = 5,
    WeaponMount = 6,

    // Ported AvorionLike voxel block types.
    Thruster = 7,
    ShieldGenerator = 8,
    HyperdriveCore = 9,
    CrewQuarters = 10,
    PodDocking = 11,
    Computer = 12,
    Battery = 13,
    IntegrityField = 14,
    Framework = 15,

    // Compatibility aliases for the C# names.
    GyroArray = Gyro,
    TurretMount = WeaponMount
};

enum class MaterialType { Iron, Titanium, Naonite, Trinium, Xanion, Ogonite, Avorion };

struct MaterialStats {
    float density = 1.0f;
    float hpMultiplier = 1.0f;
    float energyBonus = 0.0f;
    float baseColor[4] = {0.5f, 0.5f, 0.5f, 1.0f}; // r, g, b, a

    // AvorionLike parity fields. Existing code may ignore them.
    float massMultiplier = 1.0f;
    float shieldMultiplier = 1.0f;
    int techLevel = 1;
    std::uint32_t colorRGB = 0x808080;
};

class MaterialDatabase {
public:
    static const MaterialStats& Get(MaterialType type);
    static MaterialType FromName(const std::string& name, MaterialType fallback = MaterialType::Iron);
    static std::string ToName(MaterialType type);

private:
    static std::unordered_map<MaterialType, MaterialStats> s_materials;
    static void Initialize();
    static bool s_initialized;
};

struct Block {
    Vector3Int   gridPos = Vector3Int::Zero();
    Vector3Int   size = Vector3Int::One();          // integer dimensions
    int          rotationIndex = 0;                 // 0-3, 90-degree increments
    BlockShape   shape = BlockShape::Cube;
    BlockType    type = BlockType::Hull;
    MaterialType material = MaterialType::Iron;
    float        maxHP = 100.0f;
    float        currentHP = 100.0f;

    // Ported C# voxel fields. Kept optional so older code remains valid.
    BlockOrientation orientation = BlockOrientation::PosY;
    std::uint32_t colorRGB = 0x808080;
    bool isDestroyed = false;
    float thrustPower = 0.0f;
    float powerGeneration = 0.0f;
    float shieldCapacity = 0.0f;

    float Volume() const;
    float EffectiveVolume() const;
    float Mass() const;
    bool IsAlive() const;
    void TakeDamage(float damage);
    bool Repair(float amount);
};

float GetBlockBaseHP(BlockType type);
bool IsValidBlockShape(int value);
bool IsValidBlockType(int value);
std::string BlockTypeName(BlockType type);
std::string BlockShapeName(BlockShape shape);
float BlockShapeVolumeMultiplier(BlockShape shape);

} // namespace subspace

// Hash specializations so enums can be keys in unordered_map
template<> struct std::hash<subspace::MaterialType> {
    std::size_t operator()(subspace::MaterialType t) const noexcept {
        return std::hash<int>{}(static_cast<int>(t));
    }
};

template<> struct std::hash<subspace::BlockType> {
    std::size_t operator()(subspace::BlockType t) const noexcept {
        return std::hash<int>{}(static_cast<int>(t));
    }
};
