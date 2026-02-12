#pragma once

#include "ships/Block.h"
#include "ships/Ship.h"

#include <cstdint>
#include <vector>

namespace subspace {

struct GhostBlockData {
    Block block;
    bool isValid;   // Green if true, Red if false
};

class GhostRenderer {
public:
    // Build ghost preview data for a block and its symmetry mirrors
    static std::vector<GhostBlockData> BuildGhostPreview(
        const Ship& ship, const Block& ghost, uint8_t symmetryFlags);
};

} // namespace subspace
