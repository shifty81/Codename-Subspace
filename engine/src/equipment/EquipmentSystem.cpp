#include "equipment/EquipmentSystem.h"

#include <algorithm>
#include <cmath>

namespace subspace {

// ---------------------------------------------------------------------------
// EquipmentItem helpers
// ---------------------------------------------------------------------------

float EquipmentItem::GetTierMultiplier() const {
    switch (tier) {
        case EquipmentTier::Mk1: return 1.0f;
        case EquipmentTier::Mk2: return 1.5f;
        case EquipmentTier::Mk3: return 2.0f;
        case EquipmentTier::Mk4: return 2.5f;
        case EquipmentTier::Mk5: return 3.0f;
    }
    return 1.0f;
}

EquipmentStats EquipmentItem::GetEffectiveStats() const {
    float mult = GetTierMultiplier();
    EquipmentStats eff;
    eff.damageBonus       = stats.damageBonus * mult;
    eff.shieldBonus       = stats.shieldBonus * mult;
    eff.speedBonus        = stats.speedBonus * mult;
    eff.sensorRangeBonus  = stats.sensorRangeBonus * mult;
    eff.armorBonus        = stats.armorBonus * mult;
    eff.energyConsumption = stats.energyConsumption * mult;
    eff.cargoBonus        = stats.cargoBonus * mult;
    eff.miningBonus       = stats.miningBonus * mult;
    return eff;
}

bool EquipmentItem::IsBroken() const {
    return durability <= 0.0f;
}

std::string EquipmentItem::GetSlotTypeName(EquipmentSlotType type) {
    switch (type) {
        case EquipmentSlotType::Weapon:  return "Weapon";
        case EquipmentSlotType::Shield:  return "Shield";
        case EquipmentSlotType::Engine:  return "Engine";
        case EquipmentSlotType::Sensor:  return "Sensor";
        case EquipmentSlotType::Utility: return "Utility";
        case EquipmentSlotType::Special: return "Special";
    }
    return "Unknown";
}

std::string EquipmentItem::GetTierName(EquipmentTier tier) {
    switch (tier) {
        case EquipmentTier::Mk1: return "Mk1";
        case EquipmentTier::Mk2: return "Mk2";
        case EquipmentTier::Mk3: return "Mk3";
        case EquipmentTier::Mk4: return "Mk4";
        case EquipmentTier::Mk5: return "Mk5";
    }
    return "Unknown";
}

// ---------------------------------------------------------------------------
// EquipmentSlot
// ---------------------------------------------------------------------------

bool EquipmentSlot::IsCompatible(const EquipmentItem& candidate) const {
    return candidate.slotType == type;
}

// ---------------------------------------------------------------------------
// EquipmentComponent
// ---------------------------------------------------------------------------

bool EquipmentComponent::AddSlot(const std::string& slotName, EquipmentSlotType type) {
    if (_slots.size() >= kMaxSlots) return false;
    // Disallow duplicate names
    for (const auto& s : _slots) {
        if (s.slotName == slotName) return false;
    }
    EquipmentSlot slot;
    slot.slotName = slotName;
    slot.type = type;
    slot.isOccupied = false;
    _slots.push_back(slot);
    return true;
}

size_t EquipmentComponent::GetSlotCount() const {
    return _slots.size();
}

size_t EquipmentComponent::GetOccupiedSlotCount() const {
    size_t count = 0;
    for (const auto& s : _slots) {
        if (s.isOccupied) ++count;
    }
    return count;
}

const std::vector<EquipmentSlot>& EquipmentComponent::GetSlots() const {
    return _slots;
}

const EquipmentSlot* EquipmentComponent::GetSlot(const std::string& slotName) const {
    for (const auto& s : _slots) {
        if (s.slotName == slotName) return &s;
    }
    return nullptr;
}

EquipmentSlot* EquipmentComponent::FindSlotMutable(const std::string& slotName) {
    for (auto& s : _slots) {
        if (s.slotName == slotName) return &s;
    }
    return nullptr;
}

bool EquipmentComponent::Equip(const std::string& slotName, const EquipmentItem& item) {
    auto* slot = FindSlotMutable(slotName);
    if (!slot) return false;
    if (slot->isOccupied) return false;
    if (!slot->IsCompatible(item)) return false;

    slot->item = item;
    slot->isOccupied = true;
    return true;
}

EquipmentItem EquipmentComponent::Unequip(const std::string& slotName) {
    auto* slot = FindSlotMutable(slotName);
    if (!slot || !slot->isOccupied) return EquipmentItem{};

    EquipmentItem removed = slot->item;
    slot->item = EquipmentItem{};
    slot->isOccupied = false;
    return removed;
}

EquipmentStats EquipmentComponent::GetTotalStats() const {
    EquipmentStats total;
    for (const auto& s : _slots) {
        if (!s.isOccupied) continue;
        if (s.item.IsBroken()) continue;

        EquipmentStats eff = s.item.GetEffectiveStats();
        total.damageBonus       += eff.damageBonus;
        total.shieldBonus       += eff.shieldBonus;
        total.speedBonus        += eff.speedBonus;
        total.sensorRangeBonus  += eff.sensorRangeBonus;
        total.armorBonus        += eff.armorBonus;
        total.energyConsumption += eff.energyConsumption;
        total.cargoBonus        += eff.cargoBonus;
        total.miningBonus       += eff.miningBonus;
    }
    return total;
}

const EquipmentSlot* EquipmentComponent::FindAvailableSlot(const EquipmentItem& item) const {
    for (const auto& s : _slots) {
        if (!s.isOccupied && s.IsCompatible(item)) return &s;
    }
    return nullptr;
}

void EquipmentComponent::DegradeAll(float amount) {
    for (auto& s : _slots) {
        if (!s.isOccupied) continue;
        s.item.durability = std::max(0.0f, s.item.durability - amount);
    }
}

void EquipmentComponent::RepairAll(float amount) {
    for (auto& s : _slots) {
        if (!s.isOccupied) continue;
        s.item.durability = std::min(s.item.maxDurability, s.item.durability + amount);
    }
}

// ---------------------------------------------------------------------------
// Serialization
// ---------------------------------------------------------------------------

ComponentData EquipmentComponent::Serialize() const {
    ComponentData cd;
    cd.componentType = "EquipmentComponent";
    cd.data["slotCount"] = std::to_string(_slots.size());

    for (size_t i = 0; i < _slots.size(); ++i) {
        std::string p = "slot_" + std::to_string(i) + "_";
        const auto& s = _slots[i];
        cd.data[p + "name"]       = s.slotName;
        cd.data[p + "type"]       = std::to_string(static_cast<int>(s.type));
        cd.data[p + "occupied"]   = s.isOccupied ? "1" : "0";

        if (s.isOccupied) {
            cd.data[p + "itemId"]        = s.item.itemId;
            cd.data[p + "displayName"]   = s.item.displayName;
            cd.data[p + "itemSlotType"]  = std::to_string(static_cast<int>(s.item.slotType));
            cd.data[p + "tier"]          = std::to_string(static_cast<int>(s.item.tier));
            cd.data[p + "durability"]    = std::to_string(s.item.durability);
            cd.data[p + "maxDurability"] = std::to_string(s.item.maxDurability);
            cd.data[p + "damageBonus"]   = std::to_string(s.item.stats.damageBonus);
            cd.data[p + "shieldBonus"]   = std::to_string(s.item.stats.shieldBonus);
            cd.data[p + "speedBonus"]    = std::to_string(s.item.stats.speedBonus);
            cd.data[p + "sensorBonus"]   = std::to_string(s.item.stats.sensorRangeBonus);
            cd.data[p + "armorBonus"]    = std::to_string(s.item.stats.armorBonus);
            cd.data[p + "energyCost"]    = std::to_string(s.item.stats.energyConsumption);
            cd.data[p + "cargoBonus"]    = std::to_string(s.item.stats.cargoBonus);
            cd.data[p + "miningBonus"]   = std::to_string(s.item.stats.miningBonus);
        }
    }
    return cd;
}

void EquipmentComponent::Deserialize(const ComponentData& data) {
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

    int slotCount = getInt("slotCount", 0);
    _slots.clear();
    _slots.reserve(static_cast<size_t>(slotCount));

    for (int i = 0; i < slotCount; ++i) {
        std::string p = "slot_" + std::to_string(i) + "_";
        EquipmentSlot s;
        s.slotName = getStr(p + "name");
        int typeVal = getInt(p + "type", 0);
        constexpr int kMaxType = static_cast<int>(EquipmentSlotType::Special);
        if (typeVal >= 0 && typeVal <= kMaxType)
            s.type = static_cast<EquipmentSlotType>(typeVal);

        s.isOccupied = getInt(p + "occupied", 0) != 0;

        if (s.isOccupied) {
            s.item.itemId      = getStr(p + "itemId");
            s.item.displayName = getStr(p + "displayName");
            int itemSlotVal = getInt(p + "itemSlotType", 0);
            if (itemSlotVal >= 0 && itemSlotVal <= kMaxType)
                s.item.slotType = static_cast<EquipmentSlotType>(itemSlotVal);
            int tierVal = getInt(p + "tier", 0);
            constexpr int kMaxTier = static_cast<int>(EquipmentTier::Mk5);
            if (tierVal >= 0 && tierVal <= kMaxTier)
                s.item.tier = static_cast<EquipmentTier>(tierVal);
            s.item.durability    = getFloat(p + "durability", 100.0f);
            s.item.maxDurability = getFloat(p + "maxDurability", 100.0f);
            s.item.stats.damageBonus       = getFloat(p + "damageBonus", 0.0f);
            s.item.stats.shieldBonus       = getFloat(p + "shieldBonus", 0.0f);
            s.item.stats.speedBonus        = getFloat(p + "speedBonus", 0.0f);
            s.item.stats.sensorRangeBonus  = getFloat(p + "sensorBonus", 0.0f);
            s.item.stats.armorBonus        = getFloat(p + "armorBonus", 0.0f);
            s.item.stats.energyConsumption = getFloat(p + "energyCost", 0.0f);
            s.item.stats.cargoBonus        = getFloat(p + "cargoBonus", 0.0f);
            s.item.stats.miningBonus       = getFloat(p + "miningBonus", 0.0f);
        }
        _slots.push_back(s);
    }
}

// ---------------------------------------------------------------------------
// EquipmentSystem
// ---------------------------------------------------------------------------

EquipmentSystem::EquipmentSystem() : SystemBase("EquipmentSystem") {}

EquipmentSystem::EquipmentSystem(EntityManager& entityManager)
    : SystemBase("EquipmentSystem")
    , _entityManager(&entityManager)
{
}

void EquipmentSystem::SetEntityManager(EntityManager* em) {
    _entityManager = em;
}

void EquipmentSystem::Update(float /*deltaTime*/) {
    // Equipment system currently applies passive degradation if needed;
    // stat bonuses are read via GetTotalStats() on demand.
    if (!_entityManager) return;

    // No per-frame work required; equipment is event-driven (equip/unequip)
    // and stats are queried on demand.
}

EquipmentItem EquipmentSystem::GenerateItem(EquipmentSlotType slotType,
                                             EquipmentTier tier,
                                             uint32_t seed) {
    EquipmentItem item;
    item.slotType = slotType;
    item.tier = tier;
    item.durability = 100.0f;
    item.maxDurability = 100.0f;

    // Deterministic stat generation from seed
    float baseVal = 5.0f + static_cast<float>(seed % 16);

    switch (slotType) {
        case EquipmentSlotType::Weapon:
            item.itemId = "wpn_" + std::to_string(seed);
            item.displayName = EquipmentItem::GetTierName(tier) + " Laser Cannon";
            item.stats.damageBonus = baseVal;
            item.stats.energyConsumption = baseVal * 0.5f;
            break;
        case EquipmentSlotType::Shield:
            item.itemId = "shd_" + std::to_string(seed);
            item.displayName = EquipmentItem::GetTierName(tier) + " Shield Generator";
            item.stats.shieldBonus = baseVal * 2.0f;
            item.stats.energyConsumption = baseVal * 0.3f;
            break;
        case EquipmentSlotType::Engine:
            item.itemId = "eng_" + std::to_string(seed);
            item.displayName = EquipmentItem::GetTierName(tier) + " Thruster";
            item.stats.speedBonus = baseVal * 1.5f;
            item.stats.energyConsumption = baseVal * 0.4f;
            break;
        case EquipmentSlotType::Sensor:
            item.itemId = "sns_" + std::to_string(seed);
            item.displayName = EquipmentItem::GetTierName(tier) + " Scanner Array";
            item.stats.sensorRangeBonus = baseVal * 3.0f;
            item.stats.energyConsumption = baseVal * 0.2f;
            break;
        case EquipmentSlotType::Utility:
            item.itemId = "utl_" + std::to_string(seed);
            item.displayName = EquipmentItem::GetTierName(tier) + " Utility Module";
            item.stats.cargoBonus = baseVal;
            item.stats.miningBonus = baseVal * 0.5f;
            break;
        case EquipmentSlotType::Special:
            item.itemId = "spc_" + std::to_string(seed);
            item.displayName = EquipmentItem::GetTierName(tier) + " Prototype Device";
            item.stats.damageBonus = baseVal * 0.5f;
            item.stats.shieldBonus = baseVal;
            item.stats.speedBonus = baseVal * 0.5f;
            item.stats.energyConsumption = baseVal * 0.8f;
            break;
    }

    return item;
}

} // namespace subspace
