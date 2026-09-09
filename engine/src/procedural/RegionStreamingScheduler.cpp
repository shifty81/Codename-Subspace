#include "procedural/RegionStreamingScheduler.h"

#include <algorithm>
#include <cmath>
#include <string>
#include <unordered_set>

namespace subspace {
namespace {
std::string KeyToken(RegionCellKey key) { return std::to_string(key.x) + ":" + std::to_string(key.y); }
int Distance(RegionCellKey a, RegionCellKey b) {
    return static_cast<int>(std::llabs(a.x - b.x) + std::llabs(a.y - b.y));
}
int EstimatedAsteroids(const BeltMacroRegion& region) {
    const int base = std::max(8, region.localAsteroidBudget);
    return region.kind == SpaceRegionKind::PlanetaryRing ? std::max(64, base + 24) : base;
}
}

RegionStreamingScheduler::RegionStreamingScheduler(RegionStreamingBudget budget)
    : budget_(budget)
{
    budget_.maxCellsPerFrame = std::max(1, budget_.maxCellsPerFrame);
    budget_.maxAsteroidsPerFrame = std::max(8, budget_.maxAsteroidsPerFrame);
    budget_.maxEstimatedWorkMs = std::max(0.25, budget_.maxEstimatedWorkMs);
}

RegionStreamingFramePlan RegionStreamingScheduler::BuildFramePlan(
    const BeltMacroRegion& region,
    RegionCellKey center,
    int radius,
    const std::vector<RegionCellKey>& loaded) const
{
    radius = std::max(0, radius);
    std::unordered_set<std::string> loadedSet;
    for (const auto& key : loaded) loadedSet.insert(KeyToken(key));

    std::vector<RegionStreamingWorkItem> candidates;
    const int asteroidEstimate = EstimatedAsteroids(region);
    for (int y = -radius; y <= radius; ++y) {
        for (int x = -radius; x <= radius; ++x) {
            RegionCellKey key{center.x + x, center.y + y};
            if (loadedSet.count(KeyToken(key)) != 0) continue;
            RegionStreamingWorkItem item;
            item.key = key;
            item.priorityDistance = Distance(center, key);
            item.estimatedAsteroids = asteroidEstimate;
            item.estimatedWorkMs = 0.18 + static_cast<double>(asteroidEstimate) * 0.0065;
            candidates.push_back(item);
        }
    }

    std::stable_sort(candidates.begin(), candidates.end(), [](const auto& a, const auto& b) {
        if (a.priorityDistance != b.priorityDistance) return a.priorityDistance < b.priorityDistance;
        if (a.key.y != b.key.y) return a.key.y < b.key.y;
        return a.key.x < b.key.x;
    });

    RegionStreamingFramePlan plan;
    plan.requestedCells = static_cast<int>(candidates.size());
    for (const auto& item : candidates) {
        const bool cellBudgetReached = static_cast<int>(plan.scheduled.size()) >= budget_.maxCellsPerFrame;
        const bool asteroidBudgetReached = !plan.scheduled.empty() && plan.estimatedAsteroids + item.estimatedAsteroids > budget_.maxAsteroidsPerFrame;
        const bool timeBudgetReached = !plan.scheduled.empty() && plan.estimatedWorkMs + item.estimatedWorkMs > budget_.maxEstimatedWorkMs;
        if (cellBudgetReached || asteroidBudgetReached || timeBudgetReached) break;
        plan.scheduled.push_back(item);
        plan.estimatedAsteroids += item.estimatedAsteroids;
        plan.estimatedWorkMs += item.estimatedWorkMs;
    }
    plan.deferredCells = plan.requestedCells - static_cast<int>(plan.scheduled.size());
    return plan;
}

std::vector<StreamedRegionCell> RegionStreamingScheduler::MaterializePlan(
    const BeltMacroRegion& region,
    const RegionStreamingFramePlan& plan,
    const RegionStreamingSystem& streaming) const
{
    std::vector<StreamedRegionCell> cells;
    cells.reserve(plan.scheduled.size());
    for (const auto& item : plan.scheduled) cells.push_back(streaming.Materialize(region, item.key));
    return cells;
}

} // namespace subspace
