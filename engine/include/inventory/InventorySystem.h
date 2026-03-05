#pragma once

#include "core/ecs/Entity.h"
#include "core/ecs/IComponent.h"
#include "core/ecs/SystemBase.h"
#include "core/ecs/EntityManager.h"
#include "core/resources/Inventory.h"
#include "core/persistence/SaveGameManager.h"

#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>
#include <functional>

namespace subspace {

/// Rarity tiers for inventory items.
enum class ItemRarity { Common, Uncommon, Rare, Epic, Legendary };

/// Represents a single item that can be stored in an inventory.
struct InventoryItem {
    std::string itemId;
    std::string name;
    std::string description;
    ItemRarity rarity = ItemRarity::Common;
    float weight = 1.0f;
    int stackSize = 1;
    int maxStackSize = 99;
    int value = 0;
    std::string category;

    /// Get the display name for a rarity tier.
    static std::string GetRarityName(ItemRarity rarity);
};

/// A single slot inside an inventory.
struct InventorySlot {
    int slotIndex = -1;
    InventoryItem item;
    bool isEmpty = true;
};

/// ECS component that gives an entity an inventory with weight and slot limits.
class InventoryComponent : public IComponent {
public:
    InventoryComponent(int maxSlots = 20, float maxWeight = 100.0f);

    int GetMaxSlots() const;
    float GetMaxWeight() const;
    float GetCurrentWeight() const;

    /// Get current weight as a percentage (0-100).
    float GetWeightPercentage() const;

    /// Count of non-empty slots.
    int GetUsedSlotCount() const;

    /// Count of empty slots.
    int GetFreeSlotCount() const;

    /// Add an item, stacking with existing items first. Returns false if no space or overweight.
    bool AddItem(const InventoryItem& item);

    /// Remove amount of an item by itemId. Returns false if insufficient quantity.
    bool RemoveItem(const std::string& itemId, int amount = 1);

    /// Check if the inventory contains at least the given amount of an item.
    bool HasItem(const std::string& itemId, int amount = 1) const;

    /// Get the total count of an item across all slots.
    int GetItemCount(const std::string& itemId) const;

    /// Get a slot by index (nullptr if out of range).
    const InventorySlot* GetSlot(int index) const;

    /// Get all non-empty slots whose item matches the given category.
    std::vector<const InventorySlot*> GetItemsByCategory(const std::string& category) const;

    /// Get all non-empty slots whose item rarity is at least minRarity.
    std::vector<const InventorySlot*> GetItemsByRarity(ItemRarity minRarity) const;

    /// Transfer items to another inventory. Returns false if insufficient or target full.
    bool TransferItem(const std::string& itemId, int amount, InventoryComponent& target);

    /// Sort non-empty slots alphabetically by item name; empty slots go to the end.
    void SortByName();

    /// Sort non-empty slots by rarity (descending); empty slots go to the end.
    void SortByRarity();

    /// Clear all slots.
    void Clear();

    /// Serialize for save-game persistence.
    ComponentData Serialize() const;

    /// Restore from previously serialized data.
    void Deserialize(const ComponentData& data);

private:
    friend class InventorySystem;

    std::vector<InventorySlot> _slots;
    float _maxWeight = 100.0f;
    float _currentWeight = 0.0f;
    int _maxSlots = 20;
    bool _weightDirty = false;

    /// Recalculate _currentWeight from slot contents.
    void RecalculateWeight();
};

/// System that updates inventory components each frame.
class InventorySystem : public SystemBase {
public:
    InventorySystem();

    void Update(float deltaTime) override;

    /// Set the entity manager used to query components.
    void SetEntityManager(EntityManager* em);

private:
    EntityManager* _entityManager = nullptr;
};

} // namespace subspace
