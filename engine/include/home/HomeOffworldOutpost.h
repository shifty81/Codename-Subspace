#pragma once

#include "home/HomeSolarSystem.h"
#include "home/HomeFactoryNetwork.h"

#include <string>
#include <vector>

namespace subspace {

enum class HomeOutpostType {
    MoonMine,
    BeltExtractor,
    SolarCollector,
    GasSkimmer,
    IceHarvester,
    OrbitalRelay
};

struct HomeOffworldOutpost {
    std::string id;
    std::string bodyId;
    std::string zoneId;
    HomeOutpostType type = HomeOutpostType::BeltExtractor;
    std::string commodity = "ore";
    int tier = 1;
    float baseUnitsPerMinute = 1.0f;
    bool exportingToHome = true;
    bool online = false;
};

struct HomeOutpostTickReport {
    float elapsedSeconds = 0.0f;
    std::vector<HomeInventoryStack> produced;
    std::string message;
};

std::vector<HomeOffworldOutpost> CreateDefaultHomeOutposts(const HomeSolarSystemState& home);
HomeOffworldOutpost CreateHomeOutpost(const std::string& id,
                                      const std::string& bodyId,
                                      const std::string& zoneId,
                                      HomeOutpostType type,
                                      const std::string& commodity,
                                      int tier = 1);
HomeOutpostTickReport TickHomeOutposts(const std::vector<HomeOffworldOutpost>& outposts,
                                       float deltaSeconds);
std::string HomeOutpostTypeName(HomeOutpostType type);
std::string HomeOutpostSummary(const HomeOffworldOutpost& outpost);

} // namespace subspace
