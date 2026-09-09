#include "home/HomeBuildableTerrain.h"

#include <sstream>

namespace subspace {

HomeTerrainTileRule EvaluateHomeTerrainTile(const HomeBuildZone& zone, int x, int y) {
    HomeTerrainTileRule rule;
    if (x < 0 || y < 0 || x >= zone.gridWidth || y >= zone.gridHeight) {
        rule.terrain = HomeTerrainType::Blocked;
        rule.buildable = false;
        rule.allowsLogistics = false;
        return rule;
    }
    if (x < 4 && y < 4 && zone.type == HomeBuildZoneType::PlanetSurface) {
        rule.terrain = HomeTerrainType::ProtectedLandingZone;
        rule.buildable = true;
        rule.allowsLogistics = true;
        return rule;
    }
    if ((x * 11 + y * 7) % 43 == 0 && zone.type == HomeBuildZoneType::PlanetSurface) {
        rule.terrain = HomeTerrainType::WaterIce;
        rule.resourceTag = "ice";
        rule.allowsExtractor = true;
        return rule;
    }
    if (!zone.localResourceTags.empty() && (x * 5 + y * 3) % 19 == 0) {
        rule.terrain = HomeTerrainType::ResourcePatch;
        rule.resourceTag = zone.localResourceTags[static_cast<std::size_t>((x + y) % static_cast<int>(zone.localResourceTags.size()))];
        rule.allowsExtractor = true;
        return rule;
    }
    if ((x * 13 + y * 17) % 47 == 0) {
        rule.terrain = HomeTerrainType::RoughRock;
        rule.buildable = false;
        rule.allowsLogistics = false;
        return rule;
    }
    return rule;
}

bool CanPlaceStructureOnTerrain(HomeStructureType structureType, const HomeTerrainTileRule& terrain) {
    if (!terrain.buildable) return false;
    if (structureType == HomeStructureType::Extractor) return terrain.allowsExtractor;
    if (structureType == HomeStructureType::ConveyorHub || structureType == HomeStructureType::StorageDepot) return terrain.allowsLogistics;
    return terrain.terrain != HomeTerrainType::Blocked;
}

std::string HomeTerrainTypeName(HomeTerrainType type) {
    switch (type) {
        case HomeTerrainType::Plain: return "Plain";
        case HomeTerrainType::ResourcePatch: return "ResourcePatch";
        case HomeTerrainType::RoughRock: return "RoughRock";
        case HomeTerrainType::WaterIce: return "WaterIce";
        case HomeTerrainType::ProtectedLandingZone: return "ProtectedLandingZone";
        case HomeTerrainType::Blocked: return "Blocked";
    }
    return "Unknown";
}

std::string HomeTerrainTileRuleSummary(const HomeTerrainTileRule& rule) {
    std::ostringstream out;
    out << HomeTerrainTypeName(rule.terrain)
        << " buildable=" << (rule.buildable ? "yes" : "no")
        << " extractor=" << (rule.allowsExtractor ? "yes" : "no")
        << " resource=" << rule.resourceTag;
    return out.str();
}

} // namespace subspace
