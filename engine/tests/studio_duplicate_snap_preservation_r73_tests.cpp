#include "ship_editor/ShipyardBuilderSystem.h"

#include <cmath>
#include <iostream>

using namespace subspace;
namespace {
int failures=0, assertions=0;
void Check(bool ok,const char* name){++assertions;if(!ok){++failures;std::cerr<<"[FAIL] "<<name<<"\n";}else std::cout<<"[PASS] "<<name<<"\n";}
bool Near(float a,float b,float e=.0006f){return std::fabs(a-b)<=e;}
ShipyardAssemblySocket Socket(const char* name,const char* type,float x){
    ShipyardAssemblySocket s;s.name=name;s.type=type;s.x=x;s.dirY=1.0f;return s;
}
ShipyardModuleRecord Parent(){
    ShipyardModuleRecord r;r.source.moduleId="snap_parent";r.moduleClass=ShipyardModuleClass::Hull;
    r.semantic=ShipyardModuleSemantic::HullMid;r.size=ShipyardModuleSize::M;r.generatorEligible=true;
    r.sockets={Socket("aft_left","hull_aft",-1.0f),Socket("aft_right","hull_aft",1.0f)};return r;
}
ShipyardModuleRecord Child(){
    ShipyardModuleRecord r;r.source.moduleId="dup_child";r.moduleClass=ShipyardModuleClass::Hull;
    r.semantic=ShipyardModuleSemantic::HullMid;r.size=ShipyardModuleSize::M;r.generatorEligible=true;
    r.sockets={Socket("forward","hull_forward",0.0f)};return r;
}
}

int main(){
    auto parent=Parent(),child=Child();
    ProceduralShipVisualRecipe recipe;
    VisualModulePlacement p0;p0.moduleId=parent.source.moduleId;
    VisualModulePlacement p1;p1.moduleId=child.source.moduleId;p1.x=4.0f;p1.y=3.0f;p1.z=.5f;
    p1.scaleX=1.35f;p1.scaleY=.72f;p1.scaleZ=1.18f;p1.yawDegrees=29.0f;
    p1.material=SpaceMaterialKind::Canopy;p1.mirrorX=true;p1.mirrorZ=true;p1.sourceMaterialsEnabled=false;
    recipe.modules={p0,p1};
    ShipyardBuilderSystem b;b.Initialize({parent,child},recipe);b.Activate(ShipyardBuilderCommand::SelectPlaced,1);
    Check(b.Activate(ShipyardBuilderCommand::DuplicateSelection),"R73 duplicate begins as staged placement");
    Check(b.Model().dragPreview.candidates.size()>=2,"R73 duplicate retains available socket candidates");
    Check(b.Activate(ShipyardBuilderCommand::NextSnapCandidate),"R73 staged duplicate can choose a socket candidate");
    const auto& g=b.Model().dragPreview.ghost;
    const auto selected=b.Model().dragPreview.selectedCandidate;
    const bool validCandidate=selected>=0&&static_cast<std::size_t>(selected)<b.Model().dragPreview.candidates.size();
    Check(validCandidate,"R82R1 snapped duplicate has authoritative candidate");
    if(!validCandidate)return 1; // Fail closed: do not index a missing candidate.
    // Snapshot before confirmation: CommitCatalogDrag clears dragPreview and
    // references into its candidate vector would become dangling (R82R4 false RED).
    const auto expected=b.Model().dragPreview.candidates[static_cast<std::size_t>(selected)].placement;
    const auto staged=g;
    Check(Near(g.scaleX,expected.scaleX)&&Near(g.scaleY,expected.scaleY)&&Near(g.scaleZ,expected.scaleZ),"R82R1 snap keeps solver-compatible scale");
    Check(g.material==p1.material&&g.sourceMaterialsEnabled==p1.sourceMaterialsEnabled,"R82R1 duplicate snap preserves material authority");
    Check(g.mirrorX==expected.mirrorX&&g.mirrorY==expected.mirrorY&&g.mirrorZ==expected.mirrorZ,"R82R1 snap keeps solver-compatible mirror state");
    Check(b.Activate(ShipyardBuilderCommand::ConfirmPlacement),"R73 snapped duplicate confirms");
    const auto& committed=b.Recipe().modules.back();
    Check(Near(committed.scaleX,expected.scaleX)&&Near(committed.scaleY,expected.scaleY)&&Near(committed.scaleZ,expected.scaleZ)&&
          Near(committed.scaleX,staged.scaleX)&&Near(committed.scaleY,staged.scaleY)&&Near(committed.scaleZ,staged.scaleZ),
          "R82R5 committed snap keeps solver-compatible staged scale");
    Check(committed.material==p1.material&&committed.material==staged.material&&
          committed.sourceMaterialsEnabled==staged.sourceMaterialsEnabled&&!committed.sourceMaterialsEnabled,
          "R82R5 committed snap keeps duplicate material state");
    Check(committed.mirrorX==staged.mirrorX&&committed.mirrorY==staged.mirrorY&&committed.mirrorZ==staged.mirrorZ&&
          Near(committed.x,staged.x)&&Near(committed.y,staged.y)&&Near(committed.z,staged.z)&&
          Near(committed.yawDegrees,staged.yawDegrees)&&Near(committed.pitchDegrees,staged.pitchDegrees)&&
          Near(committed.rollDegrees,staged.rollDegrees)&&committed.moduleId==staged.moduleId,
          "R82R5 confirmed placement exactly matches staged geometry and identity");
    Check(b.AuthoringUndoCount()==1&&b.UndoAuthoring()&&b.Recipe().modules.size()==2,"R82R5 snapped duplicate remains one undo transaction");
    std::cout<<"R82R5 duplicate-snap assertions: "<<(assertions-failures)<<" / "<<assertions<<" passed\n";
    return failures?1:0;
}
