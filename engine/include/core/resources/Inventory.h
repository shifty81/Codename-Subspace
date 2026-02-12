#pragma once

#include <functional>
#include <unordered_map>

namespace subspace {

/// Types of resources in the game (mirrors C# ResourceType).
enum class ResourceType {
    Iron,
    Titanium,
    Naonite,
    Trinium,
    Xanion,
    Ogonite,
    Avorion,
    Credits,
    Count  // sentinel for iteration
};

} // namespace subspace

// Hash specialization for ResourceType (must precede unordered_map usage)
template<>
struct std::hash<subspace::ResourceType> {
    std::size_t operator()(subspace::ResourceType t) const noexcept {
        return std::hash<int>{}(static_cast<int>(t));
    }
};

namespace subspace {

/// Manages resource inventory (port of C# Inventory class).
class Inventory {
public:
    Inventory();

    int  GetMaxCapacity() const { return _maxCapacity; }
    void SetMaxCapacity(int cap) { _maxCapacity = cap; }
    int  GetCurrentCapacity() const { return _currentCapacity; }

    /// Add resources to inventory. Returns false if capacity exceeded.
    bool AddResource(ResourceType type, int amount);

    /// Remove resources from inventory. Returns false if insufficient.
    bool RemoveResource(ResourceType type, int amount);

    /// Get amount of a specific resource.
    int GetResourceAmount(ResourceType type) const;

    /// Check if inventory has enough of a resource.
    bool HasResource(ResourceType type, int amount) const;

    /// Get all resources.
    const std::unordered_map<ResourceType, int>& GetAllResources() const;

    /// Clear all resources.
    void Clear();

private:
    std::unordered_map<ResourceType, int> _resources;
    int _maxCapacity = 1000;
    int _currentCapacity = 0;
};

} // namespace subspace
