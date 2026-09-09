#include "home/HomeSolarSystem.h"
#include "home/HomeFactoryNetwork.h"
#include "home/HomeSurfaceWorldView.h"
#include "home/HomeSystemOverview.h"
#include "home/HomeOffworldOutpost.h"
#include "home/HomeImportExportLogistics.h"
#include "home/HomeSurfaceProductionGraph.h"
#include "home/HomeBuildableTerrain.h"
#include "home/HomeShipPrep.h"
#include "client/VisualAcceptanceModel.h"
#include "migration/CppNormalizationRoadmap.h"

#include <cassert>
#include <iostream>

using namespace subspace;

int main() {
    auto home = CreateDefaultHomeSolarSystem();
    auto factory = CreateStarterHomeFactoryNetwork(home);

    auto surface = BuildPrimaryHomeSurfaceWorldView(home, factory, "", 8, 8, HomeSurfaceViewMode::SurfaceBuild);
    assert(!surface.zoneId.empty());
    assert(!surface.visibleTiles.empty());

    auto overview = BuildHomeSystemOverviewModel(home);
    assert(!overview.bodies.empty());

    auto outposts = CreateDefaultHomeOutposts(home);
    auto routes = BuildDefaultHomeImportRoutes(outposts, surface.zoneId);
    auto report = TickHomeImportExportRoutes(routes, factory, 120.0f);
    (void)report;

    auto graph = BuildHomeSurfaceProductionGraph(home, factory, surface.zoneId);
    assert(!graph.nodes.empty());

    const auto terrain = EvaluateHomeTerrainTile(home.buildZones.front(), 2, 2);
    assert(terrain.buildable);

    auto prep = CreateStarterHomeShipPrepState();
    auto validation = ValidateHomeShipPrepForAdventure(prep, factory);
    assert(validation.canLaunch);

    auto visuals = CreateSubspaceVisualAcceptanceChecklist();
    assert(!visuals.items.empty());

    auto roadmap = CreatePostPass114CppNormalizationRoadmap();
    assert(!roadmap.gates.empty());

    std::cout << HomeSurfaceWorldViewSummary(surface) << "\n";
    std::cout << HomeSystemOverviewSummary(overview) << "\n";
    std::cout << HomeSurfaceProductionGraphSummary(graph) << "\n";
    std::cout << HomeShipPrepValidationSummary(validation) << "\n";
    std::cout << VisualAcceptanceReportSummary(visuals) << "\n";
    std::cout << CppNormalizationRoadmapSummary(roadmap) << "\n";
    std::cout << "Pass105-114 home surface refocus smoke passed\n";
    return 0;
}
