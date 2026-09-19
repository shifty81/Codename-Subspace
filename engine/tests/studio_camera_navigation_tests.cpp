#include "studio/StudioCameraNavigation.h"
#include "rendering/StrategicCamera.h"
#include "rendering/StrategicViewProjection.h"
#include <cmath>
#include <iostream>
#include <cstdlib>
using namespace subspace;
namespace {
int checks=0;
void Check(bool condition,const char* label){++checks;if(!condition){std::cerr<<"FAIL "<<label<<'\n';std::exit(1);}}
float Dist(const Vector3&a,const Vector3&b){return (a-b).length();}
}
int main(){
    ConstructionEditorCameraState state{};
    ConstructionEditorCameraSystem::Reset(state,{4,3,2},8);
    const auto initialCenter=state.assemblyCenter;
    const float distance=Dist(state.eye,state.assemblyCenter);
    Check(distance>1,"actual 3D orbit distance");
    Check(state.mode==ConstructionCameraMode::CenteredInspect,"orbit starts centered");
    StudioCameraNavigation::Orbit(state,360.0f/.38f,0,false);
    Check(Dist(state.eye,state.assemblyCenter)>1,"360 horizontal rotation keeps view");
    Check(Dist(state.assemblyCenter,initialCenter)<.001f,"full turn cannot edit ship pivot");
    Check(Dist(state.eye,state.assemblyCenter)-distance<.001f,"orbit distance stable");
    Check(std::fabs(state.yawDegrees)<=180,"360 yaw wrapped");
    StudioCameraNavigation::Orbit(state,0,800,false);
    Check(state.eye.z<initialCenter.z,"can inspect ship from underneath");
    Check(state.pitchDegrees>=-89.0f&&state.pitchDegrees<=89.0f,"nondegenerate pole guard");
    StudioCameraNavigation::Orbit(state,0,-1600,false);
    Check(state.eye.z>initialCenter.z,"can inspect ship from above");
    const auto beforeRollEye=state.eye, beforeRollCenter=state.assemblyCenter;
    StudioCameraNavigation::Orbit(state,90,0,true);
    Check(std::fabs(state.rollDegrees)>1,"Shift right drag rolls camera");
    Check(Dist(state.eye,beforeRollEye)<.001f,"roll does not move eye");
    Check(Dist(state.assemblyCenter,beforeRollCenter)<.001f,"roll preserves pivot");
    Check(state.mode==ConstructionCameraMode::CenteredInspect,"roll does not enter freefly");
    StudioCameraNavigation::Orbit(state,NAN,0,false);
    Check(Dist(state.eye,beforeRollEye)<.001f,"bad pointer samples cannot corrupt camera");
    StrategicCamera render;render.SetEditorView(state.eye,ConstructionEditorCameraSystem::Target(state),state.rollDegrees);
    const auto basis=StrategicViewProjection::Build(render,1280,720);
    Check(basis.eye.z>initialCenter.z,"perspective consumes actual camera elevation");
    Check(std::isfinite(basis.right.x)&&std::isfinite(basis.up.y),"projection basis finite");
    const auto projection=StrategicViewProjection::WorldToScreen(state.assemblyCenter,1280,720,render);
    Check(projection.visible,"orbital target projects into viewport");
    const auto targetBeforePan=state.assemblyCenter,eyeBeforePan=state.eye;
    ConstructionEditorCameraSystem::PanPixels(state,35,25,720);
    Check(Dist(state.assemblyCenter,targetBeforePan)>.01f,"MMB pans target");
    Check(std::fabs(Dist(state.eye,eyeBeforePan)-Dist(state.assemblyCenter,targetBeforePan))<.001f,"pan translates eye and target together");
    const float beforeDolly=Dist(state.eye,state.assemblyCenter);
    ConstructionEditorCameraSystem::Dolly(state,1);
    Check(Dist(state.eye,state.assemblyCenter)<beforeDolly,"wheel dollies real camera");
    Check(Dist(state.assemblyCenter,targetBeforePan)>.01f,"dolly retains panned pivot");
    std::cout<<"PASS "<<checks<<"/"<<checks<<" Studio 3D orbit assertions\n";
}
