#include "studio/StudioOverlayPlacementPolicy.h"
#include <cassert>
#include <cmath>
#include <iostream>
using namespace subspace;
namespace {
bool near(float a,float b){return std::fabs(a-b)<.001f;}
ShipyardPanelCompositorSystem::Layers Base(){
    return {{"viewport","3D Viewport",{100,50,900,650},1,true,true,false}};
}
void Float(ShipyardPanelCompositorSystem::Layers& l,const char* id,SubspaceUiRect r){
    l.push_back({id,id,r,1,true,true,true});
}
}
int main(){
    using P=StudioOverlayPlacementPolicy;
    auto layers=Base();
    auto p=P::Choose(layers,100,50,1000,700);
    assert(p.visible&&p.slot==0&&near(p.rect.x,109)&&near(p.rect.y,59));

    Float(layers,"tl",{100,50,400,180});
    p=P::Choose(layers,100,50,1000,700);
    assert(p.visible&&p.slot==1&&near(p.rect.x,556)&&near(p.rect.y,59));

    Float(layers,"tr",{550,50,450,180});
    p=P::Choose(layers,100,50,1000,700);
    assert(p.visible&&p.slot==2&&near(p.rect.x,109)&&near(p.rect.y,561));

    Float(layers,"bl",{100,560,400,140});
    p=P::Choose(layers,100,50,1000,700);
    assert(p.visible&&p.slot==3&&near(p.rect.x,556)&&near(p.rect.y,561));

    Float(layers,"br",{550,560,450,140});
    p=P::Choose(layers,100,50,1000,700);
    assert(!p.visible&&p.slot==-1);

    auto edge=Base();
    // Exact edge contact is intentionally not overlap under half-open geometry.
    Float(edge,"edge",{109+P::kHudWidth,59,80,80});
    p=P::Choose(edge,100,50,1000,700);
    assert(p.visible&&p.slot==0);

    auto docked=Base();
    docked.push_back({"docked","Docked",{100,50,400,180},1,true,true,false});
    p=P::Choose(docked,100,50,1000,700);
    assert(p.visible&&p.slot==0); // docked geometry is already excluded by viewport layout

    assert(!P::Choose(Base(),100,50,500,180).visible);
    assert(!P::Choose(Base(),NAN,50,1000,700).visible);
    std::cout<<"R33 adaptive Studio overlay placement: PASS\n";
}
