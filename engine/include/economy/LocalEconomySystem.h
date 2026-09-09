#pragma once

#include "economy/MarketContractSystem.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace subspace {

struct CommodityEconomyState { double stock=0;double desiredStock=100;double basePrice=10;double currentPrice=10;std::vector<double> priceHistory; };
struct LocalEconomyState { std::string id;std::unordered_map<std::string,CommodityEconomyState> commodities;double industrialDemand=1;double security=1; };

class LocalEconomySystem {
public:
    void Register(LocalEconomyState& state,const std::string& commodity,double stock,double desired,double basePrice) const;
    void ApplySupplyShock(LocalEconomyState& state,const std::string& commodity,double deltaStock) const;
    void Tick(LocalEconomyState& state,double hours) const;
    SandboxContract GenerateContract(const LocalEconomyState& state,const std::string& commodity) const;
};

} // namespace subspace
