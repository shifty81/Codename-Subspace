// Focused acceptance suite for Pass246-265.
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include "combat/AdvancedCombatSystem.h"
#include "combat/ShipFailureSystem.h"
#include "ships/EngineeringRepairSystem.h"
#include "combat/ElectronicWarfareSystem.h"
#include "fleet/DroneOperationsSystem.h"
#include "fleet/FleetDoctrineSystem.h"
#include "factions/DynamicFactionSystem.h"
#include "runtime/SectorSimulationSystem.h"
#include "scanning/DeepExplorationSystem.h"
#include "navigation/AnomalousSpaceSystem.h"
#include "ships/EnvironmentalHazardSystem.h"
#include "economy/ResourceMaterialSystem.h"
#include "trade_route/LogisticsAutomationSystem.h"
#include "station/StationPopulationSystem.h"
#include "station/OrbitalInfrastructureSystem.h"
#include "ships/CapitalConstructionSystem.h"
#include "fleet/CarrierOperationsSystem.h"
#include "fleet/CorporationProgressionSystem.h"
#include "runtime/PersistentUniverseSystem.h"
#include "runtime/SandboxAcceptanceIISystem.h"

using namespace subspace;
static int testsPassed=0;static int testsFailed=0;
#define TEST(name,expr) do { if(expr){testsPassed++;std::cout<<"  PASS: "<<name<<"\n";}else{testsFailed++;std::cout<<"  FAIL: "<<name<<" ("<<__FILE__<<":"<<__LINE__<<")\n";} } while(0)

// ===================================================================
// Pass246-265: deep sandbox systems
// ===================================================================
static void TestPass246AdvancedCombatII() {
    std::cout << "[Pass246AdvancedCombatII]\n";
    AdvancedCombatSystem sys; CombatWeaponSpec rail;rail.id="rail_m";rail.family=AdvancedWeaponFamily::Rail;rail.damage=45;rail.heatPerShot=6;rail.powerPerShot=12;rail.cooldownSeconds=1;rail.magazineCapacity=4;
    TEST("Pass246 registers a real weapon runtime",sys.Register(rail,2));
    auto fired=sys.Fire("rail_m",20);TEST("Pass246 firing consumes power/ammo and produces damage",fired.fired&&fired.damage==45&&fired.ammoConsumed==1&&sys.Get("rail_m")->ammo==1);
    TEST("Pass246 cooldown prevents impossible fire rate",!sys.Fire("rail_m",20).fired);
    sys.Tick(1.1);TEST("Pass246 cooldown completion allows another shot",sys.Fire("rail_m",20).fired&&sys.Get("rail_m")->ammo==0);
    sys.Tick(1.1);TEST("Pass246 empty magazine prevents firing",!sys.Fire("rail_m",20).fired);
    TEST("Pass246 reload restores finite ammunition",sys.Reload("rail_m",3)&&sys.Get("rail_m")->ammo==3);
    sys.Tick(1.1);TEST("Pass246 insufficient power prevents firing",!sys.Fire("rail_m",2).fired);
}
static void TestPass247DamageFailures() {
    std::cout << "[Pass247DamageFailures]\n";
    ShipFailureSystem s;TEST("Pass247 registers physical reactor subsystem",s.Register({"reactor",ShipSubsystemType::Reactor,100,100,FailureState::Nominal,true}));
    TEST("Pass247 subsystem damage applies",s.ApplyDamage("reactor",50));
    TEST("Pass247 damage creates degraded state",s.Get("reactor")&&s.Get("reactor")->state==FailureState::Degraded);
    s.ApplyDamage("reactor",40);TEST("Pass247 severe damage creates critical failure",!s.CriticalFailures().empty());
    TEST("Pass247 subsystem performance follows actual integrity",s.OperationalFraction(ShipSubsystemType::Reactor)<0.2);
    s.ApplyDamage("reactor",20);TEST("Pass247 destroyed subsystem stops operating",s.Get("reactor")->state==FailureState::Destroyed&&s.OperationalFraction(ShipSubsystemType::Reactor)==0);
    TEST("Pass247 restored integrity updates failure state",s.Restore("reactor",50)&&s.Get("reactor")->state==FailureState::Degraded);
}
static void TestPass248EngineeringRepair() {
    std::cout << "[Pass248EngineeringRepair]\n";
    EngineeringRepairSystem r;r.SetResources({10,5,1});EngineeringRepairJob j;j.id="repair_thruster";j.subsystemId="thruster_l";j.remainingIntegrity=20;j.integrityPerSecond=5;j.partsPerIntegrity=0.2;
    TEST("Pass248 starts resource-backed repair job",r.Start(j));
    double done=r.Advance("repair_thruster",2);TEST("Pass248 repair advances real integrity work",done==10);
    TEST("Pass248 repair consumes spare parts",r.Resources().spareParts<10);
    r.Advance("repair_thruster",2);TEST("Pass248 completed repair closes job",r.Get("repair_thruster")->complete&&!r.Get("repair_thruster")->active);
    TEST("Pass248 completed repair cannot consume more resources",r.Advance("repair_thruster",5)==0);
    EngineeringRepairJob bad;TEST("Pass248 rejects invalid repair jobs",!r.Start(bad));
}
static void TestPass249ElectronicWarfare() {
    std::cout << "[Pass249ElectronicWarfare]\n";
    ElectronicWarfareSystem e;TEST("Pass249 applies PvE sensor jamming",e.Apply({"jam1",EWarEffectType::SensorJam,0.45,5,"pirate"}));
    TEST("Pass249 stacks independent EWAR effects",e.Apply({"jam2",EWarEffectType::SensorJam,0.35,5,"frigate"})&&e.ActiveCount()==2);
    TEST("Pass249 EWAR reduces actual capability",e.CapabilityMultiplier(EWarEffectType::SensorJam)<0.25);
    TEST("Pass249 effect strength cannot exceed full suppression",e.CombinedStrength(EWarEffectType::SensorJam)<=1.0);
    e.Tick(6);TEST("Pass249 timed EWAR effects expire",e.ActiveCount()==0&&e.CapabilityMultiplier(EWarEffectType::SensorJam)==1.0);
    TEST("Pass249 rejects zero-duration effects",!e.Apply({"bad",EWarEffectType::PowerDrain,1,0,"x"}));
}
static void TestPass250DroneEcosystem() {
    std::cout << "[Pass250DroneEcosystem]\n";
    DroneOperationsSystem d(3);TEST("Pass250 registers mining drone",d.Register({"mine1",DroneRole::Mining,DroneState::Stored,2,0,10,"",0}));
    TEST("Pass250 registers repair drone",d.Register({"repair1",DroneRole::Repair,DroneState::Stored,2,0,0,"",0}));
    TEST("Pass250 launch consumes drone bandwidth",d.Launch("mine1")&&d.BandwidthUsed()==2);
    TEST("Pass250 bandwidth prevents excess deployment",!d.Launch("repair1"));
    TEST("Pass250 deployed drone accepts physical task target",d.Assign("mine1","asteroid_77"));d.Tick(5);
    TEST("Pass250 mining drone accumulates returnable cargo",d.Get("mine1")->cargo>0&&d.Get("mine1")->taskProgress>0);
    TEST("Pass250 recall returns drone to bay",d.Recall("mine1"));d.Tick(0.1);TEST("Pass250 returned drone cargo transfers to carrier",d.CollectCargo("mine1")>0&&d.Get("mine1")->cargo==0);
}
static void TestPass251FleetDoctrine() {
    std::cout << "[Pass251FleetDoctrine]\n";
    FleetDoctrineSystem s;FleetDoctrine d;d.id="convoy_guard";d.formation=FleetFormation::Convoy;d.desiredCombatFraction=0.4;d.desiredSupportFraction=0.2;d.desiredIndustrialFraction=0.2;
    TEST("Pass251 registers reusable fleet doctrine",s.Register(d)&&s.Get("convoy_guard")!=nullptr);
    std::vector<FleetMember> good={{1,"flag",FleetRole::Flagship,1,true},{2,"escort",FleetRole::Combat,1,true},{3,"tender",FleetRole::Support,1,true},{4,"hauler",FleetRole::Trading,1,true}};
    auto report=s.Evaluate(d,good);TEST("Pass251 evaluates mixed-role fleet composition",report.combatFraction>=0.5&&report.supportFraction>0);
    TEST("Pass251 viable doctrine passes",report.valid);
    std::vector<FleetMember> weak={{5,"hauler",FleetRole::Trading,1,true}};auto bad=s.Evaluate(d,weak);TEST("Pass251 warns when combat/support doctrine coverage is absent",!bad.valid&&bad.warnings.size()>=2);
    TEST("Pass251 empty fleet fails doctrine validation",!s.Evaluate(d,{}).valid);
}
static void TestPass252DynamicFactions() {
    std::cout << "[Pass252DynamicFactions]\n";
    DynamicFactionSystem f;DynamicFactionState state;state.id="frontier_union";state.resources=150;state.military=80;state.industry=75;state.expansion=0.8;state.aggression=0.8;
    TEST("Pass252 registers systemic NPC faction state",f.Register(state));
    TEST("Pass252 aggressive strong faction can choose raids",f.ChooseAction("frontier_union",0.2,0.1)==FactionStrategicAction::Raid);
    double before=f.Get("frontier_union")->resources;f.Advance("frontier_union",FactionStrategicAction::Mine,2);TEST("Pass252 faction mining changes real resource state",f.Get("frontier_union")->resources>before);
    TEST("Pass252 high threat changes strategic behavior",f.ChooseAction("frontier_union",0.9,0.1)==FactionStrategicAction::Patrol);
    DynamicFactionState poor;poor.id="poor";poor.resources=5;TEST("Pass252 struggling faction prioritizes recovery",f.Register(poor)&&f.ChooseAction("poor",0.2,0.2)==FactionStrategicAction::Recover);
}
static void TestPass253DynamicSectorSimulation() {
    std::cout << "[Pass253DynamicSectorSimulation]\n";
    SectorSimulationSystem sys;DynamicSectorState s;s.id="belt_alpha";s.lod=SimulationLod::Full;s.piratePressure=0.8;s.resourcePressure=0.8;s.security=0.4;
    auto events=sys.Advance(s,1);TEST("Pass253 sector simulation advances persistent tick",s.tick==1);
    bool raid=false,shortage=false;for(auto&e:events){raid|=e.type==SectorEventType::PirateRaid;shortage|=e.type==SectorEventType::Shortage;}TEST("Pass253 pirate pressure generates PvE raid",raid);
    TEST("Pass253 resource pressure generates shortage",shortage);
    TEST("Pass253 regional prosperity responds to security/threat",s.prosperity<0.51);
    s.tick=4;s.piratePressure=0;s.resourcePressure=0;auto discovery=sys.Advance(s,1);bool found=false;for(auto&e:discovery)found|=e.type==SectorEventType::Discovery;TEST("Pass253 sector activity can create exploration signatures",found);
}
static void TestPass254DeepExploration() {
    std::cout << "[Pass254DeepExploration]\n";
    DeepExplorationSystem e;DeepExplorationSite site;site.id="ancient_1";site.type=DeepSiteType::AncientStructure;site.scanDifficulty=2;site.value=500;
    TEST("Pass254 registers unresolved deep-space site",e.Register(site));
    TEST("Pass254 unresolved site cannot be scanned before detection",e.Scan("ancient_1",10,1)==0);
    TEST("Pass254 site enters detected intelligence state",e.Detect("ancient_1")&&e.Get("ancient_1")->state==DeepSiteState::Detected);
    TEST("Pass254 probes resolve difficult deep sites",e.Scan("ancient_1",10,3)>=1.0&&e.Get("ancient_1")->state==DeepSiteState::Resolved);
    TEST("Pass254 resolved discoveries can be exploited",e.Exploit("ancient_1")&&e.Get("ancient_1")->state==DeepSiteState::Exploited);
    TEST("Pass254 exploited site cannot be exploited twice",!e.Exploit("ancient_1"));
}
static void TestPass255AnomalousSpace() {
    std::cout << "[Pass255AnomalousSpace]\n";
    AnomalousSpaceSystem a;auto id=a.Open(AnomalousSpaceType::SubspaceTear,"unknown-7",0.9,12,1000,0.7);
    TEST("Pass255 opens temporary anomalous-space connection",id!=0&&a.Get(id));
    TEST("Pass255 ship mass consumes tear stability budget",a.Traverse(id,200)&&a.Get(id)->massUsed==200);
    TEST("Pass255 mass limit rejects oversized traversal",!a.Traverse(id,900));
    double stability=a.Get(id)->stability;a.Tick(2);TEST("Pass255 tears decay over real time",a.Get(id)->stability<stability&&a.Get(id)->remainingHours<12);
    a.Tick(20);TEST("Pass255 expired anomalous connection collapses",a.Get(id)->collapsed);
    TEST("Pass255 collapsed tear rejects traversal",!a.Traverse(id,10));
}
static void TestPass256EnvironmentalHazardsII() {
    std::cout << "[Pass256EnvironmentalHazardsII]\n";
    EnvironmentalHazardSystem h;std::vector<EnvironmentalCondition> env={{HazardType::Radiation,0.8,10},{HazardType::IonStorm,0.6,2},{HazardType::Gravitic,0.4,3}};
    EnvironmentalProtection none;auto exposed=h.Evaluate(env,none);TEST("Pass256 unprotected environment causes real damage rate",exposed.totalDamagePerHour>0);
    TEST("Pass256 ion storms impair sensors independently of visual fog",exposed.sensorPenalty>0);
    TEST("Pass256 gravitic regions impair propulsion",exposed.propulsionPenalty>0);
    EnvironmentalProtection protectedShip;protectedShip.resistance[static_cast<int>(HazardType::Radiation)]=0.9;protectedShip.resistance[static_cast<int>(HazardType::IonStorm)]=0.8;auto safe=h.Evaluate(env,protectedShip);TEST("Pass256 real protection modules reduce exposure",safe.totalDamagePerHour<exposed.totalDamagePerHour);
    TEST("Pass256 dangerous hazard list tracks insufficient certification",!exposed.dangerous.empty());
}
static void TestPass257ResourceMaterialsExpansion() {
    std::cout << "[Pass257ResourceMaterialsExpansion]\n";
    ResourceMaterialSystem m;TEST("Pass257 default raw material catalog exists",m.Get("ore_iron")&&m.Get("alloy_structural"));
    TEST("Pass257 strategic construction materials exist",m.Get("command_matrix")&&m.Get("command_matrix")->tier==ResourceTier::Strategic);
    std::unordered_map<std::string,double> inv{{"ore_iron",6}};TEST("Pass257 production validates actual input inventory",m.CanProduce("smelt_structural",inv));
    TEST("Pass257 material processing consumes inputs and creates output",m.Produce("smelt_structural",inv)&&inv["ore_iron"]==3&&inv["alloy_structural"]==1);
    inv["ore_iron"]=1;TEST("Pass257 insufficient resources fail closed",!m.Produce("smelt_structural",inv));
    TEST("Pass257 advanced/strategic material catalog supports late construction",m.MaterialsAtLeast(ResourceTier::Advanced).size()>=2);
}
static void TestPass258LogisticsAutomationII() {
    std::cout << "[Pass258LogisticsAutomationII]\n";
    LogisticsAutomationSystem l;AutomatedLogisticsNode mine;mine.id="belt_refinery";mine.inventory["alloy"]=100;mine.minimum["alloy"]=20;mine.throughputPerHour=50;AutomatedLogisticsNode yard;yard.id="shipyard";yard.inventory["alloy"]=0;yard.target["alloy"]=60;yard.throughputPerHour=40;
    TEST("Pass258 registers persistent logistics endpoints",l.RegisterNode(mine)&&l.RegisterNode(yard));
    TEST("Pass258 rejects routes to nonexistent endpoints",!l.RegisterRoute({"bad","missing","shipyard","alloy",20,1,true,0}));
    TEST("Pass258 registers valid automated freight route",l.RegisterRoute({"r1","belt_refinery","shipyard","alloy",50,1,true,0}));
    double moved=l.AdvanceRoute("r1",1);TEST("Pass258 route obeys node throughput and destination demand",moved==40);
    TEST("Pass258 route moves actual inventory",l.Node("shipyard")->inventory.at("alloy")==40&&l.Node("belt_refinery")->inventory.at("alloy")==60);
    TEST("Pass258 automation never violates source reserve",l.AdvanceRoute("r1",10)<=20&&l.Node("belt_refinery")->inventory.at("alloy")>=20);
}
static void TestPass259StationEconomyPopulation() {
    std::cout << "[Pass259StationEconomyPopulation]\n";
    StationPopulationSystem p;StationPopulationState s;s.stationId="hub";s.population=100;s.capacity=200;s.jobs=80;s.food=100;s.supplies=50;s.serviceCapacity=100;s.happiness=0.8;
    TEST("Pass259 station workforce is bounded by population and jobs",p.Workforce(s)==60);
    TEST("Pass259 population creates real consumption demand",p.ConsumptionRate(s)>0);
    double beforeFood=s.food,beforePop=s.population;p.Advance(s,10);TEST("Pass259 station population consumes physical supplies",s.food<beforeFood);
    TEST("Pass259 healthy supplied station can grow",s.population>beforePop);
    s.food=0;double beforeH=s.happiness;p.Advance(s,10);TEST("Pass259 shortages reduce station quality of life",s.happiness<beforeH);
}
static void TestPass260AdvancedOrbitalInfrastructure() {
    std::cout << "[Pass260AdvancedOrbitalInfrastructure]\n";
    OrbitalInfrastructureSystem o;auto refinery=o.Build(OrbitalInfrastructureType::RefineryComplex,"corp","gas-giant-orbit",20,500);auto dock=o.Build(OrbitalInfrastructureType::FleetDock,"corp","gas-giant-orbit",10,200);
    TEST("Pass260 builds freeform orbital infrastructure",refinery&&dock);
    TEST("Pass260 region throughput aggregates connected infrastructure",o.TotalThroughput("gas-giant-orbit")==700);
    TEST("Pass260 infrastructure can be taken offline",o.SetOnline(refinery,false)&&o.TotalThroughput("gas-giant-orbit")==200);
    TEST("Pass260 infrastructure retains owner/region authority",o.Get(dock)&&o.Get(dock)->ownerId=="corp"&&o.Get(dock)->parentRegionId=="gas-giant-orbit");
    TEST("Pass260 invalid owner/region placement fails closed",o.Build(OrbitalInfrastructureType::Shipyard,"","",1,1)==0);
}
static void TestPass261CapitalConstruction() {
    std::cout << "[Pass261CapitalConstruction]\n";
    CapitalConstructionSystem c;auto id=c.Start(CapitalHullType::Carrier,{{"alloy",100},{"drive_component",10}},100);
    TEST("Pass261 capital construction creates material-backed job",id!=0);
    TEST("Pass261 incomplete BOM blocks construction work",c.Advance(id,50)==0);
    TEST("Pass261 deliveries are capped at required material amount",c.Deliver(id,"alloy",150)==100);
    TEST("Pass261 job waits for every strategic component",!c.MaterialsReady(id));c.Deliver(id,"drive_component",10);TEST("Pass261 complete capital BOM unlocks shipyard work",c.MaterialsReady(id));
    TEST("Pass261 construction work advances only after resources arrive",c.Advance(id,60)==60&&!c.Get(id)->complete);
    c.Advance(id,40);TEST("Pass261 capital vessel reaches commissioning completion",c.Get(id)->complete&&c.Get(id)->stage==CapitalBuildStage::Complete);
}
static void TestPass262CarrierNestedCraft() {
    std::cout << "[Pass262CarrierNestedCraft]\n";
    CarrierOperationsSystem c(2);TEST("Pass262 carrier registers nested fighter",c.Register({"fighter1",NestedCraftRole::Fighter,NestedCraftState::Stored,100,100,100,"pilot"}));
    TEST("Pass262 carrier registers utility craft",c.Register({"miner1",NestedCraftRole::MiningCraft,NestedCraftState::Stored,80,0,80,"miner"}));
    TEST("Pass262 hangar capacity limits nested craft",!c.Register({"extra",NestedCraftRole::Scout}));
    TEST("Pass262 nested craft physically deploys from carrier",c.Launch("fighter1")&&c.DeployedCount()==1);
    TEST("Pass262 deployed craft must recover before servicing",!c.Service("fighter1",1,1,1));
    TEST("Pass262 craft recovers to carrier bay",c.Recover("fighter1")&&c.DeployedCount()==0);
    auto before=c.Get("fighter1")->ammo;c.Service("fighter1",0,-20,0);TEST("Pass262 carrier service clamps consumable state",c.Get("fighter1")->ammo<=before);
}
static void TestPass263CorporationProgression() {
    std::cout << "[Pass263CorporationProgression]\n";
    CorporationProgressionSystem p;CorporationProgressionState c;c.corporationId="cadet_industries";TEST("Pass263 new corporation begins at operational level",p.Recalculate(c)==1&&!c.unlocks.empty());
    c.assetsValue=1000000;c.ownedShips=12;c.ownedStations=3;c.developedPlanets=1;c.activeFleets=2;c.reputation=50;int lvl=p.Recalculate(c);TEST("Pass263 corporation growth derives from actual assets/infrastructure",lvl>=4);
    bool planetary=false;for(auto&u:c.unlocks)planetary|=u=="planetary_industry";TEST("Pass263 developed corporation unlocks planetary industry authority",planetary);
    c.assetsValue=10000000;c.ownedStations=10;c.developedPlanets=5;c.activeFleets=8;TEST("Pass263 mature corporation reaches regional command tier",p.Recalculate(c)==6);
    bool regional=false;for(auto&u:c.unlocks)regional|=u=="regional_command";TEST("Pass263 highest tier exposes regional command progression",regional);
}
static void TestPass264PersistentUniverseRecoveryII() {
    std::cout << "[Pass264PersistentUniverseRecoveryII]\n";
    PersistentUniverseSystem p;TEST("Pass264 records persistent depletion event",p.Record(PersistentEventType::ResourceDepleted,"belt","rock1",30)!=0);
    p.Record(PersistentEventType::ResourceDepleted,"belt","rock1",20);TEST("Pass264 mined resource remains depleted across simulation loads",p.ResourceRemaining("rock1",100)==50);
    p.Record(PersistentEventType::ShipDestroyed,"belt","ship9",1);TEST("Pass264 destroyed ship remains destroyed",p.IsDestroyed("ship9"));
    p.Record(PersistentEventType::WreckCreated,"belt","wreck9",1);TEST("Pass264 destruction can leave persistent recoverable wreck",p.EventsForRegion("belt").size()==4);
    TEST("Pass264 universe history uses monotonic event sequence",p.EventCount()==4);
    TEST("Pass264 unrelated resources retain independent persistence",p.ResourceRemaining("rock2",100)==100);
    auto serialized=p.Serialize();PersistentUniverseSystem restored;TEST("Pass264 persistent universe serializes deterministic recovery evidence",!serialized.empty()&&restored.Deserialize(serialized));
    TEST("Pass264 persistence round-trip retains depletion/destruction state",restored.ResourceRemaining("rock1",100)==50&&restored.IsDestroyed("ship9")&&restored.EventCount()==4);
}
static void TestPass265MajorSandboxAcceptanceII() {
    std::cout << "[Pass265MajorSandboxAcceptanceII]\n";
    SandboxAcceptanceIISystem a;SandboxAcceptanceIIState s;auto missing=a.Evaluate(s);TEST("Pass265 acceptance fails incomplete sandbox integration",!missing.passed&&missing.missing.size()==18);
    s.combatOperational=s.damageConsequences=s.repairsConsumeResources=s.ewarOperational=s.dronesOperational=s.fleetDoctrineOperational=s.dynamicFactions=s.explorationOperational=s.anomalousSpace=s.hazardsOperational=s.materialEconomy=s.logisticsAutomated=s.stationEconomy=s.orbitalInfrastructure=s.capitalConstruction=s.carrierOperations=s.corporationProgression=s.persistentUniverse=true;
    auto pass=a.Evaluate(s);TEST("Pass265 corporation-scale sandbox acceptance closes only when every authority is live",pass.passed&&pass.satisfied==18&&pass.missing.empty());
    s.logisticsAutomated=false;auto fail=a.Evaluate(s);TEST("Pass265 acceptance catches broken cross-system logistics",!fail.passed&&fail.satisfied==17);
    TEST("Pass265 acceptance contract remains explicitly eighteen production pillars",pass.required==18);
}


int main(){
 TestPass246AdvancedCombatII();
 TestPass247DamageFailures();
 TestPass248EngineeringRepair();
 TestPass249ElectronicWarfare();
 TestPass250DroneEcosystem();
 TestPass251FleetDoctrine();
 TestPass252DynamicFactions();
 TestPass253DynamicSectorSimulation();
 TestPass254DeepExploration();
 TestPass255AnomalousSpace();
 TestPass256EnvironmentalHazardsII();
 TestPass257ResourceMaterialsExpansion();
 TestPass258LogisticsAutomationII();
 TestPass259StationEconomyPopulation();
 TestPass260AdvancedOrbitalInfrastructure();
 TestPass261CapitalConstruction();
 TestPass262CarrierNestedCraft();
 TestPass263CorporationProgression();
 TestPass264PersistentUniverseRecoveryII();
 TestPass265MajorSandboxAcceptanceII();
 std::cout<<"\n=== Pass246-265 Summary: "<<testsPassed<<" passed, "<<testsFailed<<" failed ===\n";
 return testsFailed?1:0;
}
