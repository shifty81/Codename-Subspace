#include "studio/StudioGizmoProjectionPolicy.h"
#include "studio/StudioInteriorPreviewKey.h"
#include <array>
#include <cassert>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>
using namespace subspace;
namespace {
using Projection=StudioGizmoProjectionPolicy;
constexpr Projection::Bounds bounds{10,10,640,390};
bool blocked(float x,float y){return x>=356&&x<=400&&y>=177&&y<=205;}
void TestDockOcclusion(){
    const StudioPoint center{320,190};
    const std::array<StudioPoint,3> directions{{{1,0},{-.70710678f,-.70710678f},{0,1}}};
    const auto base=Projection::BuildHandles(center,directions,bounds);
    assert(base[0].valid&&blocked(base[0].tip.x,base[0].tip.y));
    const auto adjusted=Projection::ReflowForOcclusion(base,bounds,blocked);
    for(std::size_t i=0;i<3;++i){
        assert(adjusted[i].valid);
        assert(adjusted[i].axis==static_cast<StudioAxis>(i));
        assert(StudioGizmoMath::Hit(adjusted[i],adjusted[i].tip.x,adjusted[i].tip.y));
        for(int step=1;step<=12;++step){
            const float t=static_cast<float>(step)/12.0f;
            assert(!blocked(center.x+(adjusted[i].tip.x-center.x)*t,
                            center.y+(adjusted[i].tip.y-center.y)*t));
        }
    }
    assert(!blocked(adjusted[0].tip.x,adjusted[0].tip.y));
    assert(std::fabs(adjusted[0].tip.y-base[0].tip.y)>1);
    const auto occludedPivot=Projection::ReflowForOcclusion(base,bounds,
        [](float x,float y){return x>310&&x<330&&y>180&&y<200;});
    for(const auto& handle:occludedPivot)assert(!handle.valid);
    const auto clear=Projection::ReflowForOcclusion(base,bounds,
        [](float,float){return false;});
    for(std::size_t i=0;i<3;++i){
        assert(clear[i].valid);
        assert(std::fabs(clear[i].tip.x-base[i].tip.x)<.001f);
        assert(std::fabs(clear[i].tip.y-base[i].tip.y)<.001f);
    }
}
void TestInteriorCache(){
    ShipyardModuleRecord hull{};
    hull.source.moduleId="hull-test";
    hull.source.halfWidth=3;
    hull.source.halfLength=5;
    hull.source.halfHeight=2;
    ShipyardModuleRecord unused{};unused.source.moduleId="not-on-ship";
    std::vector<ShipyardModuleRecord> catalog{hull,unused};
    ProceduralShipVisualRecipe recipe{};
    VisualModulePlacement p{};p.moduleId="hull-test";
    recipe.modules.push_back(p);
    auto key=[&](){return StudioInteriorPreviewKey::Compute(catalog,recipe);};
    const std::string initial=key();
    assert(initial==key());
    catalog[1].source.halfWidth+=1;assert(initial==key()); // unused catalog changes are irrelevant
    recipe.modules[0].pitchDegrees=7;assert(initial!=key());
    recipe.modules[0].pitchDegrees=0;assert(initial==key());
    recipe.modules[0].x+=.125f;assert(initial!=key());
    recipe.modules[0].x=0;assert(initial==key());
    recipe.modules[0].scaleZ=1.3f;assert(initial!=key());
    recipe.modules[0].scaleZ=1;assert(initial==key());
    recipe.modules[0].mirrorX=true;assert(initial!=key());
    recipe.modules[0].mirrorX=false;assert(initial==key());
    catalog[0].source.halfHeight+=.5f;assert(initial!=key());
    catalog[0].source.halfHeight-=.5f;assert(initial==key());
    catalog[0].surfaceOnly=true;assert(initial!=key());
    catalog[0].surfaceOnly=false;assert(initial==key());
    ShipyardAssemblySocket socket{};socket.name="walkable-door";
    catalog[0].sockets.push_back(socket);assert(initial!=key());
    catalog[0].sockets.clear();assert(initial==key());
    VisualAssemblyAttachment a{};a.parentSocket="A";a.childSocket="B";
    recipe.attachments.push_back(a);assert(initial!=key());
    recipe.attachments.clear();assert(initial==key());
}
}
int main(){TestDockOcclusion();TestInteriorCache();std::cout<<"Null Harbor R4 dock reflow and interior invalidation: PASS\n";}
