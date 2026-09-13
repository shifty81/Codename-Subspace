#include "construction/ShipyardAssemblyBridgeSystem.h"
#include "editor/ShipyardCanonicalIntegrationSystem.h"
#include "runtime/AssemblyRuntimeProductSystem.h"
#include "runtime/WorldStreamingIntegrationSystem.h"

#include <cmath>
#include <iostream>
#include <string>
#include <vector>

using namespace subspace;
namespace {
int failures=0,assertions=0;
void Check(bool ok,const char* name){++assertions;std::cout<<(ok?"[PASS] ":"[FAIL] ")<<name<<"\n";if(!ok)++failures;}
ShipyardModuleRecord Module(const char* id,ShipyardPartRole role,float hw,float hl,float hh,bool functional=false,bool surface=false){
    ShipyardModuleRecord r;r.source.moduleId=id;r.source.halfWidth=hw;r.source.halfLength=hl;r.source.halfHeight=hh;r.partRole=role;r.functional=functional;r.surfaceOnly=surface;r.generatorEligible=true;return r;
}
VisualModulePlacement Place(const char* id,float x,float y,float z=0){VisualModulePlacement p;p.moduleId=id;p.x=x;p.y=y;p.z=z;return p;}
bool HasCap(const BuildElement& e,const std::string& cap){for(const auto& c:e.capabilityTags)if(c==cap)return true;return false;}
const BuildElement* Find(const AssemblyDefinition& a,const std::string& def){for(const auto& e:a.elements)if(e.definitionId==def)return &e;return nullptr;}
}

int main(){
    std::cout<<"[Pass998-1022 Shipyard Canonical + Streaming Integration]\n";
    std::vector<ShipyardModuleRecord> catalog{
        Module("bridge",ShipyardPartRole::Bridge,2.0f,2.0f,1.5f,true,false),
        Module("hull",ShipyardPartRole::PrimaryHull,2.5f,2.5f,1.7f,false,false),
        Module("cargo",ShipyardPartRole::Cargo,2.1f,2.0f,1.6f,true,false),
        Module("engine",ShipyardPartRole::MainEngine,1.7f,2.1f,1.4f,true,false),
        Module("wing",ShipyardPartRole::Wing,2.5f,1.2f,.35f,false,true)
    };
    catalog[1].sockets={{"fore","universal",0,2.5f,0,0,1,0,0},{"aft","universal",0,-2.5f,0,0,-1,0,0}};

    ProceduralShipVisualRecipe recipe;recipe.recipeId="integration_ship";recipe.role="INDUSTRIAL";recipe.forwardVisualYawDegrees=180.0f;
    recipe.modules={Place("bridge",0,5),Place("hull",0,1),Place("cargo",0,-2),Place("engine",0,-5),Place("wing",3.1f,1)};
    recipe.modules[4].mirrorX=true;
    recipe.attachments={{0,1,"auto","auto",0,true},{1,2,"auto","auto",0,true},{2,3,"auto","auto",0,true},{1,4,"auto","auto",0,true}};

    const auto shipId=StableIdentitySystem::Deterministic(PersistentEntityKind::Ship,"test","ship-998");
    const auto imported=ShipyardAssemblyBridgeSystem::Import(catalog,recipe,shipId);
    Check(imported.valid,"Pass998 legacy/certified Shipyard recipe imports into canonical AssemblyDefinition");
    Check(imported.assembly.assemblyId==shipId,"Pass999 imported assembly preserves persistent ship identity");
    Check(imported.assembly.elements.size()==5,"Pass1000 every authored recipe module becomes one canonical element");
    bool quantized=true;for(const auto& e:imported.assembly.elements)quantized&=AssemblyConstructionSystem::IsQuantized(e.transform.position,imported.assembly.quantumMeters);
    Check(quantized,"Pass1001 imported translations use canonical 0.25m construction quantum");
    const auto* bridge=Find(imported.assembly,"bridge");
    Check(bridge&&std::fabs(bridge->transform.rotation.w)<0.01,"Pass1002 recipe forward-visual yaw is folded into canonical element orientation");
    const auto* wing=Find(imported.assembly,"wing");
    Check(wing&&wing->transform.scale.x<0.0,"Pass1003 authored mirror handedness survives as signed canonical scale");
    Check(imported.generatedSockets>0,"Pass1004 modules lacking authored sockets receive deterministic semantic fallback sockets");
    Check(bridge&&HasCap(*bridge,"command"),"Pass1005 Shipyard role taxonomy maps command capability into canonical assembly");
    const auto* engine=Find(imported.assembly,"engine");
    Check(engine&&HasCap(*engine,"propulsion"),"Pass1006 propulsion taxonomy maps into canonical capability tags");
    Check(imported.assembly.attachments.size()==4,"Pass1007 legacy attachment graph becomes canonical structural edges");
    AssemblyConstructionSystem editor;std::string error;bool began=editor.Begin(imported.assembly,&error);auto validation=began?editor.Validate():AssemblyValidationResult{};
    Check(began&&validation.valid,"Pass1008 imported assembly passes canonical structural/socket validation");

    WorldSimulationAuthority world;
    const auto registeredShip=world.Register(PersistentEntityKind::Ship,"test","ship-998",{},InvalidSpatialFrameId,"integration_ship");
    Check(registeredShip==shipId&&world.Find(shipId),"Pass1009 world authority owns the same stable ship identity used by the Shipyard");
    ShipyardCanonicalIntegrationSystem live;
    Check(live.Begin(world,shipId,catalog,recipe,&error),"Pass1010 visible-Shipyard bridge begins a canonical persistent-ship edit session");
    Check(live.WorkingAssembly().assemblyId==shipId,"Pass1011 Shipyard session never substitutes an editor-only ship identity");
    const auto products=live.PreviewProducts({"command","propulsion"});
    Check(products.valid,"Pass1012 live Shipyard preview compiles validated canonical runtime products");
    Check(products.renderInstances.size()==5,"Pass1013 render product count derives directly from canonical elements");
    Check(products.collisionProxies.size()==5,"Pass1014 collision proxies derive from the same canonical assembly source");
    Check(products.boundsMax.x>5.0&&products.boundsMin.y<-6.0,"Pass1015 aggregate runtime bounds include asymmetric wing and aft engine geometry");
    Check(std::isfinite(products.centerOfMass.x)&&std::isfinite(products.centerOfMass.y),"Pass1016 mass-weighted center of mass is produced for flight/physics consumers");
    Check(products.interiorVolumes.size()>=3,"Pass1017 habitable/interior candidate volumes are compiled from module semantics");
    Check(products.portals.size()==4&&products.navAnchors.size()==4,"Pass1018 structural edges produce portal and navigation handoff products");
    const auto products2=live.PreviewProducts({"command","propulsion"});
    Check(products.propulsionPorts.size()==1&&!products.fingerprint.empty()&&products.fingerprint==products2.fingerprint,
          "Pass1019 propulsion presentation and deterministic runtime-product fingerprints share canonical source");

    const auto planetLow=world.Register(PersistentEntityKind::Planet,"test","planet-low");
    const auto planetHigh=world.Register(PersistentEntityKind::Planet,"test","planet-high");
    WorldStreamingIntegrationSystem streaming;
    const auto low=streaming.Queue(world,planetLow,SimulationRepresentation::Proxy,1,"map-hover");
    const auto high=streaming.Queue(world,planetHigh,SimulationRepresentation::Full,50,"player-arrival");
    Check(low.ticket>0&&high.ticket>low.ticket&&world.PendingPrefetchCount()==2,"Pass1020 system/interior/planet destinations enter explicit prioritized prefetch authority");
    StreamingHandoff handoff;const bool acquired=streaming.AcquireNext(world,handoff);
    Check(acquired&&handoff.destinationId==planetHigh&&handoff.state==StreamingHandoffState::WaitingForResidency,
          "Pass1021 streaming bridge acquires highest-priority destination without prematurely promoting representation");
    const bool completed=streaming.CompleteResidency(world,handoff,true,"test-stream-ready");const auto* highState=world.Find(planetHigh);
    Check(completed&&handoff.state==StreamingHandoffState::Activated&&highState&&highState->resident&&highState->representation==SimulationRepresentation::Full,
          "Pass1022 successful stream completion atomically publishes resident Full destination state");

    std::cout<<"Assertions: "<<assertions<<" failures: "<<failures<<"\n";
    return failures==0?0:1;
}
