#pragma once
#include <vector>
namespace subspace {
struct VehicleClearanceProfile {float width=2.0f,height=2.0f,length=4.0f,turningRadius=4.0f,groundClearance=.25f,maxRampAngleDegrees=18.0f;};
struct AccessClearanceSample {float width=0.0f,height=0.0f,availableTurnRadius=0.0f,rampAngleDegrees=0.0f;};
struct VehicleAccessValidation {bool valid=false;std::size_t failedSample=static_cast<std::size_t>(-1);const char* reason="";};
class VehicleClearanceSystem {
public: static VehicleAccessValidation Validate(const VehicleClearanceProfile& vehicle,const std::vector<AccessClearanceSample>& route);
};
} // namespace subspace
