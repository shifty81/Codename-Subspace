#pragma once

#include "home/HomeSolarSystem.h"

#include <string>
#include <vector>

namespace subspace {

struct HomeWorldConfigPreset {
    std::string id;
    std::string displayName;
    std::string description;
    HomeWorldConfig config;
};

std::vector<HomeWorldConfigPreset> CreateHomeWorldConfigPresets();
HomeWorldConfig CreateHomeWorldConfigById(const std::string& id);
std::string HomeWorldConfigSummary(const HomeWorldConfig& config);

} // namespace subspace
