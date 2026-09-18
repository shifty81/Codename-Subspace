#include "editor/ConstructionEditorCameraSystem.h"
#include "rendering/StrategicViewProjection.h"
#include "ship_editor/ShipyardPanelCompositorSystem.h"

#include <cmath>
#include <iostream>
#include <string>
#include <vector>

using namespace subspace;
namespace {
int checks=0, failures=0;
void Check(bool ok,const char* message){++checks;if(!ok){++failures;std::cerr<<"[FAIL] "<<message<<'\n';}}
float Dot(const Vector3&a,const Vector3&b){return a.x*b.x+a.y*b.y+a.z*b.z;}
StrategicViewBasis Basis(const ConstructionEditorCameraState& s){
    StrategicCamera camera;
    camera.SetEditorView(s.eye,ConstructionEditorCameraSystem::Target(s),s.rollDegrees);
    return StrategicViewProjection::Build(camera,1280,720);
}
StrategicScreenPoint Project(const ConstructionEditorCameraState& s,const Vector3& world){
    StrategicCamera camera;
    camera.SetEditorView(s.eye,ConstructionEditorCameraSystem::Target(s),s.rollDegrees);
    return StrategicViewProjection::WorldToScreen(world,1280,720,camera);
}
}
int main(){
    ConstructionEditorCameraState s;
    ConstructionEditorCameraSystem::Reset(s,{1,2,3},6);
    const Vector3 subject=s.assemblyCenter;
    const auto baseline=Project(s,subject);
    ConstructionEditorCameraSystem::TruckPedestal(s,0.6f,0.0f); // left mouse => inverse/right camera motion
    const auto left=Project(s,subject);
    Check(left.x<baseline.x-1.0f,"leftward MMB drag shifts subject left");
    ConstructionEditorCameraSystem::TruckPedestal(s,0.0f,0.6f); // down mouse => up camera motion
    const auto down=Project(s,subject);
    Check(down.y>left.y+1.0f,"downward MMB drag shifts subject down");
    // Source code routes BOTH editor entry points through PanPixels; here
    // prove its FOV-based translation matches visible mouse displacement.
    ConstructionEditorCameraSystem::Reset(s,{1,2,3},6);
    const auto exactBefore=Project(s,s.assemblyCenter);
    ConstructionEditorCameraSystem::PanPixels(s,-30.0f,20.0f,720.0f);
    const auto exactAfter=Project(s,subject);
    Check(std::fabs((exactAfter.x-exactBefore.x)+30.0f)<.1f,
        "pixel pan follows cursor left at a projection-consistent scale");
    Check(std::fabs((exactAfter.y-exactBefore.y)-20.0f)<.1f,
        "pixel pan follows cursor down at a projection-consistent scale");
    Check((s.eye-s.assemblyCenter).length()>0.1f,"pan leaves valid camera-target ray");
    const auto afterPan=Basis(s);
    const Vector3 relative=afterPan.eye-afterPan.target;
    Check(std::fabs(relative.length()-s.orbitDistance)<.001f,"pan translates eye and center without losing distance");

    // The old rendering implementation switched from Z-up to Y-up at
    // abs(forward.z) > 0.985 (~80 degrees), abruptly rotating the view.
    ConstructionEditorCameraSystem::Reset(s,{0,0,0},6);
    ConstructionEditorCameraSystem::Orbit(s,0,76.0f-s.pitchDegrees);
    Vector3 previousRight=Basis(s).right;
    Vector3 previousUp=Basis(s).up;
    for(float pitch: {78.0f,79.0f,80.0f,81.0f,83.0f,86.0f,88.0f,89.0f}){
        ConstructionEditorCameraSystem::Orbit(s,0,pitch-s.pitchDegrees);
        const auto basis=Basis(s);
        Check(Dot(previousRight,basis.right)>.99f,"top-pole sweep preserves screen-right direction");
        Check(Dot(previousUp,basis.up)>.99f,"top-pole sweep preserves screen-up direction");
        Check(std::isfinite(basis.right.x)&&std::isfinite(basis.up.y),"pole basis remains finite");
        previousRight=basis.right;previousUp=basis.up;
    }
    ConstructionEditorCameraSystem::Orbit(s,0,-178.0f); // clamps at -89
    Check(std::fabs(s.pitchDegrees+89.0f)<.01f,"orbit clamps instead of flipping through pole");
    previousRight=Basis(s).right;
    for(float pitch: {-86.0f,-83.0f,-80.0f,-79.0f,-76.0f}){
        ConstructionEditorCameraSystem::Orbit(s,0,pitch-s.pitchDegrees);
        const auto basis=Basis(s);
        Check(Dot(previousRight,basis.right)>.99f,"bottom-pole sweep preserves screen-right direction");
        previousRight=basis.right;
    }
    const Vector3 initialTarget=s.assemblyCenter;
    ConstructionEditorCameraSystem::Orbit(s,38.0f,0.0f);
    Check((s.assemblyCenter-initialTarget).length()<.001f,"orbit does not relocate actual ship");

    // G4: camera orientation/framing remains independent of authored geometry.
    ConstructionEditorCameraSystem::Reset(s,{0,0,0},2);
    ConstructionEditorCameraSystem::Orbit(s,42.0f,-8.0f);
    const Vector3 preservedRay=(s.eye-s.assemblyCenter).normalized();
    ConstructionEditorCameraSystem::FramePreservingOrientation(s,{50,-20,8},7.0f);
    Check((s.assemblyCenter-Vector3{50,-20,8}).length()<.001f,"G4 frame selection assigns independent new pivot");
    Check(Dot((s.eye-s.assemblyCenter).normalized(),preservedRay)>.999f,"G4 frame preserves editor viewing angle");
    Check(std::fabs(s.orbitDistance-19.6f)<.01f,"G4 frame scales distance from selection bounds");
    ConstructionEditorCameraSystem::SetAxisView(s,ConstructionAxisView::Right);
    Check(std::fabs(s.yawDegrees-90.0f)<.01f && std::fabs(s.pitchDegrees)<.01f,"G4 right view is stable");
    ConstructionEditorCameraSystem::SetAxisView(s,ConstructionAxisView::Top);
    Check(s.pitchDegrees==89.0f && std::isfinite(s.forward.z),"G4 near-top perspective avoids pole singularity");
    ConstructionEditorCameraSystem::Orbit(s,9.0f,-12.0f);
    Check(std::fabs(s.pitchDegrees-77.0f)<.01f,"G4 orbit remains interactive after top view");

    auto workspace=SubspaceDockSystem::CreateMinimalWorkspace("gui-compositor");
    SubspaceDockPanel a;a.id="assets";a.title="Assets";a.defaultLeafId="bottom";
    SubspaceDockPanel b=a;b.id="properties";b.title="Properties";b.defaultLeafId="right";
    Check(SubspaceDockSystem::RegisterPanel(workspace,a),"register Assets");
    Check(SubspaceDockSystem::RegisterPanel(workspace,b),"register Properties");
    Check(SubspaceDockSystem::FloatPanel(workspace,"assets",{150,150,420,350}),"float Assets");
    Check(SubspaceDockSystem::FloatPanel(workspace,"properties",{200,200,420,350}),"float Properties");
    auto layers=ShipyardPanelCompositorSystem::Snapshot(workspace,1280,720,32);
    const auto* top=ShipyardPanelCompositorSystem::TopFloatingAt(layers,220,220);
    Check(top&&top->panelId=="properties","overlapping floating panels hit the painted frontmost panel");
    Check(!ShipyardPanelCompositorSystem::TopFloatingAt(layers,10,10),"outside all floating windows is not occluded");
    Check(ShipyardPanelCompositorSystem::Panel(layers,"assets")!=nullptr,"panel lookup preserves asset frame");
    Check(ShipyardPanelCompositorSystem::IsFloating(workspace,"assets"),"compositor identifies floating assets");
    Check(SubspaceDockSystem::RaiseFloatingPanel(workspace,"assets"),"raise dragged Assets");
    layers=ShipyardPanelCompositorSystem::Snapshot(workspace,1280,720,32);
    top=ShipyardPanelCompositorSystem::TopFloatingAt(layers,220,220);
    Check(top&&top->panelId=="assets","raising Assets moves its backing and controls together to foreground");
    Check(SubspaceDockSystem::RaiseFloatingPanel(workspace,"properties"),"raise dragged Properties");
    layers=ShipyardPanelCompositorSystem::Snapshot(workspace,1280,720,32);
    top=ShipyardPanelCompositorSystem::TopFloatingAt(layers,220,220);
    Check(top&&top->panelId=="properties","raising Properties restores its foreground and hit-test priority");
    Check(SubspaceDockSystem::DockPanel(workspace,"properties","right"),"dock Properties");
    layers=ShipyardPanelCompositorSystem::Snapshot(workspace,1280,720,32);
    top=ShipyardPanelCompositorSystem::TopFloatingAt(layers,220,220);
    Check(top&&top->panelId=="assets","redocking exposes surviving floating window");
    std::cout<<"PASS1508R5 compositor+navigation: "<<checks-failures<<"/"<<checks<<" assertions passed\n";
    return failures?1:0;
}
