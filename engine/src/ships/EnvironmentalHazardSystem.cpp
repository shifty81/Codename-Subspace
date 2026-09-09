#include "ships/EnvironmentalHazardSystem.h"
#include <algorithm>
namespace subspace {
EnvironmentalExposureReport EnvironmentalHazardSystem::Evaluate(const std::vector<EnvironmentalCondition>& c,const EnvironmentalProtection& p) const {EnvironmentalExposureReport r;for(auto& h:c){double resist=0;auto it=p.resistance.find(static_cast<int>(h.type));if(it!=p.resistance.end())resist=std::clamp(it->second,0.0,1.0);double exposed=std::max(0.0,h.intensity*(1.0-resist));r.totalDamagePerHour+=exposed*h.exposurePerHour;if(exposed>0.5)r.dangerous.push_back(h.type);if(h.type==HazardType::IonStorm)r.sensorPenalty=std::max(r.sensorPenalty,std::clamp(exposed,0.0,1.0));if(h.type==HazardType::Gravitic||h.type==HazardType::Debris)r.propulsionPenalty=std::max(r.propulsionPenalty,std::clamp(exposed*0.5,0.0,1.0));}return r;}
}
