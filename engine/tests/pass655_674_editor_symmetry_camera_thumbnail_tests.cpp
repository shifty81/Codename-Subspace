#include "editor/ConstructionSymmetrySystem.h"
#include "editor/ConstructionEditorCameraSystem.h"
#include "editor/EditorAssetThumbnailSystem.h"
#include "editor/EditorAssetBrowser.h"
#include "editor/ProjectWideEditorNormalizationSystem.h"
#include "ships/PropulsionRoleSystem.h"

#include <cmath>
#include <iostream>
#include <string>
#include <vector>

using namespace subspace;

namespace {
int passed=0,failed=0;
void Check(bool condition,const std::string& name){if(condition){++passed;std::cout<<"[PASS] "<<name<<"\n";}else{++failed;std::cout<<"[FAIL] "<<name<<"\n";}}
bool Near(float a,float b,float e=.001f){return std::fabs(a-b)<=e;}
bool NearV(const Vector3&a,const Vector3&b,float e=.001f){return Near(a.x,b.x,e)&&Near(a.y,b.y,e)&&Near(a.z,b.z,e);}
ShipyardModuleRecord EngineRecord(){
    ShipyardModuleRecord r;r.source.moduleId="test_engine";r.moduleClass=ShipyardModuleClass::Propulsion;r.semantic=ShipyardModuleSemantic::MainEngine;r.size=ShipyardModuleSize::M;r.generatorEligible=true;r.mountFaceConfidence=.95f;r.preferredMountFace="AFT";r.mirrorPreferred=true;r.sockets.push_back({"engine_mount","ENGINE",0,1,0,0,1,0,.05f});return r;
}
}

int main(){
    ConstructionSymmetryFrame f;f.origin={2,3,4};f.axis=ConstructionSymmetryAxis::PortStarboard;
    Check(NearV(ConstructionSymmetrySystem::ReflectPoint({5,7,9},f),{-1,7,9}),"655 editable port/starboard plane reflects around non-zero origin");
    f.axis=ConstructionSymmetryAxis::ForeAft;Check(NearV(ConstructionSymmetrySystem::ReflectPoint({5,7,9},f),{5,-1,9}),"658 fore/aft plane reflection");
    f.axis=ConstructionSymmetryAxis::DorsalVentral;Check(NearV(ConstructionSymmetrySystem::ReflectPoint({5,7,9},f),{5,7,-1}),"659 dorsal/ventral plane reflection");

    VisualModulePlacement p;p.moduleId="pipes";p.x=5;p.y=7;p.z=9;p.yawDegrees=25;p.pitchDegrees=-12;p.rollDegrees=18;p.mirrorX=false;
    f.axis=ConstructionSymmetryAxis::PortStarboard;auto mx=ConstructionSymmetrySystem::ReflectPlacement(p,f);
    Check(mx.mirrorX&&Near(mx.x,-1),"657 mirror copy changes geometric handedness instead of duplicate rotation");
    auto round=ConstructionSymmetrySystem::ReflectPlacement(mx,f);
    Check(!round.mirrorX&&Near(round.x,p.x)&&Near(round.yawDegrees,p.yawDegrees)&&Near(round.rollDegrees,p.rollDegrees),"657 exact reflection is involutive");

    ShipyardAssemblySocket socket{"port_engine","ENGINE",2,1,.5f,1,0,0,.1f,0,0,1,false};
    auto rs=ConstructionSymmetrySystem::ReflectSocket(socket,ConstructionSymmetryAxis::PortStarboard);
    Check(Near(rs.x,-2)&&Near(rs.dirX,-1)&&rs.name.find("starboard")!=std::string::npos,"660 socket origin/frame/name reflect with geometry");
    Check(ConstructionSymmetrySystem::ValidatePair(p,mx,f).valid,"664 linked pair validation accepts exact reflection");

    auto engine=EngineRecord();VisualModulePlacement e;e.moduleId=engine.source.moduleId;e.mirrorX=true;
    const Vector3 reflected=PropulsionRoleSystem::TransformDirection(e,{1,0,0});
    Check(reflected.x<-.99f,"661 propulsion direction consumes reflected module handedness");

    ConstructionEditorCameraState camera;ConstructionEditorCameraSystem::Reset(camera,{10,20,5},8.0f);
    Check(NearV(ConstructionEditorCameraSystem::Target(camera),{10,20,5}),"666 centered inspection target is stable assembly center");
    const Vector3 orbitEye=camera.eye;ConstructionEditorCameraSystem::Orbit(camera,25,-10);
    Check((camera.eye-orbitEye).length()>.1f&&NearV(ConstructionEditorCameraSystem::Target(camera),{10,20,5}),"667 RMB-style orbit moves camera but keeps assembly centered");
    const Vector3 panEye=camera.eye;ConstructionEditorCameraSystem::TruckPedestal(camera,1.5f,-.75f);
    Check((camera.eye-panEye).length()>.1f&&NearV(ConstructionEditorCameraSystem::Target(camera),{10,20,5}),"668 MMB truck/pedestal moves camera while preserving target");
    ConstructionEditorCameraSystem::BeginFreeFly(camera);const Vector3 freeEye=camera.eye;ConstructionEditorCameraSystem::MoveFree(camera,1,1,.5f,.5f);
    Check(camera.mode==ConstructionCameraMode::FreeFly&&(camera.eye-freeEye).length()>.1f,"669 Alt+WASD free-fly translation authority");
    const Vector3 forwardBefore=camera.forward;ConstructionEditorCameraSystem::Look(camera,12,-7);ConstructionEditorCameraSystem::Roll(camera,15);
    Check((camera.forward-forwardBefore).length()>.01f&&Near(camera.rollDegrees,15),"670 FPS look and Q/E roll authority");
    const float speed=camera.moveSpeed;ConstructionEditorCameraSystem::AdjustSpeed(camera,3);Check(camera.moveSpeed>speed,"671 adaptive free-camera speed adjustment");
    ConstructionEditorCameraSystem::EndFreeFly(camera);Check(NearV(ConstructionEditorCameraSystem::Target(camera),camera.assemblyCenter),"671 releasing free camera reacquires assembly center");
    const float dist=camera.orbitDistance;ConstructionEditorCameraSystem::Dolly(camera,2);Check(camera.orbitDistance<dist,"671 inspection wheel dolly changes camera distance");

    const auto thumb=EditorAssetThumbnailSystem::Build(engine,4,3);
    Check(thumb.viewPreset==EditorThumbnailViewPreset::Axial&&thumb.propulsion,"672 propulsion thumbnails use normalized functional preview metadata");
    Check(thumb.sizeBadge=="M"&&!thumb.cacheKey.empty(),"672 thumbnail cache key carries size/material/recipe revision");
    Check(!thumb.thrustLabel.empty()&&!thumb.exhaustLabel.empty(),"673 propulsion thumbnail publishes thrust/exhaust overlays");
    auto manual=engine;manual.generatorEligible=false;const auto manualThumb=EditorAssetThumbnailSystem::Build(manual);
    Check(manualThumb.reviewState==EditorThumbnailReviewState::ManualOnly,"673 thumbnail certification badge distinguishes manual-only content");

    EditorAssetBrowserModel browser;browser.SetAssets({
        {"a","Alpha","Hull",{},"a","","PCG",true,""},
        {"b","Beta","Hull",{"review"},"b","","REVIEW",true,""}
    });browser.SetFavorite("a",true);browser.MarkRecent("b");
    EditorAssetBrowserFilter ff;ff.favoritesOnly=true;Check(browser.Filtered(ff).size()==1&&browser.Filtered(ff)[0].assetId=="a","674 favorites filtering remains normalized");
    ff={};ff.recentOnly=true;Check(browser.Filtered(ff).size()==1&&browser.Filtered(ff)[0].assetId=="b","674 recent filtering");
    ff={};ff.reviewOnly=true;Check(browser.Filtered(ff).size()==1&&browser.Filtered(ff)[0].assetId=="b","674 review filtering");

    const auto audit=ProjectWideEditorNormalizationSystem::Audit();
    bool sym=false,cam=false,thumbAuth=false;for(const auto& x:audit.entries){sym|=x.domain==EditorNormalizationDomain::ConstructionSymmetry&&x.normalized;cam|=x.domain==EditorNormalizationDomain::ConstructionCamera&&x.normalized;thumbAuth|=x.domain==EditorNormalizationDomain::ThumbnailAuthority&&x.normalized;}
    Check(sym&&cam&&thumbAuth&&audit.runtimeNormalized,"655-674 project-wide normalization names symmetry/camera/thumbnail authorities");

    std::cout<<"Pass655-674 assertions: "<<passed<<" passed / "<<failed<<" failed\n";
    return failed==0?0:1;
}
