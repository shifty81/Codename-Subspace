#include "debug_tools/ProductionPerformanceSystem.h"

#include <algorithm>
#include <cmath>
#include <numeric>

namespace subspace {

ProductionPerformanceSystem::ProductionPerformanceSystem(ProductionPerformanceBudget budget)
    : budget_(budget)
{
    budget_.targetFrameMs = std::max(1.0, budget_.targetFrameMs);
    budget_.warningFrameMs = std::max(budget_.targetFrameMs, budget_.warningFrameMs);
    budget_.criticalFrameMs = std::max(budget_.warningFrameMs, budget_.criticalFrameMs);
    budget_.renderBudgetMs = std::max(0.0, budget_.renderBudgetMs);
    budget_.simulationBudgetMs = std::max(0.0, budget_.simulationBudgetMs);
    budget_.streamingBudgetMs = std::max(0.0, budget_.streamingBudgetMs);
}

void ProductionPerformanceSystem::RecordFrame(const ProductionFrameSample& input)
{
    ProductionFrameSample sample = input;
    sample.frameMs = std::max(0.0, sample.frameMs);
    sample.renderMs = std::max(0.0, sample.renderMs);
    sample.simulationMs = std::max(0.0, sample.simulationMs);
    sample.streamingMs = std::max(0.0, sample.streamingMs);
    sample.entityCount = std::max(0, sample.entityCount);
    sample.drawCalls = std::max(0, sample.drawCalls);
    sample.gpuSubmissions = std::max(0, sample.gpuSubmissions);

    if (sample.frameMs > budget_.criticalFrameMs) ++hitchCount_;
    if (sample.renderMs > budget_.renderBudgetMs) ++renderBudgetViolations_;
    if (sample.simulationMs > budget_.simulationBudgetMs) ++simulationBudgetViolations_;
    if (sample.streamingMs > budget_.streamingBudgetMs) ++streamingBudgetViolations_;
    if (sample.transientAllocationBytes > budget_.transientAllocationBudgetBytes) ++allocationBudgetViolations_;

    history_.push_back(sample);
    while (history_.size() > kHistoryLimit) history_.pop_front();
}

ProductionPerformanceSnapshot ProductionPerformanceSystem::Snapshot() const
{
    ProductionPerformanceSnapshot out;
    if (history_.empty()) return out;

    out.sampleCount = static_cast<int>(history_.size());
    out.latestFrameMs = history_.back().frameMs;
    out.hitchCount = hitchCount_;
    out.renderBudgetViolations = renderBudgetViolations_;
    out.simulationBudgetViolations = simulationBudgetViolations_;
    out.streamingBudgetViolations = streamingBudgetViolations_;
    out.allocationBudgetViolations = allocationBudgetViolations_;
    out.entityCount = history_.back().entityCount;
    out.drawCalls = history_.back().drawCalls;
    out.gpuSubmissions = history_.back().gpuSubmissions;

    std::vector<double> frames;
    frames.reserve(history_.size());
    double total = 0.0;
    for (const auto& sample : history_) {
        frames.push_back(sample.frameMs);
        total += sample.frameMs;
    }
    out.averageFrameMs = total / static_cast<double>(frames.size());
    std::sort(frames.begin(), frames.end());
    const std::size_t p95Index = std::min(frames.size() - 1,
        static_cast<std::size_t>(std::ceil(static_cast<double>(frames.size()) * 0.95)) - 1);
    out.p95FrameMs = frames[p95Index];

    if (out.latestFrameMs > budget_.criticalFrameMs || out.p95FrameMs > budget_.criticalFrameMs) {
        out.health = FrameHealth::Critical;
    } else if (out.latestFrameMs > budget_.warningFrameMs || out.p95FrameMs > budget_.warningFrameMs) {
        out.health = FrameHealth::Warning;
    } else {
        out.health = FrameHealth::Good;
    }

    const auto& latest = history_.back();
    if (latest.renderMs > budget_.renderBudgetMs) out.activeWarnings.emplace_back("RENDER_BUDGET");
    if (latest.simulationMs > budget_.simulationBudgetMs) out.activeWarnings.emplace_back("SIMULATION_BUDGET");
    if (latest.streamingMs > budget_.streamingBudgetMs) out.activeWarnings.emplace_back("STREAMING_BUDGET");
    if (latest.transientAllocationBytes > budget_.transientAllocationBudgetBytes) out.activeWarnings.emplace_back("ALLOCATION_BUDGET");
    if (latest.frameMs > budget_.criticalFrameMs) out.activeWarnings.emplace_back("FRAME_HITCH");
    return out;
}

void ProductionPerformanceSystem::Reset()
{
    history_.clear();
    hitchCount_ = 0;
    renderBudgetViolations_ = 0;
    simulationBudgetViolations_ = 0;
    streamingBudgetViolations_ = 0;
    allocationBudgetViolations_ = 0;
}

} // namespace subspace
