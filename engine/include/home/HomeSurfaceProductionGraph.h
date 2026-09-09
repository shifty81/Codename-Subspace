#pragma once

#include "home/HomeFactoryNetwork.h"
#include "home/HomeSolarSystem.h"

#include <string>
#include <vector>

namespace subspace {

struct HomeProductionGraphNode {
    std::string id;
    std::string label;
    HomeStructureType structureType = HomeStructureType::Unknown;
    std::string commodity;
    float ratePerMinute = 0.0f;
    bool active = false;
};

struct HomeProductionGraphEdge {
    std::string fromNodeId;
    std::string toNodeId;
    std::string commodity;
    float ratePerMinute = 0.0f;
};

struct HomeSurfaceProductionGraph {
    std::vector<HomeProductionGraphNode> nodes;
    std::vector<HomeProductionGraphEdge> edges;
    std::vector<std::string> missingLinks;
};

HomeSurfaceProductionGraph BuildHomeSurfaceProductionGraph(const HomeSolarSystemState& home,
                                                           const HomeFactoryNetworkState& factory,
                                                           const std::string& zoneId);
std::string HomeSurfaceProductionGraphSummary(const HomeSurfaceProductionGraph& graph);

} // namespace subspace
