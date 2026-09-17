#include "ui/SubspaceUiFramework.h"
#include "ship_editor/ShipyardDockPointerSystem.h"
#include "ship_editor/ShipyardOverlayLayoutStore.h"
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>
using namespace subspace;
namespace {
int checks=0;
void Check(bool v,const char* why){if(!v){std::cerr<<"[FAIL] "<<why<<'\n';std::exit(1);}++checks;}
const SubspaceDockLayout* Find(const std::vector<SubspaceDockLayout>& layers,const char* id){
    for(const auto& layer:layers)if(layer.panelId==id&&layer.visible)return &layer;
    return nullptr;
}
// Return an owned snapshot; a pointer into a temporary Materialize vector dies
// at the end of its expression and can crash outside the Windows toolchain.
SubspaceDockLayout FindLive(const SubspaceDockWorkspace& w,const char* id){
    const auto layouts=SubspaceDockSystem::Materialize(w,1280,740,78);
    const auto* layer=Find(layouts,id);
    Check(layer!=nullptr,"materialized overlay panel exists");
    return *layer;
}
bool Same(const SubspaceUiRect&a,const SubspaceUiRect&b){
    return std::fabs(a.x-b.x)<.01f&&std::fabs(a.y-b.y)<.01f&&
        std::fabs(a.width-b.width)<.01f&&std::fabs(a.height-b.height)<.01f;
}
SubspaceDockWorkspace Make(){
    SubspaceDockWorkspace w;w.id="shipyard";w.rootNodeId="root";
    w.nodes={{"root",true,SubspaceDockSplitAxis::Horizontal,.035f,"tool_left","content",{}, {},false},
      {"content",false,SubspaceDockSplitAxis::Horizontal,.5f,{},{},{},{},false},
      {"tool_left",false,SubspaceDockSplitAxis::Horizontal,.5f,{},{},{},{},false},
      {"center",false,SubspaceDockSplitAxis::Horizontal,.5f,{},{},{},{},false},
      {"right_top",false,SubspaceDockSplitAxis::Horizontal,.5f,{},{},{},{},false},
      {"right_bottom",false,SubspaceDockSplitAxis::Horizontal,.5f,{},{},{},{},false},
      {"bottom",false,SubspaceDockSplitAxis::Horizontal,.5f,{},{},{},{},false}};
    auto add=[&](const char* id,const char* leaf,float pw,float ph,bool canFloat=true){
        SubspaceDockPanel p;p.id=id;p.title=id;p.defaultLeafId=leaf;
        p.preferredWidth=pw;p.preferredHeight=ph;p.minWidth=pw<150?pw:150;
        p.minHeight=90;p.floatable=canFloat;p.closable=canFloat;
        Check(SubspaceDockSystem::RegisterPanel(w,p),"register panel");
    };
    add("viewport","center",900,600,false);
    add("tool_rail","tool_left",48,420);
    add("asset_browser","bottom",900,220);
    add("outliner","right_top",340,240);
    add("properties","right_bottom",380,460);
    return w;
}
}
int main(){
    auto w=Make();Check(SubspaceDockSystem::Validate(w),"initial workspace valid");
    auto layers=SubspaceDockSystem::Materialize(w,1280,740,78);
    const auto original=Find(layers,"viewport")->rect;
    Check(original.x==0&&original.y==78&&original.width==1280&&original.height==662,
          "canvas fills all available space independently");
    Check(Find(layers,"asset_browser")->rect.y<original.y+original.height,
          "asset browser overlays canvas rather than dividing it");
    Check(Find(layers,"tool_rail")->rect.x>=0,"tool rail is overlay with valid anchor");
    const auto outliner=Find(layers,"outliner")->rect;
    for(const auto dims:std::vector<std::pair<int,int>>{{1120,740},{1280,768},{1653,930},{1852,797},{1920,1080}}){
        const auto snapshot=SubspaceDockSystem::Materialize(w,dims.first,dims.second,78);
        const auto* shelf=Find(snapshot,"asset_browser");
        const auto* inspector=Find(snapshot,"properties");
        const auto* canvas=Find(snapshot,"viewport");
        Check(shelf&&inspector&&canvas&&
              shelf->rect.x+shelf->rect.width+4.0f<=inspector->rect.x,
              "default overlay shelf and docked Properties do not intersect");
        Check(canvas&&canvas->rect.x==0&&canvas->rect.width==dims.first,
              "viewport continues behind independent dock anchors");
    }
    Check(ShipyardDockPointerSystem::CoversFloatingPanel(w,1280,740,78,
        outliner.x+15,outliner.y+85),"docked tool body occludes canvas selection");
    Check(!ShipyardDockPointerSystem::CoversFloatingPanel(w,1280,740,78,640,165),
        "uncovered canvas remains pickable");
    Check(SubspaceDockSystem::FloatPanel(w,"asset_browser",{250,200,600,300}),"float assets");
    Check(Same(FindLive(w,"viewport").rect,original),
          "canvas invariant after floating assets");
    Check(SubspaceDockSystem::FloatPanel(w,"outliner",{300,150,320,240}),"float outliner");
    Check(SubspaceDockSystem::FloatPanel(w,"properties",{400,170,380,400}),"float properties");
    Check(SubspaceDockSystem::FloatPanel(w,"tool_rail",{120,120,48,400}),"float rail");
    layers=SubspaceDockSystem::Materialize(w,1280,740,78);
    Check(Same(Find(layers,"viewport")->rect,original),"all floating tools do not change viewport");
    Check(Find(layers,"tool_rail")->floating,"tool rail is actually floating");
    Check(SubspaceDockSystem::ClosePanel(w,"asset_browser"),"hide assets");
    Check(SubspaceDockSystem::ClosePanel(w,"properties"),"hide properties");
    Check(Same(FindLive(w,"viewport").rect,original),
          "hidden panels leave no reserved holes");
    Check(SubspaceDockSystem::DockPanel(w,"tool_rail","tool_left"),"redock tool rail");
    Check(!FindLive(w,"tool_rail").floating,
          "tool rail restores anchored position");
    ShipyardDockPointerSystem pointer;
    const auto rail=FindLive(w,"tool_rail").rect;
    Check(pointer.Begin(w,1280,740,78,rail.x+13,rail.y+8),"rail header begins drag");
    Check(pointer.Drag(w,80,90,1280,740,78),"rail moves through mouse drag");
    Check(FindLive(w,"tool_rail").floating,
          "rail drag actually undocks");
    Check(pointer.End(w,420,400,1280,740,78),"end rail drag over canvas");
    Check(SubspaceDockSystem::Validate(w),"no double panel ownership after gesture");
    const auto temp=std::filesystem::temp_directory_path()/"subspace_overlay_foundation_test.layout";
    std::error_code ec;std::filesystem::remove(temp,ec);auto backup=temp;backup+=".bak";
    std::filesystem::remove(backup,ec);
    std::string error;
    Check(ShipyardOverlayLayoutStore::Save(w,temp,&error),"save layout");
    auto restored=Make();Check(ShipyardOverlayLayoutStore::Load(restored,temp,&error),"restore layout");
    Check(SubspaceDockSystem::Serialize(w)==SubspaceDockSystem::Serialize(restored),
          "restored layout is identical to saved workspace");
    Check(Same(FindLive(restored,"viewport").rect,original),
          "restart does not shrink viewport");
    {std::ofstream corrupt(temp,std::ios::binary|std::ios::trunc);corrupt<<"corrupt";}
    auto safe=Make();Check(!ShipyardOverlayLayoutStore::Load(safe,temp,&error),
         "malformed layout rejected without modifying default");
    Check(SubspaceDockSystem::Validate(safe),"defaults survive corrupt layout");
    std::filesystem::remove(temp,ec);std::filesystem::remove(backup,ec);
    std::cout<<"Shipyard overlay foundation: "<<checks<<" checks passed\n";
}
