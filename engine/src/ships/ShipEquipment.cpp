#include "ships/ShipEquipment.h"

#include <algorithm>

namespace subspace {

bool EquipmentSlot::IsOccupied() const {
    return equippedItem.has_value();
}

bool EquipmentSlot::CanEquip(const EquipmentItem& item) const {
    return item.type == allowedType && item.size <= maxSize;
}

void ShipEquipmentComponent::AddSlot(EquipmentSlot slot) {
    equipmentSlots.push_back(std::move(slot));
}

bool ShipEquipmentComponent::EquipItem(const std::string& slotId, const EquipmentItem& item) {
    auto it = std::find_if(equipmentSlots.begin(), equipmentSlots.end(),
        [&](const EquipmentSlot& slot) { return slot.id == slotId; });
    if (it == equipmentSlots.end()) return false;
    if (!it->CanEquip(item)) return false;
    it->equippedItem = item;
    return true;
}

std::optional<EquipmentItem> ShipEquipmentComponent::UnequipItem(const std::string& slotId) {
    auto it = std::find_if(equipmentSlots.begin(), equipmentSlots.end(),
        [&](const EquipmentSlot& slot) { return slot.id == slotId; });
    if (it == equipmentSlots.end() || !it->equippedItem) return std::nullopt;
    auto item = it->equippedItem;
    it->equippedItem.reset();
    return item;
}

std::vector<EquipmentItem> ShipEquipmentComponent::GetEquippedWeapons() const {
    std::vector<EquipmentItem> result;
    for (const auto& slot : equipmentSlots) {
        if (!slot.equippedItem) continue;
        const auto type = slot.equippedItem->type;
        if (type == EquipmentType::PrimaryWeapon ||
            type == EquipmentType::Turret ||
            type == EquipmentType::Missile) {
            result.push_back(*slot.equippedItem);
        }
    }
    return result;
}

std::vector<EquipmentItem> ShipEquipmentComponent::GetMiningEquipment() const {
    std::vector<EquipmentItem> result;
    for (const auto& slot : equipmentSlots) {
        if (slot.equippedItem && slot.equippedItem->type == EquipmentType::MiningLaser) {
            result.push_back(*slot.equippedItem);
        }
    }
    return result;
}

float ShipEquipmentComponent::TotalPowerConsumption() const {
    float total = 0.0f;
    for (const auto& slot : equipmentSlots) {
        if (slot.equippedItem) total += slot.equippedItem->powerConsumption;
    }
    return total;
}

float ShipEquipmentComponent::TotalMiningPower() const {
    float total = 0.0f;
    for (const auto& item : GetMiningEquipment()) {
        total += item.miningPower;
    }
    return total;
}

float ShipEquipmentComponent::TotalWeaponDamage() const {
    float total = 0.0f;
    for (const auto& item : GetEquippedWeapons()) {
        total += item.damage;
    }
    return total;
}

float ShipEquipmentComponent::TotalSalvagePower() const {
    float total = 0.0f;
    for (const auto& slot : equipmentSlots) {
        if (slot.equippedItem) total += slot.equippedItem->salvagePower;
    }
    return total;
}

EquipmentItem EquipmentFactory::CreatePulseLaser(int tier) {
    EquipmentItem item;
    item.id = "pulse_laser_mk" + std::to_string(tier);
    item.name = "Pulse Laser Mk" + std::to_string(tier);
    item.type = EquipmentType::PrimaryWeapon;
    item.size = 1;
    item.damage = 50.0f * static_cast<float>(tier);
    item.range = 1000.0f + 200.0f * static_cast<float>(tier);
    item.fireRate = 5.0f;
    item.powerConsumption = 20.0f * static_cast<float>(tier);
    item.heatGeneration = 15.0f * static_cast<float>(tier);
    item.techLevel = tier;
    item.mass = 50.0f;
    item.modelPath = "equipment/weapons/pulse_laser.obj";
    item.color = {100, 150, 255};
    return item;
}

EquipmentItem EquipmentFactory::CreateMiningLaser(int tier) {
    EquipmentItem item;
    item.id = "mining_laser_mk" + std::to_string(tier);
    item.name = "Mining Laser Mk" + std::to_string(tier);
    item.type = EquipmentType::MiningLaser;
    item.size = 1;
    item.miningPower = 100.0f * static_cast<float>(tier);
    item.range = 500.0f + 100.0f * static_cast<float>(tier);
    item.powerConsumption = 30.0f * static_cast<float>(tier);
    item.heatGeneration = 20.0f * static_cast<float>(tier);
    item.techLevel = tier;
    item.mass = 60.0f;
    item.modelPath = "equipment/tools/mining_laser.obj";
    item.color = {255, 200, 80};
    return item;
}

EquipmentItem EquipmentFactory::CreateSalvageBeam(int tier) {
    EquipmentItem item;
    item.id = "salvage_beam_mk" + std::to_string(tier);
    item.name = "Salvage Beam Mk" + std::to_string(tier);
    item.type = EquipmentType::SalvageBeam;
    item.size = 1;
    item.salvagePower = 80.0f * static_cast<float>(tier);
    item.range = 450.0f + 90.0f * static_cast<float>(tier);
    item.powerConsumption = 25.0f * static_cast<float>(tier);
    item.heatGeneration = 18.0f * static_cast<float>(tier);
    item.techLevel = tier;
    item.mass = 55.0f;
    item.modelPath = "equipment/tools/salvage_beam.obj";
    item.color = {120, 255, 180};
    return item;
}

EquipmentItem EquipmentFactory::CreateShieldGenerator(int tier) {
    EquipmentItem item;
    item.id = "shield_generator_mk" + std::to_string(tier);
    item.name = "Shield Generator Mk" + std::to_string(tier);
    item.type = EquipmentType::Shield;
    item.size = tier <= 2 ? 1 : 2;
    item.powerConsumption = 35.0f * static_cast<float>(tier);
    item.techLevel = tier;
    item.mass = 75.0f * static_cast<float>(tier);
    item.modelPath = "equipment/defense/shield_generator.obj";
    item.color = {80, 160, 255};
    return item;
}

EquipmentItem EquipmentFactory::CreateScanner(int tier) {
    EquipmentItem item;
    item.id = "scanner_mk" + std::to_string(tier);
    item.name = "Scanner Mk" + std::to_string(tier);
    item.type = EquipmentType::Scanner;
    item.size = 1;
    item.range = 1500.0f + 500.0f * static_cast<float>(tier);
    item.powerConsumption = 10.0f * static_cast<float>(tier);
    item.techLevel = tier;
    item.mass = 35.0f;
    item.modelPath = "equipment/support/scanner.obj";
    item.color = {180, 220, 255};
    return item;
}

std::string EquipmentTypeName(EquipmentType type) {
    switch (type) {
        case EquipmentType::PrimaryWeapon: return "PrimaryWeapon";
        case EquipmentType::Turret: return "Turret";
        case EquipmentType::Missile: return "Missile";
        case EquipmentType::MiningLaser: return "MiningLaser";
        case EquipmentType::SalvageBeam: return "SalvageBeam";
        case EquipmentType::TractorBeam: return "TractorBeam";
        case EquipmentType::Shield: return "Shield";
        case EquipmentType::CounterMeasure: return "CounterMeasure";
        case EquipmentType::Scanner: return "Scanner";
        case EquipmentType::Drone: return "Drone";
        case EquipmentType::RepairBot: return "RepairBot";
    }
    return "Unknown";
}

} // namespace subspace
