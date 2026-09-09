#include "editor/EditorAssetBrowser.h"
#include "editor/EditorGizmoSystem.h"
#include "editor/SubspaceEditorAcceptanceSystem.h"
#include "editor/SubspaceEditorCore.h"
#include "ship_editor/ShipyardBuilderSystem.h"
#include "ship_editor/ShipyardWorkspaceSystem.h"
#include "ships/ShipyardDesignDnaSystem.h"
#include "ui/SubspaceUiFramework.h"
#include <cmath>
#include <iostream>
#include <memory>
using namespace subspace;
static int passed=0,failed=0;
#define CHECK(n,e) do{if(e){++passed;std::cout<<"[PASS] "<<n<<"\n";}else{++failed;std::cout<<"[FAIL] "<<n<<"\n";}}while(0)
static ShipyardModuleRecord H(){ShipyardModuleRecord r;r.source={"kit_hull",2,4,1};r.moduleClass=ShipyardModuleClass::Hull;r.semantic=ShipyardModuleSemantic::HullMid;r.size=ShipyardModuleSize::L;r.primaryHull=true;r.generatorEligible=true;r.placementRole="HULL_CORE";r.sockets={{"aft_engine","engine_mount",0,-4,0,0,-1,0,.05f},{"port_wing","structural",-2,0,0,-1,0,0,.05f}};return r;}
static ShipyardModuleRecord E(){ShipyardModuleRecord r;r.source={"kit_engine",1.2f,2,.8f};r.moduleClass=ShipyardModuleClass::Propulsion;r.semantic=ShipyardModuleSemantic::MainEngine;r.size=ShipyardModuleSize::M;r.functional=true;r.generatorEligible=true;r.placementRole="AFT_DRIVE";r.sockets={{"engine_root","engine_mount",0,2,0,0,1,0,.05f}};return r;}
static ShipyardModuleRecord W(){ShipyardModuleRecord r;r.source={"kit_wing",2.2f,2.8f,.35f};r.moduleClass=ShipyardModuleClass::Wing;r.semantic=ShipyardModuleSemantic::Wing;r.size=ShipyardModuleSize::M;r.pairedPlacement=r.mirrorPreferred=true;r.generatorEligible=true;r.sockets={{"wing_root","structural",2,0,0,1,0,0,.05f}};return r;}
static std::vector<ShipyardModuleRecord>C(){return{H(),E(),W()};}
static ProceduralShipVisualRecipe R(){ProceduralShipVisualRecipe r;r.role="COMBAT";VisualModulePlacement a;a.moduleId="kit_hull";VisualModulePlacement b;b.moduleId="kit_engine";b.y=-6;VisualModulePlacement c;c.moduleId="kit_wing";c.x=-4;r.modules={a,b,c};r.attachments={{0,1,"aft_engine","engine_root",0,true},{0,2,"port_wing","wing_root",0,true}};return r;}
int main(){auto cat=C();auto rec=R();
 auto reg=SubspaceEditorAcceptanceSystem::BuildDefaultWorkspaceRegistry();CHECK("Pass535 project-wide editor workspace contracts",reg.All().size()>=7&&reg.Find(EditorWorkspaceKind::Shipyard));
 auto th=SubspaceUiTheme::Dark();CHECK("Pass536 normalized UI tokens/theme",th.spacingXs<th.spacingS&&th.StateColor(SubspaceUiState::Error).r>th.raised.r);
 EditorInputRouter ir;ir.SetHover("part");ir.Capture("part");CHECK("Pass537 pointer hover/capture routing",ir.HoverId()=="part"&&ir.IsCapturedBy("part"));
 auto help=ShipyardWorkspaceSystem::BuildHelpRegistry();CHECK("Pass538 project-wide tooltip/help authority",help.Tooltip("shipyard.sockets",true,"in use").find("in use")!=std::string::npos);
 auto ins=ShipyardWorkspaceSystem::BuildInspector(&cat[0],&rec.modules[0],ShipyardWorkspaceMode::Build,false);CHECK("Pass539 schema-driven contextual inspector",ins.size()>=2&&ins[0].title=="Summary");
 EditorAssetBrowserModel browser;browser.SetAssets(ShipyardWorkspaceSystem::BuildAssetCards(cat));EditorAssetBrowserFilter f;f.query="engine";CHECK("Pass540 universal visual asset browser",browser.Filtered(f).size()==1);
 auto cards=ShipyardWorkspaceSystem::BuildAssetCards(cat);CHECK("Pass541 real module preview identity",cards[0].previewKey=="shipyard:kit_hull");
 EditorDragSession drag;CHECK("Pass542 inventory-style generic drag payload",drag.Begin({EditorDragPayloadKind::ShipModule,"kit_engine","PARTS","Engine"})&&drag.Active());
 EditorPlacementCandidate good;good.targetSocket="aft";good.compatibilityScore=10;EditorPlacementCandidate bad=good;bad.targetSocket="blocked";bad.collision=true;bad.compatibilityScore=12;auto pr=EditorPlacementResolver::Resolve({bad,good});CHECK("Pass543 ranked ghost-placement resolver",pr.state==EditorPlacementState::Valid&&pr.ranked.front().targetSocket=="aft");
 ShipyardBuilderSystem b;b.Initialize(cat,rec);auto ctr=ShipyardBuilderSystem::BuildControls(b.Model(),1440,900);int tabs=0;bool raw=false;for(auto&c:ctr){tabs+=c.command==ShipyardBuilderCommand::WorkspaceBuild||c.command==ShipyardBuilderCommand::WorkspaceAppearance||c.command==ShipyardBuilderCommand::WorkspaceSystems||c.command==ShipyardBuilderCommand::WorkspaceAuthoring;raw|=c.command==ShipyardBuilderCommand::AddSocket;}CHECK("Pass544 coherent Build/Appearance/Systems/Authoring hierarchy",tabs==4&&!raw);
 EditorSelectionService sel;sel.Select({"module:1","ShipModule","Engine"});auto outline=ShipyardWorkspaceSystem::BuildOutliner(cat,rec);CHECK("Pass545 shared selection/outliner authority",sel.Contains("module:1")&&outline.size()==4);
 int v=0;EditorCommandStack stack;stack.Execute(std::make_unique<LambdaEditorCommand>("edit",[&](){v=7;return true;},[&](){v=0;}));bool ur=stack.Undo()&&v==0&&stack.Redo()&&v==7;CHECK("Pass546 global undo/redo command stack",ur);
 bool orbit=true;for(int a=0;a<360;a+=15){auto d=EditorGizmoSystem::ProjectPlanarDirection({0,1,0},(float)a);float l=std::sqrt(d.x*d.x+d.y*d.y);orbit&=d.valid&&std::fabs(l-1)<.001f;}CHECK("Pass547 360-degree forward/gizmo projection",orbit);
 auto surf=ShipyardWorkspaceSystem::DefaultSurfaceSections(cat[1]);CHECK("Pass548 semantic material/surface authority",surf.size()>=2&&surf[0].primaryPaint&&surf[1].emissiveOverride);
 ShipyardAppearanceLayer al{"l","Pattern","stripe",.8f,1.2f,15,true};CHECK("Pass549 layered pattern/camouflage/decal contract",al.type=="Pattern"&&al.opacity<1&&al.rotationDegrees==15);
 auto item=ShipyardWorkspaceSystem::BuildItemDefinition(cat[1]);CHECK("Pass550 kitbash module itemization/stats identity",item.hitPoints>100&&item.mass>0&&item.powerDraw>0&&!item.iconKey.empty());
 auto dna=ShipyardDesignDnaSystem::Extract(cat[2]);CHECK("Pass551 kitbash DesignDNA extraction",dna.bilateralSymmetry&&dna.socketCount==1&&!dna.allowedOperators.empty());
 auto ex=ShipyardDesignDnaSystem::BuildExemplar("ex","COMBAT",cat,rec);auto gram=ShipyardDesignDnaSystem::BuildGrammar("g",{ex});CHECK("Pass552 PCG exemplar/design grammar",ex.modules.size()==3&&gram.exemplarCount==1&&!gram.preferredSemantics.empty());
 CHECK("Pass553 station/turret/material/PCG workspace adapters",reg.Find(EditorWorkspaceKind::StationBuilder)&&reg.Find(EditorWorkspaceKind::TurretLab)&&reg.Find(EditorWorkspaceKind::MaterialStudio)&&reg.Find(EditorWorkspaceKind::PcgStudio));
 auto acc=SubspaceEditorAcceptanceSystem::ValidateProjectWideEditor(reg,true,true,true,true,true,true);CHECK("Pass554 production editor acceptance gate",acc.passed&&acc.failures.empty());
 b.Activate(ShipyardBuilderCommand::WorkspaceAuthoring);auto ac=ShipyardBuilderSystem::BuildControls(b.Model(),1440,900);bool socket=false,teach=false;for(auto&c:ac){socket|=c.command==ShipyardBuilderCommand::AddSocket;teach|=c.command==ShipyardBuilderCommand::InspectorAuthoring;}CHECK("Pass535-554 advanced authoring preserved behind Authoring",socket&&teach);
 std::cout<<"Pass535-554 assertions: "<<passed<<" passed / "<<failed<<" failed\n";return failed?1:0;}
