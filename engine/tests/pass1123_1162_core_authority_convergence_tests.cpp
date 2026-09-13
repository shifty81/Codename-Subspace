#include "editor/EditorTransformSpaceSystem.h"
#include "editor/ForgeWorkspaceSystem.h"
#include "generator/GeneratorParitySystem.h"
#include "ships/ShipClassGenerationAuthoritySystem.h"
#include "ships/ShipClassRoleSystem.h"

#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using namespace subspace;
namespace {
int failures=0, assertions=0;
void Check(bool ok,const char* name){++assertions;std::cout<<(ok?"[PASS] ":"[FAIL] ")<<name<<"\n";if(!ok)++failures;}
bool Near(float a,float b,float e=.025f){return std::fabs(a-b)<=e;}
ShipyardModuleRecord Module(const char* id,float hw=1.5f,float hl=2.0f,float hh=1.0f){ShipyardModuleRecord r;r.source.moduleId=id;r.source.halfWidth=hw;r.source.halfLength=hl;r.source.halfHeight=hh;r.moduleClass=ShipyardModuleClass::Hull;r.semantic=ShipyardModuleSemantic::HullMid;r.size=ShipyardModuleSize::M;r.builderCategory=ShipyardPartCategory::Hull;r.partRole=ShipyardPartRole::PrimaryHull;r.primaryHull=true;r.generatorEligible=true;r.placementRole="STRUCTURAL";return r;}
ProceduralShipVisualRecipe Recipe(const ShipyardModuleRecord& m,int count,float lo,float hi){ProceduralShipVisualRecipe r;r.recipeId="draft";for(int i=0;i<count;++i){VisualModulePlacement p;p.moduleId=m.source.moduleId;float t=count<=1?.5f:float(i)/float(count-1);p.y=lo+(hi-lo)*t;r.modules.push_back(p);}return r;}
}
int main(){
    std::cout<<"[Pass1123-1162 Core Authority Convergence]\n";
    const float ninety=EditorTransformSpaceSystem::DegreesToRadians(90.0f);
    Check(Near(ninety,1.5707963f),"1123 degree/radian conversion is deterministic");
    Check(Near(EditorTransformSpaceSystem::RenderedRootYawRadians(.25f,90.0f),.25f+ninety),"1124 rendered root yaw includes runtime and forward-visual yaw");
    auto d0=EditorTransformSpaceSystem::WorldToAssemblyDelta({1,0,0},0);Check(Near(d0.x,1)&&Near(d0.y,0),"1125 zero-root editor movement preserves world direction");
    auto d90=EditorTransformSpaceSystem::WorldToAssemblyDelta({1,0,0},ninety);Check(Near(d90.x,0)&&Near(d90.y,-1),"1126 view-space movement inversely removes ship facing");
    VisualModulePlacement p;p.yawDegrees=90;p.scaleX=p.scaleY=p.scaleZ=1;auto local=EditorTransformSpaceSystem::AssemblyToModuleLocalDelta({1,0,0},p);Check(Near(local.x,0)&&Near(local.y,-1),"1127 local transform explicitly removes selected-module yaw");
    p={};p.scaleX=2;p.scaleY=4;p.scaleZ=5;local=EditorTransformSpaceSystem::AssemblyToModuleLocalDelta({2,4,5},p);Check(Near(local.x,1)&&Near(local.y,1)&&Near(local.z,1),"1128 local transform accounts for selected-module scale");
    p.mirrorX=true;local=EditorTransformSpaceSystem::AssemblyToModuleLocalDelta({2,0,0},p);Check(local.x<0,"1129 local transform accounts for mirrored module axes");
    auto view=EditorTransformSpaceSystem::ResolveTranslation(ShipyardTransformSpace::View,{1,0,0},{9,9,9},ninety,nullptr);Check(Near(view.x,0)&&Near(view.y,-1),"1130 View is the facing-independent default transform space");
    auto ship=EditorTransformSpaceSystem::ResolveTranslation(ShipyardTransformSpace::Ship,{1,0,0},{2,3,4},ninety,nullptr);Check(Near(ship.x,2)&&Near(ship.y,3)&&Near(ship.z,4),"1131 Ship transform remains explicit opt-in behavior");

    GeneratorParityRequest q;q.domain=GeneratorDomain::Ship;q.seed=77123;q.profileId="TEST";q.role="COMBAT";q.shipClass=ShipClass::Frigate;q.size=UniversalSizeClass::S;
    auto qa=GeneratorParitySystem::StableRequestIdentity(q);auto qb=GeneratorParitySystem::StableRequestIdentity(q);Check(qa==qb,"1132 same visible seed produces stable generator identity");
    auto qr=q;qr.seed++;Check(GeneratorParitySystem::StableRequestIdentity(qr)!=qa,"1133 only explicit seed change changes request identity");
    Check(GeneratorParitySystem::Validate(q).valid,"1134 canonical ship request validates");
    auto station=q;station.domain=GeneratorDomain::Station;Check(GeneratorParitySystem::Validate(station).valid,"1135 domain remains a canonical request input independent of workspace");
    Check(std::string(GeneratorParitySystem::DomainId(GeneratorDomain::Ship))=="ship","1136 ship domain id remains stable across clients");
    Check(GeneratorParitySystem::RequestJson(q).find("77123")!=std::string::npos,"1137 serialized request preserves visible seed");

    auto hull=Module("hull");std::vector<ShipyardModuleRecord> catalog{hull};
    auto tiny=Recipe(hull,2,-2,2);auto tr=ShipClassGenerationAuthoritySystem::Resolve(tiny,catalog,ShipClass::Battlecruiser,UniversalSizeClass::L);Check(!tr.valid&&tr.topologyRegenerationRequired,"1138 undersized Battlecruiser still cannot be globally inflated");
    Check(ShipClassGenerationAuthoritySystem::IsSafeDraft(tr),"1139 physically sane topology-incomplete seed may remain a deterministic draft");
    auto flatM=Module("flat",1.5f,2.0f,.02f);std::vector<ShipyardModuleRecord> flatC{flatM};auto flat=Recipe(flatM,2,-2,2);auto fr=ShipClassGenerationAuthoritySystem::Resolve(flat,flatC,ShipClass::Battlecruiser,UniversalSizeClass::L);Check(!ShipClassGenerationAuthoritySystem::IsSafeDraft(fr),"1140 collapsed planar generation can never be surfaced as a draft");
    auto huge=tiny;huge.modules.front().scaleX=28.2f;auto hr=ShipClassGenerationAuthoritySystem::Resolve(huge,catalog,ShipClass::Battlecruiser,UniversalSizeClass::L);Check(!ShipClassGenerationAuthoritySystem::IsSafeDraft(hr),"1141 historical 28x scale failure is rejected from draft path");
    auto sane=Recipe(hull,6,-18,18);auto sr=ShipClassGenerationAuthoritySystem::Resolve(sane,catalog,ShipClass::Frigate,UniversalSizeClass::XS);Check(sr.valid,"1142 class-correct topology still certifies normally");
    Check(ShipClassGenerationAuthoritySystem::MaximumGeneratedInstanceScale<=1.5f,"1143 generated instance scale remains bounded");
    Check(ShipClassGenerationAuthoritySystem::MinimumFinalCorrectionScale>=.85f&&ShipClassGenerationAuthoritySystem::MaximumFinalCorrectionScale<=1.15f,"1144 final class correction remains minor only");

    const auto tmp=std::filesystem::temp_directory_path()/"subspace_pass1123_1162_forge";std::filesystem::create_directories(tmp);
    const auto control=tmp/"project.control.json";
    std::ofstream out(control);out<<R"JSON({"schema":"forge.project.v1","project":{"id":"codename-subspace","name":"Codename Subspace","version":"1162","build":"SUBSPACE-PASS1123-1162-CORE-AUTHORITY-CONVERGENCE"},"commands":[{"key":"status","label":"Status","category":"project","mutates":false,"requiresConfirmation":false,"executable":"pwsh","arguments":["-File","SubspaceTools.ps1","-Action","status"]},{"key":"danger","label":"Danger","category":"publish","mutates":true,"requiresConfirmation":true,"executable":"pwsh","arguments":["-File","SubspaceTools.ps1","-Action","publish"]}]})JSON";out.close();
    const auto forge=ForgeWorkspaceSystem::Load(control);Check(forge.valid,"1145 native Forge panel loads forge.project.v1");
    Check(forge.projectId=="codename-subspace"&&forge.version=="1162","1146 Forge panel preserves project identity/version");
    Check(forge.commands.size()==2,"1147 native Forge panel consumes command registry instead of hardcoding buttons");
    const auto* status=ForgeWorkspaceSystem::Find(forge,"status");Check(status!=nullptr,"1148 registered project command is discoverable by key");
    Check(status&&status->executable=="pwsh","1149 command preserves declared executable verbatim");
    std::string err;Check(status&&ForgeWorkspaceSystem::InterpreterMatchesScript(*status,&err),"1150 PowerShell command accepts pwsh interpreter");
    ForgeWorkspaceCommand wrong=*status;wrong.executable="python.exe";Check(!ForgeWorkspaceSystem::InterpreterMatchesScript(wrong,&err),"1151 Python may not execute a .ps1 project provider");
    Check(!err.empty(),"1152 interpreter mismatch exposes actionable error");
    const auto preview=ForgeWorkspaceSystem::CommandPreview(*status);Check(preview.find("pwsh")!=std::string::npos&&preview.find("SubspaceTools.ps1")!=std::string::npos,"1153 command preview preserves provider executable and script");
    const auto* danger=ForgeWorkspaceSystem::Find(forge,"danger");Check(danger&&danger->requiresConfirmation,"1154 destructive project command retains confirmation policy");
    const auto unrelated=std::filesystem::temp_directory_path()/"subspace_pass1123_1162_unrelated";std::filesystem::remove_all(unrelated);std::filesystem::create_directories(unrelated);auto discovered=ForgeWorkspaceSystem::Discover(unrelated);Check(!discovered.valid,"1155 unrelated path does not fabricate project authority");
    std::filesystem::create_directories(tmp/"nested"/"deeper");discovered=ForgeWorkspaceSystem::Discover(tmp/"nested"/"deeper");Check(discovered.valid,"1156 Forge project authority is discovered by parent walk");
    Check(discovered.controlPath==control,"1157 discovered control path resolves to project-owned contract");
    Check(discovered.status.find("Forge/PCC provider ready")!=std::string::npos,"1158 project-tools status reports provider readiness");

    const auto registry=GeneratorParitySystem::Registry();Check(registry.size()>=9,"1159 one parity registry continues to cover all major generator domains");
    Check(GeneratorParitySystem::Find(registry,GeneratorDomain::Character)!=nullptr,"1160 character generator remains in shared registry for next player-scale tranche");
    Check(GeneratorParitySystem::Find(registry,GeneratorDomain::Interior)!=nullptr,"1161 interior generator remains in shared registry for next interior-authority tranche");
    Check(GeneratorParitySystem::Find(registry,GeneratorDomain::Station)!=nullptr,"1162 station generator remains in shared registry for topology/interior refactor");
    std::filesystem::remove_all(tmp);std::filesystem::remove_all(unrelated);
    std::cout<<"Pass1123-1162 assertions: "<<(assertions-failures)<<" / "<<assertions<<" passed\n";return failures?1:0;
}
