#include "economy/EconomySimulationSystem.h"

#include <algorithm>
#include <cmath>

namespace subspace {

void EconomySimulationSystem::RegisterCommodity(const MarketCommodity& commodity) {
    if (commodity.id.empty()) return;
    MarketCommodity copy = commodity;
    copy.basePrice = std::max(0.01, copy.basePrice);
    copy.targetStock = std::max(1.0, copy.targetStock);
    copy.stock = std::max(0.0, copy.stock);
    copy.demand = std::max(0.05, copy.demand);
    copy.volatility = std::clamp(copy.volatility, 0.0, 2.0);
    commodities_[copy.id] = copy;
}

bool EconomySimulationSystem::HasCommodity(const std::string& id) const { return commodities_.find(id) != commodities_.end(); }
const MarketCommodity* EconomySimulationSystem::GetCommodity(const std::string& id) const {
    auto it = commodities_.find(id); return it == commodities_.end() ? nullptr : &it->second;
}

static double MarketPrice(const MarketCommodity& c) {
    const double scarcity = std::clamp((c.targetStock - c.stock) / c.targetStock, -1.0, 2.5);
    const double factor = std::max(0.15, 1.0 + scarcity * c.volatility * c.demand);
    return c.basePrice * factor;
}

double EconomySimulationSystem::QuoteBuyPrice(const std::string& id) const {
    const auto* c = GetCommodity(id); return c ? MarketPrice(*c) * 1.04 : 0.0;
}

double EconomySimulationSystem::QuoteSellPrice(const std::string& id) const {
    const auto* c = GetCommodity(id); return c ? MarketPrice(*c) * 0.96 : 0.0;
}

void EconomySimulationSystem::AdjustStock(const std::string& id, double amount) {
    auto it = commodities_.find(id); if (it == commodities_.end()) return;
    it->second.stock = std::max(0.0, it->second.stock + amount);
}

void EconomySimulationSystem::AdvanceMarkets(double minutes) {
    if (minutes <= 0.0) return;
    for (auto& pair : commodities_) {
        auto& c = pair.second;
        const double relaxation = std::clamp(minutes / 1440.0, 0.0, 1.0) * 0.15;
        c.stock += (c.targetStock - c.stock) * relaxation;
    }
}

void EconomySimulationSystem::RegisterRecipe(const ManufacturingRecipeNative& recipe) {
    if (recipe.id.empty() || recipe.outputCommodityId.empty() || recipe.outputQuantity == 0) return;
    ManufacturingRecipeNative copy = recipe;
    copy.minutesPerCycle = std::max(0.01, copy.minutesPerCycle);
    recipes_[copy.id] = std::move(copy);
}

const ManufacturingRecipeNative* EconomySimulationSystem::GetRecipe(const std::string& id) const {
    auto it = recipes_.find(id); return it == recipes_.end() ? nullptr : &it->second;
}

std::uint64_t EconomySimulationSystem::RunManufacturing(ManufacturingSite& site, const std::string& recipeId, double minutes) const {
    const auto* recipe = GetRecipe(recipeId);
    if (!recipe || minutes <= 0.0 || site.efficiency <= 0.0) return 0;
    std::uint64_t cycles = static_cast<std::uint64_t>(std::floor(minutes * site.efficiency / recipe->minutesPerCycle));
    if (cycles == 0) return 0;
    for (const auto& input : recipe->inputs) {
        auto it = site.inventory.find(input.commodityId);
        const std::uint64_t available = it == site.inventory.end() ? 0 : it->second;
        if (input.quantity > 0) cycles = std::min(cycles, available / input.quantity);
    }
    if (cycles == 0) return 0;
    for (const auto& input : recipe->inputs) site.inventory[input.commodityId] -= input.quantity * cycles;
    site.inventory[recipe->outputCommodityId] += recipe->outputQuantity * cycles;
    return cycles;
}

} // namespace subspace
