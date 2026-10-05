#include <cmath>
#include <iostream>

#include "character/PlayerModelSourceAuthoritySystem.h"
#include "interior/SmallShipInteriorValidationSystem.h"

using namespace subspace;
static int passed=0,failed=0;
#define CHECK(n,e) do{if(e){++passed;std::cout<<"PASS: "<<n<<"\n";}else{++failed;std::cout<<"FAIL: "<<n<<"\n";}}while(0)

static InteriorCarvedVolume Room(std::size_t index,InteriorRoomType room,ExteriorInteriorCapability capability,Vector3 center){
    InteriorCarvedVolume v;v.moduleIndex=index;v.moduleId="r193.room."+std::to_string(index);v.roomType=room;v.capability=capability;
    v.center=center;v.halfExtents={1.20f,1.55f,1.12f};v.walkable=true;v.pressureCapable=true;return v;
}
static InteriorLayoutPlan Layout(std::vector<InteriorCarvedVolume> rooms,std::size_t connected){
    InteriorLayoutPlan p;p.decks=1;p.carve.valid=true;p.carve.volumes=std::move(rooms);p.carve.walkableModuleCount=p.carve.volumes.size();p.carve.connectedWalkableCount=connected;
    p.shell.ready=true;p.shell.surfaces.push_back({});p.rooms=static_cast<int>(p.carve.volumes.size());return p;
}
int main(){
    const auto source=PlayerModelSourceAuthoritySystem::QuaterniusUniversalBaseCharacters();
    CHECK("Quaternius base-character source is explicit CC0",source.provider=="Quaternius"&&source.license=="CC0-1.0"&&source.naturalHeightIsScaleAuthority);
    CHECK("runtime character formats include glTF/GLB",source.acceptedRuntimeFormats.size()>=2&&source.acceptedRuntimeFormats[0]==".gltf"&&source.acceptedRuntimeFormats[1]==".glb");

    PlayerModelRuntimeEvidence missing;const auto blocked=PlayerModelSourceAuthoritySystem::Certify(missing);
    CHECK("missing player payload fails closed",!blocked.ready&&!blocked.errors.empty());

    PlayerModelRuntimeEvidence adult;adult.sourceHydrated=true;adult.skinnedMeshLoaded=true;adult.humanoidRigResolved=true;adult.animationCompatible=true;adult.sourceHeightUnits=1.76f;adult.sourceMetersPerUnit=1.0f;
    const auto certified=PlayerModelSourceAuthoritySystem::Certify(adult);
    CHECK("measured player can become scale authority",certified.ready&&std::fabs(certified.measuredHeightMeters-1.76f)<.0001f);
    CHECK("player is not arbitrarily rescaled to 1.80m",std::fabs(certified.importScaleToWorld-1.0f)<.0001f&&std::fabs(certified.worldScale.referencePlayerHeightMeters-1.76f)<.0001f);
    CHECK("doors and corridors follow measured player",certified.worldScale.referenceDoorHeightMeters<WorldScaleAuthoritySystem::DefaultProfile().referenceDoorHeightMeters&&certified.worldScale.referenceCorridorWidthMeters<WorldScaleAuthoritySystem::DefaultProfile().referenceCorridorWidthMeters);

    auto cutter=Layout({Room(0,InteriorRoomType::Cockpit,ExteriorInteriorCapability::Cockpit,{0,0,1.12f})},1);
    cutter.carve.exclusions.push_back({1,"engine",{0,-2,1},{.5f,.5f,.5f},"engine machinery"});
    const auto cutterReport=SmallShipInteriorValidationSystem::Validate(cutter,SmallShipInteriorTier::Cutter,certified.worldScale);
    CHECK("one-room cutter is the first certified interior rung",cutterReport.certified);

    auto shuttle=Layout({Room(0,InteriorRoomType::Cockpit,ExteriorInteriorCapability::Cockpit,{0,1.6f,1.12f}),Room(1,InteriorRoomType::Airlock,ExteriorInteriorCapability::Airlock,{0,-1.6f,1.12f})},2);
    shuttle.carve.exclusions.push_back({2,"engine",{0,-3,1},{.5f,.5f,.5f},"engine machinery"});
    const auto shuttleReport=SmallShipInteriorValidationSystem::Validate(shuttle,SmallShipInteriorTier::Shuttle,certified.worldScale);
    CHECK("connected cockpit-airlock shuttle certifies",shuttleReport.certified);
    shuttle.carve.connectedWalkableCount=1;
    CHECK("disconnected small-ship cavities fail closed",!SmallShipInteriorValidationSystem::Validate(shuttle,SmallShipInteriorTier::Shuttle,certified.worldScale).certified);

    const auto cutterProfile=SmallShipInteriorValidationSystem::Profile(SmallShipInteriorTier::Cutter);
    const auto shuttleProfile=SmallShipInteriorValidationSystem::Profile(SmallShipInteriorTier::Shuttle);
    const auto corvetteProfile=SmallShipInteriorValidationSystem::Profile(SmallShipInteriorTier::Corvette);
    CHECK("certification ladder grows gradually",cutterProfile.maximumWalkableVolumes<shuttleProfile.maximumWalkableVolumes&&shuttleProfile.maximumWalkableVolumes<corvetteProfile.maximumWalkableVolumes);
    CHECK("R193 keeps early tiers single-deck",cutterProfile.maximumDecks==1&&shuttleProfile.maximumDecks==1&&corvetteProfile.maximumDecks==1);

    std::cout<<"R193 player-scale/small-ship assertions: "<<passed<<" passed / "<<failed<<" failed\n";
    return failed?1:0;
}
