#pragma once

#include "ships/Block.h"

#include <cstdint>
#include <vector>

namespace subspace {

enum SymmetryMode : uint8_t {
    SymmetryNone    = 0,
    SymmetryMirrorX = 1,
    SymmetryMirrorY = 2,
    SymmetryMirrorZ = 4
};

class SymmetrySystem {
public:
    // Create a mirrored copy of a block across the specified single axis
    static Block CreateMirroredBlock(const Block& original, uint8_t mode);

    // Get all mirrored blocks for combined symmetry modes (excludes original)
    static std::vector<Block> GetAllMirroredBlocks(const Block& original, uint8_t mode);
};

} // namespace subspace
