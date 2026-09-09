#pragma once

#include "economy/LocalEconomySystem.h"
#include "economy/MarketContractSystem.h"
#include "factions/DynamicFactionSystem.h"
#include "fleet/CorporationOperationsSystem.h"
#include "fleet/CorporationProgressionSystem.h"
#include "fleet/CorporationSystem.h"
#include "fleet/FleetDoctrineSystem.h"
#include "navigation/AnomalousSpaceSystem.h"
#include "navigation/SystemNavigationSystem.h"
#include "runtime/PersistentUniverseSystem.h"
#include "runtime/SectorSimulationSystem.h"
#include "scanning/DeepExplorationSystem.h"
#include "core/persistence/CampaignPersistenceSystem.h"

#include <cstdint>
#include <string>
#include <vector>

namespace subspace {

struct SectorIntegrationReport {
    int events = 0;
    int generatedContracts = 0;
    int persistentEvents = 0;
    double marketPressure = 0.0;
};

struct CorporationMilestoneReport {
    int level = 1;
    double payroll = 0.0;
    double logisticsReadiness = 0.0;
    bool doctrineReady = false;
    std::vector<std::string> unlocks;
};

struct RecoveryPolicy {
    double insuranceCoverage = 0.65;
    double wreckRecoveryFraction = 0.35;
    double rescueFee = 1000.0;
};

struct RecoveryResult {
    double insurancePayout = 0.0;
    double recoverableValue = 0.0;
    double netLoss = 0.0;
};

/// Pass286-295 integration authority. Fleet/corporation progression, NPC
/// faction activity, market/contracts, exploration/anomalous routes and
/// persistent loss/recovery are wired into one living-universe path.
class LivingUniverseIntegration {
public:
    CorporationMilestoneReport EvaluateCorporation(CorporationSystem& members,
                                                    CorporationOperationsSystem& operations,
                                                    CorporationProgressionSystem& progression,
                                                    CorporationProgressionState& state,
                                                    const CorporationAssetLedger& assets,
                                                    const FleetDoctrineSystem& doctrines,
                                                    const FleetDoctrine& doctrine,
                                                    const std::vector<FleetMember>& fleet) const;

    SectorIntegrationReport AdvanceSector(DynamicSectorState& sector,double hours,
                                          SectorSimulationSystem& simulation,
                                          DynamicFactionSystem& factions,const std::string& factionId,
                                          LocalEconomyState& market,LocalEconomySystem& economy,
                                          MarketContractSystem& contracts,
                                          PersistentUniverseSystem& persistence) const;

    bool PublishDiscovery(const DeepExplorationSite& site,std::uint64_t destinationId,
                          const AstronomicalPosition& position,SystemNavigationSystem& navigation,
                          PersistentUniverseSystem& persistence,const std::string& regionId) const;

    bool PublishTear(const AnomalousSpacePocket& pocket,const AstronomicalPosition& position,
                     SystemNavigationSystem& navigation) const;

    RecoveryResult RecordLoss(const std::string& region,const std::string& shipId,double insuredValue,
                              const RecoveryPolicy& policy,PersistentUniverseSystem& persistence) const;

    void EmbedUniverseLedger(CampaignState& campaign,const PersistentUniverseSystem& persistence) const;
    bool RestoreUniverseLedger(const CampaignState& campaign,PersistentUniverseSystem& persistence) const;
};

struct SandboxAcceptanceIIIState {
    bool physicalCombat=false;bool physicalDamageRepair=false;bool dronesSensorsEwar=false;
    bool systemMapVector=false;bool astronomicalScale=false;bool massiveMiningRegions=false;
    bool shipBuilderProduction=false;bool stationOperations=false;bool planetarySurveyElevator=false;
    bool planetaryManufacturing=false;bool causalEconomy=false;bool explorationRoutes=false;
    bool fleetCrewCorporation=false;bool dynamicFactions=false;bool sectorCrises=false;
    bool anomalousTravel=false;bool persistentRecovery=false;bool saveLedger=false;
};
struct SandboxAcceptanceIIIReport {bool passed=false;int satisfied=0;int required=18;std::vector<std::string> missing;};
class SandboxAcceptanceIIISystem {public:SandboxAcceptanceIIIReport Evaluate(const SandboxAcceptanceIIIState& state) const;};

} // namespace subspace
