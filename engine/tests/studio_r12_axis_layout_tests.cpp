#include "editor/ConstructionSymmetrySystem.h"
#include "editor/EditorDccShellLayoutSystem.h"
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>
using namespace subspace;
namespace {
int assertions=0;
void check(bool valid,const char* what){
    if(!valid){std::cerr<<"FAIL: "<<what<<'\n';std::exit(1);}++assertions;
}
bool near(float a,float b){return std::fabs(a-b)<0.11f;}
}
int main(){
    const ConstructionSymmetryAxis axes[]={ConstructionSymmetryAxis::PortStarboard,
       ConstructionSymmetryAxis::ForeAft,ConstructionSymmetryAxis::DorsalVentral};
    const char* labels[]={"MIRROR X","MIRROR Y","MIRROR Z"};
    const char* legacy[]={"PORT <-> STARBOARD","FORE <-> AFT","DORSAL <-> VENTRAL"};
    const Vector3 point{3,5,7};
    for(int axis=0;axis<3;++axis){
        const auto kind=axes[axis];
        check(ConstructionSymmetrySystem::AxisIndex(kind)==axis,"coordinate axis retains identity");
        check(std::string(ConstructionSymmetrySystem::AxisName(kind))==labels[axis],"generic editor label");
        check(std::string(ConstructionSymmetrySystem::LegacyAxisName(kind))==legacy[axis],"legacy label available");
        ConstructionSymmetryFrame frame;frame.origin={1,2,3};frame.axis=kind;
        const auto reflected=ConstructionSymmetrySystem::ReflectPoint(point,frame);
        const float expected[]={-1, -1, -1};
        check(near(axis==0?reflected.x:axis==1?reflected.y:reflected.z, expected[axis]),"reflection around nonzero frame");
        const auto twice=ConstructionSymmetrySystem::ReflectPoint(reflected,frame);
        check(near(twice.x,point.x)&&near(twice.y,point.y)&&near(twice.z,point.z),"reflection is involution");
        const auto direction=ConstructionSymmetrySystem::ReflectDirection(point,kind);
        check(near(axis==0?direction.x:axis==1?direction.y:direction.z,axis==0?-3:axis==1?-5:-7),"direction flips exact physical component");
    }
    check(ConstructionSymmetrySystem::AxisIndex(static_cast<ConstructionSymmetryAxis>(250))==-1,"invalid axis rejected");
    const int widths[]={1280,1600,1920,3840};
    const int heights[]={768,900,1080,2160};
    check(!EditorDccShellLayoutSystem::Compute(900,600).valid,"unsupported tiny window rejected");
    for(int i=0;i<4;++i){
        const auto layout=EditorDccShellLayoutSystem::Compute(widths[i],heights[i]);
        check(layout.valid,"layout valid");
        check(near(layout.toolRail.y+layout.toolRail.height,layout.statusBar.y),"rail bottom independent of shelf");
        check(layout.assetShelf.x>=layout.toolRail.x+layout.toolRail.width,"shelf does not pass below rail");
        check(near(layout.assetShelf.x,layout.viewport.x),"shelf aligned to viewport left");
        check(near(layout.assetShelf.width,layout.viewport.width),"shelf aligned to viewport width");
        check(near(layout.viewport.y+layout.viewport.height,layout.assetShelf.y),"shelf below viewport");
        check(near(layout.assetShelf.y+layout.assetShelf.height,layout.statusBar.y),"shelf ends at status");
        check(layout.toolRail.height>layout.viewport.height,"rail full height");
    }
    std::cout<<"R12 PASS "<<assertions<<" axis compatibility and baseline geometry assertions\n";
}
