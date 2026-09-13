#include "interior/ShipInteriorCarvingSystem.h"

#include <algorithm>
#include <cmath>
#include <queue>
#include <unordered_map>
#include <unordered_set>

namespace subspace {
namespace {
const ShipyardModuleRecord* Find(const std::vector<ShipyardModuleRecord>& catalog,const std::string& id){for(const auto& r:catalog)if(r.source.moduleId==id)return &r;return nullptr;}

InteriorRoomType RoomTypeFor(const ShipModuleInteriorBinding& b,const ShipyardModuleRecord& r){
    switch(b.capability){
        case ExteriorInteriorCapability::Cockpit: case ExteriorInteriorCapability::Bridge:return InteriorRoomType::Cockpit;
        case ExteriorInteriorCapability::Engineering:return InteriorRoomType::Engineering;
        case ExteriorInteriorCapability::Cargo:return InteriorRoomType::Cargo;
        case ExteriorInteriorCapability::Habitation:return InteriorRoomType::CrewQuarters;
        case ExteriorInteriorCapability::Airlock: case ExteriorInteriorCapability::Hangar:return InteriorRoomType::Airlock;
        case ExteriorInteriorCapability::Corridor:return InteriorRoomType::Corridor;
        default:break;
    }
    if(r.partRole==ShipyardPartRole::Tank)return InteriorRoomType::Reactor;
    if(r.partRole==ShipyardPartRole::StructuralFrame||r.partRole==ShipyardPartRole::StructuralBlock||r.partRole==ShipyardPartRole::StructuralAttachment)return InteriorRoomType::Corridor;
    return InteriorRoomType::CrewQuarters;
}

bool ExclusionRole(const ShipyardModuleRecord& r){
    switch(r.partRole){
        case ShipyardPartRole::MainEngine: case ShipyardPartRole::EngineNozzle:
        case ShipyardPartRole::RcsThruster: case ShipyardPartRole::WeaponTurret:
        case ShipyardPartRole::MissileMount: case ShipyardPartRole::HardpointBase:
        case ShipyardPartRole::SensorDish: case ShipyardPartRole::SensorMast:
        case ShipyardPartRole::SensorAntenna: case ShipyardPartRole::Telescope:
        case ShipyardPartRole::SurfaceDetail:
        case ShipyardPartRole::Wing: case ShipyardPartRole::Fin:
            return true;
        default:return false;
    }
}

Vector3 InsetHalfExtents(const ShipyardModuleRecord& r,const VisualModulePlacement& p,const WorldScaleProfile& scale){
    // Preserve an exterior shell thickness while never fabricating a walkable
    // dimension larger than the authored module. Small modules remain useful
    // service corridors if they clear a human shoulder/door envelope.
    const float shell=std::max(0.12f,scale.referencePlayerHeightMeters*0.08f);
    return {std::max(0.20f,r.source.halfWidth*std::fabs(p.scaleX)-shell),
            std::max(0.20f,r.source.halfLength*std::fabs(p.scaleY)-shell),
            std::max(0.20f,r.source.halfHeight*std::fabs(p.scaleZ)-shell)};
}

bool HumanClearance(const Vector3& e,const WorldScaleProfile& scale){
    const float fullW=2.0f*std::max(e.x,e.y);
    const float fullH=2.0f*e.z;
    return fullW>=std::min(scale.referenceDoorWidthMeters,scale.referenceCorridorWidthMeters)*0.80f &&
           fullH>=scale.referencePlayerHeightMeters*0.90f;
}
}

ShipInteriorCarvePlan ShipInteriorCarvingSystem::Carve(const std::vector<ShipyardModuleRecord>& catalog,
                                                        const ProceduralShipVisualRecipe& recipe,
                                                        const WorldScaleProfile& scale)
{
    ShipInteriorCarvePlan out;
    if(recipe.modules.empty()){out.errors.push_back("Cannot carve interior from an empty ship assembly");return out;}

    const auto linkPlan=ShipModuleInteriorLinkSystem::BuildPlan(catalog,recipe,scale);
    std::unordered_map<std::size_t,std::size_t> volumeByModule;
    float minWalkZ=1.0e30f,maxWalkZ=-1.0e30f;

    for(std::size_t i=0;i<recipe.modules.size();++i){
        const auto& p=recipe.modules[i];const auto* r=Find(catalog,p.moduleId);
        if(!r){out.warnings.push_back("Interior carve skipped unknown module: "+p.moduleId);continue;}
        const auto binding=ShipModuleInteriorLinkSystem::InferBinding(*r,scale);
        const auto e=InsetHalfExtents(*r,p,scale);
        const Vector3 center{p.x*recipe.widthScale,p.y*recipe.lengthScale,p.z};

        if(ExclusionRole(*r)||(!binding.walkable&&binding.capability==ExteriorInteriorCapability::None)){
            InteriorExclusionVolume x;x.moduleIndex=i;x.moduleId=p.moduleId;x.center=center;x.halfExtents=e;
            x.reason=ExclusionRole(*r)?"non-habitable machinery / weapon / aerodynamic surface":"no certified walkable interior capability";
            out.exclusions.push_back(x);continue;
        }

        if(!binding.walkable){
            InteriorExclusionVolume x;x.moduleIndex=i;x.moduleId=p.moduleId;x.center=center;x.halfExtents=e;x.reason="service-access only";out.exclusions.push_back(x);continue;
        }
        if(!HumanClearance(e,scale)){
            InteriorExclusionVolume x;x.moduleIndex=i;x.moduleId=p.moduleId;x.center=center;x.halfExtents=e;x.reason="insufficient human-scale clearance after hull inset";out.exclusions.push_back(x);out.warnings.push_back("Walkable module too small after pressure-hull inset: "+p.moduleId);continue;
        }

        InteriorCarvedVolume v;v.moduleIndex=i;v.moduleId=p.moduleId;v.capability=binding.capability;v.roomType=RoomTypeFor(binding,*r);v.center=center;v.halfExtents=e;v.yawDegrees=p.yawDegrees;v.pitchDegrees=p.pitchDegrees;v.rollDegrees=p.rollDegrees;v.walkable=true;v.pressureCapable=true;
        volumeByModule[i]=out.volumes.size();out.volumes.push_back(v);minWalkZ=std::min(minWalkZ,center.z-e.z);maxWalkZ=std::max(maxWalkZ,center.z+e.z);
    }

    out.walkableModuleCount=out.volumes.size();
    if(out.volumes.empty()){out.errors.push_back("Finished assembly exposes no human-scale walkable pressure-hull volume");return out;}

    // Infer deck bands from the actual carved vertical span. Deck assignment is
    // recomputed below from each transformed module center.
    const float verticalSpan=std::max(scale.referenceDeckHeightMeters,maxWalkZ-minWalkZ);
    out.deckCount=std::clamp(static_cast<int>(std::ceil(verticalSpan/std::max(1.0f,scale.referenceDeckHeightMeters))),1,12);
    for(auto& v:out.volumes){
        const float rel=v.center.z-minWalkZ;
        v.deck=std::clamp(static_cast<int>(std::floor(rel/std::max(1.0f,scale.referenceDeckHeightMeters))),0,out.deckCount-1);
    }

    std::unordered_map<std::size_t,std::vector<std::size_t>> adjacency;
    for(const auto& e:linkPlan.edges){
        auto ia=volumeByModule.find(e.moduleA),ib=volumeByModule.find(e.moduleB);
        if(ia==volumeByModule.end()||ib==volumeByModule.end())continue;
        InteriorCarvedPortal p;p.moduleA=e.moduleA;p.moduleB=e.moduleB;p.kind=e.kind;p.walkable=e.walkable;
        const auto& a=out.volumes[ia->second];const auto& b=out.volumes[ib->second];p.center=(a.center+b.center)*0.5f;
        if(!p.walkable){
            // Both sides are physically carved and attached. A missing logical
            // portal is a real continuity defect, not permission to invent a
            // detached generic room list.
            out.warnings.push_back("Attached carved hull volumes require portal repair: "+a.moduleId+" <-> "+b.moduleId);
        }else{
            adjacency[e.moduleA].push_back(e.moduleB);adjacency[e.moduleB].push_back(e.moduleA);
        }
        out.portals.push_back(p);
    }

    // Legacy/generated recipes may omit explicit attachment metadata. If two
    // carved volumes materially overlap/touch in the actual assembly, stitch a
    // generated corridor portal instead of fabricating unrelated rooms.
    for(std::size_t ai=0;ai<out.volumes.size();++ai)for(std::size_t bi=ai+1;bi<out.volumes.size();++bi){
        const auto& a=out.volumes[ai];const auto& b=out.volumes[bi];
        bool already=false;for(const auto& p:out.portals)if((p.moduleA==a.moduleIndex&&p.moduleB==b.moduleIndex)||(p.moduleA==b.moduleIndex&&p.moduleB==a.moduleIndex)){already=true;break;}if(already)continue;
        const Vector3 d{std::fabs(a.center.x-b.center.x),std::fabs(a.center.y-b.center.y),std::fabs(a.center.z-b.center.z)};
        const bool touches=d.x<=a.halfExtents.x+b.halfExtents.x+scale.referenceDoorWidthMeters &&
                           d.y<=a.halfExtents.y+b.halfExtents.y+scale.referenceDoorWidthMeters &&
                           d.z<=a.halfExtents.z+b.halfExtents.z+scale.referenceDeckHeightMeters*0.35f;
        if(!touches)continue;
        InteriorCarvedPortal p;p.moduleA=a.moduleIndex;p.moduleB=b.moduleIndex;p.center=(a.center+b.center)*0.5f;p.kind=InteriorPortalKind::OpenPassage;p.walkable=true;out.portals.push_back(p);adjacency[a.moduleIndex].push_back(b.moduleIndex);adjacency[b.moduleIndex].push_back(a.moduleIndex);
    }

    // Connectivity starts at command if present, otherwise first carved cell.
    std::size_t root=out.volumes.front().moduleIndex;
    for(const auto& v:out.volumes)if(v.capability==ExteriorInteriorCapability::Cockpit||v.capability==ExteriorInteriorCapability::Bridge){root=v.moduleIndex;break;}
    std::unordered_set<std::size_t> visited;std::queue<std::size_t> q;visited.insert(root);q.push(root);
    while(!q.empty()){auto n=q.front();q.pop();for(auto m:adjacency[n])if(visited.insert(m).second)q.push(m);}
    out.connectedWalkableCount=visited.size();
    if(out.connectedWalkableCount!=out.walkableModuleCount){
        out.errors.push_back("Carved pressure hull is not cohesive: "+std::to_string(out.connectedWalkableCount)+"/"+std::to_string(out.walkableModuleCount)+" walkable module volumes connect to the command/root interior");
        for(const auto& v:out.volumes)if(!visited.count(v.moduleIndex))out.warnings.push_back("Disconnected interior cavity: module "+v.moduleId+" ["+std::to_string(v.moduleIndex)+"]");
    }
    for(const auto& w:linkPlan.warnings)out.warnings.push_back(w);
    for(const auto& e:linkPlan.errors)out.errors.push_back(e);
    out.valid=out.errors.empty();
    return out;
}

} // namespace subspace
