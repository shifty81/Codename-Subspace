#pragma once

#include "home/HomeFactoryNetwork.h"
#include "home/HomeSolarSystem.h"

#include <string>
#include <vector>

namespace subspace {

enum class HomeSurfaceViewMode {
    SurfaceBuild,
    Logistics,
    Power,
    Production,
    ShipPrep
};

struct HomeSurfaceTileView {
    int x = 0;
    int y = 0;
    std::string terrain = "plain";
    std::string resourceTag;
    bool buildable = true;
    bool occupied = false;
    std::string structureId;
    HomeStructureType structureType = HomeStructureType::Unknown;
    int tier = 0;
};

struct HomeSurfaceWorldView {
    std::string zoneId;
    std::string title;
    HomeSurfaceViewMode mode = HomeSurfaceViewMode::SurfaceBuild;
    int gridWidth = 0;
    int gridHeight = 0;
    int cursorX = 0;
    int cursorY = 0;
    int visibleRadius = 18;
    std::string instructionText;
    std::string inventorySummary;
    std::string productionSummary;
    std::string powerSummary;
    std::vector<HomeSurfaceTileView> visibleTiles;
    std::vector<std::string> warnings;
};

HomeSurfaceWorldView BuildPrimaryHomeSurfaceWorldView(const HomeSolarSystemState& home,
                                                      const HomeFactoryNetworkState& factory,
                                                      const std::string& preferredZoneId,
                                                      int cursorX,
                                                      int cursorY,
                                                      HomeSurfaceViewMode mode);
std::string HomeSurfaceViewModeName(HomeSurfaceViewMode mode);
std::string HomeSurfaceWorldViewSummary(const HomeSurfaceWorldView& view);

} // namespace subspace
