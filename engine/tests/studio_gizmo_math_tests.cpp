#include <cmath>
#include <cstdlib>
#include <iostream>
// Reproduce the platform header's legacy near macro at the gizmo include.
// Keep this test in the existing CTest target so Windows name collisions
// cannot silently slip past the otherwise headless geometry assertions.
#define near
#include "studio/StudioGizmoMath.h"
#undef near
using namespace subspace;
namespace {
int count=0;
void Check(bool value,const char* name){++count;if(!value){std::cerr<<"FAIL "<<name<<'\n';std::exit(1);}}
bool Near(float a,float b){return std::fabs(a-b)<.001f;}
}
int main(){
    const StudioAxisHandle x{StudioAxis::X,{250,250},{320,250},true};
    const StudioAxisHandle y{StudioAxis::Y,{250,250},{250,180},true};
    const StudioAxisHandle z{StudioAxis::Z,{250,250},{201,299},true};
    Check(StudioGizmoMath::Hit(x,320,250),"X tip hit");
    Check(StudioGizmoMath::Hit(x,291,251),"X shaft hit");
    Check(StudioGizmoMath::Hit(x,262,262),"expanded proximal shaft hit, not just tiny endpoint");
    Check(!StudioGizmoMath::Hit(x,250,250),"pivot must not capture axis");
    Check(!StudioGizmoMath::Hit(x,260,250),"proximal exclusion remains around pivot");
    Check(!StudioGizmoMath::Hit(x,262,265),"expanded proximal shaft does not capture distant points");
    Check(!StudioGizmoMath::Hit(x,250,180),"Y tip not captured by X");
    Check(StudioGizmoMath::Hit(y,250,180),"Y tip hit");
    Check(StudioGizmoMath::Hit(z,201,299),"Z tip hit");
    Check(!StudioGizmoMath::Hit({StudioAxis::X,{0,0},{0,0},true},0,0),"no degenerate capture");
    Check(!StudioGizmoMath::Hit({StudioAxis::X,{0,0},{70,0},false},70,0),"disabled handle rejected");
    Check(Near(StudioGizmoMath::DragScalar(x,18,10,false),18),"X translation projects onto X");
    Check(Near(StudioGizmoMath::DragScalar(y,18,-10,false),10),"Y translation projects onto Y");
    Check(Near(StudioGizmoMath::DragScalar(x,18,10,true),10),"X rotation tracks tangent");
    Check(Near(StudioGizmoMath::DragScalar(y,18,-10,true),18),"Y rotation tracks tangent");
    Check(Near(StudioGizmoMath::Component(StudioAxis::X,15,30,45),15),"pitch axis");
    Check(Near(StudioGizmoMath::Component(StudioAxis::Y,15,30,45),30),"yaw axis");
    Check(Near(StudioGizmoMath::Component(StudioAxis::Z,15,30,45),45),"roll axis");
    Check(Near(StudioGizmoMath::Component(StudioAxis::None,15,30,45),0),"invalid axis neutral");
    Check(Near(StudioGizmoMath::WrappedAngle(360),0),"360 degrees wraps to canonical zero");
    Check(Near(StudioGizmoMath::WrappedAngle(-190),170),"negative wrap");
    Check(Near(StudioGizmoMath::WrappedAngle(270),-90),"positive wrap");
    Check(Near(StudioGizmoMath::WrappedAngle(15),15),"snap degree identity");
    Check(Near(StudioGizmoMath::RotationComponent(StudioAxis::X,15,30,45),15),"X maps pitch / Rx");
    Check(Near(StudioGizmoMath::RotationComponent(StudioAxis::Y,15,30,45),45),"Y maps roll / Ry");
    Check(Near(StudioGizmoMath::RotationComponent(StudioAxis::Z,15,30,45),30),"Z maps yaw / Rz");
    // Regression: builder filters physical X/Y/Z via packed transform
    // components pitch/yaw/roll, while the gizmo uses pitch/roll/yaw.
    // The old identity mapping made Y and Z rotation deltas zero after masking.
    Check(StudioGizmoMath::RotationFieldAxis(StudioAxis::X)==StudioAxis::X,
          "physical X rotation targets builder pitch field X");
    Check(StudioGizmoMath::RotationFieldAxis(StudioAxis::Y)==StudioAxis::Z,
          "physical Y rotation targets builder roll field Z");
    Check(StudioGizmoMath::RotationFieldAxis(StudioAxis::Z)==StudioAxis::Y,
          "physical Z rotation targets builder yaw field Y");
    Check(StudioGizmoMath::RotationFieldAxis(StudioAxis::None)==StudioAxis::None,
          "invalid physical rotation axis has no field");
    auto masked=[](StudioAxis axis,float pitch,float yaw,float roll){
        const auto constraint=StudioGizmoMath::RotationFieldAxis(axis);
        return StudioGizmoMath::Component(constraint,pitch,yaw,roll);
    };
    Check(Near(masked(StudioAxis::X,20,0,0),20),"X survives constraint");
    Check(Near(masked(StudioAxis::Y,0,0,20),20),"Y survives constraint");
    Check(Near(masked(StudioAxis::Z,0,20,0),20),"Z survives constraint");
    Check(Near(masked(StudioAxis::Y,0,20,0),0),"Y excludes yaw field");
    Check(Near(masked(StudioAxis::Z,0,0,20),0),"Z excludes roll field");
    // A model recipe uses XYZ geometry rotation directly. Assembly retains its
    // historical packed pitch/yaw/roll mapping; translation remains direct.
    Check(StudioGizmoMath::ConstraintFieldAxis(StudioAxis::X,true,true)==StudioAxis::X,
          "model X rotation retains physical X");
    Check(StudioGizmoMath::ConstraintFieldAxis(StudioAxis::Y,true,true)==StudioAxis::Y,
          "model Y rotation retains physical Y");
    Check(StudioGizmoMath::ConstraintFieldAxis(StudioAxis::Z,true,true)==StudioAxis::Z,
          "model Z rotation retains physical Z");
    Check(StudioGizmoMath::ConstraintFieldAxis(StudioAxis::X,false,true)==StudioAxis::X,
          "assembly X rotation retains pitch mapping");
    Check(StudioGizmoMath::ConstraintFieldAxis(StudioAxis::Y,false,true)==StudioAxis::Z,
          "assembly Y rotation retains roll mapping");
    Check(StudioGizmoMath::ConstraintFieldAxis(StudioAxis::Z,false,true)==StudioAxis::Y,
          "assembly Z rotation retains yaw mapping");
    Check(StudioGizmoMath::ConstraintFieldAxis(StudioAxis::Y,true,false)==StudioAxis::Y,
          "model Y movement is direct");
    Check(StudioGizmoMath::ConstraintFieldAxis(StudioAxis::Z,false,false)==StudioAxis::Z,
          "assembly Z movement is direct");
    Check(StudioGizmoMath::ConstraintFieldAxis(StudioAxis::None,true,true)==StudioAxis::None,
          "invalid model axis is neutral");
    std::cout<<"Studio gizmo math: "<<count<<"/"<<count<<" PASS\n";
}
