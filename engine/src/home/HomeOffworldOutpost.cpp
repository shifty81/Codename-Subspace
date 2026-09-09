#include "home/HomeOffworldOutpost.h"

#include <sstream>

namespace subspace {

std::vector<HomeOffworldOutpost> CreateDefaultHomeOutposts(const HomeSolarSystemState& home) {
    std::vector<HomeOffworldOutpost> outposts;
    for (const auto& zone : home.buildZones) {
        if (zone.type == HomeBuildZoneType::PlanetSurface) {
            continue;
        }
        HomeOutpostType type = HomeOutpostType::OrbitalRelay;
        std::string commodity = "logistics";
        if (zone.type == HomeBuildZoneType::MoonSurface) {
            type = HomeOutpostType::MoonMine;
            commodity = zone.localResourceTags.empty() ? "ore" : zone.localResourceTags.front();
        } else if (zone.type == HomeBuildZoneType::AsteroidSurface) {
            type = HomeOutpostType::BeltExtractor;
            commodity = zone.localResourceTags.empty() ? "ore" : zone.localResourceTags.front();
        } else if (zone.type == HomeBuildZoneType::SolarOrbit) {
            type = HomeOutpostType::SolarCollector;
            commodity = "solar-energy";
        } else if (zone.type == HomeBuildZoneType::GasGiantOrbit) {
            type = HomeOutpostType::GasSkimmer;
            commodity = "fuel-gas";
        }
        outposts.push_back(CreateHomeOutpost("outpost-" + zone.id, zone.parentBodyId, zone.id, type, commodity, 1));
    }
    return outposts;
}

HomeOffworldOutpost CreateHomeOutpost(const std::string& id,
                                      const std::string& bodyId,
                                      const std::string& zoneId,
                                      HomeOutpostType type,
                                      const std::string& commodity,
                                      int tier) {
    HomeOffworldOutpost outpost;
    outpost.id = id;
    outpost.bodyId = bodyId;
    outpost.zoneId = zoneId;
    outpost.type = type;
    outpost.commodity = commodity;
    outpost.tier = tier < 1 ? 1 : tier;
    outpost.baseUnitsPerMinute = (type == HomeOutpostType::SolarCollector) ? 4.0f : 1.5f;
    outpost.online = true;
    return outpost;
}

HomeOutpostTickReport TickHomeOutposts(const std::vector<HomeOffworldOutpost>& outposts,
                                       float deltaSeconds) {
    HomeOutpostTickReport report;
    report.elapsedSeconds = deltaSeconds;
    for (const auto& outpost : outposts) {
        if (!outpost.online || !outpost.exportingToHome) {
            continue;
        }
        const float units = (outpost.baseUnitsPerMinute * static_cast<float>(outpost.tier) * deltaSeconds) / 60.0f;
        if (units >= 1.0f) {
            report.produced.push_back({outpost.commodity, static_cast<int>(units)});
        }
    }
    std::ostringstream msg;
    msg << "Outposts ticked: " << outposts.size() << " producedStacks=" << report.produced.size();
    report.message = msg.str();
    return report;
}

std::string HomeOutpostTypeName(HomeOutpostType type) {
    switch (type) {
        case HomeOutpostType::MoonMine: return "MoonMine";
        case HomeOutpostType::BeltExtractor: return "BeltExtractor";
        case HomeOutpostType::SolarCollector: return "SolarCollector";
        case HomeOutpostType::GasSkimmer: return "GasSkimmer";
        case HomeOutpostType::IceHarvester: return "IceHarvester";
        case HomeOutpostType::OrbitalRelay: return "OrbitalRelay";
    }
    return "Unknown";
}

std::string HomeOutpostSummary(const HomeOffworldOutpost& outpost) {
    std::ostringstream out;
    out << outpost.id << " type=" << HomeOutpostTypeName(outpost.type)
        << " commodity=" << outpost.commodity
        << " tier=" << outpost.tier
        << " online=" << (outpost.online ? "yes" : "no");
    return out.str();
}

} // namespace subspace
