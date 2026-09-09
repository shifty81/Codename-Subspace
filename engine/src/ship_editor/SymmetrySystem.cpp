#include "ship_editor/SymmetrySystem.h"

namespace subspace {

Block SymmetrySystem::CreateMirroredBlock(const Block& original, uint8_t mode) {
    Block mirrored = original;

    if (mode & SymmetryMirrorX) {
        mirrored.gridPos.x = -original.gridPos.x - original.size.x;
    }
    if (mode & SymmetryMirrorY) {
        mirrored.gridPos.y = -original.gridPos.y - original.size.y;
    }
    if (mode & SymmetryMirrorZ) {
        mirrored.gridPos.z = -original.gridPos.z - original.size.z;
    }

    return mirrored;
}

std::vector<Block> SymmetrySystem::GetAllMirroredBlocks(const Block& original, uint8_t mode) {
    std::vector<Block> results;

    // Collect active axis flags
    uint8_t axes[3];
    int axisCount = 0;
    if (mode & SymmetryMirrorX) axes[axisCount++] = SymmetryMirrorX;
    if (mode & SymmetryMirrorY) axes[axisCount++] = SymmetryMirrorY;
    if (mode & SymmetryMirrorZ) axes[axisCount++] = SymmetryMirrorZ;

    // Generate all non-zero subsets of active axes (2^n - 1 mirrors)
    int totalCombinations = 1 << axisCount;
    for (int i = 1; i < totalCombinations; ++i) {
        uint8_t combinedFlags = 0;
        for (int bit = 0; bit < axisCount; ++bit) {
            if (i & (1 << bit)) {
                combinedFlags |= axes[bit];
            }
        }
        results.push_back(CreateMirroredBlock(original, combinedFlags));
    }

    return results;
}

} // namespace subspace
