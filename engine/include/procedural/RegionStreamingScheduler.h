#pragma once

#include "procedural/RegionStreamingSystem.h"

#include <unordered_set>
#include <vector>

namespace subspace {

struct RegionStreamingBudget {
    int maxCellsPerFrame = 2;
    int maxAsteroidsPerFrame = 96;
    double maxEstimatedWorkMs = 2.0;
};

struct RegionStreamingWorkItem {
    RegionCellKey key;
    int priorityDistance = 0;
    int estimatedAsteroids = 0;
    double estimatedWorkMs = 0.0;
};

struct RegionStreamingFramePlan {
    std::vector<RegionStreamingWorkItem> scheduled;
    int requestedCells = 0;
    int deferredCells = 0;
    int estimatedAsteroids = 0;
    double estimatedWorkMs = 0.0;
};

/// Pass317 scheduler for the existing deterministic RegionStreamingSystem.
/// The scheduler never changes generation results; it only limits how much
/// deterministic materialization work is admitted in one frame.
class RegionStreamingScheduler {
public:
    explicit RegionStreamingScheduler(RegionStreamingBudget budget = {});

    RegionStreamingFramePlan BuildFramePlan(
        const BeltMacroRegion& region,
        RegionCellKey center,
        int radius,
        const std::vector<RegionCellKey>& loaded) const;

    std::vector<StreamedRegionCell> MaterializePlan(
        const BeltMacroRegion& region,
        const RegionStreamingFramePlan& plan,
        const RegionStreamingSystem& streaming) const;

    const RegionStreamingBudget& Budget() const { return budget_; }

private:
    RegionStreamingBudget budget_;
};

} // namespace subspace
