#pragma once

#include <cstddef>
#include <deque>
#include <string>
#include <vector>

namespace subspace {

enum class FrameHealth { Good, Warning, Critical };

struct ProductionPerformanceBudget {
    double targetFrameMs = 16.667;
    double warningFrameMs = 22.0;
    double criticalFrameMs = 33.334;
    double renderBudgetMs = 8.0;
    double simulationBudgetMs = 5.0;
    double streamingBudgetMs = 2.5;
    std::size_t transientAllocationBudgetBytes = 4u * 1024u * 1024u;
};

struct ProductionFrameSample {
    double frameMs = 0.0;
    double renderMs = 0.0;
    double simulationMs = 0.0;
    double streamingMs = 0.0;
    std::size_t transientAllocationBytes = 0;
    int entityCount = 0;
    int drawCalls = 0;
    int gpuSubmissions = 0;
};

struct ProductionPerformanceSnapshot {
    FrameHealth health = FrameHealth::Good;
    double latestFrameMs = 0.0;
    double averageFrameMs = 0.0;
    double p95FrameMs = 0.0;
    int sampleCount = 0;
    int hitchCount = 0;
    int renderBudgetViolations = 0;
    int simulationBudgetViolations = 0;
    int streamingBudgetViolations = 0;
    int allocationBudgetViolations = 0;
    int entityCount = 0;
    int drawCalls = 0;
    int gpuSubmissions = 0;
    std::vector<std::string> activeWarnings;
};

/// Pass316 production profiling authority. This deliberately keeps detailed
/// telemetry behind the developer/diagnostic surface instead of leaking it into
/// the normal flight HUD.
class ProductionPerformanceSystem {
public:
    explicit ProductionPerformanceSystem(ProductionPerformanceBudget budget = {});

    void RecordFrame(const ProductionFrameSample& sample);
    ProductionPerformanceSnapshot Snapshot() const;
    void Reset();

    const ProductionPerformanceBudget& Budget() const { return budget_; }
    bool DeveloperOverlayVisible() const { return developerOverlayVisible_; }
    void SetDeveloperOverlayVisible(bool visible) { developerOverlayVisible_ = visible; }

private:
    static constexpr std::size_t kHistoryLimit = 240;
    ProductionPerformanceBudget budget_;
    std::deque<ProductionFrameSample> history_;
    int hitchCount_ = 0;
    int renderBudgetViolations_ = 0;
    int simulationBudgetViolations_ = 0;
    int streamingBudgetViolations_ = 0;
    int allocationBudgetViolations_ = 0;
    bool developerOverlayVisible_ = false;
};

} // namespace subspace
