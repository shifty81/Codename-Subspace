#include "economy/PlanetaryManufacturingSystem.h"

#include <algorithm>
namespace subspace {
bool PlanetaryManufacturingSystem::DeployTetherSeed(PlanetaryManufacturingColony&c,bool certified)const{if(!certified||c.tetherSeedDeployed)return false;c.tetherSeedDeployed=true;return true;}
double PlanetaryManufacturingSystem::DeliverElevatorMaterials(PlanetaryManufacturingColony&c,double units)const{if(!c.tetherSeedDeployed)return c.tetherConstructionProgress;c.tetherConstructionProgress=std::clamp(c.tetherConstructionProgress+std::max(0.0,units)/1000.0,0.0,1.0);if(c.tetherConstructionProgress>=1){c.elevatorOnline=true;if(c.elevatorThroughputPerHour<=0)c.elevatorThroughputPerHour=200;}return c.tetherConstructionProgress;}
bool PlanetaryManufacturingSystem::AddFacility(PlanetaryManufacturingColony&c,const PlanetFacility&f)const{if(!c.elevatorOnline||f.id==0)return false;for(const auto&existing:c.facilities)if(existing.id==f.id)return false;c.facilities.push_back(f);return true;}
bool PlanetaryManufacturingSystem::AddRoute(PlanetaryManufacturingColony&c,const PlanetLogisticsRoute&r)const{if(!c.elevatorOnline||r.from==0||r.to==0||r.from==r.to||r.capacityPerHour<=0||r.commodity.empty())return false;const auto has=[&](std::uint64_t id){return std::any_of(c.facilities.begin(),c.facilities.end(),[&](const auto&f){return f.id==id;});};if(!has(r.from)||!has(r.to))return false;c.routes.push_back(r);return true;}
double PlanetaryManufacturingSystem::RouteThroughput(const PlanetaryManufacturingColony&c,const std::string&commodity)const{double total=0;for(const auto&r:c.routes)if(r.commodity==commodity)total+=r.capacityPerHour;return total;}
double PlanetaryManufacturingSystem::ExportToOrbit(PlanetaryManufacturingColony&c,const std::string&commodity,double requested)const{if(!c.elevatorOnline||requested<=0)return 0;auto& stock=c.inventory[commodity];double amount=std::min({stock,requested,c.elevatorThroughputPerHour});stock-=amount;return amount;}
} // namespace subspace
