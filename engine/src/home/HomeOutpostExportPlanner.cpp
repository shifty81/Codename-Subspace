#include "home/HomeOutpostExportPlanner.h"

#include <algorithm>

namespace subspace {

const char* HomeOutpostKindName(HomeOutpostKind kind) {
    switch (kind) {
    case HomeOutpostKind::MoonMine: return "Moon Mine";
    case HomeOutpostKind::BeltExtractor: return "Belt Extractor";
    case HomeOutpostKind::GasSkimmer: return "Gas Skimmer";
    case HomeOutpostKind::SolarCollector: return "Solar Collector";
    case HomeOutpostKind::RelayStation: return "Relay Station";
    }
    return "Unknown";
}

HomeOutpostExportSummary SummarizeHomeOutpostExports(const std::vector<HomeOutpostExportNode>& nodes) {
    HomeOutpostExportSummary summary;
    for (const auto& node : nodes) {
        if (!node.routeOnline) {
            summary.warnings.push_back(node.outpostId + " route offline");
            continue;
        }
        const float production = std::max(0.0f, node.productionPerMinute);
        const float delivered = std::min(production, std::max(0.0f, node.routeCapacityPerMinute));
        summary.totalProductionPerMinute += production;
        summary.totalDeliveredPerMinute += delivered;
        if (delivered + 0.01f < production) {
            summary.warnings.push_back(node.outpostId + " export route bottleneck");
        }
    }
    summary.bottleneckPerMinute = std::max(0.0f, summary.totalProductionPerMinute - summary.totalDeliveredPerMinute);
    return summary;
}

std::vector<HomeOutpostExportNode> CreateStarterHomeOutpostNodes() {
    return {
        {"moon_iron_01", HomeOutpostKind::MoonMine, "iron_ore", 12.0f, 80.0f, 9.0f, true},
        {"belt_scrap_01", HomeOutpostKind::BeltExtractor, "scrap", 8.0f, 40.0f, 8.0f, true},
        {"solar_swarm_01", HomeOutpostKind::SolarCollector, "power", 20.0f, 0.0f, 20.0f, true},
    };
}

} // namespace subspace
