#include "home/HomeSurfaceProductionGraph.h"

#include <sstream>

namespace subspace {

HomeSurfaceProductionGraph BuildHomeSurfaceProductionGraph(const HomeSolarSystemState& home,
                                                           const HomeFactoryNetworkState&,
                                                           const std::string& zoneId) {
    HomeSurfaceProductionGraph graph;
    bool hasExtractor = false;
    bool hasRefinery = false;
    bool hasStorage = false;
    bool hasShipyard = false;
    for (const auto& structure : home.structures) {
        if (!zoneId.empty() && structure.zoneId != zoneId) continue;
        HomeProductionGraphNode node;
        node.id = structure.id;
        node.label = HomeStructureTypeName(structure.type);
        node.structureType = structure.type;
        node.active = structure.powered || structure.type == HomeStructureType::LandingPad || structure.type == HomeStructureType::ShipyardBay;
        node.ratePerMinute = static_cast<float>(structure.tier);
        if (structure.type == HomeStructureType::Extractor) { hasExtractor = true; node.commodity = "ore"; }
        if (structure.type == HomeStructureType::Refinery) { hasRefinery = true; node.commodity = "ingot"; }
        if (structure.type == HomeStructureType::StorageDepot) { hasStorage = true; node.commodity = "storage"; }
        if (structure.type == HomeStructureType::ShipyardBay) { hasShipyard = true; node.commodity = "ship-parts"; }
        graph.nodes.push_back(node);
    }
    if (hasExtractor && hasRefinery) graph.edges.push_back({"extractor", "refinery", "ore", 3.0f});
    if (hasRefinery && hasStorage) graph.edges.push_back({"refinery", "storage", "ingot", 2.0f});
    if (hasStorage && hasShipyard) graph.edges.push_back({"storage", "shipyard", "ship-parts", 1.0f});
    if (!hasExtractor) graph.missingLinks.push_back("Place an extractor on resource terrain.");
    if (!hasRefinery) graph.missingLinks.push_back("Place a refinery after extraction.");
    if (!hasStorage) graph.missingLinks.push_back("Place storage to buffer production.");
    if (!hasShipyard) graph.missingLinks.push_back("Build or upgrade a shipyard bay for launch prep.");
    return graph;
}

std::string HomeSurfaceProductionGraphSummary(const HomeSurfaceProductionGraph& graph) {
    std::ostringstream out;
    out << "productionGraph nodes=" << graph.nodes.size()
        << " edges=" << graph.edges.size()
        << " missing=" << graph.missingLinks.size();
    return out.str();
}

} // namespace subspace
