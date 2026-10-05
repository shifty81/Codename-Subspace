#include "interior/StarterShipInteriorSceneSystem.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <utility>

namespace subspace {
namespace {
float Area(const InteriorCarvedVolume& v){return v.halfExtents.x*v.halfExtents.y*4.0f;}
float ClampInside(float value,float lo,float hi){return lo<=hi?std::clamp(value,lo,hi):(lo+hi)*0.5f;}
Vector3 ForwardFor(const InteriorAvatarState& a){
    const float yaw=a.lookYawRadians+a.headLookYawOffsetRadians;
    const float pitch=a.lookPitchRadians+a.headLookPitchOffsetRadians;
    const float cp=std::cos(pitch),sp=std::sin(pitch),sy=std::sin(yaw),cy=std::cos(yaw);
    Vector3 f{-sy*cp,cy*cp,sp};const float len=f.length();return len>1.0e-6f?f*(1.0f/len):Vector3{0,1,0};
}
StarterInteriorFixture Fixture(std::uint64_t id,InteriorFixtureKind kind,const char* label,
                               Vector3 center,Vector3 half,Vector3 useFeet,bool blocks=true){
    StarterInteriorFixture f;f.interaction.fixtureId=id;f.interaction.kind=kind;f.interaction.powered=true;
    f.interaction.accessAllowed=true;f.interaction.interactionRangeMeters=2.25f;f.label=label;
    f.localCenter=center;f.halfExtents=half;f.useFeet=useFeet;f.blocksMovement=blocks;return f;
}
bool Overlaps(const StarterInteriorFixture& f,Vector3 feet,float radius){
    if(!f.blocksMovement)return false;
    const float minX=f.localCenter.x-f.halfExtents.x-radius,maxX=f.localCenter.x+f.halfExtents.x+radius;
    const float minY=f.localCenter.y-f.halfExtents.y-radius,maxY=f.localCenter.y+f.halfExtents.y+radius;
    return feet.x>minX&&feet.x<maxX&&feet.y>minY&&feet.y<maxY;
}
}

StarterInteriorScene StarterShipInteriorSceneSystem::Build(const InteriorLayoutPlan& layout){
    StarterInteriorScene out;
    if(!layout.carve.valid||!layout.shell.ready||layout.carve.volumes.empty()){
        out.warnings.push_back("starter interior requires a certified carved shell");return out;
    }
    const InteriorCarvedVolume* room=nullptr;
    for(const auto& v:layout.carve.volumes)if(v.walkable&&(!room||Area(v)>Area(*room)))room=&v;
    if(!room){out.warnings.push_back("starter interior has no walkable volume");return out;}

    const float floor=room->center.z-room->halfExtents.z;
    out.boundsMin={room->center.x-room->halfExtents.x,room->center.y-room->halfExtents.y,floor};
    out.boundsMax={room->center.x+room->halfExtents.x,room->center.y+room->halfExtents.y,room->center.z+room->halfExtents.z};
    const float spanX=out.boundsMax.x-out.boundsMin.x,spanY=out.boundsMax.y-out.boundsMin.y;
    if(spanX<1.8f||spanY<2.6f){out.warnings.push_back("starter interior fixture bay is compact; layout compressed to certified cavity");}

    const float marginX=std::min(.52f,std::max(.22f,spanX*.16f));
    const float marginY=std::min(.58f,std::max(.24f,spanY*.13f));
    const float minX=out.boundsMin.x+marginX,maxX=out.boundsMax.x-marginX;
    const float minY=out.boundsMin.y+marginY,maxY=out.boundsMax.y-marginY;
    const float cx=(out.boundsMin.x+out.boundsMax.x)*.5f,cy=(out.boundsMin.y+out.boundsMax.y)*.5f;
    const float side=std::max(.18f,std::min(.38f,spanX*.11f));
    const float frontY=ClampInside(out.boundsMax.y-.48f,minY,maxY);
    const float rearY=ClampInside(out.boundsMin.y+.58f,minY,maxY);
    const float midFront=ClampInside(cy+spanY*.12f,minY,maxY);
    const float leftX=ClampInside(out.boundsMin.x+.50f,minX,maxX);
    const float rightX=ClampInside(out.boundsMax.x-.50f,minX,maxX);

    // Forward (+Y) is the authored/default FPS facing. The helm is therefore
    // immediately legible at spawn, with support stations distributed around
    // the cabin perimeter rather than stacked in the player's path.
    const Vector3 helmCenter{cx,frontY,floor+.48f};
    const Vector3 helmUse{cx,ClampInside(frontY-.60f,minY,maxY),floor};
    out.fixtures.push_back(Fixture(1001,InteriorFixtureKind::HelmSeat,"HELM",helmCenter,{.34f,.30f,.48f},helmUse));
    auto fleet=Fixture(1002,InteriorFixtureKind::FleetCommandTerminal,"FLEET COMMAND",
                       {leftX,midFront,floor+.62f},{side,.20f,.62f},{ClampInside(leftX+.42f,minX,maxX),midFront,floor});
    fleet.interaction.linkedSystemId="fleet_command";out.fixtures.push_back(std::move(fleet));
    auto cargo=Fixture(1003,InteriorFixtureKind::CargoTerminal,"CARGO",
                       {rightX,rearY,floor+.48f},{side,.32f,.48f},{ClampInside(rightX-.42f,minX,maxX),rearY,floor});
    cargo.interaction.linkedSystemId="cargo";out.fixtures.push_back(std::move(cargo));
    auto engineering=Fixture(1004,InteriorFixtureKind::EngineeringPanel,"ENGINEERING",
                       {leftX,rearY,floor+.68f},{side,.24f,.68f},{ClampInside(leftX+.42f,minX,maxX),rearY,floor});
    engineering.interaction.linkedSystemId="engineering";out.fixtures.push_back(std::move(engineering));
    auto airlock=Fixture(1005,InteriorFixtureKind::Airlock,"AIRLOCK",
                       {cx,out.boundsMin.y+.09f,floor+.88f},{std::min(.62f,spanX*.22f),.08f,.88f},
                       {cx,ClampInside(out.boundsMin.y+.72f,minY,maxY),floor},false);
    airlock.interaction.linkedSystemId="airlock";airlock.interaction.destinationId="exterior";out.fixtures.push_back(std::move(airlock));

    out.spawnFeet={cx,ClampInside(cy-.10f,minY,maxY),floor};
    // If the exact center is occupied by a compressed fixture layout, bias
    // slightly aft while staying inside the same certified volume.
    for(const auto& f:out.fixtures)if(Overlaps(f,out.spawnFeet,.32f)){out.spawnFeet.y=ClampInside(cy-.55f,minY,maxY);break;}
    out.ready=true;return out;
}

StarterInteriorFocus StarterShipInteriorSceneSystem::Focus(const StarterInteriorScene& scene,
        const InteriorAvatarState& avatar,float maxDistance,float minAim){
    StarterInteriorFocus best;if(!scene.ready)return best;
    const Vector3 eye=avatar.localPosition+Vector3{0,0,avatar.eyeHeightMeters};
    const Vector3 forward=ForwardFor(avatar);
    float bestRank=-std::numeric_limits<float>::infinity();
    for(std::size_t i=0;i<scene.fixtures.size();++i){
        const auto& f=scene.fixtures[i];Vector3 target=f.localCenter;target.z=std::min(f.localCenter.z+f.halfExtents.z*.35f,scene.boundsMax.z-.18f);
        Vector3 delta=target-eye;const float dist=delta.length();if(dist<=.01f||dist>maxDistance)continue;
        const Vector3 dir=delta*(1.0f/dist);const float aim=dir.x*forward.x+dir.y*forward.y+dir.z*forward.z;if(aim<minAim)continue;
        const float rank=aim*2.0f-dist*.18f;if(rank<=bestRank)continue;
        bestRank=rank;best.fixtureIndex=static_cast<int>(i);best.distanceMeters=dist;best.aimScore=aim;
    }
    return best;
}

Vector3 StarterShipInteriorSceneSystem::ResolveFixtureCollision(const StarterInteriorScene& scene,
        Vector3 from,Vector3 to,float radius){
    if(!scene.ready)return to;
    bool blocked=false;for(const auto& f:scene.fixtures)if(Overlaps(f,to,radius)){blocked=true;break;}
    if(!blocked)return to;
    Vector3 x{to.x,from.y,to.z};bool xBlocked=false;for(const auto& f:scene.fixtures)if(Overlaps(f,x,radius)){xBlocked=true;break;}
    if(!xBlocked)return x;
    Vector3 y{from.x,to.y,to.z};bool yBlocked=false;for(const auto& f:scene.fixtures)if(Overlaps(f,y,radius)){yBlocked=true;break;}
    return yBlocked?from:y;
}

} // namespace subspace
