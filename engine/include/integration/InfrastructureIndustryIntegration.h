#pragma once

#include "economy/LocalEconomySystem.h"
#include "economy/MarketContractSystem.h"
#include "economy/PlanetaryIndustrializationSystem.h"
#include "economy/PlanetaryManufacturingSystem.h"
#include "scanning/PlanetSurveySystem.h"
#include "ships/ShipConstructionSystem.h"
#include "ships/ShipProductionWorkflowSystem.h"
#include "station/StationBuilderSystem.h"
#include "station/StationPopulationSystem.h"
#include "station/StationServiceSystem.h"
#include "trade_route/LogisticsAutomationSystem.h"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace subspace {

struct ShipSpatialValidation {
    bool valid = false;
    double exteriorVolume = 0.0;
    double reservedMachineryVolume = 0.0;
    double usableInteriorVolume = 0.0;
    std::vector<std::string> errors;
};

struct StationCommissioningResult {
    bool commissioned = false;
    double structuralMaterialConsumed = 0.0;
    StationServiceProfile services;
    std::vector<std::string> roles;
};

struct PlanetaryProductionOperation {
    PlanetSurveyRecord survey;
    PlanetaryIndustrializationProject project;
    std::unordered_map<std::string,double> orbitalInventory;
};

/// Pass276-285 integration authority. Ship design/production, station building
/// and operations, planetary survey/elevator/manufacturing and the local market
/// consume one material/logistics path.
class InfrastructureIndustryIntegration {
public:
    ShipSpatialValidation ValidateShipSpatialContract(const ModularShipDesign& design) const;
    std::uint64_t QueueShip(const ModularShipDesign& design,ShipProductionWorkflowSystem& shipyard,
                            std::unordered_map<int,double>& materialInventory) const;

    StationCommissioningResult CommissionStation(const StationBuildPlan& plan,
                                                  StationBuilderSystem& builder,
                                                  std::unordered_map<std::string,double>& inventory) const;

    bool BeginPlanetaryOperation(std::uint64_t seed,std::uint64_t planetId,const std::string& name,
                                 PlanetSurveySystem& surveySystem,
                                 PlanetaryIndustrializationSystem& industrialization,
                                 PlanetaryProductionOperation& operation) const;
    bool BootstrapSpaceElevator(PlanetaryIndustrializationSystem& industrialization,
                                PlanetaryProductionOperation& operation,
                                std::unordered_map<std::string,double>& inventory) const;
    bool AddBasicManufacturingNetwork(PlanetaryProductionOperation& operation,
                                      PlanetaryManufacturingSystem& manufacturing) const;
    double ExportCommodity(PlanetaryProductionOperation& operation,
                           PlanetaryManufacturingSystem& manufacturing,
                           const std::string& commodity,double amount,
                           LocalEconomyState& market,LocalEconomySystem& economy) const;

    std::vector<SandboxContract> AdvanceStationEconomy(StationPopulationState& population,
                                                       StationPopulationSystem& populationSystem,
                                                       LocalEconomyState& market,
                                                       LocalEconomySystem& economy,
                                                       double hours) const;
};

} // namespace subspace
