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
          "legacy root topology retained as metadata, not canvas geometry");
    Check(content&&content->split&&content->axis==SubspaceDockSplitAxis::Vertical&&
          content->firstChildId=="upper"&&content->secondChildId=="bottom",
          "anchor topology remains serializable");
    auto layouts=SubspaceDockSystem::Materialize(w,width,height,top);
    const auto* rail=Find(layouts,"tool_rail");
    const auto* asset=Find(layouts,"asset_browser");
    const auto* view=Find(layouts,"viewport");
    Check(rail&&asset&&view,"three independent regions materialized");
    const auto railRect=rail->rect,viewRect=view->rect,assetRect=asset->rect;
    const auto* properties=Find(layouts,"properties");
    Check(properties&&assetRect.x+assetRect.width+4.0f<=properties->rect.x,
          "default bottom overlay does not obscure the Properties inspector");
    Check(assetRect.x>=railRect.x+railRect.width-.1f,
          "default asset overlay starts to right of rail");
    Check(railRect.height>300.0f,"rail defaults to accessible compact height");
    Check(Near(assetRect.y+assetRect.height,static_cast<float>(height)-6.0f),
          "asset overlay anchors above the status boundary");
    Check(viewRect.x==0.0f,"canvas begins at window left behind overlays");
    Check(Near(viewRect.y+viewRect.height,static_cast<float>(height)),
          "canvas extends behind docked asset shelf");
    const auto* tool=SubspaceDockSystem::FindPanel(w,"tool_rail");
    Check(tool&&tool->floatable&&tool->closable,"rail is an independent dockable tool");
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
          "releasing asset over rail completes without nesting tools");
    Check(Find(SubspaceDockSystem::Materialize(w,width,height,top),"asset_browser")->floating,
          "tool rail cannot receive asset browser drop");
    Check(pointer.Begin(w,width,height,top,asset->rect.x+12-48,asset->rect.y+9-24),
          "asset can be recaptured after independent float");
    Check(pointer.Drag(w,-10,4,width,height,top),"asset drag back to bottom");
    Check(pointer.End(w,width/2.0f,height-24,width,height,top),"drop on bottom content leaf");
    layouts=SubspaceDockSystem::Materialize(w,width,height,top);
    rail=Find(layouts,"tool_rail");asset=Find(layouts,"asset_browser");
    Check(asset&&!asset->floating&&asset->leafId=="bottom"&&
          asset->rect.x>=rail->rect.x+rail->rect.width,
          "asset returns to independent bottom overlay anchor");
    Check(SubspaceDockSystem::Validate(w),"no duplicate owners after asset redock");
    const float reservedWidth=asset->rect.width;
    Check(SubspaceDockSystem::FloatPanel(w,"properties",{630,155,380,400}),
          "Properties can float and release its dock anchor");
    layouts=SubspaceDockSystem::Materialize(w,width,height,top);
    asset=Find(layouts,"asset_browser");view=Find(layouts,"viewport");
    Check(asset&&asset->rect.width>reservedWidth+300.0f,
          "asset shelf expands only when docked Properties no longer occupies the right anchor");
    Check(view&&Near(view->rect.width,viewRect.width),
          "Properties float changes no canvas width");
    Check(SubspaceDockSystem::DockPanel(w,"properties","right_bottom"),
          "Properties can return to original dock");
    layouts=SubspaceDockSystem::Materialize(w,width,height,top);
    asset=Find(layouts,"asset_browser");properties=Find(layouts,"properties");
    Check(asset&&properties&&Near(asset->rect.width,reservedWidth)&&
          asset->rect.x+asset->rect.width+4.0f<=properties->rect.x,
          "redock reestablishes non-overlapping default anchors");
    std::cout<<"PASS1508R3: "<<assertions<<" independent-region assertions passed\n";
}
