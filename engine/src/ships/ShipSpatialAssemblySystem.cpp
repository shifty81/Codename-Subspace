#include "ships/ShipSpatialAssemblySystem.h"

#include <algorithm>
#include <cmath>
#include <map>
#include <set>

namespace subspace {
namespace {
const ShipyardModuleRecord* Find(const std::vector<ShipyardModuleRecord>&c,const std::string&id){for(const auto&r:c)if(r.source.moduleId==id)return &r;return nullptr;}
float OverlapAxis(float ac,float ah,float bc,float bh){return std::max(0.0f,std::min(ac+ah,bc+bh)-std::max(ac-ah,bc-bh));}
float Volume(const ShipOccupancyBox&b){return std::max(.0001f,8.0f*b.halfExtents.x*b.halfExtents.y*b.halfExtents.z);}
bool Attached(const ProceduralShipVisualRecipe&r,std::size_t a,std::size_t b){for(const auto&e:r.attachments)if((e.parentModuleIndex==a&&e.childModuleIndex==b)||(e.parentModuleIndex==b&&e.childModuleIndex==a))return true;return false;}
}
std::vector<ShipOccupancyBox> ShipSpatialAssemblySystem::BuildOccupancy(const std::vector<ShipyardModuleRecord>&catalog,const ProceduralShipVisualRecipe&recipe){
    std::vector<ShipOccupancyBox> out;out.reserve(recipe.modules.size());
    for(std::size_t i=0;i<recipe.modules.size();++i){const auto&p=recipe.modules[i];const auto*r=Find(catalog,p.moduleId);if(!r)continue;ShipOccupancyBox b;b.moduleIndex=i;b.center={p.x,p.y,p.z};b.halfExtents={std::max(.03f,r->source.halfWidth*std::fabs(p.scaleX)),std::max(.03f,r->source.halfLength*std::fabs(p.scaleY)),std::max(.03f,r->source.halfHeight*std::fabs(p.scaleZ))};b.detail=r->surfaceOnly||r->semantic==ShipyardModuleSemantic::SurfaceDetail;b.command=r->moduleClass==ShipyardModuleClass::Command;b.propulsion=r->moduleClass==ShipyardModuleClass::Propulsion;out.push_back(b);}return out;
}
ShipSpatialRegion ShipSpatialAssemblySystem::ClassifyRegion(const VisualModulePlacement&p,const ProceduralShipVisualRecipe&r){
    if(r.modules.empty())return ShipSpatialRegion::MidHull;float minY=r.modules.front().y,maxY=minY,maxX=std::fabs(r.modules.front().x),maxZ=std::fabs(r.modules.front().z);for(const auto&m:r.modules){minY=std::min(minY,m.y);maxY=std::max(maxY,m.y);maxX=std::max(maxX,std::fabs(m.x));maxZ=std::max(maxZ,std::fabs(m.z));}const float span=std::max(.001f,maxY-minY);const float y01=(p.y-minY)/span;if(std::fabs(p.x)>.55f*std::max(.1f,maxX))return p.x<0?ShipSpatialRegion::Port:ShipSpatialRegion::Starboard;if(std::fabs(p.z)>.55f*std::max(.1f,maxZ))return p.z>0?ShipSpatialRegion::Dorsal:ShipSpatialRegion::Ventral;if(y01>.82f)return ShipSpatialRegion::Bow;if(y01>.62f)return ShipSpatialRegion::ForwardHull;if(y01<.18f)return ShipSpatialRegion::AftHull;return ShipSpatialRegion::MidHull;
}
ShipSpatialAssemblyReport ShipSpatialAssemblySystem::Validate(const std::vector<ShipyardModuleRecord>&catalog,const ProceduralShipVisualRecipe&recipe,float allowedParentOverlapRatio){
    ShipSpatialAssemblyReport report;const auto boxes=BuildOccupancy(catalog,recipe);
    for(std::size_t i=0;i<boxes.size();++i)for(std::size_t j=i+1;j<boxes.size();++j){const auto&a=boxes[i],&b=boxes[j];const float ox=OverlapAxis(a.center.x,a.halfExtents.x,b.center.x,b.halfExtents.x),oy=OverlapAxis(a.center.y,a.halfExtents.y,b.center.y,b.halfExtents.y),oz=OverlapAxis(a.center.z,a.halfExtents.z,b.center.z,b.halfExtents.z);if(ox<=0||oy<=0||oz<=0)continue;const float overlap=ox*oy*oz;const float ratio=overlap/std::min(Volume(a),Volume(b));const float allowed=Attached(recipe,a.moduleIndex,b.moduleIndex)?allowedParentOverlapRatio:.12f;if(ratio>allowed&&!a.detail&&!b.detail){++report.unintendedOverlapPairs;report.overlapPairs.push_back({a.moduleIndex,b.moduleIndex});}}
    // Detail density budget: no more than four detail-only placements may pile
    // into the same coarse ship-space cell. This catches recursive greeble balls.
    std::map<std::tuple<int,int,int>,int> cells;for(const auto&b:boxes)if(b.detail){const auto key=std::make_tuple(static_cast<int>(std::floor(b.center.x*1.5f)),static_cast<int>(std::floor(b.center.y*1.5f)),static_cast<int>(std::floor(b.center.z*1.5f)));++cells[key];}for(const auto&kv:cells)if(kv.second>4)++report.detailClusterCells;
    // Detail-on-detail attachment is forbidden unless a later authored grammar
    // explicitly certifies a composite detail family.  This prevents recursive
    // greeble trees from growing into dense mechanical balls.
    for(const auto& edge:recipe.attachments){
        if(edge.parentModuleIndex>=recipe.modules.size()||edge.childModuleIndex>=recipe.modules.size())continue;
        const auto* pr=Find(catalog,recipe.modules[edge.parentModuleIndex].moduleId);
        const auto* cr=Find(catalog,recipe.modules[edge.childModuleIndex].moduleId);
        if(!pr||!cr)continue;
        const bool pd=pr->surfaceOnly||pr->semantic==ShipyardModuleSemantic::SurfaceDetail;
        const bool cd=cr->surfaceOnly||cr->semantic==ShipyardModuleSemantic::SurfaceDetail;
        if(pd&&cd)++report.recursiveDetailAttachments;
    }
    // Command exposure: command boxes may overlap their immediate structural
    // parent, but being mostly enveloped by unrelated structure is invalid.
    for(const auto&cmd:boxes)if(cmd.command){int enclosing=0;for(const auto&b:boxes)if(b.moduleIndex!=cmd.moduleIndex&&!b.detail){const float ox=OverlapAxis(cmd.center.x,cmd.halfExtents.x,b.center.x,b.halfExtents.x),oy=OverlapAxis(cmd.center.y,cmd.halfExtents.y,b.center.y,b.halfExtents.y),oz=OverlapAxis(cmd.center.z,cmd.halfExtents.z,b.center.z,b.halfExtents.z);if(ox*oy*oz/Volume(cmd)>.55f&&!Attached(recipe,cmd.moduleIndex,b.moduleIndex))++enclosing;}if(enclosing>0){report.commandBuried=true;break;}}
    if(report.unintendedOverlapPairs>0)report.warnings.push_back("Unintended module overlap exceeds mating allowance");
    if(report.detailClusterCells>0)report.warnings.push_back("Surface-detail density budget exceeded");
    if(report.recursiveDetailAttachments>0)report.warnings.push_back("Detail-on-detail recursive attachment is not PCG certified");
    if(report.commandBuried)report.warnings.push_back("Command/cockpit exposure volume is buried by unrelated structure");
    report.valid=report.unintendedOverlapPairs==0&&report.detailClusterCells==0&&report.recursiveDetailAttachments==0&&!report.commandBuried;return report;
}
} // namespace subspace
