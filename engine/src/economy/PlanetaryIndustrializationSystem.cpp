#include "economy/PlanetaryIndustrializationSystem.h"

#include <algorithm>

namespace subspace {

std::vector<ElevatorMaterialRequirement> PlanetaryIndustrializationSystem::ElevatorBill(const PlanetSurveyRecord& s) const {
    const double gravity=std::max(0.45,s.gravityG);const double hazard=1.0+s.radiation*0.35+s.constructionCostMultiplier*0.12;
    return {{"structural_alloy",5200*gravity*hazard},{"tether_composite",2600*gravity},{"power_systems",850*hazard},{"industrial_electronics",620*hazard},{"construction_drones",120*(1.0+s.geologicalActivity*.25)}};
}
PlanetaryInvestmentReport PlanetaryIndustrializationSystem::Evaluate(const PlanetSurveyRecord& s) const {
    PlanetaryInvestmentReport r;PlanetSurveySystem surveySystem;r.industrialValue=surveySystem.IndustrialValue(s);const auto bill=ElevatorBill(s);for(const auto&b:bill)r.estimatedBootstrapCost+=b.required*18.0;r.projectedHourlyOutput=r.industrialValue*420.0;r.worthDeveloping=s.stage>=PlanetSurveyStage::Detailed&&s.elevatorViable&&r.industrialValue>=0.22;
    if(!s.elevatorViable)r.reasons.push_back("space elevator anchor not viable");if(s.resources.rareEarths>.7)r.reasons.push_back("exceptional rare-earth opportunity");if(s.geothermalPotential>.65)r.reasons.push_back("strong geothermal power");if(s.radiation>.72)r.reasons.push_back("high radiation equipment cost");if(r.worthDeveloping)r.reasons.push_back("survey supports industrial return");return r;
}
bool PlanetaryIndustrializationSystem::BindSurvey(PlanetaryIndustrializationProject&p,const PlanetSurveyRecord&s) const {if(s.stage<PlanetSurveyStage::Detailed)return false;p.survey=s;p.colony.planetId=s.planetId;p.stage=PlanetaryDevelopmentStage::Surveyed;return true;}
bool PlanetaryIndustrializationSystem::SelectAnchor(PlanetaryIndustrializationProject&p,double score) const {if(p.stage!=PlanetaryDevelopmentStage::Surveyed||!p.survey.elevatorViable||score<.30)return false;p.anchorScore=std::clamp(score,0.0,1.0);p.stage=PlanetaryDevelopmentStage::SiteSelected;return true;}
bool PlanetaryIndustrializationSystem::DeployTether(PlanetaryIndustrializationProject&p) const {if(p.stage!=PlanetaryDevelopmentStage::SiteSelected)return false;p.colony.tetherSeedDeployed=true;p.stage=PlanetaryDevelopmentStage::TetherSeeded;return true;}
double PlanetaryIndustrializationSystem::Deliver(PlanetaryIndustrializationProject&p,const std::string&commodity,double quantity) const {if((p.stage!=PlanetaryDevelopmentStage::TetherSeeded&&p.stage!=PlanetaryDevelopmentStage::ElevatorConstruction)||quantity<=0)return 0;const auto bill=ElevatorBill(p.survey);auto it=std::find_if(bill.begin(),bill.end(),[&](const auto&x){return x.commodity==commodity;});if(it==bill.end())return 0;double& delivered=p.delivered[commodity];const double accepted=std::min(quantity,std::max(0.0,it->required-delivered));delivered+=accepted;p.stage=PlanetaryDevelopmentStage::ElevatorConstruction;if(ElevatorComplete(p)){p.colony.tetherConstructionProgress=1;p.colony.elevatorOnline=true;p.colony.elevatorThroughputPerHour=250;p.stage=PlanetaryDevelopmentStage::ElevatorOnline;}return accepted;}
bool PlanetaryIndustrializationSystem::ElevatorComplete(const PlanetaryIndustrializationProject&p) const {const auto bill=ElevatorBill(p.survey);for(const auto&b:bill){auto it=p.delivered.find(b.commodity);if(it==p.delivered.end()||it->second+1e-6<b.required)return false;}return !bill.empty();}
bool PlanetaryIndustrializationSystem::UnlockManufacturing(PlanetaryIndustrializationProject&p) const {if(p.stage!=PlanetaryDevelopmentStage::ElevatorOnline||!p.colony.elevatorOnline)return false;p.stage=PlanetaryDevelopmentStage::Manufacturing;return true;}

} // namespace subspace
