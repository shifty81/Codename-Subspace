#include "interior/OperationalCoreSystem.h"
#include <algorithm>
namespace subspace {
std::vector<OperationalStationRequirement> OperationalCoreSystem::RequiredFor(ConstructionDomain d,const std::string&r,bool capital){
    std::vector<OperationalStationRequirement> o;
    if(d==ConstructionDomain::Ship){
        o.push_back({OperationalStationRole::Helm,1,1,1,1,1,true,false,false});
        o.push_back({OperationalStationRole::Navigation,1,1,1,1,1,true,false,true});
        o.push_back({OperationalStationRole::Engineering,1,2,2,2,1,true,true,true});
        if(capital){o.push_back({OperationalStationRole::Captain,1,1,1,1,3,true,true,true});o.push_back({OperationalStationRole::Tactical,1,3,2,2,2,true,false,true});}
        if(r.find("COMMAND")!=std::string::npos){o.push_back({OperationalStationRole::FleetCommand,1,4,3,4,6,true,true,true});o.push_back({OperationalStationRole::Communications,1,2,2,3,3,true,false,true});}
        if(r.find("MIN")!=std::string::npos)o.push_back({OperationalStationRole::MiningControl,1,2,2,2,1,true,true,true});
        if(r.find("CARRIER")!=std::string::npos)o.push_back({OperationalStationRole::FlightControl,1,4,3,3,2,true,true,true});
    }else if(d==ConstructionDomain::Station){
        o={{OperationalStationRole::StationTraffic,1,4,2,3,2,true,true,true},{OperationalStationRole::PowerControl,1,3,2,2,1,true,true,true},{OperationalStationRole::Logistics,1,3,2,2,1,true,true,true},{OperationalStationRole::Security,1,3,2,2,1,true,true,true}};
    }else if(d==ConstructionDomain::Planetary){
        o={{OperationalStationRole::InstallationControl,1,3,2,2,1,true,true,true},{OperationalStationRole::PowerControl,1,2,1,1,0,true,true,true},{OperationalStationRole::Logistics,1,2,1,1,0,true,true,true}};
    }else if(d==ConstructionDomain::Vehicle){
        o={{OperationalStationRole::VehicleDriver,1,1,1,1,0,true,false,false}};
        if(r.find("COMBAT")!=std::string::npos)o.push_back({OperationalStationRole::VehicleGunner,1,1,1,1,0,true,false,false});
    }
    return o;
}
} // namespace subspace
