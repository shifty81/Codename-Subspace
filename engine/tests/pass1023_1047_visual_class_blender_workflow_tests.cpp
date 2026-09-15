#include "content/ShipyardModuleSystem.h"
#include "content/UniversalKitbashAuthority.h"
#include "ship_editor/ShipyardDragDropSystem.h"
#include "ships/ShipClassGenerationAuthoritySystem.h"
#include "ships/ShipClassRoleSystem.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

using namespace subspace;
namespace {
int failures=0, assertions=0;
void Check(bool ok,const char* name){++assertions;std::cout<<(ok?"[PASS] ":"[FAIL] ")<<name<<"\n";if(!ok)++failures;}
bool Near(float a,float b,float e=.75f){return std::fabs(a-b)<=e;}
ShipyardModuleRecord Module(std::string id,ShipyardModuleSemantic semantic,ShipyardModuleSize size,float halfLength=2.0f){
    ShipyardModuleRecord r;r.source.moduleId=std::move(id);r.source.halfWidth=1.5f;r.source.halfLength=halfLength;r.source.halfHeight=1.0f;
    r.semantic=semantic;r.size=size;r.moduleClass=semantic==ShipyardModuleSemantic::HullMid?ShipyardModuleClass::Hull:ShipyardModuleClass::Component;
    r.partRole=semantic==ShipyardModuleSemantic::HullMid?ShipyardPartRole::PrimaryHull:ShipyardPartRole::StructuralFrame;
    r.primaryHull=semantic==ShipyardModuleSemantic::HullMid;r.generatorEligible=true;r.placementRole="STRUCTURAL";r.preferredMountFace="AFT";r.mountFaceConfidence=1.0f;
    auto contact=[](Vector3 point,Vector3 normal,float area){VisualModuleSurfaceContact c;c.point=point;c.normal=normal;c.supportingArea=area;c.confidence=.95f;c.valid=true;return c;};
    const float w=r.source.halfWidth,l=r.source.halfLength,h=r.source.halfHeight;
    r.source.forwardSurface=contact({0,l,0},{0,1,0},w*h*4.0f);r.source.aftSurface=contact({0,-l,0},{0,-1,0},w*h*4.0f);
    r.source.portSurface=contact({-w,0,0},{-1,0,0},l*h*4.0f);r.source.starboardSurface=contact({w,0,0},{1,0,0},l*h*4.0f);
    r.source.dorsalSurface=contact({0,0,h},{0,0,1},w*l*4.0f);r.source.ventralSurface=contact({0,0,-h},{0,0,-1},w*l*4.0f);
    r.sockets=ShipyardModuleSystem::BuildSockets(r.source,r.semantic);return r;
}
std::size_t CountSockets(const ShipyardModuleRecord& r,const std::string& prefix){return static_cast<std::size_t>(std::count_if(r.sockets.begin(),r.sockets.end(),[&](const auto&s){return s.name.rfind(prefix,0)==0;}));}
ProceduralShipVisualRecipe Recipe(const ShipyardModuleRecord& r,std::size_t count=1){
    ProceduralShipVisualRecipe q;q.recipeId="class_test";q.modules.reserve(count);
    for(std::size_t i=0;i<count;++i){VisualModulePlacement p;p.moduleId=r.source.moduleId;p.y=static_cast<float>(i)*4.0f;q.modules.push_back(p);}return q;
}
}

int main(){
    std::cout<<"[Pass1023-1047 Visual Fidelity + Class/Scale + Blender Shipyard Workflow]\n";
    using U=UniversalSizeClass;

    Check(UniversalKitbashAuthority::NominalScale(U::XS)<UniversalKitbashAuthority::NominalScale(U::S)&&
          UniversalKitbashAuthority::NominalScale(U::S)<UniversalKitbashAuthority::NominalScale(U::M)&&
          UniversalKitbashAuthority::NominalScale(U::M)<UniversalKitbashAuthority::NominalScale(U::L)&&
          UniversalKitbashAuthority::NominalScale(U::L)<UniversalKitbashAuthority::NominalScale(U::XL),"1023 XS-S-M-L-XL nominal module scales are strictly increasing");
    Check(ShipClassRoleSystem::Envelope(ShipClass::Frigate).nominalLengthMeters<ShipClassRoleSystem::Envelope(ShipClass::Cruiser).nominalLengthMeters&&
          ShipClassRoleSystem::Envelope(ShipClass::Cruiser).nominalLengthMeters<ShipClassRoleSystem::Envelope(ShipClass::Battleship).nominalLengthMeters,"1024 combat ship classes have physically increasing hull envelopes");
    Check(ShipClassRoleSystem::Envelope(ShipClass::Capital).nominalLengthMeters>ShipClassRoleSystem::Envelope(ShipClass::Battleship).nominalLengthMeters,"1025 capital hull envelope is physically larger than battleship");
    Check(ShipClassRoleSystem::ClampModuleSize(ShipClass::Frigate,U::XL)==U::M,"1026 frigate rejects structural XL and clamps to its class envelope");
    Check(ShipClassRoleSystem::ClampModuleSize(ShipClass::Capital,U::XS)==U::M,"1027 capital rejects structural XS and clamps to its class envelope");
    Check(ShipClassGenerationAuthoritySystem::TargetLengthMeters(ShipClass::Frigate,U::XS)<ShipClassGenerationAuthoritySystem::TargetLengthMeters(ShipClass::Frigate,U::S)&&
          ShipClassGenerationAuthoritySystem::TargetLengthMeters(ShipClass::Frigate,U::S)<ShipClassGenerationAuthoritySystem::TargetLengthMeters(ShipClass::Frigate,U::M),"1028 XS-S-M generation changes physical length inside Frigate class");
    Check(ShipClassGenerationAuthoritySystem::TargetLengthMeters(ShipClass::Battleship,U::M)<ShipClassGenerationAuthoritySystem::TargetLengthMeters(ShipClass::Battleship,U::L)&&
          ShipClassGenerationAuthoritySystem::TargetLengthMeters(ShipClass::Battleship,U::L)<ShipClassGenerationAuthoritySystem::TargetLengthMeters(ShipClass::Battleship,U::XL),"1029 M-L-XL generation changes physical length inside Battleship class");

    auto hull=Module("hull",ShipyardModuleSemantic::HullMid,ShipyardModuleSize::M,2.0f);std::vector<ShipyardModuleRecord> catalog{hull};
    auto measured=Recipe(hull,2);
    Check(Near(ShipClassGenerationAuthoritySystem::MeasureLengthMeters(measured,catalog),8.0f,.01f),"1030 generator measures real placed authored-module hull length");
    auto frigateXs=measured;const float beforeUnsafe=ShipClassGenerationAuthoritySystem::MeasureLengthMeters(frigateXs,catalog);auto frXs=ShipClassGenerationAuthoritySystem::ApplyAndStamp(frigateXs,catalog,ShipClass::Frigate,U::XS);
    Check(!frXs.valid&&frXs.topologyRegenerationRequired&&Near(ShipClassGenerationAuthoritySystem::MeasureLengthMeters(frigateXs,catalog),beforeUnsafe,.01f),"1031 unsafe whole-ship inflation is rejected and does not mutate the recipe");
    auto frame=Module("frame",ShipyardModuleSemantic::StructuralFrame,ShipyardModuleSize::M,2.0f);std::vector<ShipyardModuleRecord> classCatalog{hull,frame};
    ProceduralShipVisualRecipe frigateSized;frigateSized.recipeId="frigate_sized";for(int i=0;i<6;++i){VisualModulePlacement p;p.moduleId=(i==0?hull.source.moduleId:frame.source.moduleId);p.y=-18.0f+7.2f*static_cast<float>(i);frigateSized.modules.push_back(p);}
    auto frSafe=ShipClassGenerationAuthoritySystem::ApplyAndStamp(frigateSized,classCatalog,ShipClass::Frigate,U::XS);
    Check(frSafe.valid&&Near(ShipClassGenerationAuthoritySystem::MeasureLengthMeters(frigateSized,classCatalog),40.0f,.05f),"1032 correctly sized Frigate topology certifies without destructive scaling");
    auto rotated=Recipe(hull,1);rotated.modules.front().yawDegrees=90.0f;const auto rotatedBounds=ShipClassGenerationAuthoritySystem::MeasureBoundsMeters(rotated,catalog);
    Check(rotatedBounds.valid&&Near(rotatedBounds.size.y,3.0f,.05f)&&Near(rotatedBounds.size.x,4.0f,.05f),"1033 class measurement uses transformed 3D module bounds rather than Y-only authored extents");
    auto battleshipM=measured;auto bsM=ShipClassGenerationAuthoritySystem::ApplyAndStamp(battleshipM,catalog,ShipClass::Battleship,U::M);
    Check(!bsM.valid&&bsM.topologyRegenerationRequired&&bsM.appliedScale>ShipClassGenerationAuthoritySystem::MaximumFinalCorrectionScale,"1034 Battleship generation must add topology/modules instead of inflating a small draft");
    auto tooMany=Recipe(hull,ShipClassRoleSystem::Envelope(ShipClass::Frigate).maximumModules+1);const auto over=ShipClassGenerationAuthoritySystem::Resolve(tooMany,catalog,ShipClass::Frigate,U::S);
    Check(!over.valid&&!over.errors.empty(),"1035 class module-count maximum is a hard generation constraint");
    const auto clamped=ShipClassGenerationAuthoritySystem::Resolve(measured,catalog,ShipClass::Frigate,U::XL);
    Check(clamped.resolvedSize==U::M&&!clamped.warnings.empty(),"1036 impossible requested module size is reported rather than silently ignored");
    Check(frigateXs.shipClassId=="FRIGATE"&&frigateXs.lineageAuthority=="CLASS_HULL_TOPOLOGY_V4","1037 class/topology authority preserves compatible lineage stamping while rejecting unsafe scaling");

    Check(hull.sockets.size()>=30,"1038 hull modules expose a dense editor snap vocabulary");
    Check(CountSockets(hull,"surface_dorsal_")>=9,"1039 dorsal surface exposes a 3x3 editor snap grid");
    Check(CountSockets(hull,"surface_ventral_")>=9,"1040 ventral surface exposes a 3x3 editor snap grid");
    Check(CountSockets(hull,"surface_forward_")>=3&&CountSockets(hull,"surface_aft_")>=3,"1041 fore/aft faces expose multiple editor snap points");

    auto child=Module("child",ShipyardModuleSemantic::HullMid,ShipyardModuleSize::M,1.0f);child.primaryHull=false;child.partRole=ShipyardPartRole::StructuralFrame;
    auto baseRecipe=Recipe(hull,1);const auto beforeCount=baseRecipe.modules.size();
    auto preview=ShipyardDragDropSystem::Begin(child,{hull,child},baseRecipe,U::M);
    Check(preview.active&&preview.candidates.size()>4,"1042 dragged part discovers multiple candidate attachment points");
    Check(ShipyardDragDropSystem::Stage(preview)&&preview.staged&&baseRecipe.modules.size()==beforeCount,"1043 mouse release stages the part without mutating the ship recipe");
    const float x0=preview.ghost.x;Check(ShipyardDragDropSystem::TranslateStaged(preview,{1.0f,0.0f,0.0f},true,.25f)&&preview.ghost.x!=x0&&preview.freePlacement,"1044 staged part can move freely before attachment");
    const float yaw0=preview.ghost.yawDegrees;Check(ShipyardDragDropSystem::RotateStaged(preview,{0.0f,15.0f,0.0f},true,15.0f)&&preview.ghost.yawDegrees!=yaw0,"1045 staged part can rotate before attachment");
    const float scale0=preview.ghost.scaleX;Check(ShipyardDragDropSystem::ScaleStaged(preview,.25f)&&preview.ghost.scaleX>scale0,"1046 staged part can be resized before attachment");
    auto snapPreview=ShipyardDragDropSystem::Begin(child,{hull,child},baseRecipe,U::M);ShipyardDragDropSystem::Stage(snapPreview);const int snap0=snapPreview.selectedCandidate;
    Check(snapPreview.candidates.size()>1&&ShipyardDragDropSystem::CycleCandidate(snapPreview,1)&&snapPreview.selectedCandidate!=snap0,"1047 staged workflow can cycle among snap candidates before explicit confirmation");

    std::cout<<"Pass1023-1047 assertions: "<<(assertions-failures)<<" / "<<assertions<<" passed\n";
    return failures?1:0;
}
