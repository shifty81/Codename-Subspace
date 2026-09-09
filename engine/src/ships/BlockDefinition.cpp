#include "ships/BlockDefinition.h"

#include <utility>

namespace subspace {

bool BlockDefinitionDatabase::s_initialized = false;
std::unordered_map<BlockType, BlockDefinition> BlockDefinitionDatabase::s_definitions;

BlockDefinition MakeBlockDefinition(BlockType type,
                                    std::string id,
                                    std::string displayName,
                                    std::string description,
                                    std::string function,
                                    float hpPerVolume,
                                    float massPerVolume,
                                    std::uint32_t colorRgb) {
    BlockDefinition def;
    def.blockType = type;
    def.id = std::move(id);
    def.displayName = std::move(displayName);
    def.description = std::move(description);
    def.function = std::move(function);
    def.hitPointsPerVolume = hpPerVolume;
    def.massPerUnitVolume = massPerVolume;
    def.defaultColorRGB = colorRgb;
    return def;
}

void BlockDefinitionDatabase::Initialize() {
    if (s_initialized) return;

    auto add = [](BlockDefinition def) {
        BlockType key = def.blockType;
        s_definitions[key] = std::move(def);
    };

    {
        auto def = MakeBlockDefinition(BlockType::Hull, "hull_basic", "Hull Block",
            "Basic structural hull block", "structure", 100.0f, 1.0f, 0x808080);
        def.resourceCosts = {{"Iron", 10}};
        def.aiPlacementPriority = 5;
        add(std::move(def));
    }
    {
        auto def = MakeBlockDefinition(BlockType::Armor, "armor_plating", "Armor Plating",
            "Heavy armor protection for hull", "protection", 500.0f, 1.5f, 0xA0A0A0);
        def.resourceCosts = {{"Iron", 15}};
        def.aiPlacementPriority = 8;
        add(std::move(def));
    }
    {
        auto def = MakeBlockDefinition(BlockType::Framework, "framework_block", "Framework Block",
            "Lightweight shaping block", "shaping", 20.0f, 0.1f, 0x606060);
        def.resourceCosts = {{"Iron", 3}};
        def.aiPlacementPriority = 3;
        add(std::move(def));
    }
    {
        auto def = MakeBlockDefinition(BlockType::Engine, "engine_main", "Main Engine",
            "Primary propulsion engine providing forward thrust", "generateThrust", 80.0f, 1.2f, 0xFF8040);
        def.resourceCosts = {{"Iron", 20}, {"Titanium", 5}};
        def.thrustPowerPerVolume = 50.0f;
        def.powerConsumptionPerVolume = 5.0f;
        def.aiPlacementPriority = 9;
        def.requiresInternalPlacement = false;
        def.suitableForExterior = true;
        add(std::move(def));
    }
    {
        auto def = MakeBlockDefinition(BlockType::Thruster, "thruster_omni", "Omnidirectional Thruster",
            "Maneuvering, strafing, and braking thruster", "generateManeuverThrust", 70.0f, 1.1f, 0xFFAA55);
        def.resourceCosts = {{"Iron", 16}, {"Titanium", 4}};
        def.thrustPowerPerVolume = 30.0f;
        def.powerConsumptionPerVolume = 4.0f;
        def.aiPlacementPriority = 8;
        add(std::move(def));
    }
    {
        auto def = MakeBlockDefinition(BlockType::Gyro, "gyro_array", "Gyro Array",
            "Rotation and torque control system", "generateTorque", 70.0f, 1.0f, 0x40E0D0);
        def.resourceCosts = {{"Iron", 10}, {"Titanium", 8}};
        def.thrustPowerPerVolume = 20.0f;
        def.powerConsumptionPerVolume = 3.0f;
        def.aiPlacementPriority = 7;
        add(std::move(def));
    }
    {
        auto def = MakeBlockDefinition(BlockType::Generator, "generator_core", "Generator",
            "Ship power generation block", "generatePower", 90.0f, 1.4f, 0xFFFF66);
        def.resourceCosts = {{"Iron", 20}, {"Naonite", 3}};
        def.powerGenerationPerVolume = 100.0f;
        def.aiPlacementPriority = 10;
        def.requiresInternalPlacement = true;
        def.suitableForExterior = false;
        add(std::move(def));
    }
    {
        auto def = MakeBlockDefinition(BlockType::ShieldGenerator, "shield_generator", "Shield Generator",
            "Shield capacity generator", "generateShields", 95.0f, 1.3f, 0x66AAFF);
        def.resourceCosts = {{"Titanium", 12}, {"Naonite", 8}};
        def.shieldCapacityPerVolume = 200.0f;
        def.powerConsumptionPerVolume = 8.0f;
        def.aiPlacementPriority = 8;
        def.requiresInternalPlacement = true;
        def.suitableForExterior = false;
        def.minTechLevel = 2;
        add(std::move(def));
    }
    {
        auto def = MakeBlockDefinition(BlockType::WeaponMount, "turret_mount", "Turret Mount",
            "External mount point for weapons and turrets", "mountWeapon", 75.0f, 0.9f, 0xFF5555);
        def.resourceCosts = {{"Iron", 12}, {"Titanium", 6}};
        def.powerConsumptionPerVolume = 1.0f;
        def.aiPlacementPriority = 7;
        add(std::move(def));
    }
    {
        auto def = MakeBlockDefinition(BlockType::Cargo, "cargo_hold", "Cargo Hold",
            "Storage volume for cargo and salvage", "storeCargo", 60.0f, 0.8f, 0xB08040);
        def.resourceCosts = {{"Iron", 8}};
        def.cargoCapacityPerVolume = 100.0f;
        def.aiPlacementPriority = 5;
        def.requiresInternalPlacement = true;
        def.suitableForExterior = false;
        add(std::move(def));
    }
    {
        auto def = MakeBlockDefinition(BlockType::CrewQuarters, "crew_quarters", "Crew Quarters",
            "Habitation volume for crew", "houseCrew", 80.0f, 0.9f, 0x80C080);
        def.resourceCosts = {{"Iron", 8}, {"Titanium", 2}};
        def.crewCapacityPerVolume = 0.5f;
        def.requiresInternalPlacement = true;
        def.suitableForExterior = false;
        add(std::move(def));
    }
    {
        auto def = MakeBlockDefinition(BlockType::HyperdriveCore, "hyperdrive_core", "Hyperdrive Core",
            "Jump capability core for sector transitions", "enableHyperdrive", 150.0f, 2.0f, 0xAA66FF);
        def.resourceCosts = {{"Naonite", 25}, {"Trinium", 10}};
        def.powerConsumptionPerVolume = 25.0f;
        def.aiPlacementPriority = 10;
        def.requiresInternalPlacement = true;
        def.suitableForExterior = false;
        def.minTechLevel = 3;
        add(std::move(def));
    }
    {
        auto def = MakeBlockDefinition(BlockType::PodDocking, "pod_docking", "Pod Docking Port",
            "Docking port for player pod and small service craft", "dockPod", 120.0f, 1.1f, 0x66CCCC);
        def.resourceCosts = {{"Iron", 15}, {"Titanium", 5}};
        def.aiPlacementPriority = 6;
        add(std::move(def));
    }
    {
        auto def = MakeBlockDefinition(BlockType::Computer, "computer_core", "Computer Core",
            "Upgrade/computing capacity block", "provideComputing", 65.0f, 0.7f, 0x99CCFF);
        def.resourceCosts = {{"Titanium", 10}, {"Naonite", 5}};
        def.computerSlotsPerVolume = 0.15f;
        def.powerConsumptionPerVolume = 4.0f;
        def.requiresInternalPlacement = true;
        def.suitableForExterior = false;
        add(std::move(def));
    }
    {
        auto def = MakeBlockDefinition(BlockType::Battery, "battery_bank", "Battery Bank",
            "Energy storage block", "storeEnergy", 85.0f, 1.2f, 0x88FF88);
        def.resourceCosts = {{"Iron", 10}, {"Naonite", 4}};
        def.batteryCapacityPerVolume = 150.0f;
        def.aiPlacementPriority = 6;
        def.requiresInternalPlacement = true;
        def.suitableForExterior = false;
        add(std::move(def));
    }
    {
        auto def = MakeBlockDefinition(BlockType::IntegrityField, "integrity_field", "Integrity Field Generator",
            "Structural integrity field support block", "reinforceStructure", 90.0f, 1.0f, 0x55FFCC);
        def.resourceCosts = {{"Titanium", 8}, {"Naonite", 6}};
        def.powerConsumptionPerVolume = 5.0f;
        def.aiPlacementPriority = 7;
        def.requiresInternalPlacement = true;
        def.suitableForExterior = false;
        add(std::move(def));
    }

    s_initialized = true;
}

const std::unordered_map<BlockType, BlockDefinition>& BlockDefinitionDatabase::GetDefinitions() {
    Initialize();
    return s_definitions;
}

const BlockDefinition& BlockDefinitionDatabase::GetDefinition(BlockType blockType) {
    Initialize();
    auto it = s_definitions.find(blockType);
    if (it != s_definitions.end()) return it->second;
    return s_definitions.at(BlockType::Hull);
}

std::vector<BlockType> BlockDefinitionDatabase::GetBuildableTypes() {
    Initialize();
    std::vector<BlockType> result;
    result.reserve(s_definitions.size());
    for (const auto& pair : s_definitions) {
        result.push_back(pair.first);
    }
    return result;
}

} // namespace subspace
