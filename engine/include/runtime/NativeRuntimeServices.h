#pragma once

#include "cargo/CargoSystem.h"
#include "combat/AdvancedCombatSystem.h"
#include "combat/ShipFailureSystem.h"
#include "ships/EngineeringRepairSystem.h"
#include "combat/ElectronicWarfareSystem.h"
#include "fleet/DroneOperationsSystem.h"
#include "fleet/FleetDoctrineSystem.h"
#include "factions/DynamicFactionSystem.h"
#include "runtime/SectorSimulationSystem.h"
#include "scanning/DeepExplorationSystem.h"
#include "navigation/AnomalousSpaceSystem.h"
#include "ships/EnvironmentalHazardSystem.h"
#include "economy/ResourceMaterialSystem.h"
#include "trade_route/LogisticsAutomationSystem.h"
#include "station/StationPopulationSystem.h"
#include "station/OrbitalInfrastructureSystem.h"
#include "ships/CapitalConstructionSystem.h"
#include "fleet/CarrierOperationsSystem.h"
#include "fleet/CorporationProgressionSystem.h"
#include "runtime/PersistentUniverseSystem.h"
#include "runtime/SandboxAcceptanceIISystem.h"
#include "integration/CombatShipIntegration.h"
#include "integration/NavigationMiningIntegration.h"
#include "integration/InfrastructureIndustryIntegration.h"
#include "integration/LivingUniverseIntegration.h"
#include "core/persistence/CampaignPersistenceSystem.h"
#include "debug_tools/DeveloperConsole.h"
#include "economy/EconomySimulationSystem.h"
#include "factions/FactionSecuritySystem.h"
#include "fleet/FleetMissionSystem.h"
#include "interior/ShipInteriorSystem.h"
#include "procedural/SystemTopologySystem.h"
#include "station/StationServiceSystem.h"
#include "ui/GameUIState.h"

namespace subspace {

/// Native service container replacing the final legacy C# runtime authorities.
/// It is owned by Engine and has no CLR/NuGet dependency.
class NativeRuntimeServices {
public:
    NativeRuntimeServices();
    void InitializeDefaults();
    void Update(double deltaSeconds);

    CargoHold playerCargo{25000.0, 12000.0};
    EconomySimulationSystem economy;
    StationServiceSystem stationServices;
    FactionSecuritySystem factionSecurity;
    FleetMissionSystemNative fleetMissions;
    ShipInteriorSystem interiors;
    CampaignPersistenceSystem campaignPersistence;
    GameUIState uiState;
    SystemTopologySystem topology;
    DeveloperConsole developerConsole;

    // Pass246-265 deep-sandbox authorities. These are owned by the native
    // runtime container so gameplay/UI layers reference one canonical state.
    AdvancedCombatSystem advancedCombat;
    ShipFailureSystem shipFailures;
    EngineeringRepairSystem engineeringRepair;
    ElectronicWarfareSystem electronicWarfare;
    DroneOperationsSystem droneOperations{8};
    FleetDoctrineSystem fleetDoctrine;
    DynamicFactionSystem dynamicFactions;
    SectorSimulationSystem sectorSimulation;
    DeepExplorationSystem deepExploration;
    AnomalousSpaceSystem anomalousSpace;
    EnvironmentalHazardSystem environmentalHazards;
    ResourceMaterialSystem resourceMaterials;
    LogisticsAutomationSystem logisticsAutomation;
    StationPopulationSystem stationPopulation;
    OrbitalInfrastructureSystem orbitalInfrastructure;
    CapitalConstructionSystem capitalConstruction;
    CarrierOperationsSystem carrierOperations{8};
    CorporationProgressionSystem corporationProgression;
    PersistentUniverseSystem persistentUniverse;
    SandboxAcceptanceIISystem sandboxAcceptanceII;

    // Pass266-295 normalized production integration authorities.
    CombatShipIntegration combatShipIntegration;
    NavigationMiningIntegration navigationMiningIntegration;
    InfrastructureIndustryIntegration infrastructureIndustryIntegration;
    LivingUniverseIntegration livingUniverseIntegration;
    SandboxAcceptanceIIISystem sandboxAcceptanceIII;

private:
    bool initialized_ = false;
};

} // namespace subspace
