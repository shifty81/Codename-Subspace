#include "runtime/SimulationLodSystem.h"

#include <algorithm>
namespace subspace {
SimulationLod SimulationLodSystem::Select(const SimulationSubject&s)const{if(s.playerPresent||s.distanceMeters<250000)return SimulationLod::Full;if(s.important||s.distanceMeters<50000000)return SimulationLod::Regional;if(s.distanceMeters<5e11)return SimulationLod::Aggregate;return SimulationLod::Dormant;}
void SimulationLodSystem::AdvanceEconomy(NpcEconomicAgent&a,NpcEconomicAction action,double h)const{h=std::max(0.0,h);switch(action){case NpcEconomicAction::Mine:a.inventory["ore"]+=25*a.productionCapacity*h;break;case NpcEconomicAction::Refine:{double q=std::min(a.inventory["ore"],18*a.productionCapacity*h);a.inventory["ore"]-=q;a.inventory["alloy"]+=q*.7;break;}case NpcEconomicAction::Manufacture:{double q=std::min(a.inventory["alloy"],8*a.productionCapacity*h);a.inventory["alloy"]-=q;a.inventory["components"]+=q*.5;break;}case NpcEconomicAction::Haul:a.credits+=40*a.logisticsCapacity*h;break;case NpcEconomicAction::Consume:a.inventory["supplies"]=std::max(0.0,a.inventory["supplies"]-3*h);break;case NpcEconomicAction::BuildShip:if(a.inventory["components"]>=10){a.inventory["components"]-=10;++a.shipsBuilt;}break;case NpcEconomicAction::ReplaceLosses:while(a.shipsLost>0&&a.inventory["components"]>=10){a.inventory["components"]-=10;--a.shipsLost;++a.shipsBuilt;}break;}}
} // namespace subspace
