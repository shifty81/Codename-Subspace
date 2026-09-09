#pragma once

#include "home/HomeLogisticsNetwork.h"

#include <string>
#include <vector>

namespace subspace {

enum class HomeAutomationJobType {
    Extract,
    Refine,
    Assemble,
    MoveCargo,
    Research,
    PrintModule,
    MaintainStructures
};

struct HomeAutomationJob {
    std::string id;
    HomeAutomationJobType type = HomeAutomationJobType::Extract;
    std::string displayName;
    float secondsRemaining = 0.0f;
    bool repeat = true;
    bool active = true;
};

struct HomeAutomationSchedulerState {
    std::vector<HomeAutomationJob> jobs;
    float offlineSecondsSimulated = 0.0f;
    int completedJobs = 0;
};

HomeAutomationSchedulerState CreateDefaultHomeAutomationSchedule(const HomeSolarSystemState& home);
void RefreshHomeAutomationSchedule(HomeAutomationSchedulerState& scheduler,
                                   const HomeSolarSystemState& home,
                                   const HomeProductionPlan& production,
                                   const HomeLogisticsReport& logistics);
HomeFactoryTickReport TickHomeAutomation(HomeAutomationSchedulerState& scheduler,
                                         HomeSolarSystemState& home,
                                         HomeFactoryNetworkState& factory,
                                         float deltaSeconds);
std::string HomeAutomationJobTypeName(HomeAutomationJobType type);
std::string HomeAutomationSchedulerSummary(const HomeAutomationSchedulerState& scheduler);

} // namespace subspace
