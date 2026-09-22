#include "studio/StudioGuiInteractionPolicy.h"
#include "editor/EditorForgeGuiStyleSystem.h"
#include <cassert>
#include <iostream>
using namespace subspace;
int main(){
    using P=StudioGuiInteractionPolicy;
    assert(!P::DragActivated({0,0}));
    assert(!P::DragActivated({2.0f,2.0f}));
    assert(P::DragActivated({3.0f,0.0f}));
    assert(P::DragActivated({-4.0f,1.0f}));
    assert(!P::DragActivated({NAN,0.0f}));
    const SubspaceUiRect hud{20,20,435,106};
    assert(ShipyardPanelCompositorSystem::Intersects(hud,{450,50,80,70}));
    assert(!ShipyardPanelCompositorSystem::Intersects(hud,{455,50,80,70}));
    ShipyardPanelCompositorSystem::Layers layers;
    layers.push_back({"viewport","viewport",{0,0,1280,720},1,true,true,false});
    layers.push_back({"inspector","",{450,50,80,70},1,true,true,true});
    assert(!P::ClearOverlayArea(layers,hud));
    layers.back().rect={455,50,80,70};
    assert(P::ClearOverlayArea(layers,hud));
    layers.back().rect={450,50,80,70};layers.back().visible=false;
    assert(P::ClearOverlayArea(layers,hud));
    assert(!P::ClearOverlayArea(layers,{20,20,0,106}));
    for(bool compact:{false,true}){
        const auto m=EditorForgeGuiStyleSystem::Metrics(compact);
        assert(m.propertyRowHeight>=28&&m.assetRowHeight>=27&&m.toolRailWidth>=40);
        assert(m.panelHeaderHeight>0&&m.statusBarHeight>0);
    }
    const auto color=EditorForgeGuiStyleSystem::Palette();
    assert(color.accent.b>color.accent.g&&color.text.r>color.panel.r);
    std::cout<<"R6 Studio GUI interaction and theme: PASS\n";
}
