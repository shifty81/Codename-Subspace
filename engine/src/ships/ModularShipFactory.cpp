#include "ships/ModularShipFactory.h"

#include <algorithm>
#include <sstream>
#include <utility>

namespace subspace {
namespace {

std::uint32_t SeedOrDefault(std::uint32_t seed) {
    return seed == 0 ? 0x5EED1234u : seed;
}

ModularShipSize RandomCapitalSize(std::mt19937& random) {
    std::uniform_int_distribution<int> dist(0, 1);
    return dist(random) == 0 ? ModularShipSize::Cruiser : ModularShipSize::Battleship;
}

} // namespace

bool ModularGeneratedShip::Empty() const {
    return modules.empty();
}

std::size_t ModularGeneratedShip::ModuleCount() const {
    return modules.size();
}

ModularShipFactory::ModularShipFactory(ShipModuleLibrary& library, std::uint32_t seed)
    : m_library(library), m_random(SeedOrDefault(seed)) {
    if (m_library.Empty()) {
        m_library.InitializeBuiltInModules();
    }
}

ModularGeneratedShip ModularShipFactory::CreateShipForAI(AIPersonality personality,
                                                         const std::string& name,
                                                         const std::string& material) {
    return CreateCustomShip(CreateConfigForPersonality(personality, name, material, NextSeed()));
}

ModularGeneratedShip ModularShipFactory::CreateFighter(const std::string& name, const std::string& material) {
    ModularShipConfig config;
    config.shipName = name;
    config.size = ModularShipSize::Fighter;
    config.role = ModularShipRole::Combat;
    config.material = material;
    config.seed = NextSeed();
    config.addWings = true;
    config.addWeapons = true;
    config.addCargo = false;
    config.addHyperdrive = false;
    config.desiredWeaponMounts = 2;
    config.minimumEngines = 1;
    return CreateCustomShip(config);
}

ModularGeneratedShip ModularShipFactory::CreateMiner(const std::string& name, const std::string& material) {
    ModularShipConfig config;
    config.shipName = name;
    config.size = ModularShipSize::Corvette;
    config.role = ModularShipRole::Mining;
    config.material = material;
    config.seed = NextSeed();
    config.addWings = false;
    config.addWeapons = true;
    config.addCargo = true;
    config.addHyperdrive = false;
    config.desiredWeaponMounts = 1;
    config.minimumEngines = 1;
    return CreateCustomShip(config);
}

ModularGeneratedShip ModularShipFactory::CreateTrader(const std::string& name, const std::string& material) {
    ModularShipConfig config;
    config.shipName = name;
    config.size = ModularShipSize::Corvette;
    config.role = ModularShipRole::Trading;
    config.material = material;
    config.seed = NextSeed();
    config.addWings = false;
    config.addWeapons = true;
    config.addCargo = true;
    config.addHyperdrive = true;
    config.desiredWeaponMounts = 1;
    config.minimumEngines = 1;
    return CreateCustomShip(config);
}

ModularGeneratedShip ModularShipFactory::CreateCapitalShip(const std::string& name, const std::string& material) {
    ModularShipConfig config;
    config.shipName = name;
    config.size = RandomCapitalSize(m_random);
    config.role = ModularShipRole::Combat;
    config.material = material;
    config.seed = NextSeed();
    config.addWings = false;
    config.addWeapons = true;
    config.addCargo = true;
    config.addHyperdrive = true;
    config.desiredWeaponMounts = 5;
    config.minimumEngines = 3;
    return CreateCustomShip(config);
}

ModularGeneratedShip ModularShipFactory::CreateCustomShip(const ModularShipConfig& config) {
    if (config.seed != 0) {
        m_random.seed(config.seed);
    }

    ModularGeneratedShip ship;
    ship.entityId = "modship-" + std::to_string(NextSeed());
    ship.name = config.shipName;
    ship.config = config;

    const bool smallCraft = config.size == ModularShipSize::Fighter;
    const std::string cockpit = smallCraft ? "cockpit_small" : "cockpit_basic";
    const std::string hull = smallCraft ? "hull_section_small" : "hull_section_basic";
    const std::string engine = smallCraft ? "engine_small" : "engine_main_basic";
    const std::string thruster = smallCraft ? "thruster_small" : "thruster_basic";

    AddPart(ship, cockpit, {0.0f, 0.0f, 0.0f}, "core");
    if (!ship.modules.empty()) {
        ship.coreModuleId = ship.modules.front().id;
    }
    ship.moduleCounts["Cockpit"] = 1;

    const int hullCount = HullSectionCountForSize(config.size);
    for (int i = 0; i < hullCount; ++i) {
        const float z = -3.0f - static_cast<float>(i) * 3.0f;
        AddPart(ship, hull, {0.0f, 0.0f, z}, "rear");
    }
    ship.moduleCounts["Hull"] = hullCount;

    const int engineCount = EngineCountForSize(config.size, config.minimumEngines);
    for (int i = 0; i < engineCount; ++i) {
        const float x = (engineCount == 1) ? 0.0f : (-1.5f + static_cast<float>(i) * 3.0f);
        AddPart(ship, engine, {x, 0.0f, -5.0f - static_cast<float>(hullCount) * 2.0f}, "rear");
    }
    ship.moduleCounts["Engine"] = engineCount;

    AddPart(ship, thruster, {-1.5f, 0.0f, -1.5f}, "left");
    AddPart(ship, thruster, {1.5f, 0.0f, -1.5f}, "right");
    ship.moduleCounts["Thruster"] = 2;

    if (config.addWings && config.size <= ModularShipSize::Destroyer) {
        AddPart(ship, smallCraft ? "wing_small_left" : "wing_basic", {-3.0f, 0.0f, -2.0f}, "left");
        AddPart(ship, smallCraft ? "wing_small_right" : "wing_basic", {3.0f, 0.0f, -2.0f}, "right");
        ship.moduleCounts["Wing"] = 2;
    }

    AddPart(ship, "power_core_basic", {0.0f, -0.5f, -3.0f}, "internal");
    ship.moduleCounts["PowerCore"] = 1;

    if (config.addWeapons) {
        for (int i = 0; i < std::max(0, config.desiredWeaponMounts); ++i) {
            const float x = (i % 2 == 0) ? -1.0f : 1.0f;
            const float z = -1.0f - static_cast<float>(i / 2) * 2.0f;
            AddPart(ship, "weapon_mount_basic", {x, 1.0f, z}, "top");
            AddPart(ship, "turret_basic", {x, 1.5f, z}, "mount");
        }
        ship.moduleCounts["WeaponMount"] = std::max(0, config.desiredWeaponMounts);
        ship.moduleCounts["Weapon"] = std::max(0, config.desiredWeaponMounts);
    }

    if (config.addCargo || config.role == ModularShipRole::Trading || config.role == ModularShipRole::Mining) {
        AddPart(ship, "cargo_bay_basic", {0.0f, -1.0f, -4.5f}, "internal");
        ship.moduleCounts["Cargo"] = 1;
    }

    if (config.addHyperdrive && config.size >= ModularShipSize::Corvette) {
        AddPart(ship, "hyperdrive_basic", {0.0f, -0.8f, -6.0f}, "internal");
        ship.moduleCounts["Hyperdrive"] = 1;
    }

    AddPart(ship, "sensor_basic", {0.0f, 1.2f, 1.0f}, "top");
    ship.moduleCounts["Sensor"] = 1;

    AddPart(ship, "crew_quarters_basic", {0.0f, -0.8f, -2.0f}, "internal");
    ship.moduleCounts["CrewQuarters"] = 1;

    if (config.role == ModularShipRole::Mining) {
        AddPart(ship, "mining_laser_basic", {0.0f, 1.0f, 1.8f}, "front");
        ship.moduleCounts["Mining"] = 1;
    }

    if (config.role == ModularShipRole::Exploration) {
        AddPart(ship, "antenna_basic", {0.0f, 2.0f, -1.0f}, "top");
        ship.moduleCounts["Antenna"] = 1;
    }

    CompileGeneratedStats(ship);
    return ship;
}

ModularShipConfig ModularShipFactory::CreateConfigForPersonality(AIPersonality personality,
                                                                 const std::string& name,
                                                                 const std::string& material,
                                                                 std::uint32_t seed) {
    ModularShipConfig config;
    config.shipName = name;
    config.material = material;
    config.seed = seed;

    switch (personality) {
        case AIPersonality::Trader:
            config.size = ModularShipSize::Corvette;
            config.role = ModularShipRole::Trading;
            config.addWings = false;
            config.addWeapons = true;
            config.addCargo = true;
            config.addHyperdrive = true;
            config.desiredWeaponMounts = 1;
            config.minimumEngines = 1;
            break;
        case AIPersonality::Miner:
            config.size = ModularShipSize::Corvette;
            config.role = ModularShipRole::Mining;
            config.addWings = false;
            config.addWeapons = true;
            config.addCargo = true;
            config.addHyperdrive = false;
            config.desiredWeaponMounts = 1;
            config.minimumEngines = 1;
            break;
        case AIPersonality::Aggressive:
            config.size = ModularShipSize::Frigate;
            config.role = ModularShipRole::Combat;
            config.addWings = true;
            config.addWeapons = true;
            config.addCargo = false;
            config.addHyperdrive = true;
            config.desiredWeaponMounts = 3;
            config.minimumEngines = 2;
            break;
        case AIPersonality::Defensive:
            config.size = ModularShipSize::Frigate;
            config.role = ModularShipRole::Combat;
            config.addWings = true;
            config.addWeapons = true;
            config.addCargo = false;
            config.addHyperdrive = false;
            config.desiredWeaponMounts = 2;
            config.minimumEngines = 1;
            break;
        case AIPersonality::Explorer:
            config.size = ModularShipSize::Fighter;
            config.role = ModularShipRole::Exploration;
            config.addWings = true;
            config.addWeapons = true;
            config.addCargo = false;
            config.addHyperdrive = true;
            config.desiredWeaponMounts = 1;
            config.minimumEngines = 1;
            break;
        case AIPersonality::Salvager:
            config.size = ModularShipSize::Corvette;
            config.role = ModularShipRole::Salvage;
            config.addWings = false;
            config.addWeapons = true;
            config.addCargo = true;
            config.addHyperdrive = false;
            config.desiredWeaponMounts = 1;
            config.minimumEngines = 1;
            break;
        case AIPersonality::Balanced:
        case AIPersonality::Coward:
        default:
            config.size = ModularShipSize::Corvette;
            config.role = ModularShipRole::Multipurpose;
            config.addWings = true;
            config.addWeapons = true;
            config.addCargo = true;
            config.addHyperdrive = true;
            config.desiredWeaponMounts = 2;
            config.minimumEngines = 1;
            break;
    }

    return config;
}

std::uint32_t ModularShipFactory::NextSeed() {
    return m_random();
}

std::string ModularShipFactory::NextModuleId() {
    return "module-" + std::to_string(m_nextId++);
}

ShipModulePart ModularShipFactory::MakePart(const ShipModuleDefinition& definition,
                                            Vector3 position,
                                            const std::string& material,
                                            const std::string& attachment) {
    ShipModulePart part(definition.id, position, material);
    part.id = NextModuleId();
    part.attachmentPointUsed = attachment;
    part.maxHealth = definition.GetHealthForMaterial(material);
    part.health = part.maxHealth;
    part.mass = definition.GetMassForMaterial(material);
    part.functionalStats = definition.GetStatsForMaterial(material);
    part.colorTint = ModuleMaterialProperties::GetMaterial(material).color;
    return part;
}

void ModularShipFactory::AddPart(ModularGeneratedShip& ship,
                                 const std::string& definitionId,
                                 Vector3 position,
                                 const std::string& attachment) {
    const auto* definition = m_library.GetDefinition(definitionId);
    if (!definition) {
        ship.warnings.push_back("Missing module definition: " + definitionId);
        return;
    }
    ship.modules.push_back(MakePart(*definition, position, ship.config.material, attachment));
}

void ModularShipFactory::CompileGeneratedStats(ModularGeneratedShip& ship) const {
    ship.compiledStats = {};
    ship.totalMass = 0.0f;
    ship.totalHealth = 0.0f;

    for (const auto& module : ship.modules) {
        ship.totalMass += module.mass;
        ship.totalHealth += module.maxHealth;
        ship.compiledStats.Add(module.functionalStats);
    }

    if (ship.compiledStats.powerGeneration < ship.compiledStats.powerConsumption) {
        ship.warnings.push_back("Power consumption exceeds generation.");
    }
    if (ship.compiledStats.thrustPower <= 0.0f) {
        ship.warnings.push_back("Ship has no thrust power.");
    }
}

int HullSectionCountForSize(ModularShipSize size) {
    switch (size) {
        case ModularShipSize::Fighter: return 1;
        case ModularShipSize::Corvette: return 2;
        case ModularShipSize::Frigate: return 3;
        case ModularShipSize::Destroyer: return 4;
        case ModularShipSize::Cruiser: return 6;
        case ModularShipSize::Battleship: return 8;
        case ModularShipSize::Carrier: return 10;
    }
    return 2;
}

int EngineCountForSize(ModularShipSize size, int minimumEngines) {
    int base = 1;
    switch (size) {
        case ModularShipSize::Fighter:
        case ModularShipSize::Corvette: base = 1; break;
        case ModularShipSize::Frigate:
        case ModularShipSize::Destroyer: base = 2; break;
        case ModularShipSize::Cruiser: base = 3; break;
        case ModularShipSize::Battleship:
        case ModularShipSize::Carrier: base = 4; break;
    }
    return std::max(base, minimumEngines);
}

ModuleShipClass ToModuleShipClass(ModularShipSize size, ModularShipRole role) {
    switch (role) {
        case ModularShipRole::Mining: return ModuleShipClass::Miner;
        case ModularShipRole::Trading: return ModuleShipClass::Hauler;
        case ModularShipRole::Salvage: return ModuleShipClass::Salvager;
        case ModularShipRole::Exploration: return ModuleShipClass::Scout;
        case ModularShipRole::Combat:
        case ModularShipRole::Multipurpose:
            break;
    }

    switch (size) {
        case ModularShipSize::Fighter: return ModuleShipClass::Fighter;
        case ModularShipSize::Corvette: return ModuleShipClass::Corvette;
        case ModularShipSize::Frigate: return ModuleShipClass::Frigate;
        case ModularShipSize::Destroyer: return ModuleShipClass::Destroyer;
        case ModularShipSize::Cruiser: return ModuleShipClass::Cruiser;
        case ModularShipSize::Battleship: return ModuleShipClass::Battleship;
        case ModularShipSize::Carrier: return ModuleShipClass::Carrier;
    }
    return ModuleShipClass::Corvette;
}

std::string ModularShipSizeName(ModularShipSize size) {
    switch (size) {
        case ModularShipSize::Fighter: return "Fighter";
        case ModularShipSize::Corvette: return "Corvette";
        case ModularShipSize::Frigate: return "Frigate";
        case ModularShipSize::Destroyer: return "Destroyer";
        case ModularShipSize::Cruiser: return "Cruiser";
        case ModularShipSize::Battleship: return "Battleship";
        case ModularShipSize::Carrier: return "Carrier";
    }
    return "Unknown";
}

std::string ModularShipRoleName(ModularShipRole role) {
    switch (role) {
        case ModularShipRole::Multipurpose: return "Multipurpose";
        case ModularShipRole::Combat: return "Combat";
        case ModularShipRole::Mining: return "Mining";
        case ModularShipRole::Trading: return "Trading";
        case ModularShipRole::Exploration: return "Exploration";
        case ModularShipRole::Salvage: return "Salvage";
    }
    return "Unknown";
}

} // namespace subspace
