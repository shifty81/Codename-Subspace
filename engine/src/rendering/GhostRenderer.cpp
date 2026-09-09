#include "rendering/GhostRenderer.h"
#include "ship_editor/SymmetrySystem.h"
#include "ships/BlockPlacement.h"

namespace subspace {

std::vector<GhostBlockData> GhostRenderer::BuildGhostPreview(
    const Ship& ship, const Block& ghost, uint8_t symmetryFlags)
{
    std::vector<GhostBlockData> result;

    // Primary ghost block
    result.push_back({ghost, BlockPlacement::CanPlace(ship, ghost)});

    // Generate mirrored copies from symmetry flags
    auto mirrors = SymmetrySystem::GetAllMirroredBlocks(ghost, symmetryFlags);
    for (const auto& mirrored : mirrors) {
        result.push_back({mirrored, BlockPlacement::CanPlace(ship, mirrored)});
    }

    return result;
}

} // namespace subspace
