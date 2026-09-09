// Pass353-360 attachment/camera/vector/solar/celestial acceptance suite.
#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

#include "navigation/TravelArrivalSystem.h"
#include "rendering/CelestialEnvironmentSystem.h"
#include "rendering/CinematicFlightPresentationSystem.h"
#include "rendering/ProceduralVisualVariantSystem.h"
#include "rendering/SolarPresentationSystem.h"
#include "rendering/SpaceMaterialSystem.h"
#include "rendering/StrategicCamera.h"
#include "rendering/StrategicViewProjection.h"

using namespace subspace;
static int testsPassed=0; static int testsFailed=0;
#define TEST(name,expr) do { if(expr){++testsPassed;std::cout<<"  PASS: "<<name<<"\n";}else{++testsFailed;std::cout<<"  FAIL: "<<name<<" ("<<__FILE__<<":"<<__LINE__<<")\n";} } while(0)

static std::vector<VisualModuleSource> Sources(){
    return {
        {"cargo_bay",4.20f,3.20f,3.00f},{"cockpit_basic",2.80f,3.50f,2.20f},{"cockpit_small",2.00f,2.50f,1.70f},
        {"engine_main",2.20f,2.50f,2.20f},{"engine_small",1.80f,3.00f,1.80f},{"hull_section",3.00f,4.20f,2.00f},
        {"hull_section_enhanced",3.00f,3.00f,3.50f},{"hull_section_small",2.10f,3.20f,1.70f},{"power_core",2.50f,2.50f,2.00f},
        {"sensor_array",2.80f,2.80f,1.15f},{"thruster",1.20f,1.50f,1.20f},{"thruster_small",1.10f,1.50f,1.10f},
        {"weapon_mount",1.20f,2.00f,1.20f},{"wing_left",4.50f,3.20f,.60f},{"wing_right",4.50f,3.20f,.60f},
        {"wing_small_left",4.00f,2.80f,.90f},{"wing_small_right",4.00f,2.80f,.90f}
    };
}

static void TestPass353AttachmentIntegrity(){
    std::cout<<"[Pass353AttachmentIntegrity]\n";
    const auto c=ProceduralVisualVariantSystem::Build(Sources(),353u,12);
    const auto* r=ProceduralVisualVariantSystem::Select(c,"Heavy Hauler",7u);
    int bridges=0;for(const auto& d:r->details)if(d.kind==VisualDetailKind::MountBridge)++bridges;
    TEST("Pass353 heavy generated ship receives visible mount bridges",r&&bridges>=2);
    TEST("Pass353 bridge-equipped recipe still clears art director",r&&r->acceptedByArtDirector);
}

static void TestPass354InspectionCameraAuthority(){
    std::cout<<"[Pass354InspectionCameraAuthority]\n";
    StrategicCamera camera;
    camera.SetElevationOverrideDegrees(34.0f);
    camera.SetZoom(4.8f);
    const auto basis=StrategicViewProjection::Build(camera,1920.0f,1080.0f);
    StrategicViewProjectionConfig config;
    TEST("Pass354 camera supports explicit elevation override",camera.HasElevationOverride()&&std::fabs(camera.GetElevationOverrideDegrees()-34.0f)<.01f);
    TEST("Pass354 close inspection projection permits sub-16-unit camera distance",config.minDistance<8.0f);
    TEST("Pass354 close inspection still produces a valid perspective basis",basis.eye.z>0.0f&&basis.farPlane>=10000.0f);
}

static void TestPass355VectorCinematicCamera(){
    std::cout<<"[Pass355VectorCinematicCamera]\n";
    const auto align0=CinematicFlightPresentationSystem::Evaluate(VectorTravelStage::Aligning,0.0);
    const auto align1=CinematicFlightPresentationSystem::Evaluate(VectorTravelStage::Aligning,1.0);
    const auto cruise=CinematicFlightPresentationSystem::Evaluate(VectorTravelStage::Cruise,.5);
    TEST("Pass355 alignment progressively moves toward chase view",align1.chaseBlend>align0.chaseBlend+.5f);
    TEST("Pass355 Vector cruise is true low-elevation rear chase",cruise.chaseBlend>.99f&&cruise.elevationDegrees<=23.0f);
    TEST("Pass355 Vector cruise camera closes toward the ship",cruise.targetZoom>=1.85f);
}

static void TestPass356ArrivalCinematic(){
    std::cout<<"[Pass356ArrivalCinematic]\n";
    const auto early=CinematicFlightPresentationSystem::Evaluate(VectorTravelStage::Complete,1.0,.7);
    const auto middle=CinematicFlightPresentationSystem::Evaluate(VectorTravelStage::Complete,1.0,1.8);
    const auto late=CinematicFlightPresentationSystem::Evaluate(VectorTravelStage::Complete,1.0,3.9);
    TEST("Pass356 centered destination title becomes visible after arrival",early.arrivalTitleAlpha>.6f&&middle.arrivalTitleAlpha>.8f);
    TEST("Pass356 destination title fades instead of snapping away",late.arrivalTitleAlpha<.2f);
    TEST("Pass356 camera progressively blends back after arrival",late.arrivalBlend>early.arrivalBlend);
}

static void TestPass357SolarAuthority(){
    std::cout<<"[Pass357SolarAuthority]\n";
    StarData star;star.starClass=StarClass::Yellow;star.radius=1240.0f;star.luminosity=1.0f;
    const auto nearLight=SolarPresentationSystem::EvaluateLight(star,{0,0,0},{1200,0,0});
    const auto farLight=SolarPresentationSystem::EvaluateLight(star,{0,0,0},{7200,0,0});
    const auto visual=SolarPresentationSystem::VisualFor(star);
    TEST("Pass357 solar illumination falls with orbital distance",nearLight.diffuseIntensity>farLight.diffuseIntensity);
    TEST("Pass357 deep-space ambient remains restrained",nearLight.ambientIntensity<.06f&&farLight.ambientIntensity<.06f);
    TEST("Pass357 star presentation is no longer a tiny 14-32 unit sphere",visual.worldRadius>=90.0f);
}

static void TestPass358PlanetaryScale(){
    std::cout<<"[Pass358PlanetaryScale]\n";
    CelestialEnvironmentSystem env;
    PlanetData rocky;rocky.radius=720.0f;rocky.type=PlanetType::Rocky;
    PlanetData giant=rocky;giant.type=PlanetType::GasGiant;giant.radius=1350.0f;giant.hasRings=true;
    const float rockyR=env.WorldRadius(rocky),giantR=env.WorldRadius(giant);
    TEST("Pass358 terrestrial planet can dominate orbital viewport",rockyR>=180.0f);
    TEST("Pass358 gas giant is substantially larger than terrestrial body",giantR>rockyR*1.8f&&giantR>=500.0f);
    TEST("Pass358 gas giant allows beyond-screen astronomical presentation",env.ProfileFor(giant).maximumScreenFraction>=4.0f);
}

static void TestPass359SafeLargePlanetArrival(){
    std::cout<<"[Pass359SafeLargePlanetArrival]\n";
    GalaxySector sector;sector.hasStar=true;
    PlanetData p;p.planetId="p1";p.name="Aurelia";p.radius=880.0f;p.type=PlanetType::Rocky;p.position={200000.0f,0.0f,0.0f};
    sector.planets.push_back(p);
    SystemDestination d;d.id=42;d.name="Aurelia";d.type=SystemDestinationType::Planet;d.discovered=true;d.warpable=true;d.position.localX=200000.0*1000000.0;
    TravelArrivalSystem arrival;
    const auto a=arrival.Resolve(d,sector,18.0f);
    CelestialEnvironmentSystem env;const float r=env.WorldRadius(p);
    TEST("Pass359 orbital arrival remains outside enlarged planetary surface",a.valid&&a.standOffWorld>r*1.05f&&a.safeFromPlanetSurface);
    TEST("Pass359 enlarged planet still preserves practical local approach time",a.finalApproachSeconds>=12.0&&a.finalApproachSeconds<=60.0);
}

static void TestPass360ShaderAndScaleClosure(){
    std::cout<<"[Pass360ShaderAndScaleClosure]\n";
    const auto sun=SpaceMaterialSystem::GetProfile(SpaceMaterialKind::Sun);
    const std::string shader=SpaceMaterialSystem::FragmentShader120();
    StrategicViewProjectionConfig config;
    TEST("Pass360 sun uses procedural stellar photosphere shader mode",sun.surfaceMode>7.5f&&sun.emissive>=1.0f);
    TEST("Pass360 stellar shader carries animated time authority",shader.find("uTime")!=std::string::npos&&shader.find("stellar photosphere")!=std::string::npos);
    TEST("Pass360 astronomical renderer far plane supports reconstructed planet scale",config.farPlane>=10000.0f);
}

int main(){
    TestPass353AttachmentIntegrity();TestPass354InspectionCameraAuthority();TestPass355VectorCinematicCamera();TestPass356ArrivalCinematic();
    TestPass357SolarAuthority();TestPass358PlanetaryScale();TestPass359SafeLargePlanetArrival();TestPass360ShaderAndScaleClosure();
    std::cout<<"\n=== Pass353-360 Cinematic Celestial Summary: "<<testsPassed<<" passed, "<<testsFailed<<" failed ===\n";
    return testsFailed?1:0;
}
