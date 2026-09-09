#include "ships/ShipModuleDefinition.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <sstream>
#include <utility>

namespace subspace {
namespace {

using MaterialPair = std::pair<const char*, MaterialData>;

const std::array<MaterialPair, 7>& MaterialTable() {
    static const std::array<MaterialPair, 7> materials = {{
        {"Iron",     {1.0f, 1.0f, 0.8f, 0.5f, 0xB8B8C0}},
        {"Titanium", {1.5f, 0.9f, 1.0f, 0.8f, 0xD0DEF2}},
        {"Naonite",  {2.0f, 0.8f, 1.2f, 1.2f, 0x26EB59}},
        {"Trinium",  {2.5f, 0.6f, 1.5f, 1.5f, 0x40A6FF}},
        {"Xanion",   {3.0f, 0.5f, 1.8f, 2.0f, 0xFFD126}},
        {"Ogonite",  {4.0f, 0.4f, 2.2f, 2.5f, 0xFF6626}},
        {"Avorion",  {5.0f, 0.3f, 3.0f, 3.5f, 0xD933FF}},
    }};
    return materials;
}

std::string ToLower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return value;
}

void AppendClassIfSet(std::vector<std::string>& names,
                      ModuleShipClass mask,
                      ModuleShipClass value,
                      const char* name) {
    if (Any(mask & value)) {
        names.emplace_back(name);
    }
}

} // namespace

ModuleShipClass operator|(ModuleShipClass lhs, ModuleShipClass rhs) {
    return static_cast<ModuleShipClass>(static_cast<std::uint32_t>(lhs) |
                                        static_cast<std::uint32_t>(rhs));
}

ModuleShipClass operator&(ModuleShipClass lhs, ModuleShipClass rhs) {
    return static_cast<ModuleShipClass>(static_cast<std::uint32_t>(lhs) &
                                        static_cast<std::uint32_t>(rhs));
}

ModuleShipClass& operator|=(ModuleShipClass& lhs, ModuleShipClass rhs) {
    lhs = lhs | rhs;
    return lhs;
}

bool Any(ModuleShipClass value) {
    return static_cast<std::uint32_t>(value) != 0u;
}

const MaterialData& ModuleMaterialProperties::GetMaterial(const std::string& name) {
    const std::string requested = ToLower(name);
    for (const auto& entry : MaterialTable()) {
        if (ToLower(entry.first) == requested) {
            return entry.second;
        }
    }
    return MaterialTable()[0].second;
}

std::vector<std::string> ModuleMaterialProperties::GetAllMaterialNames() {
    std::vector<std::string> names;
    names.reserve(MaterialTable().size());
    for (const auto& entry : MaterialTable()) {
        names.emplace_back(entry.first);
    }
    return names;
}

void ModuleFunctionalStats::Add(const ModuleFunctionalStats& other) {
    thrustPower += other.thrustPower;
    maxSpeed = std::max(maxSpeed, other.maxSpeed);
    powerGeneration += other.powerGeneration;
    powerConsumption += other.powerConsumption;
    powerStorage += other.powerStorage;
    shieldCapacity += other.shieldCapacity;
    shieldRechargeRate += other.shieldRechargeRate;
    weaponDamage += other.weaponDamage;
    weaponRange = std::max(weaponRange, other.weaponRange);
    weaponMountPoints += other.weaponMountPoints;
    cargoCapacity += other.cargoCapacity;
    crewCapacity += other.crewCapacity;
    crewRequired += other.crewRequired;
    hasHyperdrive = hasHyperdrive || other.hasHyperdrive;
    hyperdriveRange = std::max(hyperdriveRange, other.hyperdriveRange);
    miningPower += other.miningPower;
    sensorRange = std::max(sensorRange, other.sensorRange);
}

bool ModuleClassificationInfo::IsCompatibleWith(ModuleShipClass shipClass) const {
    return Any(compatibleClasses & shipClass);
}

std::string ModuleClassificationInfo::GetSizeDisplayName() const {
    return ModuleSizeName(size);
}

std::string ModuleClassificationInfo::GetCompatibleClassesString() const {
    if (compatibleClasses == ModuleShipClass::All) return "All Classes";
    if (compatibleClasses == ModuleShipClass::AllCombat) return "All Combat";
    if (compatibleClasses == ModuleShipClass::AllCapital) return "Capital Only";
    if (compatibleClasses == ModuleShipClass::AllIndustrial) return "Industrial Only";

    std::vector<std::string> names;
    AppendClassIfSet(names, compatibleClasses, ModuleShipClass::Fighter, "Fighter");
    AppendClassIfSet(names, compatibleClasses, ModuleShipClass::Corvette, "Corvette");
    AppendClassIfSet(names, compatibleClasses, ModuleShipClass::Frigate, "Frigate");
    AppendClassIfSet(names, compatibleClasses, ModuleShipClass::Destroyer, "Destroyer");
    AppendClassIfSet(names, compatibleClasses, ModuleShipClass::Cruiser, "Cruiser");
    AppendClassIfSet(names, compatibleClasses, ModuleShipClass::Battleship, "Battleship");
    AppendClassIfSet(names, compatibleClasses, ModuleShipClass::Carrier, "Carrier");
    AppendClassIfSet(names, compatibleClasses, ModuleShipClass::Miner, "Miner");
    AppendClassIfSet(names, compatibleClasses, ModuleShipClass::Hauler, "Hauler");
    AppendClassIfSet(names, compatibleClasses, ModuleShipClass::Salvager, "Salvager");
    AppendClassIfSet(names, compatibleClasses, ModuleShipClass::Refinery, "Refinery");
    AppendClassIfSet(names, compatibleClasses, ModuleShipClass::Constructor, "Constructor");
    AppendClassIfSet(names, compatibleClasses, ModuleShipClass::Scout, "Scout");
    AppendClassIfSet(names, compatibleClasses, ModuleShipClass::Science, "Science");
    AppendClassIfSet(names, compatibleClasses, ModuleShipClass::Support, "Support");

    if (names.empty()) return "None";
    std::ostringstream stream;
    for (std::size_t i = 0; i < names.size(); ++i) {
        if (i > 0) stream << ", ";
        stream << names[i];
    }
    return stream.str();
}

ModuleFunctionalStats ShipModuleDefinition::GetStatsForMaterial(const std::string& materialType) const {
    const auto& material = ModuleMaterialProperties::GetMaterial(materialType);
    ModuleFunctionalStats stats = baseStats;
    stats.thrustPower *= material.energyEfficiency;
    stats.powerGeneration *= material.energyEfficiency;
    stats.powerStorage *= material.energyEfficiency;
    stats.shieldCapacity *= material.shieldMultiplier;
    stats.shieldRechargeRate *= material.shieldMultiplier;
    stats.hyperdriveRange *= material.energyEfficiency;
    return stats;
}

float ShipModuleDefinition::GetHealthForMaterial(const std::string& materialType) const {
    return baseHealth * ModuleMaterialProperties::GetMaterial(materialType).durabilityMultiplier;
}

float ShipModuleDefinition::GetMassForMaterial(const std::string& materialType) const {
    return baseMass * ModuleMaterialProperties::GetMaterial(materialType).massMultiplier;
}

bool ShipModuleDefinition::HasTag(const std::string& tag) const {
    const std::string requested = ToLower(tag);
    return std::any_of(tags.begin(), tags.end(), [&](const std::string& existing) {
        return ToLower(existing) == requested;
    });
}

ShipModulePart::ShipModulePart(std::string definitionId, Vector3 pos, std::string material)
    : moduleDefinitionId(std::move(definitionId)), position(pos), materialType(std::move(material)) {}

bool ShipModulePart::IsDestroyed() const {
    return health <= 0.0f;
}

float ShipModulePart::DamageLevel() const {
    if (maxHealth <= 0.0f) return 1.0f;
    return std::max(0.0f, std::min(1.0f, 1.0f - (health / maxHealth)));
}

void ShipModulePart::TakeDamage(float damage) {
    health = std::max(0.0f, health - std::max(0.0f, damage));
}

void ShipModulePart::Repair(float amount) {
    health = std::min(maxHealth, health + std::max(0.0f, amount));
}

std::string ModuleCategoryName(ModuleCategory category) {
    switch (category) {
        case ModuleCategory::Hull: return "Hull";
        case ModuleCategory::Wing: return "Wing";
        case ModuleCategory::Tail: return "Tail";
        case ModuleCategory::Engine: return "Engine";
        case ModuleCategory::Thruster: return "Thruster";
        case ModuleCategory::WeaponMount: return "WeaponMount";
        case ModuleCategory::Weapon: return "Weapon";
        case ModuleCategory::PowerCore: return "PowerCore";
        case ModuleCategory::Shield: return "Shield";
        case ModuleCategory::Cargo: return "Cargo";
        case ModuleCategory::CrewQuarters: return "CrewQuarters";
        case ModuleCategory::Hyperdrive: return "Hyperdrive";
        case ModuleCategory::Sensor: return "Sensor";
        case ModuleCategory::Mining: return "Mining";
        case ModuleCategory::Decorative: return "Decorative";
        case ModuleCategory::Antenna: return "Antenna";
    }
    return "Unknown";
}

std::string AttachmentSizeName(AttachmentSize size) {
    switch (size) {
        case AttachmentSize::Small: return "Small";
        case AttachmentSize::Medium: return "Medium";
        case AttachmentSize::Large: return "Large";
        case AttachmentSize::ExtraLarge: return "ExtraLarge";
    }
    return "Unknown";
}

std::string ModuleSizeName(ModuleSize size) {
    switch (size) {
        case ModuleSize::S: return "S";
        case ModuleSize::M: return "M";
        case ModuleSize::L: return "L";
        case ModuleSize::XL: return "XL";
    }
    return "Unknown";
}

std::string ModuleVisibilityName(ModuleVisibility visibility) {
    switch (visibility) {
        case ModuleVisibility::External: return "External";
        case ModuleVisibility::Internal: return "Internal";
        case ModuleVisibility::Both: return "Both";
    }
    return "Unknown";
}

} // namespace subspace
