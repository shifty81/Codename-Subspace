#include "editor/ConstructionUiLayoutPolicy.h"
#include "editor/ConstructionAxisContextSystem.h"
#include <cmath>
#include <iostream>

using namespace subspace;
static int failures=0;
static void Check(bool ok,const char* what){if(!ok){++failures;std::cerr<<"FAIL: "<<what<<"\n";}}
static bool Near(float a,float b,float e=.001f){return std::fabs(a-b)<=e;}

int main(){
    const float w=ConstructionUiLayoutPolicy::TabWidth(800.0f,8,2.0f);
    Check(Near(w,98.25f),"eight DEV tabs fill one row deterministically");
    Check(ConstructionUiLayoutPolicy::TabWidth(100,0,2)==0,"zero tabs yield zero width");
    Check(ConstructionUiLayoutPolicy::ShowAssetDensity(520,1),"density appears at safe threshold");
    Check(!ConstructionUiLayoutPolicy::ShowAssetDensity(519,1),"density hides before collision threshold");
    Check(ConstructionUiLayoutPolicy::ShowAssetSecondaryActions(650,1),"secondary actions appear at safe threshold");
    Check(!ConstructionUiLayoutPolicy::ShowAssetSecondaryActions(649,1),"secondary actions hide before collision threshold");
    Check(ConstructionUiLayoutPolicy::Overlaps(0,0,10,10,9,0,10,10),"rect overlap detected");
    Check(!ConstructionUiLayoutPolicy::Overlaps(0,0,10,10,10,0,10,10),"touching edges are not overlap");
    const auto x=ConstructionAxisContextSystem::Describe(ConstructionAxis::X);
    const auto y=ConstructionAxisContextSystem::Describe(ConstructionAxis::Y);
    const auto z=ConstructionAxisContextSystem::Describe(ConstructionAxis::Z);
    Check(std::string(x.dimension)=="WIDTH","X is generic width");
    Check(std::string(y.dimension)=="LENGTH","Y is generic length");
    Check(std::string(z.dimension)=="HEIGHT","Z is generic height");
    Check(std::string(x.positive)=="+X","X direction is generic");
    Check(std::string(y.negative)=="-Y","Y direction is generic");
    Check(std::string(z.positive)=="+Z","Z direction is generic");
    if(failures){std::cerr<<failures<<" R32 contract assertion(s) failed\n";return 1;}
    std::cout<<"R32 contract PASS: 14 assertions\n";return 0;
}
