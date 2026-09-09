// Pass347-352 ship visual reconstruction acceptance suite.
#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

#include "rendering/ProceduralVisualVariantSystem.h"
#include "rendering/SpaceMaterialSystem.h"

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

static std::vector<const VisualModulePlacement*> Find(const ProceduralShipVisualRecipe& r,const std::string& id){
    std::vector<const VisualModulePlacement*> out;
    for(const auto& p:r.modules) if(p.moduleId==id) out.push_back(&p);
    return out;
}

static void TestPass347BoundsAwareAssembly(){
    std::cout<<"[Pass347BoundsAwareAssembly]\n";
    auto c=ProceduralVisualVariantSystem::Build(Sources(),347u,12);
    auto* r=ProceduralVisualVariantSystem::Select(c,"Industrial Mining Ship",4u);
    TEST("Pass347 measured module metrics are retained",c.sourceMetrics.size()==Sources().size());
    TEST("Pass347 industrial recipe exists",r!=nullptr&&!r->modules.empty());
    std::vector<float> hullY;
    for(const auto& p:r->modules) if(p.moduleId=="hull_section"||p.moduleId=="hull_section_enhanced") hullY.push_back(p.y);
    std::sort(hullY.begin(),hullY.end());
    float minGap=999.0f; for(std::size_t i=1;i<hullY.size();++i) minGap=std::min(minGap,hullY[i]-hullY[i-1]);
    TEST("Pass347 heavy hull sections no longer occupy nearly the same center",hullY.size()>=2&&minGap>3.4f);
    TEST("Pass347 recipe remains bounded in module count",r->modules.size()<24);
}

static void TestPass348ReadableStaging(){
    std::cout<<"[Pass348ReadableStaging]\n";
    auto c=ProceduralVisualVariantSystem::Build(Sources(),348u,12);
    auto* r=ProceduralVisualVariantSystem::Select(c,"Heavy Hauler",9u);
    auto cockpits=Find(*r,"cockpit_basic"); auto engines=Find(*r,"engine_main");
    float frontHull=-999.0f,aftHull=999.0f;
    for(const auto& p:r->modules) if(p.moduleId=="hull_section"||p.moduleId=="hull_section_enhanced"){frontHull=std::max(frontHull,p.y);aftHull=std::min(aftHull,p.y);}
    TEST("Pass348 cockpit is staged ahead of the hull spine",!cockpits.empty()&&cockpits.front()->y>frontHull);
    TEST("Pass348 paired engines are staged behind the hull spine",engines.size()>=2&&engines[0]->y<aftHull&&engines[1]->y<aftHull);
    TEST("Pass348 engine pods are separated laterally",engines.size()>=2&&std::fabs(engines[0]->x-engines[1]->x)>2.0f);
    bool canted=false; for(auto* e:engines) canted=canted||std::fabs(e->yawDegrees)>.1f||std::fabs(e->rollDegrees)>.1f;
    TEST("Pass348 propulsion pods use authored canting",canted);
}

static void TestPass349FunctionalMasses(){
    std::cout<<"[Pass349FunctionalMasses]\n";
    auto c=ProceduralVisualVariantSystem::Build(Sources(),349u,12);
    auto* miner=ProceduralVisualVariantSystem::Select(c,"Mining",3u);
    auto cargo=Find(*miner,"cargo_bay"); auto shoulders=Find(*miner,"hull_section_small");
    bool cargoOutside=false; for(auto* p:cargo) cargoOutside=cargoOutside||std::fabs(p->x)>2.0f;
    bool shoulderOutside=false; for(auto* p:shoulders) shoulderOutside=shoulderOutside||std::fabs(p->x)>1.5f;
    TEST("Pass349 cargo pods sit outside the central spine",cargoOutside);
    TEST("Pass349 shoulder armor creates readable side masses",shoulderOutside);
    TEST("Pass349 mining role retains visible functional mounts",Find(*miner,"weapon_mount").size()>=2);
    TEST("Pass349 mining role retains exposed maneuver thruster modules",Find(*miner,"thruster").size()>=2);
}

static void TestPass350RoleSurfaceIdentity(){
    std::cout<<"[Pass350RoleSurfaceIdentity]\n";
    auto c=ProceduralVisualVariantSystem::Build(Sources(),350u,12);
    auto* combat=ProceduralVisualVariantSystem::Select(c,"Combat Frigate",1u);
    auto* industrial=ProceduralVisualVariantSystem::Select(c,"Industrial Hauler",1u);
    auto* explore=ProceduralVisualVariantSystem::Select(c,"Exploration Scout",1u);
    TEST("Pass350 combat receives stronger accent authority than industrial",combat->accentStrength>industrial->accentStrength);
    TEST("Pass350 industrial retains stronger armor breakup than exploration",industrial->armorBreakup>explore->armorBreakup);
    TEST("Pass350 role silhouettes keep independent width/length authority",std::fabs(combat->widthScale-industrial->widthScale)>.01f||std::fabs(combat->lengthScale-industrial->lengthScale)>.01f);
}

static void TestPass351Determinism(){
    std::cout<<"[Pass351Determinism]\n";
    auto a=ProceduralVisualVariantSystem::Build(Sources(),0xA352u,12);
    auto b=ProceduralVisualVariantSystem::Build(Sources(),0xA352u,12);
    auto* ra=ProceduralVisualVariantSystem::Select(a,"Carrier",77u);
    auto* rb=ProceduralVisualVariantSystem::Select(b,"Carrier",77u);
    TEST("Pass351 regenerated visual recipe selects same id",ra&&rb&&ra->recipeId==rb->recipeId);
    TEST("Pass351 regenerated recipe preserves module count",ra&&rb&&ra->modules.size()==rb->modules.size());
    bool same=true; if(ra&&rb&&ra->modules.size()==rb->modules.size()) for(std::size_t i=0;i<ra->modules.size();++i){
        const auto& x=ra->modules[i]; const auto& y=rb->modules[i];
        if(x.moduleId!=y.moduleId||std::fabs(x.x-y.x)>1e-6f||std::fabs(x.y-y.y)>1e-6f||std::fabs(x.yawDegrees-y.yawDegrees)>1e-6f) same=false;
    }
    TEST("Pass351 module transforms remain deterministic",same);
}

static void TestPass352CompactVsHeavy(){
    std::cout<<"[Pass352CompactVsHeavy]\n";
    auto c=ProceduralVisualVariantSystem::Build(Sources(),352u,12);
    auto* heavy=ProceduralVisualVariantSystem::Select(c,"Heavy Hauler",6u);
    auto* scout=ProceduralVisualVariantSystem::Select(c,"Exploration Scout",6u);
    TEST("Pass352 heavy and scout recipes are materially different",heavy&&scout&&heavy->modules.size()!=scout->modules.size());
    TEST("Pass352 heavy ship has twin main engines",Find(*heavy,"engine_main").size()==2);
    TEST("Pass352 scout remains compact with small engines",!Find(*scout,"engine_small").empty());
}


static void TestPass351HardSurfaceFinish(){
    std::cout<<"[Pass351HardSurfaceFinish]\n";
    auto c=ProceduralVisualVariantSystem::Build(Sources(),35151u,12);
    auto* r=ProceduralVisualVariantSystem::Select(c,"Industrial Hauler",17u);
    auto countKind=[&](VisualDetailKind kind){int n=0;for(const auto& d:r->details)if(d.kind==kind)++n;return n;};
    TEST("Pass351 generated ship receives layered armor plates",r&&countKind(VisualDetailKind::ArmorPlate)>=6);
    TEST("Pass351 generated ship receives structural spine/ribs",r&&countKind(VisualDetailKind::StructuralRib)>=3);
    TEST("Pass351 generated ship receives vents and radiator machinery",r&&countKind(VisualDetailKind::Vent)>=2&&countKind(VisualDetailKind::Radiator)>=2);
    TEST("Pass351 generated ship receives maintenance/decal scale cues",r&&countKind(VisualDetailKind::MaintenanceHatch)>=1&&countKind(VisualDetailKind::DecalStripe)>=2);
    TEST("Pass351 manufacturer visual DNA is assigned",r&&!r->manufacturerFamily.empty()&&!r->decalCode.empty());
    TEST("Pass351 controlled asymmetry remains bounded",r&&r->asymmetryStrength>=0.0f&&r->asymmetryStrength<0.5f);
    const auto armor=SpaceMaterialSystem::GetProfile(SpaceMaterialKind::ArmorPlate);
    const auto structural=SpaceMaterialSystem::GetProfile(SpaceMaterialKind::StructuralMetal);
    const auto heat=SpaceMaterialSystem::GetProfile(SpaceMaterialKind::HeatShield);
    TEST("Pass351 armor receives hard-surface edge response",armor.edgeHighlight>.25f&&armor.metallic>.75f);
    TEST("Pass351 structural metal receives stronger cavity treatment",structural.cavityStrength>armor.cavityStrength);
    TEST("Pass351 heat shield carries role-appropriate wear",heat.wearStrength>.20f&&heat.roughness>.85f);
}

static void TestPass352ArtDirector(){
    std::cout<<"[Pass352ArtDirector]\n";
    auto c=ProceduralVisualVariantSystem::Build(Sources(),35252u,12);
    float minimum=999.0f; int accepted=0; int rich=0;
    for(const auto& r:c.shipRecipes){minimum=std::min(minimum,r.qualityScore);if(r.acceptedByArtDirector)++accepted;if(r.details.size()>=10)++rich;}
    TEST("Pass352 all generated recipes clear visual art-director gate",accepted==static_cast<int>(c.shipRecipes.size()));
    TEST("Pass352 catalog quality floor remains production-readable",minimum>=78.0f);
    TEST("Pass352 all generated recipes receive finishing detail",rich==static_cast<int>(c.shipRecipes.size()));
    TEST("Pass352 generated catalog still exposes broad variation",c.shipRecipes.size()>=120);
    TEST("Pass352 shader carries edge/cavity/wear uniforms",std::string(SpaceMaterialSystem::FragmentShader120()).find("uEdgeHighlight")!=std::string::npos&&std::string(SpaceMaterialSystem::FragmentShader120()).find("uCavityStrength")!=std::string::npos&&std::string(SpaceMaterialSystem::FragmentShader120()).find("uWearStrength")!=std::string::npos);
}

int main(){
    TestPass347BoundsAwareAssembly(); TestPass348ReadableStaging(); TestPass349FunctionalMasses();
    TestPass350RoleSurfaceIdentity(); TestPass351Determinism(); TestPass351HardSurfaceFinish(); TestPass352CompactVsHeavy(); TestPass352ArtDirector();
    std::cout<<"\n=== Pass347-352 Ship Visual Reconstruction Summary: "<<testsPassed<<" passed, "<<testsFailed<<" failed ===\n";
    return testsFailed?1:0;
}
