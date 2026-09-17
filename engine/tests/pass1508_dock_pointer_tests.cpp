#include "ship_editor/ShipyardDockPointerSystem.h"
#include <cstdlib>
#include <iostream>
#include <string>
using namespace subspace;
namespace {
int assertions=0;
void Check(bool valid,const char* reason){
    if(!valid){std::cerr<<"[FAIL] "<<reason<<'\n';std::exit(EXIT_FAILURE);}
    ++assertions;std::cout<<"[PASS] "<<reason<<'\n';
}
SubspaceDockWorkspace Workspace(){
    auto w=SubspaceDockSystem::CreateMinimalWorkspace("pointer");
    SubspaceDockPanel p;p.id="outliner";p.title="Outliner";p.defaultLeafId="right";
    p.preferredWidth=360;p.preferredHeight=280;
    Check(SubspaceDockSystem::RegisterPanel(w,p),"register Outliner");
    p.id="properties";p.title="Properties";
    Check(SubspaceDockSystem::RegisterPanel(w,p),"register Properties");
    p.id="assets";p.title="Assets";p.defaultLeafId="bottom";
    Check(SubspaceDockSystem::RegisterPanel(w,p),"register Assets");
    return w;
}
const SubspaceFloatingPanel* Floating(const SubspaceDockWorkspace& w,const char* id){
    for(const auto& f:w.floatingPanels)if(f.panelId==id)return &f;
    return nullptr;
}
}
int main(){
    auto w=Workspace();ShipyardDockPointerSystem pointer;
    Check(!pointer.Begin(w,1280,740,60,1270,89),"header button strip is never captured");
    Check(!pointer.Begin(w,1280,740,60,1060,165),"content area is never captured");
    Check(pointer.Begin(w,1280,740,60,1050,84),"dock title begins capture");
    Check(pointer.Active()&&!pointer.Dragged(),"press is inert until real movement");
    Check(pointer.Drag(w,2,1,1280,740,60),"small motion handled");
    Check(!Floating(w,"outliner"),"sub-threshold motion does not detach");
    Check(!pointer.End(w,1052,85,1280,740,60),"short click does not mutate dock");
    Check(SubspaceDockSystem::Validate(w),"dock remains valid after header click");
    Check(pointer.Begin(w,1280,740,60,1050,84),"begin dock-to-float gesture");
    Check(pointer.Drag(w,-190,5,1280,740,60),"drag clears detach threshold");
    Check(Floating(w,"outliner")!=nullptr,"dock title drag creates real floating panel");
    Check(pointer.End(w,860,89,1280,740,60),"release floating panel on viewport");
    Check(Floating(w,"outliner")!=nullptr,"viewport drop keeps panel floating");
    Check(SubspaceDockSystem::Validate(w),"workspace valid after float");
    const auto* floating=Floating(w,"outliner");
    Check(floating->rect.width>=359.0f && floating->rect.height>=279.0f,
          "undocked panel restores usable preferred dimensions, not a narrow leaf width");
    Check(ShipyardDockPointerSystem::CoversFloatingPanel(w,1280,740,60,
          floating->rect.x+25.0f,floating->rect.y+55.0f),
          "floating panel body blocks viewport picking");
    Check(!ShipyardDockPointerSystem::CoversFloatingPanel(w,1280,740,60,10.0f,110.0f),
          "viewport outside floating panel remains selectable");
    const float fx=floating->rect.x,fy=floating->rect.y;
    Check(pointer.Begin(w,1280,740,60,fx+14,fy+9),"floating header captures move");
    Check(pointer.Drag(w,-35,23,1280,740,60),"floating header actually moves");
    Check(Floating(w,"outliner")->rect.x<fx,"floating X changes");
    Check(pointer.End(w,fx-21,fy+32,1280,740,60),"finish floating move");
    const auto f=*Floating(w,"outliner");
    Check(pointer.Begin(w,1280,740,60,f.rect.x+f.rect.width-4,f.rect.y+f.rect.height-4),"bottom right captures resize");
    Check(pointer.Drag(w,24,35,1280,740,60),"resize gesture processed");
    Check(Floating(w,"outliner")->rect.width>f.rect.width,"resize expands width");
    Check(Floating(w,"outliner")->rect.height>f.rect.height,"resize expands height");
    Check(pointer.End(w,0,0,1280,740,60),"resize release completes");
    Check(Floating(w,"outliner")!=nullptr,"resize never docks panel");
    const auto revised=*Floating(w,"outliner");
    Check(pointer.Begin(w,1280,740,60,revised.rect.x+11,revised.rect.y+7),"capture floating panel for drag dock");
    Check(pointer.Drag(w,-20,15,1280,740,60),"drag float toward bottom");
    Check(pointer.End(w,400,700,1280,740,60),"drop on bottom leaf");
    Check(!Floating(w,"outliner"),"drop moves panel to dock leaf");
    const auto layouts=SubspaceDockSystem::Materialize(w,1280,740,60);
    bool isBottom=false;for(const auto& l:layouts)if(l.panelId=="outliner"&&l.leafId=="bottom")isBottom=true;
    Check(isBottom,"redocked Outliner occupies bottom leaf");
    Check(SubspaceDockSystem::Validate(w),"one owner per panel after redock");
    const std::string saved=SubspaceDockSystem::Serialize(w);
    SubspaceDockWorkspace recovered;
    Check(SubspaceDockSystem::Deserialize(saved,recovered),"redocked layout persists and restores");
    ShipyardDockPointerSystem absent;
    Check(!absent.Drag(recovered,4,5,1280,740,60),"drag without capture is rejected");
    Check(!absent.End(recovered,100,100,1280,740,60),"release without capture is rejected");
    std::cout<<"PASS1508: "<<assertions<<" assertions passed\n";
}
