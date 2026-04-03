#pragma once

#include "core/Math.h"

#include <unordered_map>

namespace subspace {

enum class BlockShape  { Cube, Rect, Wedge, Corner, Slope,
                        HalfBlock, Tetrahedron, InnerCorner, SlopedPlate };
enum class BlockType   { Hull, Armor, Engine, Generator, Gyro, Cargo, WeaponMount,
                        Framework, Thruster, GyroArray, ShieldGenerator,
                        Cockpit, CargoHold };
enum class MaterialType { Iron, Titanium, Naonite, Trinium, Xanion, Ogonite, Avorion };

struct MaterialStats {
    float density;
    float hpMultiplier;
    float energyBonus;
    float baseColor[4]; // r, g, b, a
};

class MaterialDatabase {
public:
    static const MaterialStats& Get(MaterialType type);

private:
    static std::unordered_map<MaterialType, MaterialStats> s_materials;
    static void Initialize();
    static bool s_initialized;
};

struct Block {
    Vector3Int   gridPos;
    Vector3Int   size;          // integer dimensions
    int          rotationIndex; // 0-3, 90-degree increments
    BlockShape   shape;
    BlockType    type;
    MaterialType material;
    float        maxHP;
    float        currentHP;

    float Volume() const;
    float Mass() const;
};

float GetBlockBaseHP(BlockType type);

} // namespace subspace

// Hash specializations so enums can be keys in unordered_map
template<> struct std::hash<subspace::MaterialType> {
    std::size_t operator()(subspace::MaterialType t) const noexcept {
        return std::hash<int>{}(static_cast<int>(t));
    }
};
