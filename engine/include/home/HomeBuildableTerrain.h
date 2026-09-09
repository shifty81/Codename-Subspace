#pragma once

#include "home/HomeSolarSystem.h"

#include <string>
#include <vector>

namespace subspace {

enum class HomeTerrainType {
    Plain,
    ResourcePatch,
    RoughRock,
    WaterIce,
    ProtectedLandingZone,
    Blocked
};

struct HomeTerrainTileRule {
    HomeTerrainType terrain = HomeTerrainType::Plain;
    std::string resourceTag;
    bool buildable = true;
    bool allowsExtractor = false;
    bool allowsLogistics = true;
};

HomeTerrainTileRule EvaluateHomeTerrainTile(const HomeBuildZone& zone, int x, int y);
bool CanPlaceStructureOnTerrain(HomeStructureType structureType, const HomeTerrainTileRule& terrain);
std::string HomeTerrainTypeName(HomeTerrainType type);
std::string HomeTerrainTileRuleSummary(const HomeTerrainTileRule& rule);

} // namespace subspace
