#include "studio/StudioGizmoProjectionPolicy.h"
#include <cassert>
#include <cmath>
#include <iostream>
using namespace subspace;
static bool close(float a,float b){return std::fabs(a-b)<.0001f;}
int main(){
    assert(StudioGizmoProjectionPolicy::UsesScreenSpaceFallback({0,0}));
    assert(StudioGizmoProjectionPolicy::UsesScreenSpaceFallback({1,0}));
    assert(!StudioGizmoProjectionPolicy::UsesScreenSpaceFallback({20,0}));
    const auto x=StudioGizmoProjectionPolicy::Direction(StudioAxis::X,{0,0});
    const auto y=StudioGizmoProjectionPolicy::Direction(StudioAxis::Y,{0,0});
    const auto z=StudioGizmoProjectionPolicy::Direction(StudioAxis::Z,{0,0});
    assert(close(x.x,1)&&close(x.y,0));
    assert(y.x<0&&y.y<0);
    assert(close(z.x,0)&&close(z.y,1));
    assert(!close(x.x,y.x)&&!close(y.x,z.x));
    const auto normal=StudioGizmoProjectionPolicy::Direction(StudioAxis::Z,{3,4});
    assert(close(normal.x,.6f)&&close(normal.y,.8f));
    assert(StudioGizmoProjectionPolicy::UsesScreenSpaceFallback({NAN,0}));
    std::cout<<"Studio gizmo camera-aligned projection: PASS\n";
}
