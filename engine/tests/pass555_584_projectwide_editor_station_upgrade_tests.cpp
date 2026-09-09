#include "assets/CanonicalSurfacePolicySystem.h"
#include "editor/EditorActionRegistry.h"
#include "editor/EditorWorkspaceStateSystem.h"
#include "editor/ProjectWideEditorNormalizationSystem.h"
#include "editor/SubspaceEditorLayoutSystem.h"
#include "station/StationBlueprintLibrarySystem.h"
#include "station/StationDesignDnaSystem.h"
#include "station/StationDesignGrammarSystem.h"
#include "station/StationEditorPlacementSystem.h"
#include "station/StationKitbashCatalogSystem.h"
#include "station/StationKitbashVisualSystem.h"
#include "station/StationModuleRole.h"
#include "station/StationPcgExemplarSystem.h"
#include "station/StationRuntimeQualitySystem.h"
#include "station/StationWorkspaceSystem.h"
#include "ui/GameUiFramework.h"
#include "ui/SubspaceUiFramework.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <type_traits>
#include <vector>

using namespace subspace;
namespace {
int failures=0,assertions=0;
void Check(bool ok,const char*name){++assertions;std::cout<<(ok?"[PASS] ":"[FAIL] ")<<name<<"\n";if(!ok)++failures;}
ShipyardModuleRecord Module(const char*id,ShipyardPartRole role,ShipyardModuleSemantic semantic,float w=1,float l=1,float h=1){ShipyardModuleRecord r;r.source.moduleId=id;r.source.halfWidth=w;r.source.halfLength=l;r.source.halfHeight=h;r.partRole=role;r.semantic=semantic;r.generatorEligible=true;r.builderCategory=ShipyardPartCategory::Component;return r;}
std::vector<ShipyardModuleRecord> Catalog(){return{
    Module("station_structural_frame",ShipyardPartRole::StructuralFrame,ShipyardModuleSemantic::StructuralFrame,1.2f,1.8f,.55f),
    Module("station_connector",ShipyardPartRole::StructuralAttachment,ShipyardModuleSemantic::Adapter,.65f,1.25f,.45f),
    Module("station_bridge",ShipyardPartRole::Bridge,ShipyardModuleSemantic::CommandBridge,.8f,1.0f,.55f),
    Module("station_cargo",ShipyardPartRole::Cargo,ShipyardModuleSemantic::Component,1.0f,1.1f,.7f),
    Module("station_tank",ShipyardPartRole::Tank,ShipyardModuleSemantic::Component,.8f,1.25f,.8f),
    Module("station_hangar",ShipyardPartRole::Hangar,ShipyardModuleSemantic::Component,1.4f,1.6f,.65f),
    Module("station_sensor",ShipyardPartRole::SensorDish,ShipyardModuleSemantic::Sensor,.55f,.65f,.7f),
    Module("station_hardpoint",ShipyardPartRole::HardpointBase,ShipyardModuleSemantic::TurretHardpoint,.55f,.55f,.4f),
    Module("station_engine_housing",ShipyardPartRole::EngineHousing,ShipyardModuleSemantic::EngineHousing,.75f,1.1f,.6f)
};}
}

int main(){
    std::cout<<"[Pass555-584 Project-wide Editor + Station Upgrade]\n";
    const auto catalog=Catalog();

    // 555: one station taxonomy authority with historical API aliases.
    Check((std::is_same_v<StationModuleType,StationModuleRole>&&std::is_same_v<StationModuleFunction,StationModuleRole>),"Pass555 station construction/assembly taxonomy aliases one StationModuleRole authority");

    // 556-557: logical station kitbash built from current certified geometry.
    const auto pieces=StationKitbashCatalogSystem::Build(catalog);
    Check(pieces.size()==28,"Pass556 station logical kitbash exposes 28 reusable pieces");
    Check(std::all_of(pieces.begin(),pieces.end(),[](const auto&p){return p.generatorEligible&&!p.sourceModuleId.empty();}),"Pass557 logical station pieces resolve to certified current kitbash geometry");

    // 558-564: connected graph, grammar, QA, and dock anchor.
    const auto military=StationKitbashVisualSystem::Build(catalog,StationArchetype::Military,558,false);
    Check(military.resolved&&military.attachments.size()+1==military.modules.size(),"Pass558 generated station records every non-root module as a graph attachment");
    Check(military.maxMeasuredGap<=.15f,"Pass559 bounds-aware station assembly keeps certified connection gaps closed");
    const auto grammar=StationDesignGrammarSystem::ForArchetype(StationArchetype::Military,false);
    Check(grammar.id.find("military")!=std::string::npos&&StationDesignGrammarSystem::Requires(grammar,StationKitbashPieceRole::DefensePylon),"Pass560 station archetypes own explicit deterministic design grammars");
    Check(std::find(military.logicalRoles.begin(),military.logicalRoles.end(),StationKitbashPieceRole::DefensePylon)!=military.logicalRoles.end(),"Pass561 required station grammar pieces are deterministically materialized");
    const auto qa=StationRuntimeQualitySystem{}.Audit(catalog,StationArchetype::Military,558,false);
    Check(qa.connectedGraph&&qa.certified,"Pass562 station runtime QA requires a connected structural graph");
    Check(qa.overlapConflicts==0,"Pass563 runtime QA distinguishes intentional mating overlap from unrelated co-location");
    Check(military.usedDedicatedDock&&std::abs(military.primaryDockLocal.y)>.1f,"Pass564 docking readability anchors to an actual generated dock module");

    // 565-567: DNA + exemplar family.
    const auto dna=StationDesignDnaSystem::Extract(military,catalog);
    Check(dna.moduleCount==static_cast<int>(military.modules.size())&&dna.attachmentCount==static_cast<int>(military.attachments.size()),"Pass565 station DesignDNA captures generated topology and scale");
    StationPcgExemplarSystem exemplar;Check(exemplar.Add({"military exemplar",dna,1.0f,true}),"Pass566 approved station can be registered as a PCG exemplar");
    const auto family=exemplar.Compile("military_family",StationArchetype::Military);
    Check(family.exemplarCount==1&&!family.vocabulary.empty(),"Pass567 station exemplars compile into reusable design-language families");

    // 568-571: Station Builder joins common project-wide editor shell.
    const auto descriptor=StationWorkspaceSystem::Descriptor();
    Check(descriptor.kind==EditorWorkspaceKind::StationBuilder&&descriptor.playerVisible,"Pass568 Station Builder is a project-wide editor workspace");
    const auto cards=StationWorkspaceSystem::BuildAssetCards(pieces);
    Check(cards.size()==pieces.size()&&cards.front().previewKey.size()>0,"Pass569 station kitbash uses shared visual asset-card model with real source preview identity");
    const auto outliner=StationWorkspaceSystem::BuildOutliner(military);
    Check(outliner.size()==military.modules.size()+1,"Pass570 station attachment hierarchy projects into the shared Outliner");
    const auto inspector=StationWorkspaceSystem::BuildInspector(military,0,StationWorkspaceMode::Authoring,true);const auto help=StationWorkspaceSystem::BuildHelpRegistry();
    Check(!inspector.empty()&&help.Find("station.connected"),"Pass571 station inspector/tooltips use shared schema/help infrastructure");

    // 572-576: common layout/input/action/state behavior.
    const auto layout=SubspaceEditorLayoutSystem::Build(1920,1080,true,true);std::string layoutError;
    Check(SubspaceEditorLayoutSystem::Validate(layout,&layoutError),"Pass572 editor workspaces share one viewport-dominant layout authority");
    EditorFocusService focus;focus.Request("asset.search");Check(focus.HasFocus("asset.search"),"Pass573 keyboard/text focus uses shared editor focus authority");
    EditorActionRegistry actions;bool invoked=false;actions.Register({{"test.action","TEST ACTION","Ctrl+T","test.help",true,""},"TEST",[&](){invoked=true;return true;}});Check(actions.Execute("test.action")&&invoked,"Pass574 project-wide actions share one registry/execution contract");
    actions.Register({{"disabled.action","DISABLED","","",false,"Needs selection"},"TEST",[]{return true;}});std::string disabledReason;Check(!actions.Execute("disabled.action",&disabledReason)&&disabledReason=="Needs selection","Pass575 disabled actions surface plain-language reasons");
    EditorWorkspaceStateSystem ws;EditorWorkspacePreferences pref;pref.workspace=EditorWorkspaceKind::StationBuilder;pref.leftPanelWidth=999;ws.Set(pref);Check(ws.Get(EditorWorkspaceKind::StationBuilder).leftPanelWidth<=640.0f,"Pass576 editor panel preferences persist through one bounded workspace-state authority");

    // 577: gameplay UI derives sizing/typography from same UI token authority.
    const auto tokens=SubspaceUiTheme::Dark();const auto gameTheme=GameUiFramework{}.DefaultTheme();
    Check(std::abs(gameTheme.spacing-tokens.spacingS)<.001f&&std::abs(gameTheme.headingTextPx-tokens.fontTitle)<.001f,"Pass577 runtime game UI projects from shared SubspaceUI design tokens");

    // 578-579: canonical project-wide material/surface semantics.
    assets::PbrMaterial glass;glass.name="Cockpit Glass";glass.domain=assets::MaterialDomain::Canopy;const auto gp=assets::CanonicalSurfacePolicySystem::Infer(0,glass,"test");
    Check(gp.semantic==assets::SurfaceSemantic::CanopyGlass&&!gp.primaryPaint,"Pass578 canonical surface inference protects glass from ordinary hull paint");
    assets::CanonicalAsset ca;ca.assetId="test_asset";ca.materials.push_back(glass);assets::PbrMaterial hull;hull.name="Hull Primary";ca.materials.push_back(hull);const auto policies=assets::CanonicalSurfacePolicySystem::Build(ca);
    Check(policies.size()==2&&policies[1].primaryPaint&&policies[1].decal,"Pass579 surface paint/pattern/decal eligibility is normalized at CanonicalAsset level");

    // 580: Station Builder uses universal placement resolver semantics.
    const auto placement=StationEditorPlacementSystem::Resolve(military,catalog,pieces.front(),{military.modules.front().x,military.modules.front().y,military.modules.front().z});
    Check(placement.state==EditorPlacementState::Valid||placement.state==EditorPlacementState::Warning,"Pass580 Station Builder placement routes through universal valid/warning/invalid resolver");

    // 581: station blueprints persist generated graph/provenance.
    StationBlueprintDocument doc;doc.blueprintId="";doc.name="QA Citadel";doc.recipe=military;doc.designDna=dna;doc.tags={"QA","MILITARY"};const auto temp=std::filesystem::temp_directory_path()/"subspace_station_pass581.bp";std::string ioError;const bool saved=StationBlueprintLibrarySystem::Save(doc,temp.string(),&ioError);StationBlueprintDocument loaded;const bool loadedOk=saved&&StationBlueprintLibrarySystem::Load(temp.string(),loaded,&ioError);std::error_code ec;std::filesystem::remove(temp,ec);
    Check(loadedOk&&loaded.recipe.modules.size()==military.modules.size()&&loaded.recipe.connectedGraph,"Pass581 station blueprints preserve connected visual assembly graph");

    // 582: shared generic attachment record remains ship compatibility API.
    Check((std::is_same_v<VisualAssemblyAttachment,ShipVisualAttachment>),"Pass582 ships and stations share one VisualAssemblyAttachment contract");

    // 583-584: project-wide normalization audit and explicit repository-only remainder.
    const auto norm=ProjectWideEditorNormalizationSystem::Audit();
    Check(norm.runtimeNormalized,"Pass583 runtime/editor normalization audit reports one authority per active project-wide domain");
    Check(!norm.repositoryLayoutNormalized&&!norm.remainingRepositoryActions.empty(),"Pass584 overwrite-safe audit explicitly isolates remaining physical repository moves instead of hiding normalization debt");

    std::cout<<"Pass555-584 assertions: "<<(assertions-failures)<<" / "<<assertions<<" passed\n";
    return failures?1:0;
}
