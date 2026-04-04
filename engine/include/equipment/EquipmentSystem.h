#pragma once

#include "core/ecs/Entity.h"
#include "core/ecs/IComponent.h"
#include "core/ecs/SystemBase.h"
#include "core/ecs/EntityManager.h"
#include "core/persistence/SaveGameManager.h"

#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>

namespace subspace {

/// Types of equipment slots available on a ship.
enum class EquipmentSlotType {
    Weapon,
    Shield,
    Engine,
    Sensor,
    Utility,
    Special
};

/// Quality tier of an equipment item.
enum class EquipmentTier {
    Mk1,
    Mk2,
    Mk3,
    Mk4,
    Mk5
};

/// Stat modifiers that an equipment item provides.
struct EquipmentStats {
    float damageBonus = 0.0f;
    float shieldBonus = 0.0f;
    float speedBonus = 0.0f;
    float sensorRangeBonus = 0.0f;
    float armorBonus = 0.0f;
    float energyConsumption = 0.0f;
    float cargoBonus = 0.0f;
    float miningBonus = 0.0f;
};

/// An individual equipment item that can be mounted in a slot.
struct EquipmentItem {
    std::string itemId;
    std::string displayName;
    EquipmentSlotType slotType = EquipmentSlotType::Utility;
    EquipmentTier tier = EquipmentTier::Mk1;
    EquipmentStats stats;
    float durability = 100.0f;
    float maxDurability = 100.0f;

    /// Get the tier multiplier (Mk1 1.0 … Mk5 3.0).
    float GetTierMultiplier() const;

    /// Get effective stats (base stats * tier multiplier).
    EquipmentStats GetEffectiveStats() const;

    /// Is the item broken (durability == 0)?
    bool IsBroken() const;

    /// Get the display name for a slot type.
    static std::string GetSlotTypeName(EquipmentSlotType type);

    /// Get the display name for a tier.
    static std::string GetTierName(EquipmentTier tier);
};

/// A slot on a ship that can hold one equipment item.
struct EquipmentSlot {
    std::string slotName;
    EquipmentSlotType type = EquipmentSlotType::Utility;
    bool isOccupied = false;
    EquipmentItem item;

    /// Check if a given item is compatible with this slot.
    bool IsCompatible(const EquipmentItem& candidate) const;
};

/// ECS component that manages equipment slots and mounted items on an entity.
class EquipmentComponent : public IComponent {
public:
    EquipmentComponent() = default;

    /// Add a slot of the given type with a name.
    bool AddSlot(const std::string& slotName, EquipmentSlotType type);

    /// Get the number of slots.
    size_t GetSlotCount() const;

    /// Get the number of occupied slots.
    size_t GetOccupiedSlotCount() const;

    /// Get all slots.
    const std::vector<EquipmentSlot>& GetSlots() const;

    /// Find a slot by name. Returns nullptr if not found.
    const EquipmentSlot* GetSlot(const std::string& slotName) const;

    /// Equip an item into a named slot. Returns false if slot not found,
    /// incompatible, or already occupied.
    bool Equip(const std::string& slotName, const EquipmentItem& item);

    /// Unequip an item from a named slot. Returns the removed item
    /// (empty itemId if nothing was equipped).
    EquipmentItem Unequip(const std::string& slotName);

    /// Get the combined stats from all equipped items.
    EquipmentStats GetTotalStats() const;

    /// Find the first empty slot compatible with an item. Returns nullptr if none.
    const EquipmentSlot* FindAvailableSlot(const EquipmentItem& item) const;

    /// Reduce durability of all equipped items by the given amount.
    void DegradeAll(float amount);

    /// Repair all equipped items by the given amount.
    void RepairAll(float amount);

    /// Maximum number of slots per entity.
    static constexpr size_t kMaxSlots = 12;

    /// Serialize for save-game persistence.
    ComponentData Serialize() const;

    /// Restore from previously serialized data.
    void Deserialize(const ComponentData& data);

private:
    std::vector<EquipmentSlot> _slots;

    EquipmentSlot* FindSlotMutable(const std::string& slotName);

    friend class EquipmentSystem;
};

/// System that updates equipment durability and applies stat bonuses.
class EquipmentSystem : public SystemBase {
public:
    EquipmentSystem();
    explicit EquipmentSystem(EntityManager& entityManager);

    void Update(float deltaTime) override;

    void SetEntityManager(EntityManager* em);

    /// Create a randomised equipment item of a given slot type and tier.
    static EquipmentItem GenerateItem(EquipmentSlotType slotType,
                                       EquipmentTier tier,
                                       uint32_t seed);

private:
    EntityManager* _entityManager = nullptr;
};

} // namespace subspace
