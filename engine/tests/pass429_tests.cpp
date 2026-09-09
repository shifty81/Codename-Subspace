#include "content/ShipyardModuleSystem.h"
#include "content/ShipyardNameClassification.h"
#include "rendering/CelestialEnvironmentSystem.h"
#include "rendering/SpaceMaterialSystem.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

using namespace subspace;
static int testsPassed=0,testsFailed=0;
#define TEST(name,expr) do{if(expr){++testsPassed;std::cout<<"[PASS] "<<name<<"\n";}else{++testsFailed;std::cout<<"[FAIL] "<<name<<"\n";}}while(0)

static std::vector<VisualModuleSource> Fixture(){
    return {
        {"shipyard_a_hull_001_long_hull",2.6f,4.8f,1.2f},
        {"shipyard_a_hull_002_mid_hull",2.2f,3.4f,1.0f},
        {"shipyard_a_hull_003_aft_hull",2.5f,3.8f,1.1f},
        {"shipyard_a_command_001_cockpit",1.4f,2.0f,.8f},
        {"shipyard_a_propulsion_001_engine",.9f,1.7f,.8f},
        {"shipyard_a_propulsion_002_nozzle",.7f,1.2f,.7f},
        {"shipyard_a_propulsion_004_rcs_thruster",.32f,.55f,.30f},
        {"shipyard_a_hardpoint_001_turret_mount",.55f,.55f,.30f},
        {"shipyard_a_detail_001_greeble",.40f,.65f,.18f}
    };
}

static void Pass429_AuthoredPropulsion(){
    const auto sources=Fixture();
    const auto catalog=ShipyardModuleSystem::BuildCatalog(sources);
    const auto recipes=ShipyardModuleSystem::BuildShowcaseRecipes(sources,0x429u);
    TEST("Pass429 fixture produces certified Shipyard showcase recipes",!recipes.empty());
    if(recipes.empty()) return;
    const auto ports=ShipyardModuleSystem::BuildPropulsionPorts(catalog,recipes.front());
    TEST("Pass429 propulsion effects resolve from authored Shipyard propulsion modules",!ports.empty());
    bool valid=true;
    for(const auto& port:ports){
        const float len=port.exhaustDirection.length();
        valid&=port.moduleId.rfind("shipyard_a_",0)==0;
        valid&=std::fabs(len-1.0f)<0.001f;
        valid&=port.nozzleRadiusHint>0.0f&&port.plumeLengthHint>0.0f;
    }
    TEST("Pass429 authored propulsion ports carry normalized exhaust directions and positive effect dimensions",valid);
}

static void Pass429_OrbitalPlanetPresentation(){
    PlanetData planet;
    planet.planetId="pass429_rock";
    planet.radius=620.0f;
    planet.type=PlanetType::Rocky;
    planet.hasRings=false;
    const CelestialEnvironmentSystem celestial;
    const float body=celestial.WorldRadius(planet);
    const float safe=celestial.SafeLocalOrbitRadius(planet);
    TEST("Pass429 non-ringed safe orbit remains outside the atmosphere but close to the visible planet",safe>body*1.055f&&safe<=body*1.11f);

    planet.type=PlanetType::GasGiant;
    planet.hasRings=true;
    const float ringBody=celestial.WorldRadius(planet);
    const float ringSafe=celestial.SafeLocalOrbitRadius(planet);
    TEST("Pass429 ringed worlds still preserve a large ring-clearance envelope",ringSafe>ringBody*4.0f);
}

static void Pass429_RotatingPlanetSurfaceShader(){
    const std::string shader=SpaceMaterialSystem::FragmentShader120();
    TEST("Pass429 planet shader time-rotates procedural surface sampling",shader.find("sampleOn")!=std::string::npos&&shader.find("spinRate")!=std::string::npos&&shader.find("uTime*spinRate")!=std::string::npos);
}


static void Pass429_FilenameFirstClassification(){
    TEST("Pass429 certified ID prefixes do not contaminate leaf classification",
         ShipyardNameClassifier::CanonicalLeafName("shipyard_a_propulsion_137_shipyard_propulsion_020_engineStrutCylindar") == "enginestrutcylindar");

    struct Expected { const char* name; const char* cls; const char* semantic; };
    static const Expected engineBuilder[] = {
        {"engine5Engine","propulsion","MAIN_ENGINE"},
        {"engineBit","propulsion","MAIN_ENGINE"},
        {"engineBlockSimple","propulsion","MAIN_ENGINE"},
        {"engineBodyChin","propulsion","ENGINE_HOUSING"},
        {"engineBodyFish","propulsion","ENGINE_HOUSING"},
        {"engineBodySplit","propulsion","ENGINE_HOUSING"},
        {"engineBodySplit2","propulsion","ENGINE_HOUSING"},
        {"engineBodyStart","propulsion","ENGINE_HOUSING"},
        {"engineBodyTooth","propulsion","ENGINE_HOUSING"},
        {"engineBodyWideMouth","propulsion","ENGINE_HOUSING"},
        {"engineBodyYBlock","propulsion","ENGINE_HOUSING"},
        {"engineCubeEngine","propulsion","MAIN_ENGINE"},
        {"engineFishEngine","propulsion","MAIN_ENGINE"},
        {"engineFlap","propulsion","ENGINE_NOZZLE"},
        {"engineMulti","propulsion","MAIN_ENGINE"},
        {"engineOnion","propulsion","MAIN_ENGINE"},
        {"engineRect","propulsion","MAIN_ENGINE"},
        {"engineSPole","propulsion","MAIN_ENGINE"},
        {"engineSplit","propulsion","MAIN_ENGINE"},
        {"engineStrutCylindar","component","STRUCTURAL_FRAME"},
        {"engineStrutFoot","component","STRUCTURAL_FRAME"},
        {"engineStrutLadder","component","STRUCTURAL_FRAME"},
        {"engineStrutVacum","component","STRUCTURAL_FRAME"},
        {"engineTrapEngine","propulsion","MAIN_ENGINE"},
        {"engineTrumpet","propulsion","ENGINE_NOZZLE"},
        {"engineTrussworkWing","wing","WING"},
        {"engineVanes","propulsion","ENGINE_NOZZLE"},
        {"enigneBracketEngine","propulsion","ENGINE_HOUSING"}
    };
    bool all=true;
    int mainEngines=0,housings=0,nozzles=0,frames=0,wings=0;
    for(const auto& e:engineBuilder){
        const auto c=ShipyardNameClassifier::Classify(e.name);
        all &= c.moduleClass==e.cls && c.semantic==e.semantic;
        mainEngines += c.semantic=="MAIN_ENGINE";
        housings += c.semantic=="ENGINE_HOUSING";
        nozzles += c.semantic=="ENGINE_NOZZLE";
        frames += c.semantic=="STRUCTURAL_FRAME";
        wings += c.semantic=="WING";
    }
    TEST("Pass429 all 28 Greyoxide engine-builder filenames match reviewed class/semantic table",all);
    TEST("Pass429 engine-builder batch splits into 11 drives, 9 housings, 3 nozzle/control, 4 struts, 1 wing",
         mainEngines==11 && housings==9 && nozzles==3 && frames==4 && wings==1);

    const auto bipolar=ShipyardNameClassifier::Classify("miscBiPolarEngine");
    TEST("Pass429 miscBiPolarEngine remains an explicit complete engine",
         bipolar.moduleClass=="propulsion" && bipolar.semantic=="MAIN_ENGINE");

    const auto joined=ShipyardNameClassifier::Classify("shipyard_a_adapter_001_shipyard_adapter_001_hullJoined");
    const auto missile=ShipyardNameClassifier::Classify("miscVertMissTube");
    const auto mast=ShipyardNameClassifier::Classify("miscInstrumentMast2");
    TEST("Pass429 hullJoined is corrected from adapter to hull",joined.moduleClass=="hull" && joined.semantic=="HULL_MID");
    TEST("Pass429 vertical missile tube is a weapon mount",missile.moduleClass=="hardpoint" && missile.semantic=="WEAPON_MOUNT");
    TEST("Pass429 instrument mast is sensor detail",mast.moduleClass=="detail" && mast.semantic=="SENSOR");
}

static void Pass429_RuntimePoolSemantics(){
    std::vector<VisualModuleSource> sources;
    static const char* names[] = {
        "engine5Engine","engineBit","engineBlockSimple","engineBodyChin","engineBodyFish","engineBodySplit",
        "engineBodySplit2","engineBodyStart","engineBodyTooth","engineBodyWideMouth","engineBodyYBlock","engineCubeEngine",
        "engineFishEngine","engineFlap","engineMulti","engineOnion","engineRect","engineSPole","engineSplit",
        "engineStrutCylindar","engineStrutFoot","engineStrutLadder","engineStrutVacum","engineTrapEngine","engineTrumpet",
        "engineTrussworkWing","engineVanes","enigneBracketEngine","miscBiPolarEngine"
    };
    int serial=100;
    for(const char* name:names){
        sources.push_back({"shipyard_a_propulsion_"+std::to_string(serial++)+"_shipyard_propulsion_001_"+std::string(name),1.0f,1.8f,.8f});
    }
    // Required non-propulsion pieces so BuildShowcaseRecipes can produce a ship.
    sources.push_back({"shipyard_a_hull_001_shipyard_hull_001_hullLong",2.4f,4.2f,1.1f});
    sources.push_back({"shipyard_a_command_001_shipyard_command_001_miscBridge1",1.4f,1.8f,.8f});
    sources.push_back({"shipyard_a_hardpoint_001_shipyard_hardpoint_001_hardpointBase3X",.6f,.6f,.3f});

    const auto catalog=ShipyardModuleSystem::BuildCatalog(sources);
    int propulsion=0,mainEngine=0,housing=0,nozzle=0,structural=0,wing=0;
    for(const auto& r:catalog){
        propulsion += r.moduleClass==ShipyardModuleClass::Propulsion;
        mainEngine += r.semantic==ShipyardModuleSemantic::MainEngine;
        housing += r.semantic==ShipyardModuleSemantic::EngineHousing;
        nozzle += r.semantic==ShipyardModuleSemantic::EngineNozzle;
        structural += r.semantic==ShipyardModuleSemantic::StructuralFrame;
        wing += r.semantic==ShipyardModuleSemantic::Wing;
    }
    TEST("Pass429 runtime catalog contains 24 propulsion-class engine-builder objects",propulsion==24);
    TEST("Pass429 runtime semantics preserve 12 complete drives including miscBiPolarEngine",mainEngine==12);
    TEST("Pass429 runtime semantics preserve 9 engine housings",housing==9);
    TEST("Pass429 runtime semantics preserve 3 nozzle/control pieces",nozzle==3);
    TEST("Pass429 runtime removes four engine struts from propulsion",structural==4);
    TEST("Pass429 runtime removes engineTrussworkWing from propulsion",wing==1);

    const auto recipes=ShipyardModuleSystem::BuildShowcaseRecipes(sources,0x429C1A55u);
    bool driveAnchorsAreMainEngines=!recipes.empty();
    for(const auto& recipe:recipes){
        for(const auto& anchor:recipe.anchors){
            if(anchor.id.rfind("DRIVE_",0)!=0)continue;
            const auto it=std::find_if(catalog.begin(),catalog.end(),[&](const ShipyardModuleRecord& r){return r.source.moduleId==anchor.moduleId;});
            driveAnchorsAreMainEngines &= it!=catalog.end() && it->semantic==ShipyardModuleSemantic::MainEngine;
        }
    }
    TEST("Pass429 generator DRIVE anchors can only select MAIN_ENGINE records",driveAnchorsAreMainEngines);
}

int main(){
    Pass429_FilenameFirstClassification();
    Pass429_RuntimePoolSemantics();
    Pass429_AuthoredPropulsion();
    Pass429_OrbitalPlanetPresentation();
    Pass429_RotatingPlanetSurfaceShader();
    std::cout<<"\nPass429 tests: "<<testsPassed<<" passed, "<<testsFailed<<" failed\n";
    return testsFailed?1:0;
}
