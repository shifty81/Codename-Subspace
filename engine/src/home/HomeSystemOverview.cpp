#include "home/HomeSystemOverview.h"

#include <sstream>

namespace subspace {

HomeSystemOverviewModel BuildHomeSystemOverviewModel(const HomeSolarSystemState& home) {
    HomeSystemOverviewModel overview;
    overview.title = "Home Solar System Overview";
    for (const auto& body : home.systemDefinition.bodies) {
        HomeOverviewBodyNode node;
        node.bodyId = body.id;
        node.displayName = body.displayName.empty() ? body.id : body.displayName;
        node.role = CelestialBodyTypeName(body.type);
        node.orbitRadius = body.orbitRadius;
        node.orbitAngleRadians = body.orbitAngleRadians;
        node.primaryHomeWorld = body.id.find("home") != std::string::npos || node.displayName.find("Founder") != std::string::npos;
        node.supportsOutposts = body.type != CelestialBodyType::Star && body.type != CelestialBodyType::GalaxyBackdrop;
        std::ostringstream resources;
        for (std::size_t i = 0; i < body.resourceTags.size(); ++i) {
            if (i > 0) resources << ",";
            resources << body.resourceTags[i];
        }
        node.resourceSummary = resources.str();
        overview.bodies.push_back(node);
    }

    for (const auto& zone : home.buildZones) {
        if (zone.type == HomeBuildZoneType::MoonSurface ||
            zone.type == HomeBuildZoneType::AsteroidSurface ||
            zone.type == HomeBuildZoneType::SolarOrbit ||
            zone.type == HomeBuildZoneType::OrbitalPlatform) {
            HomeOverviewRouteNode route;
            route.id = "route-" + zone.id;
            route.fromBodyId = zone.parentBodyId;
            route.toBodyId = "home-planet";
            route.commodity = zone.localResourceTags.empty() ? "mixed-resources" : zone.localResourceTags.front();
            route.unitsPerMinute = 0.0f;
            route.active = false;
            for (const auto& structure : home.structures) {
                if (structure.zoneId == zone.id && structure.type == HomeStructureType::Extractor) {
                    route.unitsPerMinute += 2.0f * static_cast<float>(structure.tier);
                    route.active = true;
                }
            }
            overview.routes.push_back(route);
        }
    }
    overview.summary = HomeSystemOverviewSummary(overview);
    return overview;
}

std::string HomeSystemOverviewSummary(const HomeSystemOverviewModel& overview) {
    int activeRoutes = 0;
    for (const auto& route : overview.routes) {
        if (route.active) ++activeRoutes;
    }
    std::ostringstream out;
    out << overview.title << ": bodies=" << overview.bodies.size()
        << " routes=" << overview.routes.size()
        << " activeRoutes=" << activeRoutes;
    return out.str();
}

} // namespace subspace
