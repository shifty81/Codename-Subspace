#include "ship_editor/ShipyardWorkspaceSystem.h"
#include "ship_editor/ShipyardDockPointerSystem.h"
#include "ship_editor/ShipyardBuilderSystem.h"
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

using namespace subspace;
namespace {
int assertions=0;
void Check(bool ok,const char* description){
    if(!ok){std::cerr<<"[FAIL] "<<description<<'\n';std::exit(1);}
    ++assertions;
}
const SubspaceDockLayout* Find(const std::vector<SubspaceDockLayout>& layouts,const std::string& id){
    for(const auto& l:layouts)if(l.panelId==id&&l.visible)return &l;
    return nullptr;
}
bool Near(float a,float b){return std::fabs(a-b)<.1f;}
}
int main(){
    constexpr int width=1280,height=740;
    constexpr float top=78;
    auto w=ShipyardWorkspaceSystem::BuildDefaultDockWorkspace();
    Check(SubspaceDockSystem::Validate(w),"default workspace valid");
    const auto* root=SubspaceDockSystem::FindNode(w,"root");
    const auto* content=SubspaceDockSystem::FindNode(w,"content");
    Check(root&&root->split&&root->axis==SubspaceDockSplitAxis::Horizontal&&
          root->firstChildId=="tool_left"&&root->secondChildId=="content",
          "tool rail is a dedicated fixed root column");
    Check(content&&content->split&&content->axis==SubspaceDockSplitAxis::Vertical&&
          content->firstChildId=="upper"&&content->secondChildId=="bottom",
          "asset shelf is a content sibling, not attached to the rail");
    auto layouts=SubspaceDockSystem::Materialize(w,width,height,top);
    const auto* rail=Find(layouts,"tool_rail");
    const auto* asset=Find(layouts,"asset_browser");
    const auto* view=Find(layouts,"viewport");
    Check(rail&&asset&&view,"three independent regions materialized");
    const auto railRect=rail->rect,viewRect=view->rect,assetRect=asset->rect;
    Check(assetRect.x>=railRect.x+railRect.width-.1f,
          "bottom shelf begins after reserved tool column");
    Check(Near(railRect.height,static_cast<float>(height)-top),
          "tool rail extends full usable editor height");
    Check(Near(assetRect.y+assetRect.height,railRect.y+railRect.height),
          "bottom shelf ends at the status boundary independently");
    Check(Near(assetRect.x,viewRect.x),"shelf and viewport share the content left edge");
    Check(Near(viewRect.y+viewRect.height,assetRect.y),"viewport ends at docked shelf");
    const auto* tool=SubspaceDockSystem::FindPanel(w,"tool_rail");
    Check(tool&&!tool->floatable&&!tool->closable,"rail cannot be floated or closed");
    Check(!SubspaceDockSystem::FloatPanel(w,"tool_rail",{100,100,200,300}),
          "model rejects floating fixed tool rail");
    Check(SubspaceDockSystem::FloatPanel(w,"asset_browser",{430,320,600,280}),
          "asset shelf can float independently");
    layouts=SubspaceDockSystem::Materialize(w,width,height,top);
    rail=Find(layouts,"tool_rail");asset=Find(layouts,"asset_browser");view=Find(layouts,"viewport");
    Check(rail&&asset&&view&&asset->floating,"floating asset is an overlay");
    Check(Near(rail->rect.x,railRect.x)&&Near(rail->rect.y,railRect.y)&&
          Near(rail->rect.width,railRect.width)&&Near(rail->rect.height,railRect.height),
          "floating asset changes no tool rail geometry");
    Check(Near(view->rect.x,viewRect.x)&&Near(view->rect.y,viewRect.y)&&
          Near(view->rect.width,viewRect.width)&&Near(view->rect.height,viewRect.height),
          "floating asset changes no 3D view geometry");
    ShipyardDockPointerSystem pointer;
    Check(pointer.Begin(w,width,height,top,asset->rect.x+12,asset->rect.y+9),
          "floating asset header can be captured");
    Check(pointer.Drag(w,-48,-24,width,height,top),"asset is moved by pointer");
    Check(pointer.End(w,12,height-24,width,height,top),
          "releasing asset over fixed rail completes without docking");
    Check(Find(SubspaceDockSystem::Materialize(w,width,height,top),"asset_browser")->floating,
          "tool rail cannot receive asset browser drop");
    Check(pointer.Begin(w,width,height,top,asset->rect.x+12-48,asset->rect.y+9-24),
          "asset can be recaptured after independent float");
    Check(pointer.Drag(w,-10,4,width,height,top),"asset drag back to bottom");
    Check(pointer.End(w,width/2.0f,height-24,width,height,top),"drop on bottom content leaf");
    layouts=SubspaceDockSystem::Materialize(w,width,height,top);
    rail=Find(layouts,"tool_rail");asset=Find(layouts,"asset_browser");
    Check(asset&&!asset->floating&&asset->leafId=="bottom"&&
          Near(asset->rect.x,rail->rect.x+rail->rect.width),
          "asset redocks under content but never underneath tool rail");
    Check(SubspaceDockSystem::Validate(w),"no duplicate owners after asset redock");
    std::cout<<"PASS1508R3: "<<assertions<<" independent-region assertions passed\n";
}
