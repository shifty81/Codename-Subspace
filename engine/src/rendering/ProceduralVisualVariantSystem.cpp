#include "rendering/ProceduralVisualVariantSystem.h"

#include <algorithm>
#include <cmath>
#include <cctype>
#include <unordered_map>
#include <unordered_set>

namespace subspace {
namespace {

std::uint32_t Next(std::uint32_t& state) {
    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;
    return state ? state : 0x9E3779B9u;
}

float Unit(std::uint32_t& state) {
    return static_cast<float>(Next(state) & 0x00FFFFFFu) / static_cast<float>(0x00FFFFFFu);
}

bool Has(const std::unordered_set<std::string>& available, const char* id) {
    return available.find(id) != available.end();
}

void Add(std::vector<VisualModulePlacement>& out,
         const std::unordered_set<std::string>& available,
         const char* module,
         float x, float y, float z,
         float sx, float sy, float sz,
         SpaceMaterialKind material,
         float yaw = 0.0f,
         float pitch = 0.0f,
         float roll = 0.0f) {
    if (!Has(available, module)) return;
    VisualModulePlacement p;
    p.moduleId = module;
    p.x=x; p.y=y; p.z=z;
    p.scaleX=sx; p.scaleY=sy; p.scaleZ=sz;
    p.yawDegrees=yaw; p.pitchDegrees=pitch; p.rollDegrees=roll;
    p.material=material;
    out.push_back(std::move(p));
}

bool HeavyRole(const std::string& role) {
    return role == "INDUSTRIAL" || role == "HAULER" || role == "MINING" ||
           role == "SALVAGE" || role == "CARRIER";
}

bool CombatRole(const std::string& role) {
    return role == "COMBAT" || role == "ESCORT" || role == "PATROL";
}

VisualModuleSource LegacyMetric(const std::string& id) {
    // Current authored-library measurements after runtime axis remapping. These
    // values are fallbacks for legacy callers/tests; the renderer supplies
    // measured bounds from each loaded OBJ at runtime.
    VisualModuleSource s{id,1.0f,1.0f,1.0f};
    if(id=="cargo_bay")                 return {id,4.20f,3.20f,3.00f};
    if(id=="cockpit_basic")             return {id,2.80f,3.50f,2.20f};
    if(id=="cockpit_small")             return {id,2.00f,2.50f,1.70f};
    if(id=="engine_main")               return {id,2.20f,2.50f,2.20f};
    if(id=="engine_small")              return {id,1.80f,3.00f,1.80f};
    if(id=="hull_section")              return {id,3.00f,4.20f,2.00f};
    if(id=="hull_section_enhanced")     return {id,3.00f,3.00f,3.50f};
    if(id=="hull_section_small")        return {id,2.10f,3.20f,1.70f};
    if(id=="power_core")                return {id,2.50f,2.50f,2.00f};
    if(id=="sensor_array")              return {id,2.80f,2.80f,1.15f};
    if(id=="thruster"||id=="thruster_small") return {id,1.20f,1.50f,1.20f};
    if(id=="weapon_mount")              return {id,1.20f,2.00f,1.20f};
    if(id=="wing_left"||id=="wing_right") return {id,4.50f,3.20f,0.60f};
    if(id=="wing_small_left"||id=="wing_small_right") return {id,4.00f,2.80f,0.90f};
    return s;
}

using MetricMap = std::unordered_map<std::string,VisualModuleSource>;

const VisualModuleSource& Metric(const MetricMap& metrics,const std::string& id) {
    auto it=metrics.find(id);
    if(it!=metrics.end()) return it->second;
    static VisualModuleSource fallback{"fallback",1.0f,1.0f,1.0f};
    return fallback;
}

float HalfLength(const MetricMap& metrics,const std::string& id,float sy) {
    return std::max(0.25f,Metric(metrics,id).halfLength*std::abs(sy));
}
float HalfWidth(const MetricMap& metrics,const std::string& id,float sx) {
    return std::max(0.25f,Metric(metrics,id).halfWidth*std::abs(sx));
}
float HalfHeight(const MetricMap& metrics,const std::string& id,float sz) {
    return std::max(0.20f,Metric(metrics,id).halfHeight*std::abs(sz));
}

struct SpineLayout {
    std::vector<float> centers;
    float frontEdge=0.0f;
    float aftEdge=0.0f;
    float halfWidth=1.0f;
};

SpineLayout BuildSpine(const std::vector<std::string>& ids,
                       const std::vector<float>& sx,
                       const std::vector<float>& sy,
                       const MetricMap& metrics) {
    SpineLayout out;
    if(ids.empty()) return out;
    out.centers.resize(ids.size(),0.0f);
    for(std::size_t i=1;i<ids.size();++i){
        const float prev=HalfLength(metrics,ids[i-1],sy[i-1]);
        const float cur =HalfLength(metrics,ids[i],sy[i]);
        // Deliberate 18-28% longitudinal overlap visually welds adjacent hull
        // modules without the 55-65% overlap that created the old block pile.
        const float overlap=std::min(prev,cur)*0.46f;
        out.centers[i]=out.centers[i-1]+prev+cur-overlap;
    }
    const float minEdge=out.centers.front()-HalfLength(metrics,ids.front(),sy.front());
    const float maxEdge=out.centers.back()+HalfLength(metrics,ids.back(),sy.back());
    const float center=(minEdge+maxEdge)*0.5f;
    for(float& y:out.centers)y-=center;
    out.aftEdge=minEdge-center;
    out.frontEdge=maxEdge-center;
    for(std::size_t i=0;i<ids.size();++i)
        out.halfWidth=std::max(out.halfWidth,HalfWidth(metrics,ids[i],sx[i]));
    return out;
}


void AddDetail(std::vector<VisualDetailPlacement>& out,VisualDetailKind kind,
               float x,float y,float z,float sx,float sy,float sz,SpaceMaterialKind material,
               float yaw=0.0f,float intensity=1.0f,float pitch=0.0f,float roll=0.0f){
    VisualDetailPlacement d; d.kind=kind; d.x=x; d.y=y; d.z=z; d.sizeX=sx; d.sizeY=sy; d.sizeZ=sz;
    d.material=material; d.yawDegrees=yaw; d.pitchDegrees=pitch; d.rollDegrees=roll; d.intensity=intensity; out.push_back(d);
}

struct RecipeBounds { float minX=999.0f,maxX=-999.0f,minY=999.0f,maxY=-999.0f,maxZ=-999.0f; };
RecipeBounds BoundsFor(const ProceduralShipVisualRecipe& r,const MetricMap& metrics){
    RecipeBounds b;
    for(const auto& p:r.modules){
        const auto& m=Metric(metrics,p.moduleId);
        const float hx=std::max(.2f,m.halfWidth*std::abs(p.scaleX));
        const float hy=std::max(.2f,m.halfLength*std::abs(p.scaleY));
        const float hz=std::max(.15f,m.halfHeight*std::abs(p.scaleZ));
        b.minX=std::min(b.minX,p.x-hx); b.maxX=std::max(b.maxX,p.x+hx);
        b.minY=std::min(b.minY,p.y-hy); b.maxY=std::max(b.maxY,p.y+hy); b.maxZ=std::max(b.maxZ,p.z+hz);
    }
    if(r.modules.empty()) b={-1,1,-2,2,1};
    return b;
}


std::string ChooseCockpitFamily(const std::string& role,const std::string& manufacturer,std::uint32_t& state){
    static const char* combatFamilies[]={"WEDGE","RECESSED_MILITARY","INTERCEPTOR_CANOPY","SPLIT_COMMAND"};
    static const char* heavyFamilies[]={"ARMORED_BRIDGE","INDUSTRIAL_TOWER","HEAVY_COMMAND_DECK","SPLIT_COMMAND"};
    static const char* explorationFamilies[]={"SENSOR_FORWARD","INTERCEPTOR_CANOPY","WEDGE","SPLIT_COMMAND"};
    if(role=="EXPLORATION") return explorationFamilies[Next(state)%4u];
    if(CombatRole(role)) return combatFamilies[Next(state)%4u];
    if(HeavyRole(role)) return heavyFamilies[Next(state)%4u];
    if(manufacturer=="HELIX") return (Next(state)&1u)?"WEDGE":"INTERCEPTOR_CANOPY";
    if(manufacturer=="IRONWORKS"||manufacturer=="ORBITAL FORGE") return (Next(state)&1u)?"INDUSTRIAL_TOWER":"ARMORED_BRIDGE";
    static const char* generalFamilies[]={"WEDGE","ARMORED_BRIDGE","SENSOR_FORWARD","HEAVY_COMMAND_DECK"};
    return generalFamilies[Next(state)%4u];
}

void ApplyCockpitFamily(ProceduralShipVisualRecipe& r,const MetricMap& metrics,std::uint32_t& state){
    r.cockpitFamily=ChooseCockpitFamily(r.role,r.manufacturerFamily,state);
    std::size_t cockpitIndex=r.modules.size();
    for(std::size_t i=0;i<r.modules.size();++i){
        if(r.modules[i].moduleId.find("cockpit")!=std::string::npos){cockpitIndex=i;break;}
    }
    if(cockpitIndex>=r.modules.size()) return;
    auto& c=r.modules[cockpitIndex];
    if(r.cockpitFamily=="WEDGE"){
        c.scaleX*=1.16f;c.scaleY*=.90f;c.scaleZ*=.72f;c.z-=.05f;
    }else if(r.cockpitFamily=="ARMORED_BRIDGE"){
        c.scaleX*=1.28f;c.scaleY*=.82f;c.scaleZ*=.96f;c.z+=.08f;
    }else if(r.cockpitFamily=="RECESSED_MILITARY"){
        c.scaleX*=1.06f;c.scaleY*=.96f;c.scaleZ*=.62f;c.z-=.18f;
    }else if(r.cockpitFamily=="INDUSTRIAL_TOWER"){
        c.scaleX*=.96f;c.scaleY*=.78f;c.scaleZ*=1.34f;c.z+=.32f;
    }else if(r.cockpitFamily=="INTERCEPTOR_CANOPY"){
        c.scaleX*=.76f;c.scaleY*=1.28f;c.scaleZ*=.70f;c.z+=.04f;
    }else if(r.cockpitFamily=="SPLIT_COMMAND"){
        VisualModulePlacement twin=c;
        const float half=HalfWidth(metrics,c.moduleId,c.scaleX);
        const float separation=std::max(.58f,half*.72f);
        c.x=-separation;c.scaleX*=.72f;c.scaleY*=.94f;c.scaleZ*=.78f;
        twin.x=separation;twin.scaleX*=.72f;twin.scaleY*=.94f;twin.scaleZ*=.78f;
        r.modules.push_back(twin);
    }else if(r.cockpitFamily=="SENSOR_FORWARD"){
        c.scaleX*=.84f;c.scaleY*=1.12f;c.scaleZ*=.78f;c.z+=.14f;
    }else if(r.cockpitFamily=="HEAVY_COMMAND_DECK"){
        c.scaleX*=1.46f;c.scaleY*=.88f;c.scaleZ*=1.08f;c.z+=.12f;
    }
}



bool IsStructuralCoreModule(const VisualModulePlacement& p){
    return p.moduleId.find("cockpit")!=std::string::npos || p.moduleId.find("hull")!=std::string::npos || p.moduleId=="power_core";
}
bool IsFunctionalAnchorModule(const VisualModulePlacement& p){
    return p.moduleId.find("engine")!=std::string::npos || p.moduleId.find("thruster")!=std::string::npos ||
           p.moduleId=="cargo_bay" || p.moduleId=="weapon_mount" || p.moduleId=="sensor_array" || p.moduleId=="power_core";
}
float BridgeYawDegrees(const Vector3& from,const Vector3& to){
    const float dx=to.x-from.x,dy=to.y-from.y;
    return std::atan2(-dx,dy)*57.29577951308232f;
}
void AddStructuralFillBetween(ProceduralShipVisualRecipe& r,const Vector3& from,const Vector3& to,float width,float height){
    const float dx=to.x-from.x,dy=to.y-from.y;const float d=std::sqrt(dx*dx+dy*dy);
    if(d<.28f)return;
    AddDetail(r.details,VisualDetailKind::StructuralFill,(from.x+to.x)*.5f,(from.y+to.y)*.5f,(from.z+to.z)*.5f,
              std::max(.32f,width),d+.28f,std::max(.24f,height),SpaceMaterialKind::StructuralMetal,BridgeYawDegrees(from,to),1.0f);
}
void BuildAnchorFirstStructure(ProceduralShipVisualRecipe& r,const MetricMap& metrics){
    r.anchors.clear();r.hardpoints.clear();
    if(r.modules.empty())return;
    std::vector<const VisualModulePlacement*> cores;
    for(const auto& p:r.modules)if(IsStructuralCoreModule(p))cores.push_back(&p);
    std::sort(cores.begin(),cores.end(),[](const auto*a,const auto*b){return a->y>b->y;});
    Vector3 origin{};
    int cockpitCount=0;
    for(const auto& p:r.modules)if(p.moduleId.find("cockpit")!=std::string::npos){origin=origin+Vector3{p.x,p.y,p.z};++cockpitCount;}
    if(cockpitCount>0)origin=origin*(1.0f/static_cast<float>(cockpitCount));else if(!cores.empty())origin={cores.front()->x,cores.front()->y,cores.front()->z};
    r.anchors.push_back({"COMMAND_ORIGIN",cockpitCount?"cockpit":"hull",origin,true});

    Vector3 previous=origin;int spineIndex=0;
    for(const auto* p:cores){
        if(p->moduleId.find("cockpit")!=std::string::npos)continue;
        const Vector3 here{p->x,p->y,p->z};
        r.anchors.push_back({"SPINE_"+std::to_string(spineIndex++),p->moduleId,here,false});
        AddStructuralFillBetween(r,previous,here,HeavyRole(r.role)?.72f:.52f,HeavyRole(r.role)?.54f:.40f);
        previous=here;
    }
    // Every non-core visible module is attached to the nearest established
    // command/spine anchor. Functional equipment is marked as such, but even
    // shoulder hulls, wings and decorative module masses receive a structural
    // path. This is the anchor-first rule: detail placement defines the target;
    // structure is filled from an already-connected origin before the module is
    // allowed to read as part of the ship.
    int detailIndex=0;
    for(const auto& p:r.modules){
        if(IsStructuralCoreModule(p)||p.moduleId.find("cockpit")!=std::string::npos)continue;
        const bool functional=IsFunctionalAnchorModule(p);
        const Vector3 here{p.x,p.y,p.z};
        Vector3 nearest=origin;float best=1.0e9f;
        for(const auto& a:r.anchors){const float dx=here.x-a.position.x,dy=here.y-a.position.y,dz=(here.z-a.position.z)*.35f;const float d=dx*dx+dy*dy+dz*dz;if(d<best){best=d;nearest=a.position;}}
        const std::string id=(functional?"FUNCTION_":"DETAIL_")+std::to_string(detailIndex++);
        r.anchors.push_back({id,p.moduleId,here,functional});
        float width=functional?.48f:.40f,height=functional?.38f:.32f;
        if(p.moduleId.find("engine")!=std::string::npos||p.moduleId.find("thruster")!=std::string::npos){width=.82f;height=.60f;}
        else if(p.moduleId=="cargo_bay"){width=.90f;height=.62f;}
        else if(p.moduleId=="power_core"){width=.72f;height=.54f;}
        else if(p.moduleId.find("wing")!=std::string::npos){width=.56f;height=.34f;}
        else if(p.moduleId.find("hull")!=std::string::npos){width=.68f;height=.44f;}
        AddStructuralFillBetween(r,nearest,here,width,height);
        if(p.moduleId=="weapon_mount"){
            ShipVisualHardpoint hp;hp.id="TURRET_"+std::to_string(r.hardpoints.size()+1);hp.position=here;hp.yawDegrees=p.yawDegrees;
            hp.size=HeavyRole(r.role)?FittingHardpointSize::Medium:FittingHardpointSize::Small;r.hardpoints.push_back(hp);
            AddDetail(r.details,VisualDetailKind::TurretSocket,p.x,p.y,p.z+.08f,.82f,.82f,.20f,SpaceMaterialKind::StructuralMetal,p.yawDegrees,1.0f);
        }
    }
    // Even utility/exploration hulls need visible empty fitting points. These
    // sockets are generated on the supported spine, never as free-floating decorations.
    if(r.hardpoints.empty()){
        const RecipeBounds b=BoundsFor(r,metrics);const float w=std::max(1.0f,(b.maxX-b.minX)*.28f);const float y=(b.minY+b.maxY)*.5f;
        for(int side=-1;side<=1;side+=2){ShipVisualHardpoint hp;hp.id=side<0?"TURRET_PORT":"TURRET_STARBOARD";hp.position={side*w,y,b.maxZ-.10f};hp.size=FittingHardpointSize::Small;r.hardpoints.push_back(hp);AddStructuralFillBetween(r,{0,y,b.maxZ-.22f},hp.position,.44f,.30f);AddDetail(r.details,VisualDetailKind::TurretSocket,hp.position.x,hp.position.y,hp.position.z,.68f,.68f,.18f,SpaceMaterialKind::StructuralMetal,0,1.0f);}
    }
}
std::string Manufacturer(std::uint32_t seed,const std::string& role){
    if(role=="INDUSTRIAL"||role=="MINING"||role=="HAULER") return (seed&1u)?"IRONWORKS":"ORBITAL FORGE";
    if(role=="EXPLORATION") return (seed&1u)?"HELIX":"NOMAD";
    if(role=="SALVAGE") return "NOMAD";
    if(role=="CARRIER") return "IRON DOMINION";
    return (seed&1u)?"HELIX":"VANGUARD";
}

void FinishRecipe(ProceduralShipVisualRecipe& r,const MetricMap& metrics,std::uint32_t& state){
    const bool heavy=HeavyRole(r.role), combat=CombatRole(r.role);
    r.manufacturerFamily=Manufacturer(r.seed,r.role);
    ApplyCockpitFamily(r,metrics,state);
    BuildAnchorFirstStructure(r,metrics);
    const RecipeBounds b=BoundsFor(r,metrics);
    const float width=std::max(2.0f,b.maxX-b.minX), length=std::max(4.0f,b.maxY-b.minY);
    const float centerY=(b.minY+b.maxY)*.5f;
    r.decalCode=(combat?"VF-":heavy?"IND-":"SC-")+std::to_string(10u+(r.seed%90u));
    r.negativeSpaceStrength=std::clamp((heavy?.42f:.34f)+Unit(state)*.20f,0.0f,1.0f);
    r.asymmetryStrength=(r.role=="SALVAGE"?.30f:(heavy?.12f:(r.role=="EXPLORATION"?.16f:.05f))) * (.72f+Unit(state)*.56f);
    r.articulationDegrees=(combat?7.0f:(r.role=="MINING"?13.0f:(r.role=="SALVAGE"?10.0f:4.0f))) * (.72f+Unit(state)*.38f);

    // Surface-aware detail placement. Global max-Z made armor plates hover
    // above lower hull modules whenever a tall cockpit/command structure set
    // the recipe bound. Resolve the actual authored surface underneath each
    // detail and deliberately sink the finishing geometry a little into it.
    const auto surfaceTop=[&](float x,float y,float fallback){
        float top=-999.0f;
        for(const auto& p:r.modules){
            const float hx=HalfWidth(metrics,p.moduleId,p.scaleX)*1.08f;
            const float hy=HalfLength(metrics,p.moduleId,p.scaleY)*1.08f;
            if(std::fabs(x-p.x)>hx||std::fabs(y-p.y)>hy)continue;
            top=std::max(top,p.z+HalfHeight(metrics,p.moduleId,p.scaleZ));
        }
        return top>-900.0f?top:fallback;
    };

    // Armor/fairing pass: broad plates stay sparse so the original authored
    // module silhouette remains visible beneath layered hard-surface skinning.
    const int armorRows=heavy?4:3;
    for(int i=0;i<armorRows;++i){
        const float t=(static_cast<float>(i)+.5f)/static_cast<float>(armorRows);
        const float y=b.minY+length*(.20f+.60f*t);
        const float taper=.76f-.18f*std::abs(t-.5f)*2.0f;
        const float plateW=width*(heavy?.30f:.34f)*taper;
        const float sideGap=width*(.12f+.08f*r.negativeSpaceStrength);
        const float leftZ=surfaceTop(-sideGap,y,b.maxZ-.18f)-.055f;
        const float rightZ=surfaceTop(sideGap,y,b.maxZ-.18f)-.055f;
        AddDetail(r.details,VisualDetailKind::ArmorPlate,-sideGap,y,leftZ,plateW,length/(armorRows*3.2f),.24f,SpaceMaterialKind::ArmorPlate,-2.0f-r.articulationDegrees*.18f);
        AddDetail(r.details,VisualDetailKind::ArmorPlate, sideGap,y,rightZ,plateW,length/(armorRows*3.2f),.24f,SpaceMaterialKind::ArmorPlate, 2.0f+r.articulationDegrees*.18f);
    }
    {const float fy=b.maxY-length*.22f;AddDetail(r.details,VisualDetailKind::Fairing,0,fy,surfaceTop(0,fy,b.maxZ-.20f)-.06f,width*.34f,length*.18f,.26f,SpaceMaterialKind::ArmorPlate,0.0f);}

    // Visible structural spine and ribs under the plated shell create the
    // machinery-vs-armor contrast present in the modular concept references.
    AddDetail(r.details,VisualDetailKind::StructuralRib,0,centerY,b.maxZ-.36f,width*.12f,length*.68f,.22f,SpaceMaterialKind::StructuralMetal);
    for(int i=-1;i<=1;++i) AddDetail(r.details,VisualDetailKind::StructuralRib,0,centerY+i*length*.19f,b.maxZ-.18f,width*.46f,.16f,.18f,SpaceMaterialKind::StructuralMetal);

    // Pass361-400 refinement: every visibly offset functional module receives
    // a continuous structural path back to the central hull. The previous
    // bridge pass only closed lateral X gaps; rear engines/thrusters could
    // therefore remain visibly detached along Y. This builds an L-shaped
    // pylon path when necessary and also handles pure aft/forward separation.
    float coreHalfWidth=1.0f,coreMinY=999.0f,coreMaxY=-999.0f;
    for(const auto& p:r.modules){
        if(p.moduleId.find("hull_section")!=std::string::npos || p.moduleId=="power_core"){
            coreHalfWidth=std::max(coreHalfWidth,HalfWidth(metrics,p.moduleId,p.scaleX));
            const float hy=HalfLength(metrics,p.moduleId,p.scaleY);
            coreMinY=std::min(coreMinY,p.y-hy);coreMaxY=std::max(coreMaxY,p.y+hy);
        }
    }
    if(coreMinY>coreMaxY){coreMinY=b.minY+length*.20f;coreMaxY=b.maxY-length*.20f;}
    for(const auto& p:r.modules){
        const bool coreModule=p.moduleId.find("hull_section")!=std::string::npos || p.moduleId=="power_core";
        const bool cockpitModule=p.moduleId.find("cockpit")!=std::string::npos;
        // No external authored module is allowed to float. Wings, sensors,
        // cargo, weapons, engines and utility pieces all use the same visible
        // structural attachment grammar; only central spine/core and cockpit
        // modules are exempt because they are themselves primary structure.
        const bool bridgeable=!coreModule&&!cockpitModule;
        if(!bridgeable) continue;
        const float hx=HalfWidth(metrics,p.moduleId,p.scaleX);
        const float hy=HalfLength(metrics,p.moduleId,p.scaleY);
        const float safeCoreX=coreHalfWidth*.86f;
        const float anchorY=std::clamp(p.y,coreMinY+.12f,coreMaxY-.12f);
        const float moduleInnerX=std::max(0.0f,std::fabs(p.x)-hx*.72f);
        const bool needsLateral=moduleInnerX>safeCoreX+.10f;
        const bool aft=p.y+hy*.62f<coreMinY-.08f;
        const bool forward=p.y-hy*.62f>coreMaxY+.08f;
        const bool needsLongitudinal=aft||forward;
        const float signX=p.x<0.0f?-1.0f:1.0f;
        if(needsLateral){
            const float outerX=signX*moduleInnerX;
            const float innerX=signX*safeCoreX;
            const float span=std::fabs(outerX-innerX)+.34f;
            const float centerX=(outerX+innerX)*.5f;
            AddDetail(r.details,VisualDetailKind::MountBridge,centerX,anchorY,p.z-.02f,span,.62f,.54f,
                      SpaceMaterialKind::StructuralMetal,0.0f,.98f);
            AddDetail(r.details,VisualDetailKind::StructuralRib,outerX,anchorY,p.z-.02f,.64f,.72f,.58f,SpaceMaterialKind::StructuralMetal);
            AddDetail(r.details,VisualDetailKind::StructuralRib,innerX,anchorY,p.z-.02f,.64f,.72f,.58f,SpaceMaterialKind::StructuralMetal);
        }
        if(needsLongitudinal){
            const float hullEdge=aft?coreMinY:coreMaxY;
            const float moduleEdge=aft?(p.y+hy*.58f):(p.y-hy*.58f);
            const float span=std::fabs(moduleEdge-hullEdge)+.38f;
            const float center=(moduleEdge+hullEdge)*.5f;
            // If the module is also lateral, the longitudinal pylon runs at
            // its X position and joins the lateral pylon at anchorY.
            const float bridgeX=needsLateral?p.x:std::clamp(p.x,-safeCoreX,safeCoreX);
            AddDetail(r.details,VisualDetailKind::MountBridge,bridgeX,center,p.z-.02f,.68f,span,.54f,
                      SpaceMaterialKind::StructuralMetal,0.0f,.98f);
            AddDetail(r.details,VisualDetailKind::StructuralRib,bridgeX,moduleEdge,p.z-.02f,.74f,.66f,.58f,SpaceMaterialKind::StructuralMetal);
            AddDetail(r.details,VisualDetailKind::StructuralRib,bridgeX,hullEdge,p.z-.02f,.74f,.66f,.58f,SpaceMaterialKind::StructuralMetal);
        }
    }

    // Cockpit family fairings turn the same authored source meshes into
    // materially different command-section lines without requiring copied
    // external geometry. These are structural armor/bridge treatments, not
    // decals, so the silhouette changes from multiple viewing angles.
    const VisualModulePlacement* cockpit=nullptr;
    for(const auto& p:r.modules)if(p.moduleId.find("cockpit")!=std::string::npos){cockpit=&p;break;}
    if(cockpit){
        const float cw=HalfWidth(metrics,cockpit->moduleId,cockpit->scaleX)*2.0f;
        const float cl=HalfLength(metrics,cockpit->moduleId,cockpit->scaleY)*2.0f;
        if(r.cockpitFamily=="WEDGE"){
            AddDetail(r.details,VisualDetailKind::Fairing,-cw*.32f,cockpit->y-cl*.18f,cockpit->z-.10f,cw*.42f,cl*.74f,.18f,SpaceMaterialKind::ArmorPlate,-12.0f);
            AddDetail(r.details,VisualDetailKind::Fairing, cw*.32f,cockpit->y-cl*.18f,cockpit->z-.10f,cw*.42f,cl*.74f,.18f,SpaceMaterialKind::ArmorPlate, 12.0f);
        }else if(r.cockpitFamily=="ARMORED_BRIDGE"||r.cockpitFamily=="HEAVY_COMMAND_DECK"){
            AddDetail(r.details,VisualDetailKind::ArmorPlate,0,cockpit->y-cl*.30f,cockpit->z+.12f,cw*1.22f,cl*.44f,.24f,SpaceMaterialKind::ArmorPlate);
            AddDetail(r.details,VisualDetailKind::Fairing,0,cockpit->y-cl*.58f,cockpit->z-.05f,cw*.88f,cl*.48f,.22f,SpaceMaterialKind::ArmorPlate);
        }else if(r.cockpitFamily=="RECESSED_MILITARY"){
            AddDetail(r.details,VisualDetailKind::ArmorPlate,-cw*.48f,cockpit->y-cl*.10f,cockpit->z+.20f,cw*.32f,cl*.70f,.20f,SpaceMaterialKind::ArmorPlate,-5.0f);
            AddDetail(r.details,VisualDetailKind::ArmorPlate, cw*.48f,cockpit->y-cl*.10f,cockpit->z+.20f,cw*.32f,cl*.70f,.20f,SpaceMaterialKind::ArmorPlate, 5.0f);
        }else if(r.cockpitFamily=="INDUSTRIAL_TOWER"){
            AddDetail(r.details,VisualDetailKind::StructuralRib,0,cockpit->y-cl*.46f,cockpit->z-.18f,cw*.72f,cl*.36f,.32f,SpaceMaterialKind::StructuralMetal);
            AddDetail(r.details,VisualDetailKind::ArmorPlate,0,cockpit->y-cl*.18f,cockpit->z+.46f,cw*.82f,cl*.24f,.18f,SpaceMaterialKind::ArmorPlate);
        }else if(r.cockpitFamily=="INTERCEPTOR_CANOPY"){
            AddDetail(r.details,VisualDetailKind::Fairing,0,cockpit->y-cl*.46f,cockpit->z-.10f,cw*.72f,cl*.78f,.16f,SpaceMaterialKind::ArmorPlate);
        }else if(r.cockpitFamily=="SPLIT_COMMAND"){
            AddDetail(r.details,VisualDetailKind::StructuralRib,0,cockpit->y-cl*.30f,cockpit->z-.12f,cw*.34f,cl*.76f,.24f,SpaceMaterialKind::StructuralMetal);
        }else if(r.cockpitFamily=="SENSOR_FORWARD"){
            AddDetail(r.details,VisualDetailKind::Fairing,0,cockpit->y-cl*.42f,cockpit->z-.10f,cw*.70f,cl*.66f,.16f,SpaceMaterialKind::ArmorPlate);
            AddDetail(r.details,VisualDetailKind::NavigationLight,0,cockpit->y+cl*.48f,cockpit->z+.28f,.22f,.22f,.22f,SpaceMaterialKind::ThrusterCore);
        }
    }

    // Engines receive heat shielding, radiator/vent fields and external
    // service conduits. This makes propulsion read as an assembly, not a glow.
    AddDetail(r.details,VisualDetailKind::HeatShield,-width*.20f,b.minY+length*.12f,b.maxZ-.02f,width*.20f,length*.12f,.12f,SpaceMaterialKind::HeatShield,-4.0f);
    AddDetail(r.details,VisualDetailKind::HeatShield, width*.20f,b.minY+length*.12f,b.maxZ-.02f,width*.20f,length*.12f,.12f,SpaceMaterialKind::HeatShield, 4.0f);
    AddDetail(r.details,VisualDetailKind::Vent,-width*.23f,b.minY+length*.27f,b.maxZ+.08f,width*.18f,length*.10f,.10f,SpaceMaterialKind::StructuralMetal);
    AddDetail(r.details,VisualDetailKind::Vent, width*.23f,b.minY+length*.27f,b.maxZ+.08f,width*.18f,length*.10f,.10f,SpaceMaterialKind::StructuralMetal);
    if(heavy){
        AddDetail(r.details,VisualDetailKind::Radiator,-width*.40f,centerY-length*.08f,b.maxZ-.12f,width*.14f,length*.24f,.08f,SpaceMaterialKind::Radiator,-5.0f-r.articulationDegrees*.20f);
        AddDetail(r.details,VisualDetailKind::Radiator, width*.40f,centerY-length*.08f,b.maxZ-.12f,width*.14f,length*.24f,.08f,SpaceMaterialKind::Radiator, 5.0f+r.articulationDegrees*.20f);
    }
    AddDetail(r.details,VisualDetailKind::Conduit,-width*.10f,centerY-length*.12f,b.maxZ-.24f,.10f,length*.38f,.10f,SpaceMaterialKind::StructuralMetal);
    AddDetail(r.details,VisualDetailKind::Conduit, width*.10f,centerY-length*.12f,b.maxZ-.24f,.10f,length*.38f,.10f,SpaceMaterialKind::StructuralMetal);

    // Hardpoint bases visually integrate replaceable weapon/mining modules.
    for(const auto& p:r.modules) if(p.moduleId=="weapon_mount")
        AddDetail(r.details,VisualDetailKind::HardpointBase,p.x,p.y,p.z-.16f,.72f,.78f,.18f,SpaceMaterialKind::StructuralMetal,p.yawDegrees);

    // Controlled asymmetry is detail-level by default: industrial/salvage
    // ships can carry one offset service hatch without destabilizing thrust.
    const float asymSign=(Next(state)&1u)?1.0f:-1.0f;
    AddDetail(r.details,VisualDetailKind::MaintenanceHatch,asymSign*width*(.18f+.18f*r.asymmetryStrength),centerY+length*.08f,b.maxZ+.18f,width*.16f,length*.12f,.05f,SpaceMaterialKind::DecalSurface,asymSign*3.0f);

    // Decal/hazard language and nav lights supply scale cues at hangar LOD.
    AddDetail(r.details,VisualDetailKind::DecalStripe,-width*.24f,b.maxY-length*.31f,b.maxZ+.20f,width*.20f,length*.045f,.035f,SpaceMaterialKind::DecalSurface,-6.0f,.85f);
    AddDetail(r.details,VisualDetailKind::DecalStripe, width*.24f,b.maxY-length*.31f,b.maxZ+.20f,width*.20f,length*.045f,.035f,SpaceMaterialKind::DecalSurface, 6.0f,.85f);
    AddDetail(r.details,VisualDetailKind::NavigationLight,-width*.46f,centerY,b.maxZ+.10f,.18f,.18f,.18f,SpaceMaterialKind::ThrusterCore,0.0f,.9f);
    AddDetail(r.details,VisualDetailKind::NavigationLight, width*.46f,centerY,b.maxZ+.10f,.18f,.18f,.18f,SpaceMaterialKind::ThrusterCore,0.0f,.9f);

    r.qualityScore=ProceduralVisualVariantSystem::EvaluateQuality(r);
    r.acceptedByArtDirector=r.qualityScore>=78.0f;
}

ProceduralShipVisualRecipe BuildRecipe(const std::string& role,
                                       const std::unordered_set<std::string>& available,
                                       const MetricMap& metrics,
                                       std::uint32_t seed,
                                       int index) {
    ProceduralShipVisualRecipe r;
    r.role = role;
    r.sourceFamily = "SUBSPACE_NATIVE";
    r.seed = seed;
    r.recipeId = "generated:" + role + ":" + std::to_string(index);

    std::uint32_t state = seed ? seed : 1u;
    const bool heavy = HeavyRole(role);
    const bool combat = CombatRole(role);
    const bool exploration = role == "EXPLORATION";
    r.accentStrength = combat ? .62f : (exploration ? .50f : .34f + Unit(state)*.15f);
    r.armorBreakup = heavy ? .60f : (combat ? .52f : .36f);

    if (heavy) {
        const int hullCount = role=="CARRIER" ? 3 : (2 + static_cast<int>(Next(state)%2u));
        std::vector<std::string> hullIds;
        std::vector<float> hullSx,hullSy;
        for(int i=0;i<hullCount;++i){
            const bool center=(i==hullCount/2);
            hullIds.push_back(center&&Has(available,"hull_section_enhanced")?"hull_section_enhanced":"hull_section");
            const float taper=1.0f-0.07f*std::abs(static_cast<float>(i)-static_cast<float>(hullCount-1)*.5f);
            hullSx.push_back((.72f+Unit(state)*.08f)*taper);
            hullSy.push_back(.70f+Unit(state)*.10f);
        }
        const SpineLayout spine=BuildSpine(hullIds,hullSx,hullSy,metrics);
        const float sideSpread=spine.halfWidth*(role=="CARRIER"?1.42f:1.22f);
        r.widthScale=.90f+Unit(state)*.12f;
        r.lengthScale=.96f+Unit(state)*.08f;

        for(int i=0;i<hullCount;++i){
            Add(r.modules,available,hullIds[i].c_str(),0.0f,spine.centers[i],0.0f,
                hullSx[i],hullSy[i],.72f+Unit(state)*.08f,SpaceMaterialKind::IndustrialHull);
        }

        const float cockpitSy=.62f;
        const float cockpitHalf=HalfLength(metrics,"cockpit_basic",cockpitSy);
        const float noseY=spine.frontEdge+cockpitHalf*.68f;
        Add(r.modules,available,"cockpit_basic",0.0f,noseY,.42f,.70f,cockpitSy,.72f,SpaceMaterialKind::Canopy);

        const float engineSy=.64f;
        const float engineHalf=HalfLength(metrics,"engine_main",engineSy);
        const float aftY=spine.aftEdge-engineHalf*.78f;
        const bool twinEngine=true;
        const float engineX=std::max(1.65f,sideSpread*.54f);
        if(twinEngine){
            const float cant=role=="CARRIER"?7.0f:4.0f;
            Add(r.modules,available,"engine_main",-engineX,aftY,-.10f,.62f,engineSy,.68f,SpaceMaterialKind::EngineHousing,-cant,0.0f,-2.0f);
            Add(r.modules,available,"engine_main", engineX,aftY,-.10f,.62f,engineSy,.68f,SpaceMaterialKind::EngineHousing, cant,0.0f, 2.0f);
        }

        // Shoulder armor/utility pods sit outside the spine rather than on top
        // of it, creating readable negative space between functional masses.
        if(Has(available,"hull_section_small")){
            const float shoulderY=spine.centers[hullCount/2]+.35f;
            Add(r.modules,available,"hull_section_small",-sideSpread*.82f,shoulderY,.16f,.38f,.50f,.54f,SpaceMaterialKind::ShipHull,-7.0f,0.0f,-5.0f);
            Add(r.modules,available,"hull_section_small", sideSpread*.82f,shoulderY,.16f,.38f,.50f,.54f,SpaceMaterialKind::ShipHull, 7.0f,0.0f, 5.0f);
        }

        const int cargoCount = role=="HAULER"?4:((role=="INDUSTRIAL"||role=="MINING")?2:(role=="CARRIER"?2:1));
        for(int i=0;i<cargoCount;++i){
            const float side=(i&1)?1.0f:-1.0f;
            const int row=i/2;
            const float y=(row==0?spine.centers.front():spine.centers.back())-.30f;
            Add(r.modules,available,"cargo_bay",side*sideSpread,y,-.18f,.38f,.46f,.46f,SpaceMaterialKind::IndustrialHull,side*2.5f);
        }

        Add(r.modules,available,"wing_left",0.0f,spine.centers[hullCount/2]-.65f,-.24f,.48f,.55f,.48f,SpaceMaterialKind::IndustrialHull);
        Add(r.modules,available,"wing_right",0.0f,spine.centers[hullCount/2]-.65f,-.24f,.48f,.55f,.48f,SpaceMaterialKind::IndustrialHull);
        Add(r.modules,available,"power_core",0.0f,spine.centers.front()-.30f,1.12f,.42f,.42f,.46f,SpaceMaterialKind::EngineHousing);
        Add(r.modules,available,"sensor_array",0.0f,noseY-cockpitHalf*.42f,1.28f,.30f,.34f,.34f,SpaceMaterialKind::EngineHousing);

        if(role=="MINING"||role=="SALVAGE"||role=="CARRIER"){
            Add(r.modules,available,"weapon_mount",-sideSpread*.70f,spine.frontEdge-.55f,.62f,.40f,.44f,.40f,SpaceMaterialKind::EngineHousing,-5.0f);
            Add(r.modules,available,"weapon_mount", sideSpread*.70f,spine.frontEdge-.55f,.62f,.40f,.44f,.40f,SpaceMaterialKind::EngineHousing, 5.0f);
        }
        if(Has(available,"thruster")){
            Add(r.modules,available,"thruster",-sideSpread*.92f,spine.centers.back(),.10f,.38f,.44f,.40f,SpaceMaterialKind::ThrusterCore,90.0f);
            Add(r.modules,available,"thruster", sideSpread*.92f,spine.centers.back(),.10f,.38f,.44f,.40f,SpaceMaterialKind::ThrusterCore,-90.0f);
        }
    } else {
        const int hullCount = combat ? 2 : (1+static_cast<int>(Next(state)%2u));
        std::vector<std::string> hullIds(static_cast<std::size_t>(hullCount),"hull_section_small");
        std::vector<float> hullSx(static_cast<std::size_t>(hullCount),combat?.80f:.72f);
        std::vector<float> hullSy(static_cast<std::size_t>(hullCount),exploration?.82f:.72f);
        if(hullCount>1){ hullSx.front()*=.88f; hullSx.back()*=1.02f; }
        const SpineLayout spine=BuildSpine(hullIds,hullSx,hullSy,metrics);
        const float spread=spine.halfWidth*(combat?1.34f:1.18f);
        r.widthScale=combat?1.02f+Unit(state)*.10f:.90f+Unit(state)*.10f;
        r.lengthScale=exploration?1.08f:.96f+Unit(state)*.08f;
        for(int i=0;i<hullCount;++i){
            Add(r.modules,available,"hull_section_small",0.0f,spine.centers[i],0.0f,hullSx[i],hullSy[i],.72f,SpaceMaterialKind::ShipHull);
        }

        const float cockpitSy=.68f;
        const float cockpitHalf=HalfLength(metrics,"cockpit_small",cockpitSy);
        const float noseY=spine.frontEdge+cockpitHalf*.70f;
        Add(r.modules,available,"cockpit_small",0.0f,noseY,.30f,.72f,cockpitSy,.74f,SpaceMaterialKind::Canopy);

        const float engineSy=.62f;
        const float engineHalf=HalfLength(metrics,"engine_small",engineSy);
        const float aftY=spine.aftEdge-engineHalf*.76f;
        if(combat || ((Next(state)&1u)!=0u)){
            Add(r.modules,available,"engine_small",-spread*.46f,aftY,-.06f,.58f,engineSy,.62f,SpaceMaterialKind::EngineHousing,-6.0f,0.0f,-2.0f);
            Add(r.modules,available,"engine_small", spread*.46f,aftY,-.06f,.58f,engineSy,.62f,SpaceMaterialKind::EngineHousing, 6.0f,0.0f, 2.0f);
        } else {
            Add(r.modules,available,"engine_small",0.0f,aftY,-.06f,.72f,engineSy,.68f,SpaceMaterialKind::EngineHousing);
        }

        Add(r.modules,available,"wing_small_left",0.0f,spine.centers.back()-.25f,-.16f,.48f,.54f,.48f,SpaceMaterialKind::ShipHull);
        Add(r.modules,available,"wing_small_right",0.0f,spine.centers.back()-.25f,-.16f,.48f,.54f,.48f,SpaceMaterialKind::ShipHull);
        if(combat){
            Add(r.modules,available,"weapon_mount",-spread*.66f,spine.frontEdge-.42f,.42f,.38f,.42f,.38f,SpaceMaterialKind::EngineHousing,-5.0f);
            Add(r.modules,available,"weapon_mount", spread*.66f,spine.frontEdge-.42f,.42f,.38f,.42f,.38f,SpaceMaterialKind::EngineHousing, 5.0f);
        }
        if(exploration) Add(r.modules,available,"sensor_array",0.0f,spine.centers.front()+.45f,1.00f,.34f,.40f,.36f,SpaceMaterialKind::EngineHousing);
        if(role=="TRADER") Add(r.modules,available,"cargo_bay",0.0f,spine.centers.back()-.15f,-.18f,.38f,.42f,.40f,SpaceMaterialKind::IndustrialHull);
        if(Has(available,"thruster_small")){
            Add(r.modules,available,"thruster_small",-spread*.92f,spine.centers.back()-.35f,.05f,.30f,.36f,.32f,SpaceMaterialKind::ThrusterCore,90.0f);
            Add(r.modules,available,"thruster_small", spread*.92f,spine.centers.back()-.35f,.05f,.30f,.36f,.32f,SpaceMaterialKind::ThrusterCore,-90.0f);
        }
    }
    FinishRecipe(r,metrics,state);
    return r;
}

} // namespace

std::uint32_t ProceduralVisualVariantSystem::StableSeed(const std::string& text) {
    std::uint32_t hash = 2166136261u;
    for (unsigned char c : text) { hash ^= static_cast<std::uint32_t>(c); hash *= 16777619u; }
    return hash ? hash : 1u;
}

std::string ProceduralVisualVariantSystem::NormalizeRole(const std::string& role) {
    std::string upper; upper.reserve(role.size());
    for (unsigned char c : role) upper.push_back(static_cast<char>(std::toupper(c)));
    if (upper.find("HAUL") != std::string::npos) return "HAULER";
    if (upper.find("INDUSTR") != std::string::npos) return "INDUSTRIAL";
    if (upper.find("MIN") != std::string::npos) return "MINING";
    if (upper.find("SALV") != std::string::npos) return "SALVAGE";
    if (upper.find("CARR") != std::string::npos) return "CARRIER";
    if (upper.find("EXPLO") != std::string::npos || upper.find("SCOUT") != std::string::npos) return "EXPLORATION";
    if (upper.find("ESCORT") != std::string::npos) return "ESCORT";
    if (upper.find("PATROL") != std::string::npos) return "PATROL";
    if (upper.find("COMBAT") != std::string::npos || upper.find("FIGHT") != std::string::npos) return "COMBAT";
    if (upper.find("TRAD") != std::string::npos) return "TRADER";
    return "MULTIPURPOSE";
}

ProceduralVisualCatalog ProceduralVisualVariantSystem::Build(const std::vector<std::string>& availableModules,
                                                             std::uint32_t seed,
                                                             int variantsPerRole) {
    std::vector<VisualModuleSource> sources;
    sources.reserve(availableModules.size());
    for(const auto& id:availableModules)sources.push_back(LegacyMetric(id));
    return Build(sources,seed,variantsPerRole);
}

ProceduralVisualCatalog ProceduralVisualVariantSystem::Build(const std::vector<VisualModuleSource>& availableModules,
                                                             std::uint32_t seed,
                                                             int variantsPerRole) {
    ProceduralVisualCatalog catalog;
    catalog.seed=seed?seed:1u;
    catalog.variantsPerRole=std::max(1,variantsPerRole);
    catalog.sourceMetrics=availableModules;
    std::sort(catalog.sourceMetrics.begin(),catalog.sourceMetrics.end(),[](const auto& a,const auto& b){return a.moduleId<b.moduleId;});
    catalog.sourceMetrics.erase(std::unique(catalog.sourceMetrics.begin(),catalog.sourceMetrics.end(),[](const auto& a,const auto& b){return a.moduleId==b.moduleId;}),catalog.sourceMetrics.end());
    MetricMap metrics;
    for(const auto& s:catalog.sourceMetrics){
        catalog.sourceModules.push_back(s.moduleId);
        metrics.emplace(s.moduleId,s);
    }
    std::unordered_set<std::string> available(catalog.sourceModules.begin(),catalog.sourceModules.end());
    static const char* roles[]={"INDUSTRIAL","HAULER","MINING","SALVAGE","CARRIER","COMBAT","ESCORT","PATROL","EXPLORATION","TRADER","MULTIPURPOSE"};
    for(const char* role:roles){
        for(int i=0;i<catalog.variantsPerRole;++i){
            std::uint32_t recipeSeed=catalog.seed^StableSeed(role)^(0x9E3779B9u*static_cast<std::uint32_t>(i+1));
            catalog.shipRecipes.push_back(BuildRecipe(role,available,metrics,recipeSeed,i));
        }
    }
    return catalog;
}

float ProceduralVisualVariantSystem::EvaluateQuality(const ProceduralShipVisualRecipe& recipe) {
    if(recipe.modules.empty()) return 0.0f;
    float score=42.0f;
    bool cockpit=false,engine=false,thruster=false,weapon=false;
    float cockpitY=-999.0f,engineY=999.0f,bodyFront=-999.0f,bodyAft=999.0f,maxSide=0.0f;
    for(const auto& p:recipe.modules){
        maxSide=std::max(maxSide,std::abs(p.x));
        if(p.moduleId.find("cockpit")!=std::string::npos){cockpit=true;cockpitY=std::max(cockpitY,p.y);}
        else if(p.moduleId.find("engine")!=std::string::npos){engine=true;engineY=std::min(engineY,p.y);}
        else if(p.moduleId.find("hull")!=std::string::npos){bodyFront=std::max(bodyFront,p.y);bodyAft=std::min(bodyAft,p.y);}
        if(p.moduleId.find("thruster")!=std::string::npos)thruster=true;
        if(p.moduleId=="weapon_mount")weapon=true;
    }
    if(cockpit) score+=10.0f;
    if(engine) score+=10.0f;
    if(cockpit&&bodyFront>-900.0f&&cockpitY>bodyFront) score+=8.0f;
    if(engine&&bodyAft<900.0f&&engineY<bodyAft) score+=8.0f;
    if(maxSide>1.4f) score+=5.0f;
    if(thruster) score+=4.0f;
    if(weapon||!CombatRole(recipe.role)) score+=2.0f;
    if(recipe.details.size()>=10) score+=5.0f;
    if(!recipe.anchors.empty())score+=3.0f;
    if(!recipe.hardpoints.empty())score+=2.0f;
    if(recipe.negativeSpaceStrength>=.28f) score+=3.0f;
    if(recipe.manufacturerFamily.empty()) score-=8.0f;
    if(recipe.cockpitFamily.empty()) score-=10.0f; else score+=2.0f;
    int externalFunctional=0,coveredFunctional=0;
    for(const auto& p:recipe.modules){
        const bool functional=p.moduleId.find("engine")!=std::string::npos||p.moduleId.find("thruster")!=std::string::npos||p.moduleId=="cargo_bay"||p.moduleId=="weapon_mount";
        const bool outsideLateral=std::fabs(p.x)>=1.0f;
        const bool outsideLongitudinal=(p.moduleId.find("engine")!=std::string::npos&&bodyAft<900.0f&&p.y<bodyAft-.75f);
        if(!functional||(!outsideLateral&&!outsideLongitudinal))continue;
        ++externalFunctional;
        for(const auto& d:recipe.details)if(d.kind==VisualDetailKind::MountBridge||d.kind==VisualDetailKind::StructuralFill){
            const float reach=.5f*std::max(d.sizeX,d.sizeY)+1.6f;
            const float dx=d.x-p.x,dy=d.y-p.y;if(std::sqrt(dx*dx+dy*dy)<=reach){++coveredFunctional;break;}
        }
    }
    if(externalFunctional>0){if(coveredFunctional==externalFunctional)score+=4.0f;else score-=20.0f;}
    if(recipe.modules.size()>30) score-=12.0f;
    return std::clamp(score,0.0f,100.0f);
}

const ProceduralShipVisualRecipe* ProceduralVisualVariantSystem::Select(const ProceduralVisualCatalog& catalog,
                                                                        const std::string& role,
                                                                        std::uint32_t visualSeed) {
    if(catalog.shipRecipes.empty())return nullptr;
    const std::string normalized=NormalizeRole(role);
    std::vector<const ProceduralShipVisualRecipe*> matches;
    for(const auto& recipe:catalog.shipRecipes)if(recipe.role==normalized&&recipe.runtimePcgCertified)matches.push_back(&recipe);
    if(matches.empty())for(const auto& recipe:catalog.shipRecipes)if(recipe.role=="MULTIPURPOSE"&&recipe.runtimePcgCertified)matches.push_back(&recipe);
    // Development compatibility fallback: a catalog with no certified recipe
    // remains inspectable, but certified candidates always win normal runtime
    // selection.  Full Gate/review tooling reports the fallback condition.
    if(matches.empty())for(const auto& recipe:catalog.shipRecipes)if(recipe.role==normalized)matches.push_back(&recipe);
    if(matches.empty())for(const auto& recipe:catalog.shipRecipes)if(recipe.role=="MULTIPURPOSE")matches.push_back(&recipe);
    if(matches.empty())return &catalog.shipRecipes.front();
    const std::uint32_t seed=visualSeed?visualSeed:StableSeed(role);
    return matches[seed%static_cast<std::uint32_t>(matches.size())];
}


const ProceduralShipVisualRecipe* ProceduralVisualVariantSystem::SelectSourceFamily(const ProceduralVisualCatalog& catalog,
                                                                                    const std::string& role,
                                                                                    const std::string& sourceFamily,
                                                                                    std::uint32_t visualSeed) {
    if(catalog.shipRecipes.empty()) return nullptr;
    const std::string normalized=NormalizeRole(role);
    std::vector<const ProceduralShipVisualRecipe*> matches;
    for(const auto& recipe:catalog.shipRecipes) {
        if(recipe.role==normalized && recipe.sourceFamily==sourceFamily && recipe.runtimePcgCertified) matches.push_back(&recipe);
    }
    if(matches.empty()) for(const auto& recipe:catalog.shipRecipes) {
        if(recipe.role==normalized && recipe.sourceFamily==sourceFamily) matches.push_back(&recipe);
    }
    if(matches.empty()) return nullptr;
    const std::uint32_t seed=visualSeed?visualSeed:StableSeed(role+sourceFamily);
    return matches[seed%static_cast<std::uint32_t>(matches.size())];
}

} // namespace subspace
