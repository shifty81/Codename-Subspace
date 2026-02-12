#include "core/resources/Inventory.h"

namespace subspace {

Inventory::Inventory()
{
    for (int i = 0; i < static_cast<int>(ResourceType::Count); ++i) {
        _resources[static_cast<ResourceType>(i)] = 0;
    }
}

bool Inventory::AddResource(ResourceType type, int amount)
{
    if (_currentCapacity + amount > _maxCapacity) {
        return false;
    }
    _resources[type] += amount;
    _currentCapacity += amount;
    return true;
}

bool Inventory::RemoveResource(ResourceType type, int amount)
{
    auto it = _resources.find(type);
    if (it == _resources.end() || it->second < amount) {
        return false;
    }
    it->second -= amount;
    _currentCapacity -= amount;
    return true;
}

int Inventory::GetResourceAmount(ResourceType type) const
{
    auto it = _resources.find(type);
    return it != _resources.end() ? it->second : 0;
}

bool Inventory::HasResource(ResourceType type, int amount) const
{
    return GetResourceAmount(type) >= amount;
}

const std::unordered_map<ResourceType, int>& Inventory::GetAllResources() const
{
    return _resources;
}

void Inventory::Clear()
{
    for (auto& [type, amount] : _resources) {
        amount = 0;
    }
    _currentCapacity = 0;
}

} // namespace subspace
