#include "ship_editor/ShipyardStudioViewMode.h"
#include "editor/ConstructionEditorCameraSystem.h"
#include <cmath>
#include <iostream>
using namespace subspace;
namespace {int checks=0,fails=0;void Check(bool ok,const char* msg){++checks;if(!ok){++fails;std::cerr<<"FAIL "<<msg<<"\n";}}}
int main(){
    auto mode=ShipyardStudioViewMode::Exterior;
    Check(ShipyardStudioViewSystem::ForWorkspace(ShipyardStudioViewMode::Exterior,true)==ShipyardStudioViewMode::InteriorOnly,"entering interior chooses interior view");
    Check(ShipyardStudioViewSystem::ForWorkspace(ShipyardStudioViewMode::InteriorOnly,false)==ShipyardStudioViewMode::Exterior,"returning to build reveals hull");
    Check(ShipyardStudioViewSystem::ForWorkspace(ShipyardStudioViewMode::Cutaway,false)==ShipyardStudioViewMode::Cutaway,"cutaway remains deliberate");
    Check(ShipyardStudioViewSystem::ForWorkspace(ShipyardStudioViewMode::XRay,false)==ShipyardStudioViewMode::XRay,"X-ray remains deliberate");
    Check(ShipyardStudioViewSystem::ForWorkspace(ShipyardStudioViewMode::Exterior,false)==ShipyardStudioViewMode::Exterior,"build keeps exterior visible");
    Check(ShipyardStudioViewSystem::ForWorkspace(ShipyardStudioViewMode::InteriorOnly,false)==ShipyardStudioViewMode::Exterior,"TEST tab and keyboard cycling restore exterior");
    Check(ShipyardStudioViewSystem::ForPlacement(ShipyardStudioViewMode::InteriorOnly)==ShipyardStudioViewMode::Exterior,"placement reveals hull after manual interior-only");
    Check(ShipyardStudioViewSystem::ForPlacement(ShipyardStudioViewMode::Exterior)==ShipyardStudioViewMode::Exterior,"placement keeps exterior");
    Check(ShipyardStudioViewSystem::ForPlacement(ShipyardStudioViewMode::Cutaway)==ShipyardStudioViewMode::Cutaway,"placement keeps cutaway");
    Check(ShipyardStudioViewSystem::ForPlacement(ShipyardStudioViewMode::XRay)==ShipyardStudioViewMode::XRay,"placement keeps X-ray");
    Check(ShipyardStudioViewSystem::ForWorkspace(ShipyardStudioViewMode::Cutaway,true)==ShipyardStudioViewMode::Cutaway,"interior navigation keeps requested cutaway");
    Check(ShipyardStudioViewSystem::ForWorkspace(ShipyardStudioViewMode::XRay,true)==ShipyardStudioViewMode::XRay,"interior navigation keeps requested X-ray");
    // Catalog pointer press must reveal the ghost, not wait for mouse release.
    Check(ShipyardStudioViewSystem::ForCatalogPress(ShipyardStudioViewMode::InteriorOnly,false)==ShipyardStudioViewMode::Exterior,"Build catalog press reveals ghost before drag threshold");
    Check(ShipyardStudioViewSystem::ForCatalogPress(ShipyardStudioViewMode::InteriorOnly,true)==ShipyardStudioViewMode::InteriorOnly,"Interior workspace catalog press keeps interior inspection");
    Check(ShipyardStudioViewSystem::ForCatalogPress(ShipyardStudioViewMode::Cutaway,false)==ShipyardStudioViewMode::Cutaway,"Catalog press retains intentional cutaway");
    Check(ShipyardStudioViewSystem::ForCatalogPress(ShipyardStudioViewMode::XRay,false)==ShipyardStudioViewMode::XRay,"Catalog press retains intentional X-ray");
    Check(ShipyardStudioViewSystem::ForCatalogPress(ShipyardStudioViewMode::Exterior,false)==ShipyardStudioViewMode::Exterior,"Catalog press retains exterior");
    Check(ShipyardStudioViewSystem::ForCatalogPress(ShipyardStudioViewMode::Exterior,true)==ShipyardStudioViewMode::Exterior,"Interior workspace existing exterior stays exterior on catalog press");
    Check(ShipyardStudioViewSystem::ForCatalogPress(ShipyardStudioViewSystem::ForCatalogPress(ShipyardStudioViewMode::InteriorOnly,false),false)==ShipyardStudioViewMode::Exterior,"Repeated catalog press remains stable");
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
