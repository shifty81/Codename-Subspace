#pragma once

#include "economy/PlanetaryManufacturingSystem.h"
#include "scanning/PlanetSurveySystem.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace subspace {

enum class PlanetaryDevelopmentStage { Unsurveyed, Surveyed, SiteSelected, TetherSeeded, ElevatorConstruction, ElevatorOnline, Manufacturing };

struct ElevatorMaterialRequirement {
    std::string commodity;
    double required = 0.0;
};

struct PlanetaryIndustrializationProject {
    PlanetaryDevelopmentStage stage = PlanetaryDevelopmentStage::Unsurveyed;
    PlanetSurveyRecord survey;
    PlanetaryManufacturingColony colony;
    double anchorScore = 0.0;
    std::unordered_map<std::string,double> delivered;
};

struct PlanetaryInvestmentReport {
    bool worthDeveloping = false;
    double industrialValue = 0.0;
    double estimatedBootstrapCost = 0.0;
    double projectedHourlyOutput = 0.0;
    std::vector<std::string> reasons;
};

class PlanetaryIndustrializationSystem {
public:
    std::vector<ElevatorMaterialRequirement> ElevatorBill(const PlanetSurveyRecord& survey) const;
    PlanetaryInvestmentReport Evaluate(const PlanetSurveyRecord& survey) const;
    bool BindSurvey(PlanetaryIndustrializationProject& project,const PlanetSurveyRecord& survey) const;
    bool SelectAnchor(PlanetaryIndustrializationProject& project,double anchorScore) const;
    bool DeployTether(PlanetaryIndustrializationProject& project) const;
    double Deliver(PlanetaryIndustrializationProject& project,const std::string& commodity,double quantity) const;
    bool ElevatorComplete(const PlanetaryIndustrializationProject& project) const;
    bool UnlockManufacturing(PlanetaryIndustrializationProject& project) const;
};

} // namespace subspace
