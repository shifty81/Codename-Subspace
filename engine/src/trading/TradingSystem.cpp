#include "trading/TradingSystem.h"

namespace subspace {

TradingSystem::TradingSystem() : SystemBase("TradingSystem") {
    _basePrices = {
        {ResourceType::Iron,     10.0f},
        {ResourceType::Titanium, 25.0f},
        {ResourceType::Naonite,  50.0f},
        {ResourceType::Trinium, 100.0f},
        {ResourceType::Xanion,  200.0f},
        {ResourceType::Ogonite, 400.0f},
        {ResourceType::Avorion, 800.0f}
    };
}

float TradingSystem::GetBasePrice(ResourceType type) const {
    auto it = _basePrices.find(type);
    if (it != _basePrices.end()) return it->second;
    return 10.0f; // fallback
}

int TradingSystem::GetBuyPrice(ResourceType type, int amount) const {
    return static_cast<int>(GetBasePrice(type) * amount * 1.2f);
}

int TradingSystem::GetSellPrice(ResourceType type, int amount) const {
    return static_cast<int>(GetBasePrice(type) * amount * 0.8f);
}

bool TradingSystem::BuyResource(ResourceType type, int amount, Inventory& inventory) {
    int cost = GetBuyPrice(type, amount);
    if (!inventory.HasResource(ResourceType::Credits, cost))
        return false;

    inventory.RemoveResource(ResourceType::Credits, cost);
    inventory.AddResource(type, amount);
    return true;
}

bool TradingSystem::SellResource(ResourceType type, int amount, Inventory& inventory) {
    if (!inventory.HasResource(type, amount))
        return false;

    inventory.RemoveResource(type, amount);
    int credits = GetSellPrice(type, amount);
    inventory.AddResource(ResourceType::Credits, credits);
    return true;
}

} // namespace subspace
