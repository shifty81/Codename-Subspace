#include "home/HomeFactoryNetwork.h"

#include <algorithm>
#include <sstream>

namespace subspace {

HomeFactoryRecipe CreateRecipe(const std::string& id,
                               const std::string& displayName,
                               std::vector<HomeInventoryStack> inputs,
                               std::vector<HomeInventoryStack> outputs,
                               float secondsPerCycle,
                               int powerRequired) {
    HomeFactoryRecipe recipe;
    recipe.id = id;
    recipe.displayName = displayName;
    recipe.inputs = std::move(inputs);
    recipe.outputs = std::move(outputs);
    recipe.secondsPerCycle = std::max(0.1f, secondsPerCycle);
    recipe.powerRequired = std::max(0, powerRequired);
    return recipe;
}

std::vector<HomeFactoryRecipe> CreateStarterHomeRecipes() {
    return {
        CreateRecipe("smelt-ore", "Smelt Ore", {{"ore", 3}}, {{"ingot", 1}}, 4.0f, 1),
        CreateRecipe("refine-scrap", "Recover Scrap Parts", {{"scrap", 2}}, {{"recovered-parts", 1}}, 5.0f, 1),
        CreateRecipe("press-plate", "Press Hull Plate", {{"ingot", 2}}, {{"hull-plate", 1}}, 6.0f, 2),
        CreateRecipe("crack-ice", "Crack Ice Fuel", {{"ice", 2}}, {{"fuel", 1}, {"water", 1}}, 7.0f, 2),
        CreateRecipe("print-basic-module", "Print Basic Module Parts", {{"hull-plate", 2}, {"recovered-parts", 2}}, {{"module-component", 1}}, 12.0f, 4)
    };
}

HomeFactoryNetworkState CreateStarterHomeFactoryNetwork(const HomeSolarSystemState& home) {
    HomeFactoryNetworkState network;
    network.zoneId = home.buildZones.empty() ? "home-world-surface-a" : home.buildZones.front().id;
    network.recipes = CreateStarterHomeRecipes();
    network.availablePower = EstimateHomePowerGeneration(home);
    network.automationEnabled = true;
    AddHomeInventory(network, "ore", 12);
    AddHomeInventory(network, "scrap", 6);
    AddHomeInventory(network, "ice", 4);
    return network;
}

void AddHomeInventory(HomeFactoryNetworkState& network, const std::string& commodity, int units) {
    if (commodity.empty() || units == 0) return;
    int& current = network.inventory[commodity];
    current = std::max(0, current + units);
}

int GetHomeInventoryUnits(const HomeFactoryNetworkState& network, const std::string& commodity) {
    const auto found = network.inventory.find(commodity);
    return found == network.inventory.end() ? 0 : found->second;
}

namespace {
bool CanRunRecipe(const HomeFactoryNetworkState& network, const HomeFactoryRecipe& recipe) {
    if (!network.automationEnabled || network.availablePower < recipe.powerRequired) return false;
    for (const auto& input : recipe.inputs) {
        if (GetHomeInventoryUnits(network, input.commodity) < input.units) return false;
    }
    return true;
}

void ApplyRecipe(HomeFactoryNetworkState& network, const HomeFactoryRecipe& recipe, HomeFactoryTickReport& report) {
    for (const auto& input : recipe.inputs) {
        AddHomeInventory(network, input.commodity, -input.units);
        report.consumed.push_back(input);
    }
    for (const auto& output : recipe.outputs) {
        AddHomeInventory(network, output.commodity, output.units);
        report.produced.push_back(output);
    }
}
}

HomeFactoryTickReport TickHomeFactoryNetwork(HomeFactoryNetworkState& network, float deltaSeconds) {
    HomeFactoryTickReport report;
    report.elapsedSeconds = std::max(0.0f, deltaSeconds);
    network.accumulatedSeconds += report.elapsedSeconds;

    if (network.recipes.empty()) {
        report.message = "No home factory recipes registered.";
        return report;
    }

    bool ranAny = false;
    for (const auto& recipe : network.recipes) {
        while (network.accumulatedSeconds >= recipe.secondsPerCycle && CanRunRecipe(network, recipe)) {
            network.accumulatedSeconds -= recipe.secondsPerCycle;
            ApplyRecipe(network, recipe, report);
            ++report.cyclesCompleted;
            ranAny = true;
        }
    }

    report.message = ranAny ? "Home factory completed production cycles." : "Home factory idle or waiting for inputs.";
    return report;
}

std::string HomeFactoryInventorySummary(const HomeFactoryNetworkState& network) {
    std::ostringstream out;
    out << "Home inventory";
    for (const auto& pair : network.inventory) {
        out << " " << pair.first << "=" << pair.second;
    }
    return out.str();
}

} // namespace subspace
