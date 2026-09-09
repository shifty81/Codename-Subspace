#pragma once

#include "home/HomeProductionPlanner.h"

#include <string>
#include <vector>

namespace subspace {

struct HomeLogisticsLink {
    std::string id;
    std::string fromZoneId;
    std::string toZoneId;
    std::string commodity;
    float throughputPerMinute = 0.0f;
    bool blocked = false;
};

struct HomeLogisticsReport {
    int conveyorHubs = 0;
    int droneDepots = 0;
    int storageDepots = 0;
    float totalThroughputPerMinute = 0.0f;
    std::vector<HomeLogisticsLink> links;
    std::string bottleneck;
};

HomeLogisticsReport AnalyzeHomeLogisticsNetwork(const HomeSolarSystemState& home,
                                                const HomeProductionPlan& production);
std::string HomeLogisticsSummary(const HomeLogisticsReport& report);

} // namespace subspace
