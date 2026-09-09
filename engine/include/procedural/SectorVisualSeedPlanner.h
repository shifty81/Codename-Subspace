#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace subspace {

struct SectorVisualSeedPlan {
    std::uint32_t seed = 1;
    std::string paletteName;
    float starDensity = 0.8f;
    float dustDensity = 0.4f;
    float hazardTint = 0.0f;
    std::vector<std::string> visualTags;
};

SectorVisualSeedPlan CreateSectorVisualSeedPlan(std::uint32_t seed, int dangerDepth, bool homeSystem);

} // namespace subspace
