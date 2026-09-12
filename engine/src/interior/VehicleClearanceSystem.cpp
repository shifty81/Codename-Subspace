#include "interior/VehicleClearanceSystem.h"
namespace subspace {
VehicleAccessValidation VehicleClearanceSystem::Validate(const VehicleClearanceProfile&v,const std::vector<AccessClearanceSample>&r){
    if(r.empty())return {false,0,"NO_ROUTE"};
    for(std::size_t i=0;i<r.size();++i){const auto&s=r[i];if(s.width<v.width)return {false,i,"WIDTH"};if(s.height<v.height)return {false,i,"HEIGHT"};if(s.availableTurnRadius>0&&s.availableTurnRadius<v.turningRadius)return {false,i,"TURN_RADIUS"};if(s.rampAngleDegrees>v.maxRampAngleDegrees)return {false,i,"RAMP_ANGLE"};}
    return {true,static_cast<std::size_t>(-1),"OK"};
}
} // namespace subspace
