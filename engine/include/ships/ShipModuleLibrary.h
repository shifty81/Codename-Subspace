#pragma once

#include "ships/ShipModuleDefinition.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace subspace {

class ShipModuleLibrary {
public:
    void InitializeBuiltInModules();
    void Clear();
    bool AddDefinition(const ShipModuleDefinition& definition);
    bool RemoveDefinition(const std::string& id);

    const ShipModuleDefinition* GetDefinition(const std::string& id) const;
    std::vector<const ShipModuleDefinition*> AllDefinitions() const;
    std::vector<const ShipModuleDefinition*> GetDefinitionsByCategory(ModuleCategory category) const;
    std::vector<const ShipModuleDefinition*> GetDefinitionsByTag(const std::string& tag) const;
    std::vector<const ShipModuleDefinition*> GetCompatibleModules(ModuleShipClass shipClass) const;

    std::size_t Count() const;
    bool Empty() const;

private:
    std::unordered_map<std::string, ShipModuleDefinition> m_definitions;
};

AttachmentPoint MakeAttachmentPoint(std::string name,
                                    Vector3 position,
                                    Vector3 direction,
                                    AttachmentSize size = AttachmentSize::Medium,
                                    std::vector<ModuleCategory> allowedCategories = {},
                                    std::vector<std::string> requiredTags = {});

ShipModuleDefinition CreateCockpitModule();
ShipModuleDefinition CreateHullSectionModule();
ShipModuleDefinition CreateHullCornerModule();
ShipModuleDefinition CreateMainEngineModule();
ShipModuleDefinition CreateEngineNacelleModule();
ShipModuleDefinition CreateThrusterModule();
ShipModuleDefinition CreateWingModule();
ShipModuleDefinition CreateStabilizerModule();
ShipModuleDefinition CreateWeaponMountModule();
ShipModuleDefinition CreateTurretModule();
ShipModuleDefinition CreatePowerCoreModule();
ShipModuleDefinition CreateShieldGeneratorModule();
ShipModuleDefinition CreateCargoModule();
ShipModuleDefinition CreateCrewQuartersModule();
ShipModuleDefinition CreateHyperdriveModule();
ShipModuleDefinition CreateSensorModule();
ShipModuleDefinition CreateMiningModule();
ShipModuleDefinition CreateAntennaModule();
ShipModuleDefinition CreateSmallCockpitModule();
ShipModuleDefinition CreateSmallHullSectionModule();
ShipModuleDefinition CreateSmallEngineModule();
ShipModuleDefinition CreateSmallThrusterModule();
ShipModuleDefinition CreateSmallWingLeftModule();
ShipModuleDefinition CreateSmallWingRightModule();

} // namespace subspace
