#include "inventory/InventorySystem.h"

#include <algorithm>

namespace subspace {

// ---------------------------------------------------------------------------
// InventoryItem
// ---------------------------------------------------------------------------

std::string InventoryItem::GetRarityName(ItemRarity rarity) {
    switch (rarity) {
        case ItemRarity::Common:    return "Common";
        case ItemRarity::Uncommon:  return "Uncommon";
        case ItemRarity::Rare:      return "Rare";
        case ItemRarity::Epic:      return "Epic";
        case ItemRarity::Legendary: return "Legendary";
        default:                    return "Common";
    }
}

// ---------------------------------------------------------------------------
// InventoryComponent
// ---------------------------------------------------------------------------

InventoryComponent::InventoryComponent(int maxSlots, float maxWeight)
    : _maxWeight(maxWeight)
    , _currentWeight(0.0f)
    , _maxSlots(maxSlots)
{
    _slots.resize(static_cast<size_t>(maxSlots));
    for (int i = 0; i < maxSlots; ++i) {
        _slots[static_cast<size_t>(i)].slotIndex = i;
        _slots[static_cast<size_t>(i)].isEmpty = true;
    }
}

int InventoryComponent::GetMaxSlots() const { return _maxSlots; }
float InventoryComponent::GetMaxWeight() const { return _maxWeight; }
float InventoryComponent::GetCurrentWeight() const { return _currentWeight; }

float InventoryComponent::GetWeightPercentage() const {
    if (_maxWeight <= 0.0f) return 0.0f;
    float pct = (_currentWeight / _maxWeight) * 100.0f;
    if (pct < 0.0f) pct = 0.0f;
    if (pct > 100.0f) pct = 100.0f;
    return pct;
}

int InventoryComponent::GetUsedSlotCount() const {
    int count = 0;
    for (const auto& slot : _slots) {
        if (!slot.isEmpty) ++count;
    }
    return count;
}

int InventoryComponent::GetFreeSlotCount() const {
    return _maxSlots - GetUsedSlotCount();
}

bool InventoryComponent::AddItem(const InventoryItem& item) {
    float additionalWeight = item.weight * static_cast<float>(item.stackSize);
    if (_currentWeight + additionalWeight > _maxWeight) return false;

    // Try to stack with existing items of the same id
    int remaining = item.stackSize;
    for (auto& slot : _slots) {
        if (remaining <= 0) break;
        if (slot.isEmpty) continue;
        if (slot.item.itemId != item.itemId) continue;

        int canAdd = slot.item.maxStackSize - slot.item.stackSize;
        if (canAdd <= 0) continue;

        int toAdd = std::min(canAdd, remaining);
        slot.item.stackSize += toAdd;
        remaining -= toAdd;
    }

    // Place remainder into empty slots
    while (remaining > 0) {
        InventorySlot* emptySlot = nullptr;
        for (auto& slot : _slots) {
            if (slot.isEmpty) { emptySlot = &slot; break; }
        }
        if (!emptySlot) return false; // no space

        emptySlot->isEmpty = false;
        emptySlot->item = item;
        int toPlace = std::min(remaining, item.maxStackSize);
        emptySlot->item.stackSize = toPlace;
        remaining -= toPlace;
    }

    RecalculateWeight();
    _weightDirty = false;
    return true;
}

bool InventoryComponent::RemoveItem(const std::string& itemId, int amount) {
    if (!HasItem(itemId, amount)) return false;

    int remaining = amount;
    for (auto& slot : _slots) {
        if (remaining <= 0) break;
        if (slot.isEmpty) continue;
        if (slot.item.itemId != itemId) continue;

        if (slot.item.stackSize <= remaining) {
            remaining -= slot.item.stackSize;
            slot.item = InventoryItem{};
            slot.isEmpty = true;
        } else {
            slot.item.stackSize -= remaining;
            remaining = 0;
        }
    }

    RecalculateWeight();
    _weightDirty = false;
    return true;
}

bool InventoryComponent::HasItem(const std::string& itemId, int amount) const {
    return GetItemCount(itemId) >= amount;
}

int InventoryComponent::GetItemCount(const std::string& itemId) const {
    int total = 0;
    for (const auto& slot : _slots) {
        if (!slot.isEmpty && slot.item.itemId == itemId) {
            total += slot.item.stackSize;
        }
    }
    return total;
}

const InventorySlot* InventoryComponent::GetSlot(int index) const {
    if (index < 0 || index >= static_cast<int>(_slots.size())) return nullptr;
    return &_slots[static_cast<size_t>(index)];
}

std::vector<const InventorySlot*> InventoryComponent::GetItemsByCategory(const std::string& category) const {
    std::vector<const InventorySlot*> result;
    for (const auto& slot : _slots) {
        if (!slot.isEmpty && slot.item.category == category) {
            result.push_back(&slot);
        }
    }
    return result;
}

std::vector<const InventorySlot*> InventoryComponent::GetItemsByRarity(ItemRarity minRarity) const {
    std::vector<const InventorySlot*> result;
    for (const auto& slot : _slots) {
        if (!slot.isEmpty && slot.item.rarity >= minRarity) {
            result.push_back(&slot);
        }
    }
    return result;
}

bool InventoryComponent::TransferItem(const std::string& itemId, int amount,
                                      InventoryComponent& target) {
    if (!HasItem(itemId, amount)) return false;

    // Find the item template from the first matching slot
    InventoryItem templateItem;
    for (const auto& slot : _slots) {
        if (!slot.isEmpty && slot.item.itemId == itemId) {
            templateItem = slot.item;
            break;
        }
    }
    templateItem.stackSize = amount;

    // Check that the target can accept the items
    float targetAdditionalWeight = templateItem.weight * static_cast<float>(amount);
    if (target._currentWeight + targetAdditionalWeight > target._maxWeight) return false;

    // Attempt to add to target first so we can fail without side effects
    if (!target.AddItem(templateItem)) return false;

    RemoveItem(itemId, amount);
    return true;
}

void InventoryComponent::SortByName() {
    std::sort(_slots.begin(), _slots.end(), [](const InventorySlot& a, const InventorySlot& b) {
        if (a.isEmpty != b.isEmpty) return !a.isEmpty;
        if (a.isEmpty && b.isEmpty) return false;
        return a.item.name < b.item.name;
    });
    for (int i = 0; i < static_cast<int>(_slots.size()); ++i) {
        _slots[static_cast<size_t>(i)].slotIndex = i;
    }
}

void InventoryComponent::SortByRarity() {
    std::sort(_slots.begin(), _slots.end(), [](const InventorySlot& a, const InventorySlot& b) {
        if (a.isEmpty != b.isEmpty) return !a.isEmpty;
        if (a.isEmpty && b.isEmpty) return false;
        return a.item.rarity > b.item.rarity;
    });
    for (int i = 0; i < static_cast<int>(_slots.size()); ++i) {
        _slots[static_cast<size_t>(i)].slotIndex = i;
    }
}

void InventoryComponent::Clear() {
    for (auto& slot : _slots) {
        slot.item = InventoryItem{};
        slot.isEmpty = true;
    }
    _currentWeight = 0.0f;
    _weightDirty = false;
}

void InventoryComponent::RecalculateWeight() {
    _currentWeight = 0.0f;
    for (const auto& slot : _slots) {
        if (!slot.isEmpty) {
            _currentWeight += slot.item.weight * static_cast<float>(slot.item.stackSize);
        }
    }
}

// ---------------------------------------------------------------------------
// Serialization
// ---------------------------------------------------------------------------

ComponentData InventoryComponent::Serialize() const {
    ComponentData cd;
    cd.componentType = "InventoryComponent";
    cd.data["maxSlots"]      = std::to_string(_maxSlots);
    cd.data["maxWeight"]     = std::to_string(_maxWeight);
    cd.data["currentWeight"] = std::to_string(_currentWeight);

    int usedCount = 0;
    for (size_t i = 0; i < _slots.size(); ++i) {
        if (_slots[i].isEmpty) continue;

        std::string prefix = "slot" + std::to_string(usedCount) + "_";
        const auto& item = _slots[i].item;
        cd.data[prefix + "slotIndex"]    = std::to_string(_slots[i].slotIndex);
        cd.data[prefix + "itemId"]       = item.itemId;
        cd.data[prefix + "name"]         = item.name;
        cd.data[prefix + "description"]  = item.description;
        cd.data[prefix + "rarity"]       = std::to_string(static_cast<int>(item.rarity));
        cd.data[prefix + "weight"]       = std::to_string(item.weight);
        cd.data[prefix + "stackSize"]    = std::to_string(item.stackSize);
        cd.data[prefix + "maxStackSize"] = std::to_string(item.maxStackSize);
        cd.data[prefix + "value"]        = std::to_string(item.value);
        cd.data[prefix + "category"]     = item.category;
        ++usedCount;
    }
    cd.data["usedSlotCount"] = std::to_string(usedCount);

    return cd;
}

void InventoryComponent::Deserialize(const ComponentData& data) {
    auto getStr = [&](const std::string& key) -> std::string {
        auto it = data.data.find(key);
        return it != data.data.end() ? it->second : "";
    };
    auto getInt = [&](const std::string& key, int def = 0) -> int {
        auto it = data.data.find(key);
        if (it == data.data.end()) return def;
        try { return std::stoi(it->second); } catch (...) { return def; }
    };
    auto getFloat = [&](const std::string& key, float def = 0.0f) -> float {
        auto it = data.data.find(key);
        if (it == data.data.end()) return def;
        try { return std::stof(it->second); } catch (...) { return def; }
    };

    _maxSlots = getInt("maxSlots", 20);
    _maxWeight = getFloat("maxWeight", 100.0f);

    _slots.clear();
    _slots.resize(static_cast<size_t>(_maxSlots));
    for (int i = 0; i < _maxSlots; ++i) {
        _slots[static_cast<size_t>(i)].slotIndex = i;
        _slots[static_cast<size_t>(i)].isEmpty = true;
    }

    int usedCount = getInt("usedSlotCount", 0);
    for (int i = 0; i < usedCount; ++i) {
        std::string prefix = "slot" + std::to_string(i) + "_";

        int slotIdx = getInt(prefix + "slotIndex", i);
        if (slotIdx < 0 || slotIdx >= _maxSlots) continue;

        auto& slot = _slots[static_cast<size_t>(slotIdx)];
        slot.isEmpty = false;
        slot.item.itemId       = getStr(prefix + "itemId");
        slot.item.name         = getStr(prefix + "name");
        slot.item.description  = getStr(prefix + "description");

        constexpr int kHighestRarityValue = static_cast<int>(ItemRarity::Legendary);
        int rarityVal = getInt(prefix + "rarity", 0);
        if (rarityVal >= 0 && rarityVal <= kHighestRarityValue) {
            slot.item.rarity = static_cast<ItemRarity>(rarityVal);
        } else {
            slot.item.rarity = ItemRarity::Common;
        }

        slot.item.weight       = getFloat(prefix + "weight", 1.0f);
        slot.item.stackSize    = getInt(prefix + "stackSize", 1);
        slot.item.maxStackSize = getInt(prefix + "maxStackSize", 99);
        slot.item.value        = getInt(prefix + "value", 0);
        slot.item.category     = getStr(prefix + "category");
    }

    RecalculateWeight();
}

// ---------------------------------------------------------------------------
// InventorySystem
// ---------------------------------------------------------------------------

InventorySystem::InventorySystem() : SystemBase("InventorySystem") {}

void InventorySystem::Update(float deltaTime) {
    (void)deltaTime;
    if (!_entityManager) return;

    auto inventories = _entityManager->GetAllComponents<InventoryComponent>();
    for (auto* inv : inventories) {
        if (inv->_weightDirty) {
            inv->RecalculateWeight();
            inv->_weightDirty = false;
        }
    }
}

void InventorySystem::SetEntityManager(EntityManager* em) {
    _entityManager = em;
}

} // namespace subspace
