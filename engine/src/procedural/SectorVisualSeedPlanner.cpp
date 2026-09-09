#include "procedural/SectorVisualSeedPlanner.h"

namespace subspace {

SectorVisualSeedPlan CreateSectorVisualSeedPlan(std::uint32_t seed, int dangerDepth, bool homeSystem) {
    SectorVisualSeedPlan plan;
    plan.seed = seed;
    plan.paletteName = homeSystem ? "Home Blue-Gold" : ((seed % 3 == 0) ? "Industrial Cyan" : ((seed % 3 == 1) ? "Hazard Amber" : "Anomaly Violet"));
    plan.starDensity = homeSystem ? 0.65f : 0.75f + 0.05f * static_cast<float>(dangerDepth);
    plan.dustDensity = homeSystem ? 0.20f : 0.35f + 0.08f * static_cast<float>(dangerDepth);
    plan.hazardTint = homeSystem ? 0.0f : 0.12f * static_cast<float>(dangerDepth);
    plan.visualTags.push_back(homeSystem ? "safe-home" : "expedition");
    if (dangerDepth > 2) plan.visualTags.push_back("high-risk");
    if (seed % 5 == 0) plan.visualTags.push_back("debris-rich");
    return plan;
}

} // namespace subspace
