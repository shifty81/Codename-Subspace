#include "station/StationRuntimeQualitySystem.h"
#include "station/StationDesignGrammarSystem.h"
#include <algorithm>
#include <cmath>
#include <set>
namespace subspace {
namespace {
bool Structural(const ShipyardModuleRecord&r){return r.partRole==ShipyardPartRole::PrimaryHull||r.partRole==ShipyardPartRole::StructuralFrame||r.partRole==ShipyardPartRole::StructuralAttachment||r.partRole==ShipyardPartRole::StructuralBrace||r.partRole==ShipyardPartRole::StructuralBlock||r.semantic==ShipyardModuleSemantic::StructuralFrame||r.semantic==ShipyardModuleSemantic::HullMid;}
bool Command(const ShipyardModuleRecord&r){return r.partRole==ShipyardPartRole::Bridge||r.partRole==ShipyardPartRole::Cockpit||r.semantic==ShipyardModuleSemantic::CommandBridge||r.semantic==ShipyardModuleSemantic::CommandCockpit;}
bool Service(const ShipyardModuleRecord&r){return r.partRole==ShipyardPartRole::Cargo||r.partRole==ShipyardPartRole::Tank||r.partRole==ShipyardPartRole::Hangar||r.partRole==ShipyardPartRole::SurfaceDetail||r.semantic==ShipyardModuleSemantic::Component;}
bool Sensor(const ShipyardModuleRecord&r){return r.partRole==ShipyardPartRole::SensorDish||r.partRole==ShipyardPartRole::SensorMast||r.partRole==ShipyardPartRole::SensorAntenna||r.partRole==ShipyardPartRole::Telescope||r.semantic==ShipyardModuleSemantic::Sensor;}
bool Hardpoint(const ShipyardModuleRecord&r){return r.partRole==ShipyardPartRole::HardpointBase||r.partRole==ShipyardPartRole::WeaponTurret||r.partRole==ShipyardPartRole::MissileMount||r.semantic==ShipyardModuleSemantic::TurretHardpoint||r.semantic==ShipyardModuleSemantic::WeaponMount;}
const ShipyardModuleRecord* Find(const std::vector<ShipyardModuleRecord>&c,const std::string&id){auto it=std::find_if(c.begin(),c.end(),[&](const auto&r){return r.source.moduleId==id;});return it==c.end()?nullptr:&*it;}
float Radius(const ShipyardModuleRecord&r,const VisualModulePlacement&p){const float x=r.source.halfWidth*std::abs(p.scaleX),y=r.source.halfLength*std::abs(p.scaleY),z=r.source.halfHeight*std::abs(p.scaleZ);return std::sqrt(x*x+y*y+z*z);}
float Dist(const VisualModulePlacement&a,const VisualModulePlacement&b){const float x=a.x-b.x,y=a.y-b.y,z=a.z-b.z;return std::sqrt(x*x+y*y+z*z);}
bool RelatedByJoint(const StationKitbashVisualRecipe&r,std::size_t a,std::size_t b){
    std::size_t parentA=static_cast<std::size_t>(-1),parentB=static_cast<std::size_t>(-1);
    for(const auto&e:r.attachments){
        if((e.parentModuleIndex==a&&e.childModuleIndex==b)||(e.parentModuleIndex==b&&e.childModuleIndex==a))return true;
        if(e.childModuleIndex==a)parentA=e.parentModuleIndex;
        if(e.childModuleIndex==b)parentB=e.parentModuleIndex;
    }
    // Siblings deliberately insert into the same junction/hub volume. Treat
    // that as structural mating, not accidental free-space co-location.
    return parentA!=static_cast<std::size_t>(-1)&&parentA==parentB;
}
}
StationRuntimeQualityReport StationRuntimeQualitySystem::Audit(const std::vector<ShipyardModuleRecord>& catalog,StationArchetype archetype,std::uint64_t seed,bool asteroidEmbedded) const{
    StationRuntimeQualityReport q;
    const bool hasStructural=std::any_of(catalog.begin(),catalog.end(),[](const auto&r){return r.generatorEligible&&Structural(r);});
    q.primitiveFallbackAllowed=!hasStructural;
    const auto recipe=StationKitbashVisualSystem::Build(catalog,archetype,seed,asteroidEmbedded);
    q.moduleCount=static_cast<int>(recipe.modules.size());q.connectedGraph=recipe.connectedGraph;q.dockConnected=recipe.usedDedicatedDock;q.maxConnectionGap=recipe.maxMeasuredGap;
    for(const auto&p:recipe.modules){const auto* r=Find(catalog,p.moduleId);if(!r)continue;q.structuralModules+=Structural(*r);q.commandModules+=Command(*r);q.serviceModules+=Service(*r);q.sensorModules+=Sensor(*r);q.hardpointModules+=Hardpoint(*r);}
    const auto grammar=StationDesignGrammarSystem::ForArchetype(archetype,asteroidEmbedded);
    for(const auto role:grammar.requiredPieces)if(std::find(recipe.logicalRoles.begin(),recipe.logicalRoles.end(),role)==recipe.logicalRoles.end())++q.missingRequiredPieces;
    // Only flag severe unrelated co-location. Parent/child insertion overlap is
    // intentional and is the mechanism that prevents visible gaps.
    for(std::size_t i=0;i<recipe.modules.size();++i){const auto* a=Find(catalog,recipe.modules[i].moduleId);if(!a)continue;for(std::size_t j=i+1;j<recipe.modules.size();++j){if(RelatedByJoint(recipe,i,j))continue;const auto*b=Find(catalog,recipe.modules[j].moduleId);if(!b)continue;const float limit=(Radius(*a,recipe.modules[i])+Radius(*b,recipe.modules[j]))*.16f;if(Dist(recipe.modules[i],recipe.modules[j])<limit)++q.overlapConflicts;}}
    const int minimum=asteroidEmbedded?6:9;
    if(!recipe.resolved)q.issues.push_back("kitbash recipe unresolved");
    if(q.moduleCount<minimum)q.issues.push_back("insufficient module silhouette complexity");
    if(!q.connectedGraph)q.issues.push_back("station visual graph is disconnected");
    if(q.structuralModules<2)q.issues.push_back("insufficient structural spine/connectors");
    if(!asteroidEmbedded&&q.commandModules<1)q.issues.push_back("missing command/readability module");
    if(!q.dockConnected)q.issues.push_back("station dock is not anchored to generated structure");
    if(q.missingRequiredPieces>0)q.issues.push_back("archetype grammar is missing required logical pieces");
    if(q.maxConnectionGap>.15f)q.issues.push_back("station attachment gap exceeds certification tolerance");
    if(q.overlapConflicts>0)q.issues.push_back("unrelated station modules occupy the same structural volume");
    if(archetype==StationArchetype::Military&&q.hardpointModules<1)q.issues.push_back("military station missing hardpoint vocabulary");
    if((archetype==StationArchetype::Research||archetype==StationArchetype::CorporateHQ)&&q.sensorModules<1)q.issues.push_back("sensor-led station missing sensor vocabulary");
    if(hasStructural&&q.primitiveFallbackAllowed)q.issues.push_back("primitive fallback enabled despite certified structure");
    q.certified=q.issues.empty();return q;
}
}
