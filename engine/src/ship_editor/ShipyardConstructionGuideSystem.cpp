#include "ship_editor/ShipyardConstructionGuideSystem.h"

#include <cmath>
#include <sstream>

namespace subspace {
std::vector<ShipyardConstructionGuide> ShipyardConstructionGuideSystem::ForMountProfile(const ShipyardMountProfile& profile,const Vector3& origin,bool includePlayerScale){
    std::vector<ShipyardConstructionGuide> out;
    if(profile.primaryRoot.valid){
        out.push_back({ShipyardConstructionGuideKind::MountPlane,"primary-mount","PRIMARY MOUNT / "+profile.primaryRoot.face,origin,profile.primaryRoot.normal,1.2f,true,false});
        out.push_back({ShipyardConstructionGuideKind::SocketNormal,"mount-normal","ATTACHMENT NORMAL",origin,profile.primaryRoot.normal,.75f,true,false});
    }
    out.push_back({ShipyardConstructionGuideKind::Clearance,"clearance","CLEARANCE",origin,{0,1,0},profile.minimumClearanceMeters,true,false});
    if(includePlayerScale)out.push_back({ShipyardConstructionGuideKind::PlayerScale,"player-scale","1.80 m PLAYER",origin,{0,0,1},1.80f,false,false});
    return out;
}
ShipyardConstructionGuide ShipyardConstructionGuideSystem::DistanceGuide(const Vector3& a,const Vector3& b,std::string label){
    const Vector3 d=b-a;const float length=d.length();
    if(label.empty()){std::ostringstream ss;ss.setf(std::ios::fixed);ss.precision(2);ss<<length<<" m";label=ss.str();}
    return {ShipyardConstructionGuideKind::Distance,"distance",std::move(label),a,length>.0001f?d*(1.0f/length):Vector3{0,1,0},length,true,false};
}
}
