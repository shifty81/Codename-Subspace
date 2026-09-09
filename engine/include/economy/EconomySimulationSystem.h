#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace subspace {

struct MarketCommodity {
    std::string id;
    double basePrice = 1.0;
    double stock = 0.0;
    double targetStock = 100.0;
    double demand = 1.0;
    double volatility = 0.25;
};

struct ManufacturingInput {
    std::string commodityId;
    std::uint64_t quantity = 0;
};

struct ManufacturingRecipeNative {
    std::string id;
    std::vector<ManufacturingInput> inputs;
    std::string outputCommodityId;
    std::uint64_t outputQuantity = 1;
    double minutesPerCycle = 1.0;
};

struct ManufacturingSite {
    std::string id;
    std::unordered_map<std::string, std::uint64_t> inventory;
    double efficiency = 1.0;
};

class EconomySimulationSystem {
public:
    void RegisterCommodity(const MarketCommodity& commodity);
    bool HasCommodity(const std::string& id) const;
    const MarketCommodity* GetCommodity(const std::string& id) const;

    double QuoteBuyPrice(const std::string& id) const;
    double QuoteSellPrice(const std::string& id) const;
    void AdjustStock(const std::string& id, double amount);
    void AdvanceMarkets(double minutes);

    void RegisterRecipe(const ManufacturingRecipeNative& recipe);
    const ManufacturingRecipeNative* GetRecipe(const std::string& id) const;
    std::uint64_t RunManufacturing(ManufacturingSite& site, const std::string& recipeId, double minutes) const;

    std::size_t CommodityCount() const { return commodities_.size(); }
    std::size_t RecipeCount() const { return recipes_.size(); }

private:
    std::unordered_map<std::string, MarketCommodity> commodities_;
    std::unordered_map<std::string, ManufacturingRecipeNative> recipes_;
};

} // namespace subspace
