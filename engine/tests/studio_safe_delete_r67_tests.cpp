#include "ship_editor/ShipyardBuilderSystem.h"
#include <iostream>
using namespace subspace;
namespace {int failures=0, assertions=0; void Check(bool ok,const char* n){++assertions;if(!ok){++failures;std::cerr<<"[FAIL] "<<n<<"\n";}else std::cout<<"[PASS] "<<n<<"\n";} ShipyardModuleRecord Make(const char* id){ShipyardModuleRecord r;r.source.moduleId=id;r.moduleClass=ShipyardModuleClass::Hull;r.semantic=ShipyardModuleSemantic::HullMid;r.size=ShipyardModuleSize::M;r.generatorEligible=true;return r;}}
int main(){
    auto a=Make("root"),b=Make("middle"),c=Make("child"); ProceduralShipVisualRecipe recipe;
    VisualModulePlacement pa;pa.moduleId="root";VisualModulePlacement pb;pb.moduleId="middle";VisualModulePlacement pc;pc.moduleId="child";recipe.modules={pa,pb,pc};
    recipe.attachments.push_back({0,1,"a","b",0,true});recipe.attachments.push_back({1,2,"a","b",0,true});
    ShipyardBuilderSystem s;s.Initialize({a,b,c},recipe);s.Activate(ShipyardBuilderCommand::SelectPlaced,1);
    Check(s.Activate(ShipyardBuilderCommand::DeleteSelectionSafe),"safe Delete succeeds");
    Check(s.Recipe().modules.size()==2,"safe Delete removes only the selected module");
    Check(s.Recipe().modules[0].moduleId=="root"&&s.Recipe().modules[1].moduleId=="child","descendant survives normal Delete");
    Check(s.Recipe().attachments.empty(),"edges incident to removed parent are removed, preserved child becomes detached");
    Check(s.Model().selectedPlacedModule==1,"selection moves to surviving item at removed slot");
    Check(s.AuthoringUndoCount()==1,"safe Delete is one undo transaction");
    Check(s.UndoAuthoring()&&s.Recipe().modules.size()==3&&s.Recipe().attachments.size()==2,"Undo restores deleted module and attachment graph");
    Check(s.RedoAuthoring()&&s.Recipe().modules.size()==2,"Redo reapplies safe Delete");
    std::cout<<"R67 safe-delete assertions: "<<(assertions-failures)<<" / "<<assertions<<" passed\n";return failures?1:0;
}
