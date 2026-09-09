#include "interior/ShipModuleInteriorLinkSystem.h"

#include <algorithm>
#include <cctype>
#include <unordered_map>

namespace subspace {
namespace {
std::string Lower(std::string s){std::transform(s.begin(),s.end(),s.begin(),[](unsigned char c){return static_cast<char>(std::tolower(c));});return s;}
bool Has(const std::string&s,const char*t){return s.find(t)!=std::string::npos;}
const ShipyardModuleRecord* Find(const std::vector<ShipyardModuleRecord>&c,const std::string&id){for(const auto&r:c)if(r.source.moduleId==id)return &r;return nullptr;}
InteriorPortalKind MergeKind(InteriorPortalKind a,InteriorPortalKind b){if(a==InteriorPortalKind::Airlock||b==InteriorPortalKind::Airlock)return InteriorPortalKind::Airlock;if(a==InteriorPortalKind::Hatch||b==InteriorPortalKind::Hatch)return InteriorPortalKind::Hatch;if(a==InteriorPortalKind::ServiceHatch||b==InteriorPortalKind::ServiceHatch)return InteriorPortalKind::ServiceHatch;if(a==InteriorPortalKind::SealedBoundary||b==InteriorPortalKind::SealedBoundary)return InteriorPortalKind::SealedBoundary;if(a==InteriorPortalKind::OpenPassage&&b==InteriorPortalKind::OpenPassage)return InteriorPortalKind::OpenPassage;return InteriorPortalKind::Door;}
}

const char* ShipModuleInteriorLinkSystem::CapabilityName(ExteriorInteriorCapability c){switch(c){case ExteriorInteriorCapability::None:return "NONE";case ExteriorInteriorCapability::WalkableRoom:return "ROOM";case ExteriorInteriorCapability::Corridor:return "CORRIDOR";case ExteriorInteriorCapability::Cockpit:return "COCKPIT";case ExteriorInteriorCapability::Bridge:return "BRIDGE";case ExteriorInteriorCapability::Engineering:return "ENGINEERING";case ExteriorInteriorCapability::Cargo:return "CARGO";case ExteriorInteriorCapability::Habitation:return "HABITATION";case ExteriorInteriorCapability::Airlock:return "AIRLOCK";case ExteriorInteriorCapability::Hangar:return "HANGAR";case ExteriorInteriorCapability::ServiceAccess:return "SERVICE ACCESS";}return "NONE";}

ShipModuleInteriorBinding ShipModuleInteriorLinkSystem::InferBinding(const ShipyardModuleRecord&m,const WorldScaleProfile&scale){
    ShipModuleInteriorBinding b;b.moduleId=m.source.moduleId;b.preferredInteriorKitId="quaternius.ultimate_modular_scifi.2021";const auto n=Lower(m.source.moduleId);
    switch(m.semantic){
        case ShipyardModuleSemantic::CommandCockpit:b.capability=ExteriorInteriorCapability::Cockpit;b.walkable=true;break;
        case ShipyardModuleSemantic::CommandBridge:b.capability=ExteriorInteriorCapability::Bridge;b.walkable=true;break;
        case ShipyardModuleSemantic::HullBow:case ShipyardModuleSemantic::HullMid:case ShipyardModuleSemantic::HullAft:b.capability=ExteriorInteriorCapability::WalkableRoom;b.walkable=true;break;
        case ShipyardModuleSemantic::EngineHousing:b.capability=ExteriorInteriorCapability::Engineering;b.walkable=true;break;
        case ShipyardModuleSemantic::MainEngine:case ShipyardModuleSemantic::EngineNozzle:case ShipyardModuleSemantic::RcsThruster:b.capability=ExteriorInteriorCapability::ServiceAccess;b.interactionOnly=true;break;
        default:break;
    }
    if(Has(n,"hangar")){b.capability=ExteriorInteriorCapability::Hangar;b.walkable=true;}
    else if(Has(n,"airlock")||Has(n,"dockcollar")){b.capability=ExteriorInteriorCapability::Airlock;b.walkable=true;}
    else if(Has(n,"cargo")||Has(n,"storage")){b.capability=ExteriorInteriorCapability::Cargo;b.walkable=true;}
    else if(Has(n,"hab")||Has(n,"crew")){b.capability=ExteriorInteriorCapability::Habitation;b.walkable=true;}
    else if(Has(n,"corridor")||Has(n,"connector")||Has(n,"adapter")){b.capability=ExteriorInteriorCapability::Corridor;b.walkable=true;}
    b.deckCount=(m.size==ShipyardModuleSize::L||m.size==ShipyardModuleSize::XL)?2:1;
    b.footprintWidthCells=(m.size==ShipyardModuleSize::XS?1:m.size==ShipyardModuleSize::S?2:m.size==ShipyardModuleSize::M?3:m.size==ShipyardModuleSize::L?5:7);
    b.footprintLengthCells=b.footprintWidthCells;
    if(b.capability!=ExteriorInteriorCapability::None){
        for(const auto&s:m.sockets){ShipModuleInteriorPortal p;p.id=m.source.moduleId+".interior."+s.name;p.exteriorSocketName=s.name;p.localPosition={s.x,s.y,s.z};p.localDirection={s.dirX,s.dirY,s.dirZ};p.widthMeters=scale.referenceDoorWidthMeters;p.heightMeters=scale.referenceDoorHeightMeters;p.kind=b.capability==ExteriorInteriorCapability::Airlock?InteriorPortalKind::Airlock:(b.interactionOnly?InteriorPortalKind::ServiceHatch:InteriorPortalKind::Door);b.portals.push_back(p);}
        if(b.portals.empty()&&b.walkable){ShipModuleInteriorPortal p;p.id=m.source.moduleId+".interior.auto";p.exteriorSocketName="auto";p.widthMeters=scale.referenceDoorWidthMeters;p.heightMeters=scale.referenceDoorHeightMeters;p.kind=b.capability==ExteriorInteriorCapability::Airlock?InteriorPortalKind::Airlock:InteriorPortalKind::Door;b.portals.push_back(p);}
    }
    return b;
}

bool ShipModuleInteriorLinkSystem::CanCreateWalkableConnection(const ShipModuleInteriorBinding&a,const ShipModuleInteriorBinding&b){return a.walkable&&b.walkable&&!a.portals.empty()&&!b.portals.empty();}

ShipInteriorConnectionPlan ShipModuleInteriorLinkSystem::BuildPlan(const std::vector<ShipyardModuleRecord>&catalog,const ProceduralShipVisualRecipe&recipe,const WorldScaleProfile&scale){
    ShipInteriorConnectionPlan out;out.bindings.reserve(recipe.modules.size());for(const auto&p:recipe.modules){const auto*r=Find(catalog,p.moduleId);if(r)out.bindings.push_back(InferBinding(*r,scale));else{ShipModuleInteriorBinding b;b.moduleId=p.moduleId;out.bindings.push_back(b);out.warnings.push_back("Interior binding skipped unknown exterior module: "+p.moduleId);}}
    for(const auto&attachment:recipe.attachments){
        if(attachment.parentModuleIndex>=recipe.modules.size()||attachment.childModuleIndex>=recipe.modules.size())continue;
        const std::size_t j=attachment.parentModuleIndex,i=attachment.childModuleIndex;
        const auto&a=out.bindings[j];const auto&b=out.bindings[i];
        if(a.capability==ExteriorInteriorCapability::None&&b.capability==ExteriorInteriorCapability::None)continue;
        InteriorConnectionEdge e;e.moduleA=j;e.moduleB=i;
        const auto findPortal=[&](const ShipModuleInteriorBinding&binding,const std::string&socket)->const ShipModuleInteriorPortal*{for(const auto&q:binding.portals)if(q.exteriorSocketName==socket)return &q;return binding.portals.empty()?nullptr:&binding.portals.front();};
        const auto*pa=findPortal(a,attachment.parentSocket);const auto*pb=findPortal(b,attachment.childSocket);
        if(pa)e.portalA=pa->id;if(pb)e.portalB=pb->id;if(pa&&pb)e.kind=MergeKind(pa->kind,pb->kind);else e.kind=InteriorPortalKind::SealedBoundary;
        e.walkable=pa&&pb&&CanCreateWalkableConnection(a,b)&&e.kind!=InteriorPortalKind::ServiceHatch&&e.kind!=InteriorPortalKind::SealedBoundary;
        if(e.walkable)e.status="CONNECTED";else if((a.interactionOnly||b.interactionOnly)&&(pa||pb)){e.kind=InteriorPortalKind::ServiceHatch;e.status="SERVICE INTERACTION";}
        else{e.status="SEALED / NO WALKABLE INTERIOR";if(a.walkable||b.walkable)out.warnings.push_back("Exterior attachment does not expose a compatible walkable interior portal between "+a.moduleId+" and "+b.moduleId);}
        out.edges.push_back(e);
    }
    bool hasWalkable=false;for(const auto&b:out.bindings)hasWalkable|=b.walkable;if(hasWalkable){bool anyEdge=false;for(const auto&e:out.edges)anyEdge|=e.walkable;if(recipe.modules.size()>1&&!anyEdge)out.warnings.push_back("Ship has walkable interior-capable modules but no connected interior edge yet");}
    out.valid=out.errors.empty();return out;
}

} // namespace subspace
