#pragma once

#include "home/HomeFactoryNetwork.h"
#include "home/HomeOffworldOutpost.h"

#include <string>
#include <vector>

namespace subspace {

struct HomeImportExportRoute {
    std::string id;
    std::string sourceId;
    std::string destinationZoneId;
    std::string commodity;
    float capacityPerMinute = 1.0f;
    float queuedUnits = 0.0f;
    bool enabled = true;
};

struct HomeImportExportReport {
    float elapsedSeconds = 0.0f;
    std::vector<HomeInventoryStack> delivered;
    std::vector<std::string> bottlenecks;
    std::string message;
};

std::vector<HomeImportExportRoute> BuildDefaultHomeImportRoutes(const std::vector<HomeOffworldOutpost>& outposts,
                                                                const std::string& homeZoneId);
HomeImportExportReport TickHomeImportExportRoutes(std::vector<HomeImportExportRoute>& routes,
                                                  HomeFactoryNetworkState& homeFactory,
                                                  float deltaSeconds);
std::string HomeImportExportRouteSummary(const HomeImportExportRoute& route);
std::string HomeImportExportReportSummary(const HomeImportExportReport& report);

} // namespace subspace
