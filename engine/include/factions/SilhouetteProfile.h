#pragma once

#include "ships/Block.h"

#include <vector>

namespace subspace {

enum class LengthBias { Short, Balanced, Long };
enum class ThicknessBias { Thin, Medium, Chunky };
enum class SymmetryBias { Axial, TriRadial, QuadRadial };
enum class MassBias { Central, Spine, Winged, Distributed };
enum class VoidBias { Solid, Channelled, Ringed };

struct SilhouetteProfile {
    LengthBias length = LengthBias::Balanced;
    ThicknessBias thickness = ThicknessBias::Medium;
    SymmetryBias symmetry = SymmetryBias::Axial;
    MassBias mass = MassBias::Central;
    VoidBias voidType = VoidBias::Solid;
};

struct ShapeLanguage {
    std::vector<BlockShape> allowedShapes;
    float wedgeChance = 0.0f;
    float cornerChance = 0.0f;
};

struct FactionPalette {
    MaterialType hull = MaterialType::Iron;
    MaterialType armor = MaterialType::Titanium;
    MaterialType accent = MaterialType::Naonite;
    float primaryColor[4] = {0.5f, 0.5f, 0.5f, 1.0f};
    float accentColor[4] = {0.8f, 0.2f, 0.2f, 1.0f};
    float engineColor[4] = {0.2f, 0.5f, 1.0f, 1.0f};
};

} // namespace subspace
