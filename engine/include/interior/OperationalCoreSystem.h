#pragma once
#include "content/UniversalKitbashAuthority.h"
#include <string>
#include <vector>
namespace subspace {
enum class OperationalStationRole {
    Helm,Navigation,Captain,Tactical,Weapons,Engineering,DamageControl,Sensors,Communications,FleetCommand,
    FlightControl,MiningControl,IndustryControl,PowerControl,LifeSupport,Logistics,Security,
    VehicleDriver,VehicleGunner,VehicleCommander,InstallationControl,StationTraffic
};
struct OperationalStationRequirement {
    OperationalStationRole role=OperationalStationRole::Helm;
    int minimumCrew=1,maximumCrew=1;
    float powerDemand=0.0f,cpuDemand=0.0f,commandBandwidth=0.0f;
    bool seated=true,standing=false,remoteCapable=false;
};
class OperationalCoreSystem {
public:
    static std::vector<OperationalStationRequirement> RequiredFor(ConstructionDomain domain,const std::string& role,bool capitalScale=false);
};
} // namespace subspace
