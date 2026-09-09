#include "home/HomeSurfaceWorldView.h"
#include "home/HomeSurfaceBuilder.h"
#include "home/HomeProductionPlanner.h"
#include "home/HomePowerGrid.h"

#include <algorithm>
#include <sstream>

namespace subspace {

namespace {
const HomeBuildZone* SelectSurfaceZone(const HomeSolarSystemState& home, const std::string& preferredZoneId) {
    if (!preferredZoneId.empty()) {
        if (const HomeBuildZone* zone = FindHomeBuildZone(home, preferredZoneId)) {
            return zone;
        }
    }
    for (const auto& zone : home.buildZones) {
        if (zone.type == HomeBuildZoneType::PlanetSurface) {
            return &zone;
        }
    }
    return home.buildZones.empty() ? nullptr : &home.buildZones.front();
}

std::string TerrainForTile(const HomeBuildZone& zone, int x, int y) {
    if ((x + y) % 17 == 0 && !zone.localResourceTags.empty()) {
        return "resource";
    }
    if ((x * 3 + y * 5) % 31 == 0) {
        return "rough";
    }
    return "plain";
}

std::string ResourceForTile(const HomeBuildZone& zone, int x, int y) {
    if (zone.localResourceTags.empty() || (x + y) % 17 != 0) {
        return {};
    }
    return zone.localResourceTags[static_cast<std::size_t>((x + y) % static_cast<int>(zone.localResourceTags.size()))];
}
} // namespace

HomeSurfaceWorldView BuildPrimaryHomeSurfaceWorldView(const HomeSolarSystemState& home,
                                                      const HomeFactoryNetworkState& factory,
                                                      const std::string& preferredZoneId,
                                                      int cursorX,
                                                      int cursorY,
                                                      HomeSurfaceViewMode mode) {
    HomeSurfaceWorldView view;
    view.mode = mode;
    view.inventorySummary = HomeFactoryInventorySummary(factory);
    view.productionSummary = HomeProductionPlanSummary(BuildHomeProductionPlan(home, factory));
    view.powerSummary = HomePowerGridSummary(AnalyzeHomePowerGrid(home));
    view.instructionText = "H home surface, Tab overview, B build, arrows move cursor, 1-9 palette, P place, Del remove";

    const HomeBuildZone* zone = SelectSurfaceZone(home, preferredZoneId);
    if (!zone) {
        view.title = "Home Surface: no build zones";
        view.warnings.push_back("No home build zones exist yet.");
        return view;
    }

    view.zoneId = zone->id;
    view.title = zone->displayName + " Surface Base";
    view.gridWidth = zone->gridWidth;
    view.gridHeight = zone->gridHeight;
    view.cursorX = std::max(0, std::min(cursorX, std::max(0, zone->gridWidth - 1)));
    view.cursorY = std::max(0, std::min(cursorY, std::max(0, zone->gridHeight - 1)));

    const int minX = std::max(0, view.cursorX - view.visibleRadius);
    const int maxX = std::min(zone->gridWidth - 1, view.cursorX + view.visibleRadius);
    const int minY = std::max(0, view.cursorY - view.visibleRadius);
    const int maxY = std::min(zone->gridHeight - 1, view.cursorY + view.visibleRadius);

    for (int y = minY; y <= maxY; ++y) {
        for (int x = minX; x <= maxX; ++x) {
            HomeSurfaceTileView tile;
            tile.x = x;
            tile.y = y;
            tile.terrain = TerrainForTile(*zone, x, y);
            tile.resourceTag = ResourceForTile(*zone, x, y);
            tile.buildable = tile.terrain != "rough" || mode != HomeSurfaceViewMode::SurfaceBuild;
            if (const HomeStructure* structure = FindHomeStructureAt(home, zone->id, x, y)) {
                tile.occupied = true;
                tile.structureId = structure->id;
                tile.structureType = structure->type;
                tile.tier = structure->tier;
            }
            view.visibleTiles.push_back(tile);
        }
    }

    if (CountHomeStructures(home, HomeStructureType::Extractor) == 0) {
        view.warnings.push_back("No extractors placed on the primary home surface yet.");
    }
    if (CountHomeStructures(home, HomeStructureType::StorageDepot) == 0) {
        view.warnings.push_back("No storage depots placed on the primary home surface yet.");
    }
    return view;
}

std::string HomeSurfaceViewModeName(HomeSurfaceViewMode mode) {
    switch (mode) {
        case HomeSurfaceViewMode::SurfaceBuild: return "surface-build";
        case HomeSurfaceViewMode::Logistics: return "logistics";
        case HomeSurfaceViewMode::Power: return "power";
        case HomeSurfaceViewMode::Production: return "production";
        case HomeSurfaceViewMode::ShipPrep: return "ship-prep";
    }
    return "unknown";
}

std::string HomeSurfaceWorldViewSummary(const HomeSurfaceWorldView& view) {
    std::ostringstream out;
    out << view.title << " mode=" << HomeSurfaceViewModeName(view.mode)
        << " zone=" << view.zoneId
        << " grid=" << view.gridWidth << "x" << view.gridHeight
        << " cursor=" << view.cursorX << "," << view.cursorY
        << " visibleTiles=" << view.visibleTiles.size();
    if (!view.warnings.empty()) {
        out << " warnings=" << view.warnings.size();
    }
    return out.str();
}

} // namespace subspace
