#pragma once

#include "core/Math.h"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace subspace {

enum class EquipmentType {
    PrimaryWeapon,
    Turret,
    Missile,
    MiningLaser,
    SalvageBeam,
    TractorBeam,
    Shield,
    CounterMeasure,
    Scanner,
    Drone,
    RepairBot
};

struct EquipmentColor {
    int r = 255;
    int g = 255;
    int b = 255;
};

struct EquipmentItem {
    std::string id;
    std::string name = "Unknown Equipment";
    EquipmentType type = EquipmentType::PrimaryWeapon;
    int size = 1;

    float damage = 0.0f;
    float range = 0.0f;
    float fireRate = 0.0f;
    float powerConsumption = 0.0f;
    float heatGeneration = 0.0f;
    float miningPower = 0.0f;
    float salvagePower = 0.0f;

    int techLevel = 1;
    float mass = 100.0f;
    std::string modelPath;
    EquipmentColor color;
};

struct EquipmentSlot {
    std::string id;
    EquipmentType allowedType = EquipmentType::PrimaryWeapon;
    Vector3 position;
    Vector3 rotation;
    std::string mountName;
    int maxSize = 1;
    std::optional<EquipmentItem> equippedItem;

    bool IsOccupied() const;
    bool CanEquip(const EquipmentItem& item) const;
};

class ShipEquipmentComponent {
public:
    std::string entityId;
    std::vector<EquipmentSlot> equipmentSlots;

    void AddSlot(EquipmentSlot slot);
    bool EquipItem(const std::string& slotId, const EquipmentItem& item);
    std::optional<EquipmentItem> UnequipItem(const std::string& slotId);

    std::vector<EquipmentItem> GetEquippedWeapons() const;
    std::vector<EquipmentItem> GetMiningEquipment() const;

    float TotalPowerConsumption() const;
    float TotalMiningPower() const;
    float TotalWeaponDamage() const;
    float TotalSalvagePower() const;
};

class EquipmentFactory {
public:
    static EquipmentItem CreatePulseLaser(int tier = 1);
    static EquipmentItem CreateMiningLaser(int tier = 1);
    static EquipmentItem CreateSalvageBeam(int tier = 1);
    static EquipmentItem CreateShieldGenerator(int tier = 1);
    static EquipmentItem CreateScanner(int tier = 1);
};

std::string EquipmentTypeName(EquipmentType type);

} // namespace subspace
