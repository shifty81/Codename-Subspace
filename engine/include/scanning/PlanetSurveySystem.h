#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>

namespace subspace {

enum class PlanetSurveyStage { Unknown, Preliminary, Detailed, IndustrialCertified };
struct PlanetResourceProfile { double metals=0; double titanium=0; double rareEarths=0; double volatiles=0; double atmospheric=0; };
struct PlanetSurveyRecord { std::uint64_t planetId=0; std::string name; PlanetSurveyStage stage=PlanetSurveyStage::Unknown; double gravityG=1; double temperatureK=288; double radiation=0; double geologicalActivity=0; double constructionCostMultiplier=1; double miningYieldMultiplier=1; double geothermalPotential=0; PlanetResourceProfile resources; bool elevatorViable=false; double elevatorAnchorScore=0; };
class PlanetSurveySystem {
public:
    PlanetSurveyRecord Generate(std::uint64_t galaxySeed,std::uint64_t planetId,const std::string& name) const;
    void Advance(PlanetSurveyRecord& record,PlanetSurveyStage target) const;
    double IndustrialValue(const PlanetSurveyRecord& record) const;
};

} // namespace subspace
