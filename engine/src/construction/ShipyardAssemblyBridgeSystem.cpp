#include "construction/ShipyardAssemblyBridgeSystem.h"

#include <algorithm>
#include <cmath>
#include <sstream>
#include <unordered_map>

namespace subspace {
namespace {
constexpr double Pi = 3.14159265358979323846;

double Rad(double degrees) { return degrees * Pi / 180.0; }

DoubleQuat Mul(DoubleQuat a, DoubleQuat b) {
    return {a.w*b.x + a.x*b.w + a.y*b.z - a.z*b.y,
            a.w*b.y - a.x*b.z + a.y*b.w + a.z*b.x,
            a.w*b.z + a.x*b.y - a.y*b.x + a.z*b.w,
            a.w*b.w - a.x*b.x - a.y*b.y - a.z*b.z};
}

DoubleQuat AxisAngle(double x,double y,double z,double degrees) {
    const double h=Rad(degrees)*0.5, s=std::sin(h);
    return {x*s,y*s,z*s,std::cos(h)};
}

DoubleQuat Orientation(const VisualModulePlacement& p,float forwardVisualYawDegrees) {
    // Gameplay ship-forward is +Y.  The recipe-wide visual correction is folded
    // into the imported assembly once so renderer/editor consumers no longer
    // need to remember a parallel forward-yaw authority.
    const auto yaw=AxisAngle(0.0,0.0,1.0,p.yawDegrees+forwardVisualYawDegrees);
    const auto pitch=AxisAngle(1.0,0.0,0.0,p.pitchDegrees);
    const auto roll=AxisAngle(0.0,1.0,0.0,p.rollDegrees);
    return Mul(Mul(yaw,pitch),roll);
}

const ShipyardModuleRecord* FindRecord(const std::vector<ShipyardModuleRecord>& catalog,const std::string& moduleId){
    for(const auto& r:catalog) if(r.source.moduleId==moduleId) return &r;
    return nullptr;
}

bool HasTag(const std::vector<std::string>& tags,const std::string& tag){
    return std::find(tags.begin(),tags.end(),tag)!=tags.end();
}

void AddTag(std::vector<std::string>& tags,const char* tag){
    if(!HasTag(tags,tag)) tags.emplace_back(tag);
}


const SemanticSocket* FindSocket(const BuildElement& e,const std::string& id){
    for(const auto& s:e.sockets) if(s.id==id) return &s;
    return nullptr;
}

bool Opposite(AttachmentFace a,AttachmentFace b){
    return (a==AttachmentFace::Fore&&b==AttachmentFace::Aft)||(a==AttachmentFace::Aft&&b==AttachmentFace::Fore)||
           (a==AttachmentFace::Port&&b==AttachmentFace::Starboard)||(a==AttachmentFace::Starboard&&b==AttachmentFace::Port)||
           (a==AttachmentFace::Dorsal&&b==AttachmentFace::Ventral)||(a==AttachmentFace::Ventral&&b==AttachmentFace::Dorsal);
}

std::pair<std::string,std::string> ChooseCompatibleSockets(const BuildElement& a,const BuildElement& b){
    for(const auto& sa:a.sockets) for(const auto& sb:b.sockets)
        if(AssemblyConstructionSystem::SocketTypesCompatible(sa,sb)&&Opposite(sa.face,sb.face)) return {sa.id,sb.id};
    for(const auto& sa:a.sockets) for(const auto& sb:b.sockets)
        if(AssemblyConstructionSystem::SocketTypesCompatible(sa,sb)) return {sa.id,sb.id};
    return {};
}

SemanticSocket GeneratedSocket(const std::string& id,AttachmentFace face,Double3 p){
    SemanticSocket s;s.id=id;s.type="universal";s.face=face;s.localPosition=p;return s;
}

} // namespace

AttachmentFace ShipyardAssemblyBridgeSystem::FaceFromDirection(double x,double y,double z){
    const double ax=std::fabs(x),ay=std::fabs(y),az=std::fabs(z);
    if(ax<1e-8&&ay<1e-8&&az<1e-8) return AttachmentFace::None;
    if(ay>=ax&&ay>=az) return y>=0?AttachmentFace::Fore:AttachmentFace::Aft;
    if(ax>=az) return x>=0?AttachmentFace::Starboard:AttachmentFace::Port;
    return z>=0?AttachmentFace::Dorsal:AttachmentFace::Ventral;
}

std::vector<std::string> ShipyardAssemblyBridgeSystem::CapabilityTags(const ShipyardModuleRecord& r){
    std::vector<std::string> out;
    switch(r.partRole){
        case ShipyardPartRole::Cockpit: case ShipyardPartRole::Bridge: AddTag(out,"command"); break;
        case ShipyardPartRole::MainEngine: case ShipyardPartRole::EngineHousing:
        case ShipyardPartRole::EngineMount: case ShipyardPartRole::EngineNozzle: AddTag(out,"propulsion"); break;
        case ShipyardPartRole::RcsThruster: AddTag(out,"maneuvering"); break;
        case ShipyardPartRole::Cargo: AddTag(out,"cargo"); break;
        case ShipyardPartRole::Tank: AddTag(out,"fuel"); break;
        case ShipyardPartRole::HardpointBase: case ShipyardPartRole::WeaponTurret:
        case ShipyardPartRole::MissileMount: AddTag(out,"weapons"); break;
        case ShipyardPartRole::SensorDish: case ShipyardPartRole::SensorMast:
        case ShipyardPartRole::SensorAntenna: case ShipyardPartRole::Telescope: AddTag(out,"sensors"); break;
        case ShipyardPartRole::Hangar: AddTag(out,"hangar"); break;
        default: break;
    }
    switch(r.partRole){
        case ShipyardPartRole::PrimaryHull: case ShipyardPartRole::HullAdapter:
        case ShipyardPartRole::StructuralFrame: case ShipyardPartRole::StructuralBrace:
        case ShipyardPartRole::StructuralBlock: case ShipyardPartRole::StructuralAttachment:
            AddTag(out,"structure"); break;
        case ShipyardPartRole::Wing: case ShipyardPartRole::Fin: case ShipyardPartRole::Outrigger:
            AddTag(out,"aero_surface"); break;
        default: break;
    }
    if(r.functional) AddTag(out,"functional");
    if(!r.surfaceOnly) AddTag(out,"interior_candidate");
    return out;
}

ShipyardAssemblyImportReport ShipyardAssemblyBridgeSystem::Import(const std::vector<ShipyardModuleRecord>& catalog,
                                                                  const ProceduralShipVisualRecipe& recipe,
                                                                  PersistentEntityId shipId,
                                                                  ShipyardAssemblyImportOptions options){
    ShipyardAssemblyImportReport report;
    if(!shipId.IsValid()){report.errors.push_back("persistent ship identity is required");return report;}
    if(recipe.modules.empty()){report.errors.push_back("Shipyard recipe contains no modules");return report;}

    auto& a=report.assembly;
    a.assemblyId=shipId;
    a.id=recipe.recipeId.empty()?"shipyard-import-"+StableIdentitySystem::ToString(shipId):recipe.recipeId;
    a.quantumMeters=AssemblyConstructionSystem::DefaultQuantumMeters;
    a.rootSubAssemblyId="root";
    a.subAssemblies.push_back({"root",{}});
    a.elements.reserve(recipe.modules.size());

    for(std::size_t i=0;i<recipe.modules.size();++i){
        const auto& p=recipe.modules[i];
        const auto* r=FindRecord(catalog,p.moduleId);
        if(!r){report.errors.push_back("recipe module is not present in certified catalog: "+p.moduleId);continue;}
        BuildElement e;
        e.id="module-"+std::to_string(i)+"-"+p.moduleId;
        e.kind=BuildElementKind::AuthoredModule;
        e.definitionId=p.moduleId;
        e.subAssemblyId="root";
        e.transform.position={p.x,p.y,p.z};
        e.transform.rotation=Orientation(p,recipe.forwardVisualYawDegrees);
        e.transform.scale={p.scaleX,p.scaleY,p.scaleZ};
        if(options.preserveMirrorsAsSignedScale){if(p.mirrorX)e.transform.scale.x*=-1.0;if(p.mirrorY)e.transform.scale.y*=-1.0;if(p.mirrorZ)e.transform.scale.z*=-1.0;}
        e.transform=AssemblyConstructionSystem::Quantize(e.transform,a.quantumMeters);
        const double sx=std::fabs(e.transform.scale.x),sy=std::fabs(e.transform.scale.y),sz=std::fabs(e.transform.scale.z);
        const double volume=std::max(0.001,8.0*double(r->source.halfWidth)*double(r->source.halfLength)*double(r->source.halfHeight)*sx*sy*sz);
        e.massKg=std::max(1.0,volume*std::max(1.0,options.densityKgPerCubicMeter));
        e.capabilityTags=CapabilityTags(*r);
        for(const auto& rs:r->sockets){
            SemanticSocket s;s.id=rs.name.empty()?"socket-"+std::to_string(e.sockets.size()):rs.name;
            s.type=rs.type.empty()?"universal":rs.type;s.face=FaceFromDirection(rs.dirX,rs.dirY,rs.dirZ);s.localPosition={rs.x,rs.y,rs.z};e.sockets.push_back(std::move(s));
        }
        if(e.sockets.empty()&&options.generateFallbackSockets){
            const double hx=r->source.halfWidth*sx,hy=r->source.halfLength*sy,hz=r->source.halfHeight*sz;
            e.sockets={GeneratedSocket("auto_fore",AttachmentFace::Fore,{0,hy,0}),GeneratedSocket("auto_aft",AttachmentFace::Aft,{0,-hy,0}),
                       GeneratedSocket("auto_port",AttachmentFace::Port,{-hx,0,0}),GeneratedSocket("auto_starboard",AttachmentFace::Starboard,{hx,0,0}),
                       GeneratedSocket("auto_dorsal",AttachmentFace::Dorsal,{0,0,hz}),GeneratedSocket("auto_ventral",AttachmentFace::Ventral,{0,0,-hz})};
            report.generatedSockets+=e.sockets.size();
        }
        a.elements.push_back(std::move(e));++report.importedElements;
    }

    for(std::size_t i=0;i<recipe.attachments.size();++i){
        const auto& legacy=recipe.attachments[i];
        if(legacy.parentModuleIndex>=a.elements.size()||legacy.childModuleIndex>=a.elements.size()){
            report.errors.push_back("recipe attachment index is out of range");continue;
        }
        auto& parent=a.elements[legacy.parentModuleIndex];auto& child=a.elements[legacy.childModuleIndex];
        std::string ps=legacy.parentSocket,cs=legacy.childSocket;
        if(ps=="auto"||cs=="auto"||ps.empty()||cs.empty()||!FindSocket(parent,ps)||!FindSocket(child,cs)){
            auto chosen=ChooseCompatibleSockets(parent,child);ps=chosen.first;cs=chosen.second;
            if(ps.empty()&&options.generateFallbackSockets){
                ps="bridge_auto_parent_"+std::to_string(i);cs="bridge_auto_child_"+std::to_string(i);
                parent.sockets.push_back(GeneratedSocket(ps,AttachmentFace::Aft,{}));
                child.sockets.push_back(GeneratedSocket(cs,AttachmentFace::Fore,{}));
                report.generatedSockets+=2;
            }
        }
        if(ps.empty()||cs.empty()){report.errors.push_back("unable to resolve compatible semantic sockets for attachment");continue;}
        a.attachments.push_back({parent.id,ps,child.id,cs,true});++report.importedAttachments;
        if(!legacy.certified) report.warnings.push_back("imported an uncertified legacy attachment edge");
    }

    AssemblyConstructionSystem editor;std::string error;
    if(!editor.Begin(a,&error)){report.errors.push_back(error);return report;}
    const auto validation=editor.Validate();
    report.errors.insert(report.errors.end(),validation.errors.begin(),validation.errors.end());
    report.warnings.insert(report.warnings.end(),validation.warnings.begin(),validation.warnings.end());
    report.valid=report.errors.empty()&&validation.valid;
    return report;
}

} // namespace subspace
