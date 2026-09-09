#pragma once

#include "core/Math.h"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace subspace {

// C++ port of AvorionLike/Core/Modular/ShipModuleDefinition.cs and
// ShipModulePart.cs. Names that would collide with the older C++ ship class
// system are prefixed with Module/Modular.
enum class ModuleCategory {
    Hull,
    Wing,
    Tail,
    Engine,
    Thruster,
    WeaponMount,
    Weapon,
    PowerCore,
    Shield,
    Cargo,
    CrewQuarters,
    Hyperdrive,
    Sensor,
    Mining,
    Decorative,
    Antenna
};

enum class AttachmentSize {
    Small,
    Medium,
    Large,
    ExtraLarge
};

enum class ModuleShipClass : std::uint32_t {
    None        = 0,
    Fighter     = 1u << 0,
    Corvette    = 1u << 1,
    Frigate     = 1u << 2,
    Destroyer   = 1u << 3,
    Cruiser     = 1u << 4,
    Battleship  = 1u << 5,
    Carrier     = 1u << 6,
    Miner       = 1u << 7,
    Hauler      = 1u << 8,
    Salvager    = 1u << 9,
    Refinery    = 1u << 10,
    Constructor = 1u << 11,
    Scout       = 1u << 12,
    Science     = 1u << 13,
    Support     = 1u << 14,
    AllCombat   = Fighter | Corvette | Frigate | Destroyer | Cruiser | Battleship | Carrier,
    AllCapital  = Battleship | Carrier,
    AllIndustrial = Miner | Hauler | Salvager | Refinery | Constructor,
    AllCivilian = Scout | Science | Support | AllIndustrial,
    All         = 0xFFFFFFFFu
};

enum class ShipSizeCategory {
    S,
    M,
    L,
    XL
};

enum class ModuleSize {
    S,
    M,
    L,
    XL
};

enum class ModuleVisibility {
    External,
    Internal,
    Both
};

ModuleShipClass operator|(ModuleShipClass lhs, ModuleShipClass rhs);
ModuleShipClass operator&(ModuleShipClass lhs, ModuleShipClass rhs);
ModuleShipClass& operator|=(ModuleShipClass& lhs, ModuleShipClass rhs);
bool Any(ModuleShipClass value);

struct MaterialData {
    float durabilityMultiplier = 1.0f;
    float massMultiplier = 1.0f;
    float energyEfficiency = 1.0f;
    float shieldMultiplier = 1.0f;
    std::uint32_t color = 0xFFFFFF;
};

class ModuleMaterialProperties {
public:
    static const MaterialData& GetMaterial(const std::string& name);
    static std::vector<std::string> GetAllMaterialNames();
};

struct AttachmentPoint {
    std::string name;
    Vector3 position;
    Vector3 direction = {0.0f, 0.0f, 1.0f};
    std::vector<ModuleCategory> allowedCategories;
    std::vector<std::string> requiredTags;
    AttachmentSize size = AttachmentSize::Medium;
};

struct ModuleFunctionalStats {
    float thrustPower = 0.0f;
    float maxSpeed = 0.0f;
    float powerGeneration = 0.0f;
    float powerConsumption = 0.0f;
    float powerStorage = 0.0f;
    float shieldCapacity = 0.0f;
    float shieldRechargeRate = 0.0f;
    float weaponDamage = 0.0f;
    float weaponRange = 0.0f;
    int weaponMountPoints = 0;
    float cargoCapacity = 0.0f;
    int crewCapacity = 0;
    int crewRequired = 0;
    bool hasHyperdrive = false;
    float hyperdriveRange = 0.0f;
    float miningPower = 0.0f;
    float sensorRange = 0.0f;

    void Add(const ModuleFunctionalStats& other);
};

struct ModuleClassificationInfo {
    ModuleShipClass compatibleClasses = ModuleShipClass::All;
    ModuleSize size = ModuleSize::M;
    ModuleVisibility visibility = ModuleVisibility::External;
    std::string styleVariant = "standard";
    bool isRequired = false;
    int maxPerShip = 0;
    int minPerShip = 0;
    bool isDestructible = true;
    int targetPriority = 0;

    bool IsCompatibleWith(ModuleShipClass shipClass) const;
    std::string GetSizeDisplayName() const;
    std::string GetCompatibleClassesString() const;
};

struct ShipModuleDefinition {
    std::string id;
    std::string name;
    std::string description;
    ModuleCategory category = ModuleCategory::Hull;
    std::string subCategory;
    std::string modelPath;
    std::string texturePath;
    Vector3 size = {1.0f, 1.0f, 1.0f};
    float baseMass = 10.0f;
    float baseHealth = 100.0f;
    int baseCost = 100;
    std::unordered_map<std::string, AttachmentPoint> attachmentPoints;
    ModuleFunctionalStats baseStats;
    int techLevel = 1;
    std::vector<std::string> tags;
    ModuleClassificationInfo classification;

    ModuleFunctionalStats GetStatsForMaterial(const std::string& materialType) const;
    float GetHealthForMaterial(const std::string& materialType) const;
    float GetMassForMaterial(const std::string& materialType) const;
    bool HasTag(const std::string& tag) const;
};

struct ShipModulePart {
    std::string id;
    std::string moduleDefinitionId;
    Vector3 position;
    Vector3 rotation;
    Vector3 scale = {1.0f, 1.0f, 1.0f};
    std::string materialType = "Iron";
    float health = 100.0f;
    float maxHealth = 100.0f;
    float mass = 10.0f;
    std::vector<std::string> attachedToModules;
    std::vector<std::string> attachedModules;
    std::string attachmentPointUsed;
    std::uint32_t colorTint = 0xFFFFFF;
    ModuleFunctionalStats functionalStats;

    ShipModulePart() = default;
    ShipModulePart(std::string definitionId, Vector3 pos, std::string material = "Iron");

    bool IsDestroyed() const;
    float DamageLevel() const;
    void TakeDamage(float damage);
    void Repair(float amount);
};

std::string ModuleCategoryName(ModuleCategory category);
std::string AttachmentSizeName(AttachmentSize size);
std::string ModuleSizeName(ModuleSize size);
std::string ModuleVisibilityName(ModuleVisibility visibility);

} // namespace subspace
