#include "scanning/PlanetSurveySystem.h"

#include <algorithm>
#include <random>
namespace subspace {
PlanetSurveyRecord PlanetSurveySystem::Generate(std::uint64_t seed,std::uint64_t id,const std::string&name)const{std::mt19937_64 r(seed^(id*0x9E3779B185EBCA87ULL));std::uniform_real_distribution<double>u(0,1);PlanetSurveyRecord p;p.planetId=id;p.name=name;p.gravityG=.3+u(r)*2.1;p.temperatureK=90+u(r)*850;p.radiation=u(r);p.geologicalActivity=u(r);p.constructionCostMultiplier=.75+p.gravityG*.25+p.radiation*.3;p.miningYieldMultiplier=.7+u(r)*.8;p.geothermalPotential=p.geologicalActivity*(p.temperatureK>350?1.3:1.0);p.resources={u(r),u(r),u(r),u(r),u(r)};p.elevatorAnchorScore=std::clamp(1.2-std::abs(p.gravityG-1.0)*.4-p.radiation*.2,0.0,1.0);p.elevatorViable=p.elevatorAnchorScore>.32;return p;}
void PlanetSurveySystem::Advance(PlanetSurveyRecord&r,PlanetSurveyStage t)const{if(static_cast<int>(t)>static_cast<int>(r.stage))r.stage=t;}
double PlanetSurveySystem::IndustrialValue(const PlanetSurveyRecord&r)const{if(static_cast<int>(r.stage)<static_cast<int>(PlanetSurveyStage::Detailed))return 0;const double resources=(r.resources.metals+r.resources.titanium*1.3+r.resources.rareEarths*1.8+r.resources.volatiles+r.resources.atmospheric*.8)/5.9;return resources*r.miningYieldMultiplier*(.7+r.geothermalPotential*.3)/std::max(.5,r.constructionCostMultiplier);}
} // namespace subspace
