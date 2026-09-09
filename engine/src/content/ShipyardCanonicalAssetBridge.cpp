#include "content/ShipyardCanonicalAssetBridge.h"

#include <algorithm>
#include <cmath>

namespace subspace {
namespace {

using namespace assets;

float Dot(const Vector3& a,const Vector3& b){return a.x*b.x+a.y*b.y+a.z*b.z;}
Vector3 Cross(const Vector3& a,const Vector3& b){return {a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x};}

Matrix4 SocketFrame(const ShipyardAssemblySocket& socket){
    Vector3 y{socket.dirX,socket.dirY,socket.dirZ};
    if(y.length()<1.0e-5f)y={0,1,0};else y=y.normalized();
    Vector3 up{socket.upX,socket.upY,socket.upZ};
    if(up.length()<1.0e-5f || std::fabs(Dot(y,up.normalized()))>.96f)
        up=std::fabs(Dot(y,{0,0,1}))>.96f?Vector3{0,1,0}:Vector3{0,0,1};
    else up=up.normalized();
    Vector3 x=Cross(y,up).normalized();
    Vector3 z=Cross(x,y).normalized();
    Matrix4 m;
    // Column-major basis: X right, Y socket outward, Z up, translation.
    m.value={{x.x,x.y,x.z,0,
              y.x,y.y,y.z,0,
              z.x,z.y,z.z,0,
              socket.x,socket.y,socket.z,1}};
    return m;
}

SocketSize Size(ShipyardModuleSize size){
    switch(size){case ShipyardModuleSize::XS:return SocketSize::XS;case ShipyardModuleSize::S:return SocketSize::S;
        case ShipyardModuleSize::M:return SocketSize::M;case ShipyardModuleSize::L:return SocketSize::L;case ShipyardModuleSize::XL:return SocketSize::XL;}
    return SocketSize::M;
}

ModuleRole Role(const ShipyardModuleRecord& r){
    switch(r.partRole){
    case ShipyardPartRole::PrimaryHull:return ModuleRole::Hull;
    case ShipyardPartRole::HullAdapter:return ModuleRole::HullAdapter;
    case ShipyardPartRole::Cockpit:return ModuleRole::Cockpit;
    case ShipyardPartRole::Bridge:return ModuleRole::Bridge;
    case ShipyardPartRole::EngineHousing:return ModuleRole::EngineHousing;
    case ShipyardPartRole::MainEngine:return ModuleRole::MainEngine;
    case ShipyardPartRole::RcsThruster:return ModuleRole::RcsThruster;
    case ShipyardPartRole::Wing:case ShipyardPartRole::Fin:return ModuleRole::Wing;
    case ShipyardPartRole::StructuralFrame:case ShipyardPartRole::StructuralBrace:case ShipyardPartRole::StructuralBlock:return ModuleRole::StructuralFrame;
    case ShipyardPartRole::HardpointBase:return ModuleRole::TurretHardpoint;
    case ShipyardPartRole::WeaponTurret:case ShipyardPartRole::MissileMount:return ModuleRole::WeaponMount;
    case ShipyardPartRole::SensorDish:case ShipyardPartRole::SensorMast:case ShipyardPartRole::SensorAntenna:return ModuleRole::Sensor;
    case ShipyardPartRole::SurfaceDetail:return ModuleRole::Detail;
    default:break;
    }
    if(r.semantic==ShipyardModuleSemantic::EngineNozzle)return ModuleRole::MainEngine;
    if(r.semantic==ShipyardModuleSemantic::Sensor)return ModuleRole::Sensor;
    if(r.semantic==ShipyardModuleSemantic::SurfaceDetail)return ModuleRole::Detail;
    return ModuleRole::Unknown;
}

SocketType TypeFor(std::string_view type){
    if(type=="hull_forward")return SocketType::HullForward;
    if(type=="hull_aft")return SocketType::HullAft;
    if(type=="lateral_surface"||type=="lateral_mount")return SocketType::HullLateral;
    if(type=="engine_cavity")return SocketType::EngineCavity;
    if(type=="engine_mount")return SocketType::EngineMount;
    if(type=="hardpoint_mount")return SocketType::TurretMount;
    if(type=="dorsal_surface"||type=="ventral_surface"||type=="detail_mount")return SocketType::DetailSurface;
    return SocketType::Unknown;
}

std::vector<SocketType> Accepts(SocketType type){
    switch(type){
    case SocketType::HullForward:return {SocketType::HullAft};
    case SocketType::HullAft:return {SocketType::HullForward};
    case SocketType::HullLateral:return {SocketType::HullLateral,SocketType::DetailSurface};
    case SocketType::EngineCavity:return {SocketType::EngineMount};
    case SocketType::EngineMount:return {SocketType::EngineCavity};
    case SocketType::TurretMount:return {SocketType::DetailSurface};
    case SocketType::DetailSurface:return {SocketType::TurretMount,SocketType::DetailSurface,SocketType::HullLateral};
    default:return {};
    }
}

} // namespace

assets::CanonicalAsset ShipyardCanonicalAssetBridge::BuildAsset(const ShipyardModuleRecord& record){
    using namespace assets;
    CanonicalAsset asset;
    asset.assetId=record.source.moduleId;
    asset.provenance.sourcePath="content/derived/greyoxide_shipyard_v07/certified/modules/"+record.source.moduleId+".obj";
    asset.provenance.sourceFormat="OBJ";
    asset.provenance.importer="ShipyardCanonicalAssetBridge";
    asset.provenance.importerVersion="533A";

    CanonicalNode root;
    root.name=record.source.moduleId;
    root.extras["subspace.domain"]="SHIP";
    root.extras["subspace.sourceFamily"]="SHIPYARD_V07_CC0";
    root.extras["subspace.semantic"]=ShipyardModuleSystem::SemanticName(record.semantic);
    root.extras["subspace.moduleClass"]=ShipyardModuleSystem::ClassName(record.moduleClass);
    root.extras["subspace.sizeClass"]=ShipyardModuleSystem::SizeName(record.size);
    root.extras["subspace.pairedPlacement"]=record.pairedPlacement?"true":"false";
    root.extras["subspace.placementRole"]=record.placementRole;
    root.extras["subspace.generatorEligible"]=record.generatorEligible?"true":"false";
    root.extras["subspace.preferredMountFace"]=record.preferredMountFace;
    asset.nodes.push_back(std::move(root));

    ModuleDefinition module;
    module.moduleId=record.source.moduleId;
    module.role=Role(record);
    module.size=Size(record.size);
    module.rootNodeIndex=0;

    for(const auto& sourceSocket:record.sockets){
        if(sourceSocket.type=="exhaust"){
            BoxProxy proxy;proxy.id=record.source.moduleId+".exhaust";proxy.type=ProxyType::Exhaust;proxy.nodeIndex=0;
            proxy.localTransform=SocketFrame(sourceSocket);proxy.halfExtents={.05f,.05f,.05f};asset.proxies.push_back(std::move(proxy));
            continue;
        }
        const auto type=TypeFor(sourceSocket.type);
        if(type==SocketType::Unknown)continue; // weapon-axis and future diagnostic sockets remain non-mating metadata for now.
        ModuleSocket socket;
        socket.id=sourceSocket.name;
        socket.nodeIndex=0;
        socket.type=type;
        socket.size=Size(record.size);
        socket.authority=sourceSocket.manualOverride?SocketAuthority::ManualOverride:SocketAuthority::Reviewed;
        socket.localTransform=SocketFrame(sourceSocket);
        socket.accepts=Accepts(type);
        socket.minInsertionMeters=0.0f;
        socket.maxInsertionMeters=std::max(0.0f,sourceSocket.insertionDepth);
        module.socketIndices.push_back(static_cast<AssetIndex>(asset.sockets.size()));
        asset.sockets.push_back(std::move(socket));
    }
    asset.modules.push_back(std::move(module));
    return asset;
}

std::size_t ShipyardCanonicalAssetBridge::PopulateRegistry(assets::CanonicalAssetRegistry& registry,
                                                            const std::vector<ShipyardModuleRecord>& catalog){
    std::size_t count=0;
    for(const auto& record:catalog)if(registry.Upsert(BuildAsset(record)))++count;
    return count;
}

} // namespace subspace
