#pragma once

#include "home/HomeSolarSystem.h"
#include "mining/MiningSalvageModel.h"

#include <map>
#include <string>
#include <vector>

namespace subspace {

struct HomeInventoryStack {
    std::string commodity;
    int units = 0;
};

struct HomeFactoryRecipe {
    std::string id;
    std::string displayName;
    std::vector<HomeInventoryStack> inputs;
    std::vector<HomeInventoryStack> outputs;
    float secondsPerCycle = 5.0f;
    int powerRequired = 1;
};

struct HomeFactoryTickReport {
    float elapsedSeconds = 0.0f;
    int cyclesCompleted = 0;
    std::vector<HomeInventoryStack> produced;
    std::vector<HomeInventoryStack> consumed;
    std::string message;
};

struct HomeFactoryNetworkState {
    std::string zoneId;
    std::map<std::string, int> inventory;
    std::vector<HomeFactoryRecipe> recipes;
    float accumulatedSeconds = 0.0f;
    int availablePower = 0;
    bool automationEnabled = true;
};

HomeFactoryNetworkState CreateStarterHomeFactoryNetwork(const HomeSolarSystemState& home);
HomeFactoryRecipe CreateRecipe(const std::string& id,
                               const std::string& displayName,
                               std::vector<HomeInventoryStack> inputs,
                               std::vector<HomeInventoryStack> outputs,
                               float secondsPerCycle,
                               int powerRequired);
void AddHomeInventory(HomeFactoryNetworkState& network, const std::string& commodity, int units);
int GetHomeInventoryUnits(const HomeFactoryNetworkState& network, const std::string& commodity);
HomeFactoryTickReport TickHomeFactoryNetwork(HomeFactoryNetworkState& network, float deltaSeconds);
std::vector<HomeFactoryRecipe> CreateStarterHomeRecipes();
std::string HomeFactoryInventorySummary(const HomeFactoryNetworkState& network);

} // namespace subspace
