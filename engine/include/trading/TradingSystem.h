#pragma once

#include "core/ecs/SystemBase.h"
#include "core/resources/Inventory.h"

#include <unordered_map>

namespace subspace {

/// Manages resource trading with buy/sell price calculations.
class TradingSystem : public SystemBase {
public:
    TradingSystem();

    void Update(float /*deltaTime*/) override {}

    /// Get the base price for a resource type.
    float GetBasePrice(ResourceType type) const;

    /// Calculate buy price for a resource (20% markup).
    int GetBuyPrice(ResourceType type, int amount) const;

    /// Calculate sell price for a resource (20% markdown).
    int GetSellPrice(ResourceType type, int amount) const;

    /// Buy resources from a station, paying credits from inventory.
    bool BuyResource(ResourceType type, int amount, Inventory& inventory);

    /// Sell resources to a station, receiving credits into inventory.
    bool SellResource(ResourceType type, int amount, Inventory& inventory);

private:
    std::unordered_map<ResourceType, float> _basePrices;
};

} // namespace subspace
