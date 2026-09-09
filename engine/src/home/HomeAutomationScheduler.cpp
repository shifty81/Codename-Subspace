#include "home/HomeAutomationScheduler.h"
#include "home/HomeSurfaceBuilder.h"

#include <algorithm>
#include <sstream>

namespace subspace {
namespace {
HomeAutomationJob MakeJob(const std::string& id, HomeAutomationJobType type, const std::string& name, float seconds) {
    HomeAutomationJob job;
    job.id = id;
    job.type = type;
    job.displayName = name;
    job.secondsRemaining = seconds;
    return job;
}
}

HomeAutomationSchedulerState CreateDefaultHomeAutomationSchedule(const HomeSolarSystemState& home) {
    HomeAutomationSchedulerState scheduler;
    if (CountHomeStructures(home, HomeStructureType::Extractor) > 0) scheduler.jobs.push_back(MakeJob("extract-ore", HomeAutomationJobType::Extract, "Extract starter ore", 4.0f));
    if (CountHomeStructures(home, HomeStructureType::Refinery) > 0) scheduler.jobs.push_back(MakeJob("refine-ore", HomeAutomationJobType::Refine, "Refine starter ore", 6.0f));
    if (CountHomeStructures(home, HomeStructureType::Assembler) > 0) scheduler.jobs.push_back(MakeJob("assemble-components", HomeAutomationJobType::Assemble, "Assemble components", 9.0f));
    if (CountHomeStructures(home, HomeStructureType::ResearchLab) > 0) scheduler.jobs.push_back(MakeJob("analyze-data", HomeAutomationJobType::Research, "Analyze expedition data", 12.0f));
    return scheduler;
}

void RefreshHomeAutomationSchedule(HomeAutomationSchedulerState& scheduler,
                                   const HomeSolarSystemState& home,
                                   const HomeProductionPlan&,
                                   const HomeLogisticsReport&) {
    if (scheduler.jobs.empty()) {
        scheduler = CreateDefaultHomeAutomationSchedule(home);
    }
    const int cap = EstimateHomeAutomationCapacity(home);
    for (std::size_t i = 0; i < scheduler.jobs.size(); ++i) {
        scheduler.jobs[i].active = static_cast<int>(i) < cap;
    }
}

HomeFactoryTickReport TickHomeAutomation(HomeAutomationSchedulerState& scheduler,
                                         HomeSolarSystemState& home,
                                         HomeFactoryNetworkState& factory,
                                         float deltaSeconds) {
    HomeFactoryTickReport report = TickHomeFactoryNetwork(factory, deltaSeconds);
    scheduler.offlineSecondsSimulated += std::max(0.0f, deltaSeconds);
    for (auto& job : scheduler.jobs) {
        if (!job.active) continue;
        job.secondsRemaining -= deltaSeconds;
        if (job.secondsRemaining <= 0.0f) {
            ++scheduler.completedJobs;
            if (job.type == HomeAutomationJobType::Research) {
                AddHomeInventory(factory, "research-data", 1);
            }
            if (job.repeat) {
                job.secondsRemaining = (job.type == HomeAutomationJobType::Research) ? 12.0f : 6.0f;
            }
            else {
                job.active = false;
            }
        }
    }
    RecalculateHomeDerivedState(home, &factory);
    return report;
}

std::string HomeAutomationJobTypeName(HomeAutomationJobType type) {
    switch (type) {
        case HomeAutomationJobType::Extract: return "Extract";
        case HomeAutomationJobType::Refine: return "Refine";
        case HomeAutomationJobType::Assemble: return "Assemble";
        case HomeAutomationJobType::MoveCargo: return "MoveCargo";
        case HomeAutomationJobType::Research: return "Research";
        case HomeAutomationJobType::PrintModule: return "PrintModule";
        case HomeAutomationJobType::MaintainStructures: return "MaintainStructures";
        default: return "Unknown";
    }
}

std::string HomeAutomationSchedulerSummary(const HomeAutomationSchedulerState& scheduler) {
    const int active = static_cast<int>(std::count_if(scheduler.jobs.begin(), scheduler.jobs.end(), [](const HomeAutomationJob& job) { return job.active; }));
    std::ostringstream stream;
    stream << "Automation jobs=" << scheduler.jobs.size() << " active=" << active << " completed=" << scheduler.completedJobs;
    return stream.str();
}

} // namespace subspace
