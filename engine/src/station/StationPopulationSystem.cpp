#include "station/StationPopulationSystem.h"
#include <algorithm>
namespace subspace {
double StationPopulationSystem::Workforce(const StationPopulationState& s) const {return std::min(s.population*0.6,s.jobs);}
double StationPopulationSystem::ConsumptionRate(const StationPopulationState& s) const {return s.population*0.01;}
void StationPopulationSystem::Advance(StationPopulationState& s,double h) const {if(h<=0)return;double need=ConsumptionRate(s)*h;s.food=std::max(0.0,s.food-need);s.supplies=std::max(0.0,s.supplies-need*0.4);double support=s.population>0?std::min(1.0,s.serviceCapacity/s.population):1.0;bool fed=s.food>0;s.happiness=std::clamp(s.happiness+(fed?0.01:-0.05)*h+(support-0.5)*0.01*h,0.0,1.0);if(s.capacity>s.population&&s.happiness>0.65&&fed)s.population=std::min(s.capacity,s.population*(1.0+0.002*h));if(!fed&&s.population>0)s.population=std::max(0.0,s.population*(1.0-0.005*h));}
}
