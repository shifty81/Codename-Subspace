#include "ships/ShipClassSystem.h"

#include <algorithm>

namespace subspace {

// ---------------------------------------------------------------------------
// ShipClassDefinition
// ---------------------------------------------------------------------------

std::string ShipClassDefinition::GetClassName(ShipClass shipClass) {
    switch (shipClass) {
        case ShipClass::Fighter:    return "Fighter";
        case ShipClass::Corvette:   return "Corvette";
        case ShipClass::Frigate:    return "Frigate";
        case ShipClass::Destroyer:  return "Destroyer";
        case ShipClass::Cruiser:    return "Cruiser";
        case ShipClass::Battleship: return "Battleship";
        case ShipClass::Carrier:    return "Carrier";
        case ShipClass::Freighter:  return "Freighter";
        case ShipClass::Miner:      return "Miner";
        case ShipClass::Explorer:   return "Explorer";
    }
    return "Unknown";
}

std::string ShipClassDefinition::GetRoleName(ShipRole role) {
    switch (role) {
        case ShipRole::Combat:      return "Combat";
        case ShipRole::Trade:       return "Trade";
        case ShipRole::Mining:      return "Mining";
        case ShipRole::Exploration: return "Exploration";
        case ShipRole::Support:     return "Support";
        case ShipRole::MultiRole:   return "Multi-Role";
    }
    return "Unknown";
}

ShipClassDefinition ShipClassDefinition::GetDefaultDefinition(ShipClass shipClass) {
    ShipClassDefinition def;
    def.shipClass = shipClass;

    switch (shipClass) {
        case ShipClass::Fighter:
            def.displayName = "Fighter";
            def.description = "Small, fast combat vessel built for dogfighting.";
            def.role     = ShipRole::Combat;
            def.bonus    = {1.3f, 1.2f, 0.8f, 0.5f, 0.3f, 1.0f};
            def.minCrew  = 1;   def.maxCrew = 4;
            def.baseMass = 50.0f;  def.baseHull = 100.0f;
            def.techLevel = 1;
            break;

        case ShipClass::Corvette:
            def.displayName = "Corvette";
            def.description = "Light warship balancing speed and firepower.";
            def.role     = ShipRole::Combat;
            def.bonus    = {1.2f, 1.1f, 1.0f, 0.7f, 0.4f, 1.1f};
            def.minCrew  = 3;   def.maxCrew = 10;
            def.baseMass = 150.0f; def.baseHull = 250.0f;
            def.techLevel = 2;
            break;

        case ShipClass::Frigate:
            def.displayName = "Frigate";
            def.description = "Versatile mid-size warship with strong shields.";
            def.role     = ShipRole::Combat;
            def.bonus    = {1.0f, 1.0f, 1.2f, 0.8f, 0.5f, 1.0f};
            def.minCrew  = 10;  def.maxCrew = 30;
            def.baseMass = 500.0f; def.baseHull = 600.0f;
            def.techLevel = 3;
            break;

        case ShipClass::Destroyer:
            def.displayName = "Destroyer";
            def.description = "Heavy assault ship with devastating weapons.";
            def.role     = ShipRole::Combat;
            def.bonus    = {0.9f, 1.3f, 1.1f, 0.6f, 0.3f, 0.9f};
            def.minCrew  = 20;  def.maxCrew = 60;
            def.baseMass = 1200.0f; def.baseHull = 1200.0f;
            def.techLevel = 4;
            break;

        case ShipClass::Cruiser:
            def.displayName = "Cruiser";
            def.description = "Capital-class warship with balanced offense and defense.";
            def.role     = ShipRole::Combat;
            def.bonus    = {0.8f, 1.2f, 1.3f, 0.9f, 0.4f, 1.0f};
            def.minCrew  = 50;  def.maxCrew = 150;
            def.baseMass = 3000.0f; def.baseHull = 2500.0f;
            def.techLevel = 6;
            break;

        case ShipClass::Battleship:
            def.displayName = "Battleship";
            def.description = "Massive warship built to dominate the battlefield.";
            def.role     = ShipRole::Combat;
            def.bonus    = {0.6f, 1.5f, 1.5f, 0.7f, 0.2f, 0.8f};
            def.minCrew  = 100; def.maxCrew = 300;
            def.baseMass = 8000.0f; def.baseHull = 5000.0f;
            def.techLevel = 8;
            break;

        case ShipClass::Carrier:
            def.displayName = "Carrier";
            def.description = "Support capital ship that deploys and maintains fighter wings.";
            def.role     = ShipRole::Support;
            def.bonus    = {0.7f, 0.5f, 1.4f, 1.2f, 0.2f, 1.3f};
            def.minCrew  = 200; def.maxCrew = 500;
            def.baseMass = 10000.0f; def.baseHull = 6000.0f;
            def.techLevel = 7;
            break;

        case ShipClass::Freighter:
            def.displayName = "Freighter";
            def.description = "Large cargo hauler designed for trade routes.";
            def.role     = ShipRole::Trade;
            def.bonus    = {0.8f, 0.3f, 0.9f, 2.0f, 0.5f, 0.8f};
            def.minCrew  = 5;   def.maxCrew = 20;
            def.baseMass = 2000.0f; def.baseHull = 800.0f;
            def.techLevel = 2;
            break;

        case ShipClass::Miner:
            def.displayName = "Miner";
            def.description = "Specialized vessel equipped for asteroid mining operations.";
            def.role     = ShipRole::Mining;
            def.bonus    = {0.9f, 0.4f, 0.7f, 1.5f, 2.0f, 1.2f};
            def.minCrew  = 3;   def.maxCrew = 15;
            def.baseMass = 800.0f; def.baseHull = 400.0f;
            def.techLevel = 2;
            break;

        case ShipClass::Explorer:
            def.displayName = "Explorer";
            def.description = "Long-range scout with advanced sensor arrays.";
            def.role     = ShipRole::Exploration;
            def.bonus    = {1.1f, 0.6f, 0.8f, 1.0f, 0.8f, 2.0f};
            def.minCrew  = 5;   def.maxCrew = 25;
            def.baseMass = 600.0f; def.baseHull = 500.0f;
            def.techLevel = 3;
            break;
    }

    return def;
}

// ---------------------------------------------------------------------------
// ShipClassComponent
// ---------------------------------------------------------------------------

ShipClassComponent::ShipClassComponent()
    : _definition(ShipClassDefinition::GetDefaultDefinition(ShipClass::Fighter))
{
}

ShipClassComponent::ShipClassComponent(ShipClass shipClass)
    : _definition(ShipClassDefinition::GetDefaultDefinition(shipClass))
{
}

ShipClass ShipClassComponent::GetShipClass() const {
    return _definition.shipClass;
}

void ShipClassComponent::SetShipClass(ShipClass shipClass) {
    _definition = ShipClassDefinition::GetDefaultDefinition(shipClass);
}

ShipRole ShipClassComponent::GetRole() const {
    return _definition.role;
}

const ClassBonus& ShipClassComponent::GetClassBonus() const {
    return _definition.bonus;
}

const ShipClassDefinition& ShipClassComponent::GetDefinition() const {
    return _definition;
}

float ShipClassComponent::GetEffectiveSpeed(float baseSpeed) const {
    return baseSpeed * _definition.bonus.speedMultiplier;
}

float ShipClassComponent::GetEffectiveDamage(float baseDamage) const {
    return baseDamage * _definition.bonus.damageMultiplier;
}

float ShipClassComponent::GetEffectiveShield(float baseShield) const {
    return baseShield * _definition.bonus.shieldMultiplier;
}

float ShipClassComponent::GetEffectiveCargo(float baseCargo) const {
    return baseCargo * _definition.bonus.cargoMultiplier;
}

float ShipClassComponent::GetEffectiveMining(float baseMining) const {
    return baseMining * _definition.bonus.miningMultiplier;
}

float ShipClassComponent::GetEffectiveSensor(float baseSensor) const {
    return baseSensor * _definition.bonus.sensorMultiplier;
}

std::string ShipClassComponent::GetDisplayName() const {
    return _definition.displayName;
}

std::string ShipClassComponent::GetDescription() const {
    return _definition.description;
}

int ShipClassComponent::GetTechLevel() const {
    return _definition.techLevel;
}

// ---------------------------------------------------------------------------
// Serialization
// ---------------------------------------------------------------------------

ComponentData ShipClassComponent::Serialize() const {
    ComponentData cd;
    cd.componentType = "ShipClassComponent";

    cd.data["shipClass"]   = std::to_string(static_cast<int>(_definition.shipClass));
    cd.data["displayName"] = _definition.displayName;
    cd.data["description"] = _definition.description;
    cd.data["role"]        = std::to_string(static_cast<int>(_definition.role));
    cd.data["minCrew"]     = std::to_string(_definition.minCrew);
    cd.data["maxCrew"]     = std::to_string(_definition.maxCrew);
    cd.data["baseMass"]    = std::to_string(_definition.baseMass);
    cd.data["baseHull"]    = std::to_string(_definition.baseHull);
    cd.data["techLevel"]   = std::to_string(_definition.techLevel);

    cd.data["speedMultiplier"]  = std::to_string(_definition.bonus.speedMultiplier);
    cd.data["damageMultiplier"] = std::to_string(_definition.bonus.damageMultiplier);
    cd.data["shieldMultiplier"] = std::to_string(_definition.bonus.shieldMultiplier);
    cd.data["cargoMultiplier"]  = std::to_string(_definition.bonus.cargoMultiplier);
    cd.data["miningMultiplier"] = std::to_string(_definition.bonus.miningMultiplier);
    cd.data["sensorMultiplier"] = std::to_string(_definition.bonus.sensorMultiplier);

    return cd;
}

void ShipClassComponent::Deserialize(const ComponentData& data) {
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

    // Restore the class enum with bounds validation
    int classVal = getInt("shipClass", 0);
    constexpr int kMaxShipClass = static_cast<int>(ShipClass::Explorer);
    if (classVal >= 0 && classVal <= kMaxShipClass) {
        _definition = ShipClassDefinition::GetDefaultDefinition(
            static_cast<ShipClass>(classVal));
    } else {
        _definition = ShipClassDefinition::GetDefaultDefinition(ShipClass::Fighter);
    }

    // Override with serialized values (allows customised definitions)
    std::string name = getStr("displayName");
    if (!name.empty()) _definition.displayName = name;

    std::string desc = getStr("description");
    if (!desc.empty()) _definition.description = desc;

    int roleVal = getInt("role", static_cast<int>(_definition.role));
    constexpr int kMaxRole = static_cast<int>(ShipRole::MultiRole);
    if (roleVal >= 0 && roleVal <= kMaxRole) {
        _definition.role = static_cast<ShipRole>(roleVal);
    }

    _definition.minCrew   = getInt("minCrew", _definition.minCrew);
    _definition.maxCrew   = getInt("maxCrew", _definition.maxCrew);
    _definition.baseMass  = getFloat("baseMass", _definition.baseMass);
    _definition.baseHull  = getFloat("baseHull", _definition.baseHull);
    _definition.techLevel = getInt("techLevel", _definition.techLevel);

    _definition.bonus.speedMultiplier  = getFloat("speedMultiplier",  _definition.bonus.speedMultiplier);
    _definition.bonus.damageMultiplier = getFloat("damageMultiplier", _definition.bonus.damageMultiplier);
    _definition.bonus.shieldMultiplier = getFloat("shieldMultiplier", _definition.bonus.shieldMultiplier);
    _definition.bonus.cargoMultiplier  = getFloat("cargoMultiplier",  _definition.bonus.cargoMultiplier);
    _definition.bonus.miningMultiplier = getFloat("miningMultiplier", _definition.bonus.miningMultiplier);
    _definition.bonus.sensorMultiplier = getFloat("sensorMultiplier", _definition.bonus.sensorMultiplier);
}

// ---------------------------------------------------------------------------
// ShipClassSystem
// ---------------------------------------------------------------------------

ShipClassSystem::ShipClassSystem() : SystemBase("ShipClassSystem") {}

ShipClassSystem::ShipClassSystem(EntityManager& entityManager)
    : SystemBase("ShipClassSystem")
    , _entityManager(&entityManager)
{
}

void ShipClassSystem::SetEntityManager(EntityManager* em) {
    _entityManager = em;
}

void ShipClassSystem::Update(float /*deltaTime*/) {
    // Class bonuses are applied on-demand via component getters.
    // No per-frame processing required.
}

bool ShipClassSystem::CanUpgradeClass(ShipClass current, ShipClass target) const {
    int currentTech = ShipClassDefinition::GetDefaultDefinition(current).techLevel;
    int targetTech  = ShipClassDefinition::GetDefaultDefinition(target).techLevel;
    return targetTech <= currentTech + 2;
}

std::vector<ShipClass> ShipClassSystem::GetAvailableUpgrades(ShipClass current) const {
    static const ShipClass allClasses[] = {
        ShipClass::Fighter,    ShipClass::Corvette,  ShipClass::Frigate,
        ShipClass::Destroyer,  ShipClass::Cruiser,   ShipClass::Battleship,
        ShipClass::Carrier,    ShipClass::Freighter,  ShipClass::Miner,
        ShipClass::Explorer
    };

    std::vector<ShipClass> upgrades;
    for (ShipClass candidate : allClasses) {
        if (candidate != current && CanUpgradeClass(current, candidate)) {
            upgrades.push_back(candidate);
        }
    }
    return upgrades;
}

} // namespace subspace
