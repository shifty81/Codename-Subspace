#include "home/HomeLogisticsNetwork.h"

#include <algorithm>
#include <sstream>

namespace subspace {

HomeLogisticsReport AnalyzeHomeLogisticsNetwork(const HomeSolarSystemState& home,
                                                const HomeProductionPlan& production) {
    HomeLogisticsReport report;
    report.conveyorHubs = CountHomeStructures(home, HomeStructureType::ConveyorHub);
    report.droneDepots = CountHomeStructures(home, HomeStructureType::DroneDepot);
    report.storageDepots = CountHomeStructures(home, HomeStructureType::StorageDepot);
    const float conveyorThroughput = static_cast<float>(report.conveyorHubs) * 20.0f;
    const float droneThroughput = static_cast<float>(report.droneDepots) * 12.0f;
    report.totalThroughputPerMinute = conveyorThroughput + droneThroughput;

    for (const auto& route : production.routes) {
        HomeLogisticsLink link;
        link.id = "logistics-" + route.id;
        link.fromZoneId = "source";
        link.toZoneId = "destination";
        link.commodity = route.commodity;
        link.throughputPerMinute = std::min(route.unitsPerMinute, report.totalThroughputPerMinute);
        link.blocked = !route.active || report.totalThroughputPerMinute <= 0.0f;
        report.links.push_back(link);
    }

    if (report.totalThroughputPerMinute <= 0.0f) {
        report.bottleneck = "no logistics hubs";
    }
    else if (production.rawOrePerMinute > report.totalThroughputPerMinute) {
        report.bottleneck = "ore throughput";
    }
    else if (report.storageDepots == 0) {
        report.bottleneck = "storage";
    }
    else {
        report.bottleneck = "healthy";
    }
    return report;
}

std::string HomeLogisticsSummary(const HomeLogisticsReport& report) {
    std::ostringstream stream;
    stream << "Logistics throughput=" << report.totalThroughputPerMinute << "/min hubs=" << report.conveyorHubs
           << " drones=" << report.droneDepots << " storage=" << report.storageDepots
           << " bottleneck=" << report.bottleneck;
    return stream.str();
}

} // namespace subspace
