#include "ships/ShipModuleLibrary.h"

#include <algorithm>
#include <utility>

namespace subspace {
namespace {

void AddBasicForeAft(ShipModuleDefinition& module, float zExtent, AttachmentSize size = AttachmentSize::Medium) {
    module.attachmentPoints["front"] = MakeAttachmentPoint("front", {0.0f, 0.0f, zExtent}, {0.0f, 0.0f, 1.0f}, size);
    module.attachmentPoints["rear"] = MakeAttachmentPoint("rear", {0.0f, 0.0f, -zExtent}, {0.0f, 0.0f, -1.0f}, size);
}

void AddPortStarboard(ShipModuleDefinition& module, float xExtent, AttachmentSize size = AttachmentSize::Small) {
    module.attachmentPoints["left"] = MakeAttachmentPoint("left", {-xExtent, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}, size);
    module.attachmentPoints["right"] = MakeAttachmentPoint("right", {xExtent, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, size);
}

ShipModuleDefinition MakeBase(std::string id,
                              std::string name,
                              ModuleCategory category,
                              Vector3 size,
                              float mass,
                              float health,
                              int cost,
                              std::vector<std::string> tags) {
    ShipModuleDefinition module;
    module.id = std::move(id);
    module.name = std::move(name);
    module.category = category;
    module.size = size;
    module.baseMass = mass;
    module.baseHealth = health;
    module.baseCost = cost;
    module.tags = std::move(tags);
    return module;
}

} // namespace

void ShipModuleLibrary::InitializeBuiltInModules() {
    Clear();

    AddDefinition(CreateCockpitModule());
    AddDefinition(CreateHullSectionModule());
    AddDefinition(CreateHullCornerModule());
    AddDefinition(CreateMainEngineModule());
    AddDefinition(CreateEngineNacelleModule());
    AddDefinition(CreateThrusterModule());
    AddDefinition(CreateWingModule());
    AddDefinition(CreateStabilizerModule());
    AddDefinition(CreateWeaponMountModule());
    AddDefinition(CreateTurretModule());
    AddDefinition(CreatePowerCoreModule());
    AddDefinition(CreateShieldGeneratorModule());
    AddDefinition(CreateCargoModule());
    AddDefinition(CreateCrewQuartersModule());
    AddDefinition(CreateHyperdriveModule());
    AddDefinition(CreateSensorModule());
    AddDefinition(CreateMiningModule());
    AddDefinition(CreateAntennaModule());

    // Small craft set used by fighter/scout generation.
    AddDefinition(CreateSmallCockpitModule());
    AddDefinition(CreateSmallHullSectionModule());
    AddDefinition(CreateSmallEngineModule());
    AddDefinition(CreateSmallThrusterModule());
    AddDefinition(CreateSmallWingLeftModule());
    AddDefinition(CreateSmallWingRightModule());
}

void ShipModuleLibrary::Clear() {
    m_definitions.clear();
}

bool ShipModuleLibrary::AddDefinition(const ShipModuleDefinition& definition) {
    if (definition.id.empty()) {
        return false;
    }
    return m_definitions.emplace(definition.id, definition).second;
}

bool ShipModuleLibrary::RemoveDefinition(const std::string& id) {
    return m_definitions.erase(id) > 0;
}

const ShipModuleDefinition* ShipModuleLibrary::GetDefinition(const std::string& id) const {
    auto it = m_definitions.find(id);
    return it == m_definitions.end() ? nullptr : &it->second;
}

std::vector<const ShipModuleDefinition*> ShipModuleLibrary::AllDefinitions() const {
    std::vector<const ShipModuleDefinition*> result;
    result.reserve(m_definitions.size());
    for (const auto& entry : m_definitions) {
        result.push_back(&entry.second);
    }
    std::sort(result.begin(), result.end(), [](const auto* a, const auto* b) {
        return a->id < b->id;
    });
    return result;
}

std::vector<const ShipModuleDefinition*> ShipModuleLibrary::GetDefinitionsByCategory(ModuleCategory category) const {
    std::vector<const ShipModuleDefinition*> result;
    for (const auto& entry : m_definitions) {
        if (entry.second.category == category) {
            result.push_back(&entry.second);
        }
    }
    return result;
}

std::vector<const ShipModuleDefinition*> ShipModuleLibrary::GetDefinitionsByTag(const std::string& tag) const {
    std::vector<const ShipModuleDefinition*> result;
    for (const auto& entry : m_definitions) {
        if (entry.second.HasTag(tag)) {
            result.push_back(&entry.second);
        }
    }
    return result;
}

std::vector<const ShipModuleDefinition*> ShipModuleLibrary::GetCompatibleModules(ModuleShipClass shipClass) const {
    std::vector<const ShipModuleDefinition*> result;
    for (const auto& entry : m_definitions) {
        if (entry.second.classification.IsCompatibleWith(shipClass)) {
            result.push_back(&entry.second);
        }
    }
    return result;
}

std::size_t ShipModuleLibrary::Count() const {
    return m_definitions.size();
}

bool ShipModuleLibrary::Empty() const {
    return m_definitions.empty();
}

AttachmentPoint MakeAttachmentPoint(std::string name,
                                    Vector3 position,
                                    Vector3 direction,
                                    AttachmentSize size,
                                    std::vector<ModuleCategory> allowedCategories,
                                    std::vector<std::string> requiredTags) {
    AttachmentPoint point;
    point.name = std::move(name);
    point.position = position;
    point.direction = direction;
    point.size = size;
    point.allowedCategories = std::move(allowedCategories);
    point.requiredTags = std::move(requiredTags);
    return point;
}

ShipModuleDefinition CreateCockpitModule() {
    auto module = MakeBase("cockpit_basic", "Cockpit", ModuleCategory::Hull,
                           {2.0f, 2.0f, 3.0f}, 15.0f, 150.0f, 500,
                           {"cockpit", "core", "required"});
    module.description = "Basic cockpit/control module for small ships";
    module.subCategory = "Cockpit";
    module.modelPath = "ships/modules/cockpit_basic.obj";
    AddBasicForeAft(module, 1.5f);
    module.baseStats.crewCapacity = 2;
    module.baseStats.crewRequired = 1;
    module.baseStats.sensorRange = 1000.0f;
    module.classification.isRequired = true;
    module.classification.minPerShip = 1;
    module.classification.maxPerShip = 1;
    return module;
}

ShipModuleDefinition CreateHullSectionModule() {
    auto module = MakeBase("hull_section_basic", "Hull Section", ModuleCategory::Hull,
                           {3.0f, 3.0f, 4.0f}, 20.0f, 200.0f, 200,
                           {"hull", "structural"});
    module.description = "Standard hull section for connecting modules";
    module.subCategory = "Section";
    module.modelPath = "ships/modules/hull_section.obj";
    AddBasicForeAft(module, 2.0f);
    AddPortStarboard(module, 1.5f);
    return module;
}

ShipModuleDefinition CreateHullCornerModule() {
    auto module = MakeBase("hull_corner_basic", "Hull Corner", ModuleCategory::Hull,
                           {3.0f, 3.0f, 3.0f}, 15.0f, 150.0f, 150,
                           {"hull", "structural", "corner"});
    module.description = "Corner hull section for angled connections";
    module.subCategory = "Corner";
    AddBasicForeAft(module, 1.5f);
    AddPortStarboard(module, 1.5f);
    return module;
}

ShipModuleDefinition CreateMainEngineModule() {
    auto module = MakeBase("engine_main_basic", "Main Engine", ModuleCategory::Engine,
                           {2.0f, 2.0f, 3.0f}, 18.0f, 120.0f, 800,
                           {"engine", "propulsion", "rear"});
    module.description = "Primary forward-thrust engine";
    module.subCategory = "Main";
    module.modelPath = "ships/modules/engine_main.obj";
    module.attachmentPoints["front"] = MakeAttachmentPoint("front", {0.0f, 0.0f, 1.5f}, {0.0f, 0.0f, 1.0f});
    module.baseStats.thrustPower = 250.0f;
    module.baseStats.maxSpeed = 120.0f;
    module.baseStats.powerConsumption = 50.0f;
    module.classification.visibility = ModuleVisibility::External;
    return module;
}

ShipModuleDefinition CreateEngineNacelleModule() {
    auto module = MakeBase("engine_nacelle_basic", "Engine Nacelle", ModuleCategory::Engine,
                           {1.5f, 1.5f, 4.0f}, 14.0f, 100.0f, 600,
                           {"engine", "propulsion", "nacelle"});
    module.description = "Slim engine nacelle for side-mounted propulsion";
    module.attachmentPoints["side"] = MakeAttachmentPoint("side", {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f});
    module.baseStats.thrustPower = 180.0f;
    module.baseStats.maxSpeed = 130.0f;
    module.baseStats.powerConsumption = 35.0f;
    return module;
}

ShipModuleDefinition CreateThrusterModule() {
    auto module = MakeBase("thruster_basic", "Maneuvering Thruster", ModuleCategory::Thruster,
                           {1.0f, 1.0f, 1.0f}, 5.0f, 60.0f, 200,
                           {"thruster", "maneuvering"});
    module.description = "Small attitude-control thruster";
    module.baseStats.thrustPower = 60.0f;
    module.baseStats.powerConsumption = 10.0f;
    return module;
}

ShipModuleDefinition CreateWingModule() {
    auto module = MakeBase("wing_basic", "Wing", ModuleCategory::Wing,
                           {4.0f, 0.5f, 3.0f}, 8.0f, 80.0f, 250,
                           {"wing", "stabilizer", "external"});
    module.description = "External wing surface for small craft silhouettes";
    module.attachmentPoints["root"] = MakeAttachmentPoint("root", {0.0f, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}, AttachmentSize::Small);
    return module;
}

ShipModuleDefinition CreateStabilizerModule() {
    auto module = MakeBase("stabilizer_basic", "Stabilizer", ModuleCategory::Tail,
                           {1.0f, 2.5f, 2.0f}, 6.0f, 70.0f, 220,
                           {"tail", "stabilizer", "external"});
    module.description = "Vertical or horizontal tail stabilizer";
    module.attachmentPoints["root"] = MakeAttachmentPoint("root", {0.0f, 0.0f, 0.0f}, {0.0f, -1.0f, 0.0f}, AttachmentSize::Small);
    return module;
}

ShipModuleDefinition CreateWeaponMountModule() {
    auto module = MakeBase("weapon_mount_basic", "Weapon Mount", ModuleCategory::WeaponMount,
                           {1.0f, 0.5f, 1.0f}, 6.0f, 80.0f, 350,
                           {"weapon", "mount", "hardpoint"});
    module.description = "Hardpoint for attaching turret or fixed weapon modules";
    module.attachmentPoints["base"] = MakeAttachmentPoint("base", {0.0f, -0.25f, 0.0f}, {0.0f, -1.0f, 0.0f}, AttachmentSize::Small);
    module.baseStats.weaponMountPoints = 1;
    return module;
}

ShipModuleDefinition CreateTurretModule() {
    auto module = MakeBase("turret_basic", "Pulse Turret", ModuleCategory::Weapon,
                           {1.0f, 1.0f, 1.0f}, 8.0f, 70.0f, 600,
                           {"weapon", "turret", "pulse"});
    module.description = "Basic pulse turret weapon";
    module.baseStats.weaponDamage = 25.0f;
    module.baseStats.weaponRange = 850.0f;
    module.baseStats.powerConsumption = 20.0f;
    return module;
}

ShipModuleDefinition CreatePowerCoreModule() {
    auto module = MakeBase("power_core_basic", "Power Core", ModuleCategory::PowerCore,
                           {2.0f, 2.0f, 2.0f}, 16.0f, 120.0f, 700,
                           {"power", "core", "internal"});
    module.description = "Ship power generator and storage module";
    module.baseStats.powerGeneration = 180.0f;
    module.baseStats.powerStorage = 300.0f;
    module.classification.visibility = ModuleVisibility::Internal;
    module.classification.isRequired = true;
    return module;
}

ShipModuleDefinition CreateShieldGeneratorModule() {
    auto module = MakeBase("shield_generator_basic", "Shield Generator", ModuleCategory::Shield,
                           {2.0f, 2.0f, 2.0f}, 12.0f, 100.0f, 900,
                           {"shield", "defense", "internal"});
    module.description = "Generates defensive ship shields";
    module.baseStats.shieldCapacity = 500.0f;
    module.baseStats.shieldRechargeRate = 20.0f;
    module.baseStats.powerConsumption = 40.0f;
    module.classification.visibility = ModuleVisibility::Internal;
    return module;
}

ShipModuleDefinition CreateCargoModule() {
    auto module = MakeBase("cargo_bay_basic", "Cargo Bay", ModuleCategory::Cargo,
                           {3.0f, 2.0f, 3.0f}, 12.0f, 100.0f, 400,
                           {"cargo", "storage", "internal"});
    module.description = "Internal cargo storage bay";
    module.baseStats.cargoCapacity = 100.0f;
    module.classification.visibility = ModuleVisibility::Internal;
    return module;
}

ShipModuleDefinition CreateCrewQuartersModule() {
    auto module = MakeBase("crew_quarters_basic", "Crew Quarters", ModuleCategory::CrewQuarters,
                           {3.0f, 2.0f, 3.0f}, 10.0f, 100.0f, 350,
                           {"crew", "quarters", "internal"});
    module.description = "Crew habitation and life-support space";
    module.baseStats.crewCapacity = 8;
    module.baseStats.crewRequired = 0;
    module.classification.visibility = ModuleVisibility::Internal;
    return module;
}

ShipModuleDefinition CreateHyperdriveModule() {
    auto module = MakeBase("hyperdrive_basic", "Hyperdrive", ModuleCategory::Hyperdrive,
                           {2.5f, 2.5f, 2.5f}, 20.0f, 130.0f, 1500,
                           {"hyperdrive", "ftl", "internal"});
    module.description = "Short-range FTL jump module";
    module.baseStats.hasHyperdrive = true;
    module.baseStats.hyperdriveRange = 10.0f;
    module.baseStats.powerConsumption = 80.0f;
    module.classification.visibility = ModuleVisibility::Internal;
    return module;
}

ShipModuleDefinition CreateSensorModule() {
    auto module = MakeBase("sensor_basic", "Sensor Array", ModuleCategory::Sensor,
                           {1.5f, 1.5f, 1.5f}, 5.0f, 50.0f, 300,
                           {"sensor", "scanner", "utility"});
    module.description = "General-purpose ship sensor array";
    module.baseStats.sensorRange = 2500.0f;
    module.baseStats.powerConsumption = 15.0f;
    module.classification.visibility = ModuleVisibility::Both;
    return module;
}

ShipModuleDefinition CreateMiningModule() {
    auto module = MakeBase("mining_laser_basic", "Mining Laser", ModuleCategory::Mining,
                           {1.0f, 1.0f, 2.0f}, 8.0f, 75.0f, 700,
                           {"mining", "laser", "industrial"});
    module.description = "Mining beam module for asteroid extraction";
    module.baseStats.miningPower = 30.0f;
    module.baseStats.weaponRange = 600.0f;
    module.baseStats.powerConsumption = 25.0f;
    module.classification.compatibleClasses = ModuleShipClass::Miner | ModuleShipClass::AllCombat | ModuleShipClass::Scout;
    return module;
}

ShipModuleDefinition CreateAntennaModule() {
    auto module = MakeBase("antenna_basic", "Antenna", ModuleCategory::Antenna,
                           {0.3f, 2.0f, 0.3f}, 2.0f, 30.0f, 120,
                           {"antenna", "decorative", "sensor"});
    module.description = "External communications antenna";
    module.baseStats.sensorRange = 300.0f;
    module.classification.visibility = ModuleVisibility::External;
    return module;
}

ShipModuleDefinition CreateSmallCockpitModule() {
    auto module = CreateCockpitModule();
    module.id = "cockpit_small";
    module.name = "Small Cockpit";
    module.size = {1.5f, 1.5f, 2.0f};
    module.baseMass = 8.0f;
    module.baseHealth = 90.0f;
    module.baseCost = 320;
    module.classification.size = ModuleSize::S;
    module.classification.compatibleClasses = ModuleShipClass::Fighter | ModuleShipClass::Scout | ModuleShipClass::Corvette;
    return module;
}

ShipModuleDefinition CreateSmallHullSectionModule() {
    auto module = CreateHullSectionModule();
    module.id = "hull_section_small";
    module.name = "Small Hull Section";
    module.size = {2.0f, 2.0f, 2.5f};
    module.baseMass = 10.0f;
    module.baseHealth = 110.0f;
    module.baseCost = 120;
    module.classification.size = ModuleSize::S;
    module.classification.compatibleClasses = ModuleShipClass::Fighter | ModuleShipClass::Scout | ModuleShipClass::Corvette;
    return module;
}

ShipModuleDefinition CreateSmallEngineModule() {
    auto module = CreateMainEngineModule();
    module.id = "engine_small";
    module.name = "Small Engine";
    module.size = {1.0f, 1.0f, 1.5f};
    module.baseMass = 7.0f;
    module.baseHealth = 65.0f;
    module.baseCost = 350;
    module.baseStats.thrustPower = 120.0f;
    module.baseStats.powerConsumption = 25.0f;
    module.classification.size = ModuleSize::S;
    module.classification.compatibleClasses = ModuleShipClass::Fighter | ModuleShipClass::Scout | ModuleShipClass::Corvette;
    return module;
}

ShipModuleDefinition CreateSmallThrusterModule() {
    auto module = CreateThrusterModule();
    module.id = "thruster_small";
    module.name = "Small Thruster";
    module.size = {0.6f, 0.6f, 0.6f};
    module.baseMass = 2.0f;
    module.baseHealth = 35.0f;
    module.baseStats.thrustPower = 30.0f;
    module.classification.size = ModuleSize::S;
    module.classification.compatibleClasses = ModuleShipClass::Fighter | ModuleShipClass::Scout | ModuleShipClass::Corvette;
    return module;
}

ShipModuleDefinition CreateSmallWingLeftModule() {
    auto module = CreateWingModule();
    module.id = "wing_small_left";
    module.name = "Small Left Wing";
    module.size = {2.5f, 0.35f, 2.0f};
    module.baseMass = 4.0f;
    module.baseHealth = 45.0f;
    module.tags.push_back("left");
    module.classification.size = ModuleSize::S;
    module.classification.compatibleClasses = ModuleShipClass::Fighter | ModuleShipClass::Scout | ModuleShipClass::Corvette;
    return module;
}

ShipModuleDefinition CreateSmallWingRightModule() {
    auto module = CreateSmallWingLeftModule();
    module.id = "wing_small_right";
    module.name = "Small Right Wing";
    module.tags.erase(std::remove(module.tags.begin(), module.tags.end(), "left"), module.tags.end());
    module.tags.push_back("right");
    return module;
}

} // namespace subspace
