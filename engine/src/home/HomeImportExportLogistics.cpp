#include "home/HomeImportExportLogistics.h"

#include <sstream>

namespace subspace {

std::vector<HomeImportExportRoute> BuildDefaultHomeImportRoutes(const std::vector<HomeOffworldOutpost>& outposts,
                                                                const std::string& homeZoneId) {
    std::vector<HomeImportExportRoute> routes;
    for (const auto& outpost : outposts) {
        HomeImportExportRoute route;
        route.id = "import-" + outpost.id;
        route.sourceId = outpost.id;
        route.destinationZoneId = homeZoneId;
        route.commodity = outpost.commodity;
        route.capacityPerMinute = outpost.baseUnitsPerMinute * static_cast<float>(outpost.tier);
        route.enabled = outpost.online && outpost.exportingToHome;
        routes.push_back(route);
    }
    return routes;
}

HomeImportExportReport TickHomeImportExportRoutes(std::vector<HomeImportExportRoute>& routes,
                                                  HomeFactoryNetworkState& homeFactory,
                                                  float deltaSeconds) {
    HomeImportExportReport report;
    report.elapsedSeconds = deltaSeconds;
    for (auto& route : routes) {
        if (!route.enabled) {
            continue;
        }
        route.queuedUnits += (route.capacityPerMinute * deltaSeconds) / 60.0f;
        const int deliverable = static_cast<int>(route.queuedUnits);
        if (deliverable > 0) {
            route.queuedUnits -= static_cast<float>(deliverable);
            AddHomeInventory(homeFactory, route.commodity, deliverable);
            report.delivered.push_back({route.commodity, deliverable});
        }
        if (route.capacityPerMinute <= 0.0f) {
            report.bottlenecks.push_back(route.id + ": no capacity");
        }
    }
    report.message = HomeImportExportReportSummary(report);
    return report;
}

std::string HomeImportExportRouteSummary(const HomeImportExportRoute& route) {
    std::ostringstream out;
    out << route.id << " " << route.sourceId << " -> " << route.destinationZoneId
        << " commodity=" << route.commodity
        << " cap=" << route.capacityPerMinute << "/min"
        << " queued=" << route.queuedUnits
        << " enabled=" << (route.enabled ? "yes" : "no");
    return out.str();
}

std::string HomeImportExportReportSummary(const HomeImportExportReport& report) {
    int total = 0;
    for (const auto& stack : report.delivered) total += stack.units;
    std::ostringstream out;
    out << "Home import/export tick delivered=" << total
        << " stacks=" << report.delivered.size()
        << " bottlenecks=" << report.bottlenecks.size();
    return out.str();
}

} // namespace subspace
