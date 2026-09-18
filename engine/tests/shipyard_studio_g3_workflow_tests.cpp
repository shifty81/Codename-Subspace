#include "ship_editor/ShipyardStudioViewMode.h"
#include "editor/ConstructionEditorCameraSystem.h"
#include <cmath>
#include <iostream>
using namespace subspace;
namespace {int checks=0,fails=0;void Check(bool ok,const char* msg){++checks;if(!ok){++fails;std::cerr<<"FAIL "<<msg<<"\n";}}}
int main(){
    auto mode=ShipyardStudioViewMode::Exterior;
    const char* labels[]={"CUTAWAY","INTERIOR","X-RAY","EXTERIOR"};
    for(const char* label:labels){mode=ShipyardStudioViewSystem::Next(mode);Check(std::string(ShipyardStudioViewSystem::Name(mode))==label,"mode cycling/name");}
    ConstructionEditorCameraState camera;ConstructionEditorCameraSystem::Reset(camera,{0,0,0},2.0f);
    Check(camera.mode==ConstructionCameraMode::CenteredInspect,"empty studio starts orbit not FPS");
    auto eye=camera.eye,target=ConstructionEditorCameraSystem::Target(camera);
    ConstructionEditorCameraSystem::Orbit(camera,30,8);
    Check((camera.eye-eye).length()>.01f,"orbit moves camera");
    Check((ConstructionEditorCameraSystem::Target(camera)-target).length()<.0001f,"orbit retains pivot");
    eye=camera.eye;target=ConstructionEditorCameraSystem::Target(camera);
    ConstructionEditorCameraSystem::PanPixels(camera,40,30,768);
    Check((camera.eye-eye).length()>.001f,"pan moves eye");
    Check((ConstructionEditorCameraSystem::Target(camera)-target).length()>.001f,"pan moves pivot");
    Check(std::fabs((camera.eye-ConstructionEditorCameraSystem::Target(camera)).length()-camera.orbitDistance)<.01f,"pan preserves orbit distance");
    auto radius=(camera.eye-ConstructionEditorCameraSystem::Target(camera)).length();
    ConstructionEditorCameraSystem::Dolly(camera,1);
    Check((camera.eye-ConstructionEditorCameraSystem::Target(camera)).length()<radius,"wheel dollies toward target");
    Check(camera.mode==ConstructionCameraMode::CenteredInspect,"normal nav remains inspection");
    std::cout<<"Studio G3 focused: "<<checks-fails<<"/"<<checks<<" assertions PASS\n";
    return fails?1:0;
}
