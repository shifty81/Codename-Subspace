#pragma once

#include <string>
#include <vector>

namespace subspace {

enum class HomeOutpostKind {
    MoonMine,
    BeltExtractor,
    GasSkimmer,
    SolarCollector,
    RelayStation
};

struct HomeOutpostExportNode {
    std::string outpostId;
    HomeOutpostKind kind = HomeOutpostKind::MoonMine;
    std::string resourceTag = "ore";
    float productionPerMinute = 0.0f;
    float localStorage = 0.0f;
    float routeCapacityPerMinute = 0.0f;
    bool routeOnline = true;
};

struct HomeOutpostExportSummary {
    float totalProductionPerMinute = 0.0f;
    float totalDeliveredPerMinute = 0.0f;
    float bottleneckPerMinute = 0.0f;
    std::vector<std::string> warnings;
};

const char* HomeOutpostKindName(HomeOutpostKind kind);
HomeOutpostExportSummary SummarizeHomeOutpostExports(const std::vector<HomeOutpostExportNode>& nodes);
std::vector<HomeOutpostExportNode> CreateStarterHomeOutpostNodes();

} // namespace subspace
