#include "ship_editor/ShipyardBuilderSystem.h"
#include <algorithm>
#include <iostream>
#include <vector>

using namespace subspace;

namespace {
int failures=0, assertions=0;
void Check(bool ok,const char* name){++assertions;if(!ok){++failures;std::cerr<<"[FAIL] "<<name<<"\n";}else std::cout<<"[PASS] "<<name<<"\n";}
ShipyardModuleRecord Make(ShipyardModuleClass cls,int i){
    ShipyardModuleRecord r;r.source.moduleId="pass585_part_"+std::to_string(static_cast<int>(cls))+"_"+std::to_string(i);
    r.moduleClass=cls;r.semantic=cls==ShipyardModuleClass::Propulsion?ShipyardModuleSemantic::MainEngine:ShipyardModuleSemantic::HullMid;
    r.size=ShipyardModuleSize::M;r.generatorEligible=true;
    ShipyardAssemblySocket socket;socket.name="mount";socket.type="structural";socket.dirY=-1.0f;r.sockets.push_back(socket);
    return r;
}
ShipyardBuilderSystem Builder(){
    std::vector<ShipyardModuleRecord> cat;for(int c=0;c<8;++c)for(int i=0;i<13;++i)cat.push_back(Make(static_cast<ShipyardModuleClass>(c),i));
    ProceduralShipVisualRecipe recipe;VisualModulePlacement p;p.moduleId=cat.front().source.moduleId;recipe.modules.push_back(p);
    ShipyardBuilderSystem b;b.Initialize(cat,recipe);return b;
}
}

int main(){
    const auto uhd=ShipyardBuilderSystem::Layout(3840,2160);
    Check(uhd.valid&&uhd.uiScale>=1.5f,"Pass585 project UI is resolution-aware at 4K");
    Check(uhd.leftWidth>=800.0f&&uhd.rightWidth>=1000.0f,"Pass586 Shipyard panes visibly scale at 4K");
    Check(uhd.right>uhd.left+uhd.leftWidth+900.0f,"Pass586 scaled UI still preserves a large center viewport");

    auto b=Builder();
    auto build=ShipyardBuilderSystem::BuildControls(b.Model(),1920,1080);
    const auto socketsTab=std::find_if(build.begin(),build.end(),[](const auto& c){return c.command==ShipyardBuilderCommand::InspectorSockets&&c.width>0;});
    Check(socketsTab!=build.end()&&socketsTab->label=="SOCKETS","Pass587 SOCKETS is restored as a visible one-click Shipyard tab");
    Check(socketsTab!=build.end()&&socketsTab->width>=90.0f,"Pass587 SOCKETS tab has a practical click target");

    int visualCards=0;float minCardHeight=1e9f;
    for(const auto& c:build)if(c.command==ShipyardBuilderCommand::SelectModule&&c.width>0){++visualCards;minCardHeight=std::min(minCardHeight,c.height);}
    Check(visualCards>0&&visualCards<=5,"Pass588 module library is paged as visual inventory cards instead of ten text rows");
    Check(minCardHeight>=58.0f,"Pass588 module cards reserve thumbnail-scale vertical space");

    Check(b.Activate(ShipyardBuilderCommand::InspectorSockets)&&b.Model().inspectorTab==ShipyardInspectorTab::Sockets,"Pass589 visible SOCKETS tab opens reusable socket authoring");
    auto socketControls=ShipyardBuilderSystem::BuildControls(b.Model(),1920,1080);
    const auto visible=[&](ShipyardBuilderCommand cmd){return std::any_of(socketControls.begin(),socketControls.end(),[&](const auto& c){return c.command==cmd&&c.width>0&&c.height>0;});};
    Check(visible(ShipyardBuilderCommand::AddSocket)&&visible(ShipyardBuilderCommand::SaveSocketOverrides),"Pass590 socket correction actions remain visibly reachable");
    Check(visible(ShipyardBuilderCommand::ToolMove)&&visible(ShipyardBuilderCommand::ToolRotate),"Pass590 socket MOVE/ROTATE direct manipulation remains reachable");

    Check(b.Activate(ShipyardBuilderCommand::WorkspaceAuthoring)&&b.Model().inspectorTab==ShipyardInspectorTab::Authoring,"Pass591 AUTHORING now opens Teach-PCG definition authoring rather than stealing the socket page");
    auto authoring=ShipyardBuilderSystem::BuildControls(b.Model(),1920,1080);
    Check(std::any_of(authoring.begin(),authoring.end(),[](const auto& c){return c.command==ShipyardBuilderCommand::InspectorSockets&&c.width>0;}),"Pass592 Authoring preserves a visible route back to SOCKETS");

    b.Activate(ShipyardBuilderCommand::WorkspaceBuild);
    auto normal=ShipyardBuilderSystem::BuildControls(b.Model(),1920,1080);
    Check(!std::any_of(normal.begin(),normal.end(),[](const auto& c){return c.command==ShipyardBuilderCommand::AddSocket&&c.width>0;}),"Pass593 normal BUILD does not expose raw socket mutation controls");
    Check(ShipyardBuilderSystem::Layout(1280,768).right>ShipyardBuilderSystem::Layout(1280,768).left+ShipyardBuilderSystem::Layout(1280,768).leftWidth+300.0f,"Pass594 responsive layout preserves legacy minimum center-canvas contract");

    std::cout<<"Pass585-594 assertions: "<<assertions-failures<<" / "<<assertions<<" passed\n";
    return failures?1:0;
}
