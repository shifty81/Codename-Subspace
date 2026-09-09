#include "ship_editor/ShipyardBuilderSystem.h"
#include "ship_editor/ShipyardOrientationConstraintSystem.h"
#include "ship_editor/ShipyardSocketOverrideSystem.h"
#include "ship_editor/ShipyardDefinitionOverrideSystem.h"
#include "ship_editor/ShipyardKitbashTransformSystem.h"
#include "ships/ShipPcgRuntimeClosureSystem.h"
#include "ships/FactionShipDesignSystem.h"

#include <algorithm>
#include <cmath>
#include <queue>
#include <sstream>
#include <tuple>
#include <unordered_set>

namespace subspace {
namespace {

constexpr float kSocketCoincidenceEpsilon = 0.34f;

void ClampPlacementToMorphProfile(const ShipyardModuleRecord& record,
                                  const VisualModulePlacement& baseline,
                                  VisualModulePlacement& placement){
    const auto profile=UniversalKitbashAuthority::BuildProfile(record,KitbashMaterialCertification::NormalizedFallback);
    if(profile.morph.policy==KitbashScalingPolicy::FixedReference){
        placement.scaleX=baseline.scaleX;placement.scaleY=baseline.scaleY;placement.scaleZ=baseline.scaleZ;return;
    }
    if(profile.morph.policy==KitbashScalingPolicy::DiscreteFamily){
        const float uniform=std::clamp((placement.scaleX+placement.scaleY+placement.scaleZ)/3.0f,
                                       profile.morph.minLengthScale,profile.morph.maxLengthScale);
        placement.scaleX=placement.scaleY=placement.scaleZ=uniform;return;
    }
    placement.scaleX=std::clamp(placement.scaleX,profile.morph.minWidthScale,profile.morph.maxWidthScale);
    placement.scaleY=std::clamp(placement.scaleY,profile.morph.minLengthScale,profile.morph.maxLengthScale);
    placement.scaleZ=std::clamp(placement.scaleZ,profile.morph.minHeightScale,profile.morph.maxHeightScale);
}

float TargetUniformScale(const ShipyardModuleRecord& record,UniversalSizeClass requested){
    const auto profile=UniversalKitbashAuthority::BuildProfile(record,KitbashMaterialCertification::NormalizedFallback);
    return UniversalKitbashAuthority::SafeUniformScale(profile,UniversalKitbashAuthority::ClampSizeToMorph(profile,requested));
}

ShipyardModuleClass ClassForSemantic(ShipyardModuleSemantic semantic){
    switch(semantic){
    case ShipyardModuleSemantic::HullBow: case ShipyardModuleSemantic::HullMid: case ShipyardModuleSemantic::HullAft: case ShipyardModuleSemantic::StructuralFrame:return ShipyardModuleClass::Hull;
    case ShipyardModuleSemantic::CommandCockpit: case ShipyardModuleSemantic::CommandBridge:return ShipyardModuleClass::Command;
    case ShipyardModuleSemantic::Adapter:return ShipyardModuleClass::Adapter;
    case ShipyardModuleSemantic::EngineHousing: case ShipyardModuleSemantic::MainEngine: case ShipyardModuleSemantic::EngineNozzle: case ShipyardModuleSemantic::RcsThruster:return ShipyardModuleClass::Propulsion;
    case ShipyardModuleSemantic::TurretHardpoint: case ShipyardModuleSemantic::WeaponMount:return ShipyardModuleClass::Hardpoint;
    case ShipyardModuleSemantic::Wing:return ShipyardModuleClass::Wing;
    case ShipyardModuleSemantic::Sensor: case ShipyardModuleSemantic::SurfaceDetail:return ShipyardModuleClass::Detail;
    case ShipyardModuleSemantic::Component:return ShipyardModuleClass::Component;
    }
    return ShipyardModuleClass::Component;
}

std::string DefaultPlacementRole(ShipyardModuleSemantic semantic){
    switch(semantic){
    case ShipyardModuleSemantic::HullBow:return "HULL_FORWARD";
    case ShipyardModuleSemantic::HullMid:return "HULL_CORE";
    case ShipyardModuleSemantic::HullAft:return "HULL_AFT";
    case ShipyardModuleSemantic::StructuralFrame:return "STRUCTURAL";
    case ShipyardModuleSemantic::CommandCockpit: case ShipyardModuleSemantic::CommandBridge:return "COMMAND";
    case ShipyardModuleSemantic::EngineHousing:return "ENGINE_HOUSING";
    case ShipyardModuleSemantic::MainEngine:return "MAIN_DRIVE";
    case ShipyardModuleSemantic::EngineNozzle:return "ENGINE_NOZZLE";
    case ShipyardModuleSemantic::RcsThruster:return "RCS";
    case ShipyardModuleSemantic::TurretHardpoint: case ShipyardModuleSemantic::WeaponMount:return "HARDPOINT";
    case ShipyardModuleSemantic::Wing:return "LATERAL";
    case ShipyardModuleSemantic::Sensor:return "SENSOR";
    case ShipyardModuleSemantic::SurfaceDetail:return "DETAIL";
    case ShipyardModuleSemantic::Adapter:return "ADAPTER";
    default:return "EXCLUDED";
    }
}

ShipRole RoleFromGeneratorName(const std::string& role){
    const std::string normalized=ProceduralVisualVariantSystem::NormalizeRole(role);
    if(normalized=="ESCORT"||normalized=="PATROL")return ShipRole::Escort;
    if(normalized=="EXPLORATION")return ShipRole::Exploration;
    if(normalized=="MINING")return ShipRole::Mining;
    if(normalized=="SALVAGE")return ShipRole::Salvage;
    if(normalized=="CARRIER")return ShipRole::Carrier;
    if(normalized=="HAULER"||normalized=="TRADER")return ShipRole::Hauling;
    if(normalized=="INDUSTRIAL")return ShipRole::Logistics;
    return ShipRole::GeneralCombat;
}

Vector3 NormalizeSafe(Vector3 v,Vector3 fallback){
    if(v.length()<1.0e-5f)return fallback;
    return v.normalized();
}

Vector3 RotateSocketVector(Vector3 v,const Vector3& deltaDegrees){
    constexpr float kDegToRad=3.14159265358979323846f/180.0f;
    const float px=deltaDegrees.x*kDegToRad,yz=deltaDegrees.y*kDegToRad,ry=deltaDegrees.z*kDegToRad;
    const float cr=std::cos(ry),sr=std::sin(ry);
    const float x1=v.x*cr+v.z*sr,y1=v.y,z1=-v.x*sr+v.z*cr;
    const float cp=std::cos(px),sp=std::sin(px);
    const float x2=x1,y2=y1*cp-z1*sp,z2=y1*sp+z1*cp;
    const float cy=std::cos(yz),sy=std::sin(yz);
    return {x2*cy-y2*sy,x2*sy+y2*cy,z2};
}

void NormalizeSocketFrame(ShipyardAssemblySocket& socket){
    Vector3 dir=NormalizeSafe({socket.dirX,socket.dirY,socket.dirZ},{0,1,0});
    Vector3 up=NormalizeSafe({socket.upX,socket.upY,socket.upZ},{0,0,1});
    up=up-dir*(up.x*dir.x+up.y*dir.y+up.z*dir.z);
    if(up.length()<1.0e-5f)up=std::fabs(dir.z)>.92f?Vector3{0,1,0}:Vector3{0,0,1};
    up=NormalizeSafe(up,{0,0,1});
    socket.dirX=dir.x;socket.dirY=dir.y;socket.dirZ=dir.z;
    socket.upX=up.x;socket.upY=up.y;socket.upZ=up.z;
}

Vector3 SocketWorld(const VisualModulePlacement& p, const ShipyardAssemblySocket& s) {
    // Match the runtime renderer and ShipyardModuleSystem attachment transform:
    // Rz(yaw) * Rx(pitch) * Ry(roll), then translate.  The older builder
    // validator ignored rotation entirely, which made correctly mounted
    // dorsal command pieces, lateral wings, and rotated engine housings look
    // disconnected even when their sockets were coincident.
    constexpr float kDegToRad = 3.14159265358979323846f / 180.0f;
    const float yaw=p.yawDegrees*kDegToRad, pitch=p.pitchDegrees*kDegToRad, roll=p.rollDegrees*kDegToRad;
    const float mirrorX=p.mirrorX?-1.0f:1.0f,mirrorY=p.mirrorY?-1.0f:1.0f,mirrorZ=p.mirrorZ?-1.0f:1.0f;
    float x=s.x*p.scaleX*mirrorX, y=s.y*p.scaleY*mirrorY, z=s.z*p.scaleZ*mirrorZ;
    const float cr=std::cos(roll), sr=std::sin(roll);
    const float x1=x*cr+z*sr, y1=y, z1=-x*sr+z*cr;
    const float cp=std::cos(pitch), sp=std::sin(pitch);
    const float x2=x1, y2=y1*cp-z1*sp, z2=y1*sp+z1*cp;
    const float cy=std::cos(yaw), sy=std::sin(yaw);
    return {p.x + x2*cy-y2*sy, p.y + x2*sy+y2*cy, p.z + z2};
}
float SocketWorldX(const VisualModulePlacement& p, const ShipyardAssemblySocket& s) { return SocketWorld(p,s).x; }
float SocketWorldY(const VisualModulePlacement& p, const ShipyardAssemblySocket& s) { return SocketWorld(p,s).y; }
float SocketWorldZ(const VisualModulePlacement& p, const ShipyardAssemblySocket& s) { return SocketWorld(p,s).z; }

float Distance3(float ax,float ay,float az,float bx,float by,float bz) {
    const float dx=ax-bx,dy=ay-by,dz=az-bz;
    return std::sqrt(dx*dx+dy*dy+dz*dz);
}

VisualModulePlacement AttachPlacement(const VisualModulePlacement& parentPlacement,
                                      const ShipyardAssemblySocket& parentSocket,
                                      const ShipyardModuleRecord& child,
                                      const ShipyardAssemblySocket& childSocket,
                                      float uniformScale) {
    VisualModulePlacement result;
    result.moduleId = child.source.moduleId;
    result.scaleX = result.scaleY = result.scaleZ = uniformScale;
    result.material = child.moduleClass == ShipyardModuleClass::Command
        ? SpaceMaterialKind::Canopy
        : child.moduleClass == ShipyardModuleClass::Propulsion
            ? SpaceMaterialKind::EngineHousing
            : child.moduleClass == ShipyardModuleClass::Hull
                ? SpaceMaterialKind::IndustrialHull
                : SpaceMaterialKind::StructuralMetal;

    const float px = SocketWorldX(parentPlacement,parentSocket);
    const float py = SocketWorldY(parentPlacement,parentSocket);
    const float pz = SocketWorldZ(parentPlacement,parentSocket);
    const float insertion = parentSocket.insertionDepth * parentPlacement.scaleY;
    result.x = px - childSocket.x * uniformScale - parentSocket.dirX * insertion;
    result.y = py - childSocket.y * uniformScale - parentSocket.dirY * insertion;
    result.z = pz - childSocket.z * uniformScale - parentSocket.dirZ * insertion;
    return result;
}

bool IsStructuralClass(ShipyardModuleClass c) {
    // Connectivity BFS is the load-bearing spine graph, not every mounted
    // functional object. Command sections, engines/housings and wings mount to
    // hull surfaces/cavities and must not be mistaken for nose-to-tail spine
    // segments merely because they participate in gameplay.
    return c==ShipyardModuleClass::Hull || c==ShipyardModuleClass::Adapter;
}

std::string CompactId(const std::string& id, std::size_t maxChars=27) {
    std::string out=id;
    const std::string prefix="shipyard_a_";
    if(out.rfind(prefix,0)==0)out.erase(0,prefix.size());
    if(out.size()>maxChars)out=out.substr(0,maxChars-3)+"...";
    return out;
}

std::string ModuleSerial(const std::string& id){
    const std::string prefix="shipyard_a_";
    if(id.rfind(prefix,0)!=0)return {};
    const auto classEnd=id.find('_',prefix.size());if(classEnd==std::string::npos)return {};
    const auto serialEnd=id.find('_',classEnd+1);if(serialEnd==std::string::npos)return {};
    return id.substr(classEnd+1,serialEnd-classEnd-1);
}

std::string FriendlyModuleLabel(const ShipyardModuleRecord& record){
    std::string label=ShipyardPartTaxonomySystem::DisplayName(record);
    const auto serial=ModuleSerial(record.source.moduleId);
    if(!serial.empty())label+=" "+serial;
    return label;
}

std::vector<std::string> Roles(){return {"INDUSTRIAL","COMBAT","MINING","HAULER","EXPLORATION"};}

struct LiveryPreset { const char* name; ShipPaintLayer primary; ShipPaintLayer secondary; ShipPaintLayer trim; };

const std::vector<LiveryPreset>& LiveryPresets(){
    static const std::vector<LiveryPreset> presets={
        {"FORGE", {"PRIMARY",.22f,.31f,.36f,1,.66f,.33f}, {"SECONDARY",.075f,.11f,.14f,1,.58f,.41f}, {"TRIM",.82f,.46f,.12f,1,.72f,.27f}},
        {"NAVAL", {"PRIMARY",.16f,.25f,.39f,1,.70f,.30f}, {"SECONDARY",.055f,.075f,.12f,1,.62f,.38f}, {"TRIM",.53f,.74f,.86f,1,.68f,.24f}},
        {"MINER", {"PRIMARY",.48f,.29f,.075f,1,.63f,.36f}, {"SECONDARY",.16f,.13f,.095f,1,.56f,.44f}, {"TRIM",.94f,.65f,.16f,1,.70f,.29f}},
        {"EXPLORER", {"PRIMARY",.11f,.35f,.39f,1,.61f,.34f}, {"SECONDARY",.06f,.14f,.17f,1,.54f,.42f}, {"TRIM",.38f,.82f,.82f,1,.64f,.25f}},
        {"HAULER", {"PRIMARY",.24f,.34f,.25f,1,.58f,.39f}, {"SECONDARY",.10f,.15f,.12f,1,.52f,.46f}, {"TRIM",.70f,.76f,.43f,1,.63f,.31f}},
        {"CRIMSON", {"PRIMARY",.35f,.10f,.105f,1,.68f,.31f}, {"SECONDARY",.12f,.045f,.055f,1,.61f,.40f}, {"TRIM",.86f,.37f,.23f,1,.72f,.26f}}
    };
    return presets;
}


struct PaintChip { const char* name; float r,g,b; };
const std::vector<PaintChip>& PaintPalette(){
    static const std::vector<PaintChip> chips={
        {"BLACK",.045f,.055f,.065f},
        {"GRAPHITE",.105f,.125f,.145f},
        {"STEEL",.30f,.38f,.44f},
        {"WHITE",.78f,.82f,.82f},
        {"NAVY",.10f,.22f,.44f},
        {"COBALT",.12f,.34f,.68f},
        {"TEAL",.06f,.44f,.46f},
        {"FOREST",.12f,.34f,.20f},
        {"OLIVE",.34f,.39f,.15f},
        {"SAND",.54f,.43f,.24f},
        {"AMBER",.86f,.47f,.08f},
        {"ORANGE",.92f,.25f,.055f},
        {"CRIMSON",.58f,.075f,.085f},
        {"MAGENTA",.58f,.10f,.40f},
        {"VIOLET",.36f,.15f,.58f},
        {"CYAN",.10f,.68f,.76f}
    };
    return chips;
}
std::size_t ClosestPaint(const ShipPaintLayer& layer){
    const auto& chips=PaintPalette();std::size_t best=0;float bestD=1.0e9f;
    for(std::size_t i=0;i<chips.size();++i){
        const float dr=layer.r-chips[i].r,dg=layer.g-chips[i].g,db=layer.b-chips[i].b;
        const float d=dr*dr+dg*dg+db*db;if(d<bestD){bestD=d;best=i;}
    }
    return best;
}
std::string StepPaintLayer(ShipPaintLayer& layer,int delta){
    const auto& chips=PaintPalette();if(chips.empty())return {};
    const std::size_t start=ClosestPaint(layer);
    const auto n=static_cast<long long>(chips.size());
    const auto next=static_cast<std::size_t>((static_cast<long long>(start)+delta+n)%n);
    const auto& chip=chips[next];
    layer.r=chip.r;layer.g=chip.g;layer.b=chip.b;layer.a=1.0f;
    return chip.name;
}
std::string PaintName(const ShipPaintLayer& layer){
    const auto& chips=PaintPalette();return chips.empty()?std::string("CUSTOM"):std::string(chips[ClosestPaint(layer)].name);
}

const std::vector<std::string>& DecalPresets(){
    static const std::vector<std::string> presets={"SUBSPACE_STRIPE","HAZARD_CHEVRON","IDENT_BAR","SURVEY_MARK","FLEET_HASH"};
    return presets;
}

} // namespace

void ShipyardBuilderSystem::Initialize(std::vector<ShipyardModuleRecord> catalog,
                                       const ProceduralShipVisualRecipe& starterRecipe) {
    model_ = {};
    model_.catalog = std::move(catalog);
    initialCatalog_ = model_.catalog;
    socketHistory_.clear();
    socketHistoryCursor_=0;
    socketTransform_={};
    model_.recipe = starterRecipe;
    model_.initialized = !model_.catalog.empty();
    model_.liveApplyEnabled = true; // API compatibility; runtime design modes explicitly override this.
    // Direct construction-system users historically receive the full authoring surface.
    // Runtime entry points explicitly narrow this to PlayerDocked when appropriate.
    // Keeping Initialize() authoring-capable preserves the single Shipyard authority
    // used by regression tools, Dev Mode, and the main-menu Shipyard studio.
    model_.accessMode = ShipyardAccessMode::MainMenuStudio;
    model_.capabilities = ShipyardCapabilitySystem::For(model_.accessMode);
    model_.modeling.recipe.recipeId = "shipyard.model.session";
    model_.modeling.recipe.displayName = "Shipyard Modeled Module";
    model_.pcgStudio.request.factionId = "PLAYER";
    model_.worldAuthoring.world.planetId = "shipyard.devworld";
    model_.worldScale = WorldScaleAuthoritySystem::DefaultProfile();
    model_.animationLibrary = CharacterAnimationLibrarySystem::QuaterniusUniversalAnimationLibrary2();
    model_.animationPreview.libraryId = model_.animationLibrary.libraryId;
    model_.devWorld = ShipyardDevWorldSystem::CreateDefault(model_.worldScale);
    model_.devWorld.enabled = true;
    model_.interiorPlan = ShipModuleInteriorLinkSystem::BuildPlan(model_.catalog, model_.recipe, model_.worldScale);
    model_.role = starterRecipe.role.empty()?"INDUSTRIAL":starterRecipe.role;
    model_.seed = starterRecipe.seed?starterRecipe.seed:0x51A7D007u;
    model_.symmetryFrame.axis=ConstructionSymmetryAxis::PortStarboard;
    model_.symmetryFrame.live=true;
    model_.mirrorX=true;
    if(!starterRecipe.modules.empty()){
        Vector3 c{};for(const auto& m:starterRecipe.modules){c.x+=m.x;c.y+=m.y;c.z+=m.z;}
        const float inv=1.0f/static_cast<float>(starterRecipe.modules.size());model_.symmetryFrame.origin={c.x*inv,c.y*inv,c.z*inv};
    }
    model_.status = model_.initialized
        ? "Certified Shipyard catalog loaded - mouse controls active"
        : "Certified Shipyard catalog unavailable";
    RefreshForwardAuthority();
    initialRecipe_ = model_.recipe;
    NormalizeSelections();
    model_.validation = Validate();
}

void ShipyardBuilderSystem::SetAvailableModuleIds(std::vector<std::string> moduleIds){availableModuleIds_=std::move(moduleIds);NormalizeSelections();}

void ShipyardBuilderSystem::SetAppearance(const ShipAppearanceState& appearance){
    model_.appearance=appearance;initialAppearance_=appearance;
    model_.liveryName="CUSTOM";
    model_.primaryPaintName=PaintName(model_.appearance.primary);
    model_.secondaryPaintName=PaintName(model_.appearance.secondary);
    model_.trimPaintName=PaintName(model_.appearance.trim);
    model_.dirty=false;
}

void ShipyardBuilderSystem::ApplyLiveryPreset(){
    const auto& presets=LiveryPresets();if(presets.empty())return;
    model_.liveryPreset%=presets.size();const auto& p=presets[model_.liveryPreset];
    model_.appearance.primary=p.primary;model_.appearance.secondary=p.secondary;model_.appearance.trim=p.trim;
    model_.liveryName=p.name;
    model_.primaryPaintName=PaintName(model_.appearance.primary);
    model_.secondaryPaintName=PaintName(model_.appearance.secondary);
    model_.trimPaintName=PaintName(model_.appearance.trim);
    model_.status=std::string("Paint preset: ")+p.name;model_.dirty=true;
}

bool ShipyardBuilderSystem::AddSelectedDecal(){
    if(model_.recipe.modules.empty())return false;const auto& presets=DecalPresets();if(presets.empty())return false;
    ShipDecalLayer d;d.id="D"+std::to_string(model_.appearance.decals.size()+1);d.decalAsset=presets[model_.decalPreset%presets.size()];
    d.moduleIndex=std::min(model_.selectedPlacedModule,model_.recipe.modules.size()-1);d.u=.50f;d.v=.52f;d.scale=.78f;d.rotationDegrees=0.0f;d.opacity=.92f;d.mirror=model_.mirrorX;
    model_.appearance.decals.push_back(d);model_.status="Added decal "+d.decalAsset+" to selected module";model_.dirty=true;return true;
}

bool ShipyardBuilderSystem::RemoveSelectedDecal(){
    if(model_.appearance.decals.empty()||model_.recipe.modules.empty())return false;
    const auto selected=std::min(model_.selectedPlacedModule,model_.recipe.modules.size()-1);
    for(auto it=model_.appearance.decals.end();it!=model_.appearance.decals.begin();){--it;if(it->moduleIndex==selected){model_.status="Removed decal "+it->decalAsset;model_.appearance.decals.erase(it);model_.dirty=true;return true;}}
    model_.status="No decal on selected module";return false;
}

void ShipyardBuilderSystem::SetLiveApplyEnabled(bool enabled, bool standaloneDesign){
    model_.liveApplyEnabled=enabled;
    model_.standaloneDesign=standaloneDesign;
    SetAccessMode(standaloneDesign?ShipyardAccessMode::MainMenuStudio:ShipyardAccessMode::PlayerDocked);
    if(standaloneDesign) model_.status="Shipyard Dev Studio ready - modeling, PCG, and world authoring enabled";
    else if(enabled) model_.status="Docked player Shipyard ready";
    else model_.status="Blueprint design ready";
}

void ShipyardBuilderSystem::SetAccessMode(ShipyardAccessMode mode){
    model_.accessMode=mode;
    model_.capabilities=ShipyardCapabilitySystem::For(mode);
    if(!model_.capabilities.model&&model_.workspaceMode==ShipyardWorkspaceMode::Model)model_.workspaceMode=ShipyardWorkspaceMode::Build;
    if(!model_.capabilities.pcgStudio&&model_.workspaceMode==ShipyardWorkspaceMode::Pcg)model_.workspaceMode=ShipyardWorkspaceMode::Build;
    if(!model_.capabilities.world&&model_.workspaceMode==ShipyardWorkspaceMode::World)model_.workspaceMode=ShipyardWorkspaceMode::Build;
    if(!model_.capabilities.interior&&model_.workspaceMode==ShipyardWorkspaceMode::Interior)model_.workspaceMode=ShipyardWorkspaceMode::Build;
    if(!model_.capabilities.character&&model_.workspaceMode==ShipyardWorkspaceMode::Character)model_.workspaceMode=ShipyardWorkspaceMode::Build;
    if(!model_.capabilities.devWorld&&model_.workspaceMode==ShipyardWorkspaceMode::DevWorld)model_.workspaceMode=ShipyardWorkspaceMode::Build;
    if(!model_.capabilities.rawAuthoring&&model_.workspaceMode==ShipyardWorkspaceMode::Authoring)model_.workspaceMode=ShipyardWorkspaceMode::Build;
}

std::vector<std::size_t> ShipyardBuilderSystem::FilteredCatalogIndices() const {
    std::vector<std::size_t> out;
    const auto wantedDomain=UniversalConstructionSystem::Domain(model_.constructionMode);
    for(std::size_t i=0;i<model_.catalog.size();++i){
        const auto& record=model_.catalog[i];
        if(record.moduleClass!=model_.selectedClass)continue;
        if(!availableModuleIds_.empty()&&std::find(availableModuleIds_.begin(),availableModuleIds_.end(),record.source.moduleId)==availableModuleIds_.end())continue;
        const auto profile=UniversalKitbashAuthority::BuildProfile(record,KitbashMaterialCertification::NormalizedFallback);
        const bool domainMatch=std::any_of(profile.domainRoles.begin(),profile.domainRoles.end(),[&](const auto& role){return role.domain==wantedDomain;});
        if(!domainMatch)continue;
        if(model_.constructionMode==ConstructionWorkspaceMode::Ship && !ShipPcgRuntimeClosureSystem::ModuleFitsClass(record,model_.shipClass,true))continue;
        out.push_back(i);
    }
    return out;
}

const ShipyardModuleRecord* ShipyardBuilderSystem::SelectedCatalogModule() const {
    const auto filtered=FilteredCatalogIndices();
    if(filtered.empty())return nullptr;
    const std::size_t i=std::min(model_.selectedFilteredModule,filtered.size()-1);
    return &model_.catalog[filtered[i]];
}

const VisualModulePlacement* ShipyardBuilderSystem::SelectedPlacedModule() const {
    if(model_.recipe.modules.empty())return nullptr;
    return &model_.recipe.modules[std::min(model_.selectedPlacedModule,model_.recipe.modules.size()-1)];
}

const ShipyardModuleRecord* ShipyardBuilderSystem::SelectedPlacedRecord() const {
    if(model_.recipe.modules.empty())return nullptr;
    const auto index=std::min(model_.selectedPlacedModule,model_.recipe.modules.size()-1);
    return FindRecord(model_.recipe.modules[index].moduleId);
}

ShipyardModuleRecord* ShipyardBuilderSystem::MutableSelectedPlacedRecord(){
    if(model_.recipe.modules.empty())return nullptr;
    const auto index=std::min(model_.selectedPlacedModule,model_.recipe.modules.size()-1);
    const auto& id=model_.recipe.modules[index].moduleId;
    for(auto& record:model_.catalog)if(record.source.moduleId==id)return &record;
    return nullptr;
}

const ShipyardAssemblySocket* ShipyardBuilderSystem::SelectedSocket() const {
    const auto* record=SelectedPlacedRecord();
    if(!record||record->sockets.empty())return nullptr;
    return &record->sockets[std::min(model_.selectedSocket,record->sockets.size()-1)];
}

void ShipyardBuilderSystem::SyncSocketSelection(){
    const auto* record=SelectedPlacedRecord();
    if(!record||record->sockets.empty()){model_.selectedSocket=0;return;}
    model_.selectedSocket=std::min(model_.selectedSocket,record->sockets.size()-1);
}

const ShipyardModuleRecord* ShipyardBuilderSystem::FindRecord(const std::string& moduleId) const {
    for(const auto& r:model_.catalog)if(r.source.moduleId==moduleId)return &r;
    return nullptr;
}

void ShipyardBuilderSystem::NormalizeSelections(){
    auto filtered=FilteredCatalogIndices();
    if(filtered.empty())model_.selectedFilteredModule=0;
    else model_.selectedFilteredModule=std::min(model_.selectedFilteredModule,filtered.size()-1);
    if(model_.recipe.modules.empty())model_.selectedPlacedModule=0;
    else model_.selectedPlacedModule=std::min(model_.selectedPlacedModule,model_.recipe.modules.size()-1);
    SyncSocketSelection();
}

void ShipyardBuilderSystem::SyncCatalogSelectionToPlaced(){
    if(model_.recipe.modules.empty())return;
    const auto placedIndex=std::min(model_.selectedPlacedModule,model_.recipe.modules.size()-1);
    const auto* record=FindRecord(model_.recipe.modules[placedIndex].moduleId);
    if(!record)return;
    model_.selectedClass=record->moduleClass;
    const auto filtered=FilteredCatalogIndices();
    for(std::size_t i=0;i<filtered.size();++i){
        if(model_.catalog[filtered[i]].source.moduleId==record->source.moduleId){model_.selectedFilteredModule=i;break;}
    }
}

void ShipyardBuilderSystem::RefreshForwardAuthority(){
    if(model_.recipe.sourceFamily=="SHIPYARD_V07_CC0"&&model_.recipe.forwardAuthority=="LEGACY")
        model_.recipe.forwardVisualYawDegrees=180.0f;
    model_.recipe.cockpitModuleIndex=-1;
    for(std::size_t i=0;i<model_.recipe.modules.size();++i){
        const auto* record=FindRecord(model_.recipe.modules[i].moduleId);
        if(record&&record->moduleClass==ShipyardModuleClass::Command){
            model_.recipe.cockpitModuleIndex=static_cast<int>(i);
            model_.recipe.forwardAuthority="COCKPIT";
            return;
        }
    }
    if(model_.recipe.forwardAuthority=="COCKPIT")model_.recipe.forwardAuthority="FORWARD_MARKER";
}

void ShipyardBuilderSystem::InvalidateRecipeMetadata(){
    model_.recipe.anchors.clear();
    model_.recipe.hardpoints.clear();
    model_.recipe.acceptedByArtDirector=false;
    model_.recipe.qualityScore=0.0f;
    model_.dirty=true;
}

void ShipyardBuilderSystem::PushSocketHistory(const std::string& moduleId,
                                                const std::vector<ShipyardAssemblySocket>& before,
                                                const std::vector<ShipyardAssemblySocket>& after){
    if(ShipyardSocketOverrideSystem::SocketListsEqual(before,after))return;
    if(socketHistoryCursor_<socketHistory_.size())
        socketHistory_.erase(socketHistory_.begin()+static_cast<std::ptrdiff_t>(socketHistoryCursor_),socketHistory_.end());
    socketHistory_.push_back({moduleId,before,after});
    if(socketHistory_.size()>100){socketHistory_.erase(socketHistory_.begin());}
    else ++socketHistoryCursor_;
    model_.socketOverridesDirty=true;
}

bool ShipyardBuilderSystem::CanUndoSocketEdit() const{return socketHistoryCursor_>0;}
bool ShipyardBuilderSystem::CanRedoSocketEdit() const{return socketHistoryCursor_<socketHistory_.size();}

bool ShipyardBuilderSystem::UndoSocketEdit(){
    if(!CanUndoSocketEdit())return false;
    const auto& edit=socketHistory_[--socketHistoryCursor_];
    for(auto& record:model_.catalog)if(record.source.moduleId==edit.moduleId){record.sockets=edit.before;break;}
    SyncSocketSelection();ReflowRecipeAttachments();model_.validation=Validate();model_.socketOverridesDirty=true;
    model_.status="Socket edit undone";return true;
}

bool ShipyardBuilderSystem::RedoSocketEdit(){
    if(!CanRedoSocketEdit())return false;
    const auto& edit=socketHistory_[socketHistoryCursor_++];
    for(auto& record:model_.catalog)if(record.source.moduleId==edit.moduleId){record.sockets=edit.after;break;}
    SyncSocketSelection();ReflowRecipeAttachments();model_.validation=Validate();model_.socketOverridesDirty=true;
    model_.status="Socket edit redone";return true;
}

void ShipyardBuilderSystem::ReflowRecipeAttachments(){
    if(model_.recipe.modules.empty())return;
    const std::size_t iterations=std::max<std::size_t>(1,model_.recipe.modules.size());
    for(std::size_t pass=0;pass<iterations;++pass){
        bool any=false;
        for(const auto& edge:model_.recipe.attachments){
            if(edge.parentModuleIndex>=model_.recipe.modules.size()||edge.childModuleIndex>=model_.recipe.modules.size())continue;
            auto& parent=model_.recipe.modules[edge.parentModuleIndex];
            auto& childPlacement=model_.recipe.modules[edge.childModuleIndex];
            const auto* parentRecord=FindRecord(parent.moduleId);const auto* childRecord=FindRecord(childPlacement.moduleId);
            if(!parentRecord||!childRecord)continue;
            const ShipyardAssemblySocket* parentSocket=nullptr;const ShipyardAssemblySocket* childSocket=nullptr;
            for(const auto& socket:parentRecord->sockets)if(socket.name==edge.parentSocket){parentSocket=&socket;break;}
            for(const auto& socket:childRecord->sockets)if(socket.name==edge.childSocket){childSocket=&socket;break;}
            if(!parentSocket||!childSocket)continue;
            const float scale=std::max(.05f,childPlacement.scaleX);
            using PlacementFn=VisualModulePlacement (*)(const VisualModulePlacement&,const ShipyardAssemblySocket&,const ShipyardModuleRecord&,const ShipyardAssemblySocket&,float);
            const auto placementFn=static_cast<PlacementFn>(&ShipyardModuleSystem::BuildAttachmentPlacement);
            auto next=placementFn(parent,*parentSocket,*childRecord,*childSocket,scale);
            next.material=childPlacement.material;next.mirrorX=childPlacement.mirrorX;
            if(std::fabs(next.x-childPlacement.x)>.00001f||std::fabs(next.y-childPlacement.y)>.00001f||std::fabs(next.z-childPlacement.z)>.00001f||
               std::fabs(next.yawDegrees-childPlacement.yawDegrees)>.00001f||std::fabs(next.pitchDegrees-childPlacement.pitchDegrees)>.00001f||std::fabs(next.rollDegrees-childPlacement.rollDegrees)>.00001f){
                childPlacement=next;any=true;
            }
        }
        if(!any)break;
    }
    InvalidateRecipeMetadata();
}

bool ShipyardBuilderSystem::AddSocket(){
    auto* record=MutableSelectedPlacedRecord();if(!record)return false;
    const auto before=record->sockets;
    ShipyardAssemblySocket socket;
    int suffix=1;
    do{socket.name="manual_socket_"+std::to_string(suffix++);}while(std::any_of(record->sockets.begin(),record->sockets.end(),[&](const auto& s){return s.name==socket.name;}));
    socket.type="detail_mount";socket.x=0.0f;socket.y=0.0f;socket.z=record->source.halfHeight;
    socket.dirX=0.0f;socket.dirY=0.0f;socket.dirZ=1.0f;socket.insertionDepth=.05f;socket.manualOverride=true;
    record->sockets.push_back(socket);model_.selectedSocket=record->sockets.size()-1;
    PushSocketHistory(record->source.moduleId,before,record->sockets);model_.validation=Validate();
    model_.status="Added manual socket "+socket.name;return true;
}

bool ShipyardBuilderSystem::RemoveSocket(){
    auto* record=MutableSelectedPlacedRecord();if(!record||record->sockets.empty())return false;
    const auto index=std::min(model_.selectedSocket,record->sockets.size()-1);const auto socketName=record->sockets[index].name;
    for(const auto& edge:model_.recipe.attachments){
        if(edge.parentModuleIndex<model_.recipe.modules.size()&&model_.recipe.modules[edge.parentModuleIndex].moduleId==record->source.moduleId&&edge.parentSocket==socketName){model_.status="Cannot delete socket while an attachment uses it";return false;}
        if(edge.childModuleIndex<model_.recipe.modules.size()&&model_.recipe.modules[edge.childModuleIndex].moduleId==record->source.moduleId&&edge.childSocket==socketName){model_.status="Cannot delete socket while an attachment uses it";return false;}
    }
    const auto before=record->sockets;record->sockets.erase(record->sockets.begin()+static_cast<std::ptrdiff_t>(index));
    PushSocketHistory(record->source.moduleId,before,record->sockets);SyncSocketSelection();model_.validation=Validate();
    model_.status="Removed socket "+socketName;return true;
}

bool ShipyardBuilderSystem::MirrorSelectedSocketX(){
    auto* record=MutableSelectedPlacedRecord();if(!record||record->sockets.empty())return false;
    const auto before=record->sockets;const auto index=std::min(model_.selectedSocket,record->sockets.size()-1);
    auto mirrored=ConstructionSymmetrySystem::ReflectSocket(record->sockets[index],model_.symmetryFrame.axis);
    std::string base=mirrored.name;if(base==record->sockets[index].name)base+="_mirror";mirrored.name=base;int suffix=2;
    while(std::any_of(record->sockets.begin(),record->sockets.end(),[&](const auto& s){return s.name==mirrored.name;}))mirrored.name=base+std::to_string(suffix++);
    NormalizeSocketFrame(mirrored);record->sockets.push_back(mirrored);model_.selectedSocket=record->sockets.size()-1;
    PushSocketHistory(record->source.moduleId,before,record->sockets);model_.validation=Validate();
    model_.status=std::string("Mirrored socket across ")+ConstructionSymmetrySystem::AxisName(model_.symmetryFrame.axis);return true;
}

bool ShipyardBuilderSystem::CycleSelectedSocketType(){
    auto* record=MutableSelectedPlacedRecord();if(!record||record->sockets.empty())return false;
    static const std::vector<std::string> types={"hull_forward","hull_aft","lateral_surface","dorsal_surface","ventral_surface","engine_cavity","engine_mount","hardpoint_mount","detail_mount","weapon_axis","exhaust"};
    const auto before=record->sockets;auto& socket=record->sockets[std::min(model_.selectedSocket,record->sockets.size()-1)];
    auto it=std::find(types.begin(),types.end(),socket.type);std::size_t index=it==types.end()?0:(static_cast<std::size_t>(std::distance(types.begin(),it))+1)%types.size();
    socket.type=types[index];socket.manualOverride=true;PushSocketHistory(record->source.moduleId,before,record->sockets);ReflowRecipeAttachments();model_.validation=Validate();
    model_.status="Socket type: "+socket.type;return true;
}

bool ShipyardBuilderSystem::ResetSelectedSocket(){
    auto* record=MutableSelectedPlacedRecord();if(!record||record->sockets.empty())return false;
    const auto index=std::min(model_.selectedSocket,record->sockets.size()-1);const std::string name=record->sockets[index].name;
    const ShipyardModuleRecord* baseline=nullptr;for(const auto& candidate:initialCatalog_)if(candidate.source.moduleId==record->source.moduleId){baseline=&candidate;break;}
    if(!baseline){model_.status="No baseline definition for selected module";return false;}
    const auto source=std::find_if(baseline->sockets.begin(),baseline->sockets.end(),[&](const auto& socket){return socket.name==name;});
    if(source==baseline->sockets.end()){model_.status="Manual socket has no inferred baseline - remove it instead";return false;}
    const auto before=record->sockets;record->sockets[index]=*source;PushSocketHistory(record->source.moduleId,before,record->sockets);ReflowRecipeAttachments();model_.validation=Validate();
    model_.status="Socket reset to certified/inferred baseline";return true;
}

bool ShipyardBuilderSystem::BeginSelectedSocketTransform(){
    if(model_.inspectorTab!=ShipyardInspectorTab::Sockets)return false;
    auto* record=MutableSelectedPlacedRecord();if(!record||record->sockets.empty())return false;
    const auto index=std::min(model_.selectedSocket,record->sockets.size()-1);
    socketTransform_.active=true;socketTransform_.moduleId=record->source.moduleId;socketTransform_.socketIndex=index;
    socketTransform_.original=record->sockets[index];socketTransform_.working=record->sockets[index];
    model_.status=model_.transformTool==ShipyardTransformTool::Rotate?"SOCKET ROTATE / drag orientation; CTRL = roll":"SOCKET MOVE / drag in active space";
    return model_.transformTool==ShipyardTransformTool::Move||model_.transformTool==ShipyardTransformTool::Rotate;
}

bool ShipyardBuilderSystem::TranslateSelectedSocket(const Vector3& delta,bool fine){
    if(!socketTransform_.active&&!BeginSelectedSocketTransform())return false;
    if(model_.transformTool!=ShipyardTransformTool::Move)return false;
    const float precision=fine?.1f:1.0f;socketTransform_.working.x+=delta.x*precision;socketTransform_.working.y+=delta.y*precision;socketTransform_.working.z+=delta.z*precision;
    socketTransform_.working.manualOverride=true;
    auto* record=MutableSelectedPlacedRecord();if(!record||socketTransform_.socketIndex>=record->sockets.size())return false;
    record->sockets[socketTransform_.socketIndex]=socketTransform_.working;ReflowRecipeAttachments();model_.validation=Validate();return true;
}

bool ShipyardBuilderSystem::RotateSelectedSocket(const Vector3& deltaDegrees,bool fine){
    if(!socketTransform_.active&&!BeginSelectedSocketTransform())return false;
    if(model_.transformTool!=ShipyardTransformTool::Rotate)return false;
    const float precision=fine?.1f:1.0f;const Vector3 delta=deltaDegrees*precision;
    Vector3 dir=RotateSocketVector({socketTransform_.working.dirX,socketTransform_.working.dirY,socketTransform_.working.dirZ},delta);
    Vector3 up=RotateSocketVector({socketTransform_.working.upX,socketTransform_.working.upY,socketTransform_.working.upZ},delta);
    socketTransform_.working.dirX=dir.x;socketTransform_.working.dirY=dir.y;socketTransform_.working.dirZ=dir.z;
    socketTransform_.working.upX=up.x;socketTransform_.working.upY=up.y;socketTransform_.working.upZ=up.z;socketTransform_.working.manualOverride=true;NormalizeSocketFrame(socketTransform_.working);
    auto* record=MutableSelectedPlacedRecord();if(!record||socketTransform_.socketIndex>=record->sockets.size())return false;
    record->sockets[socketTransform_.socketIndex]=socketTransform_.working;ReflowRecipeAttachments();model_.validation=Validate();return true;
}

bool ShipyardBuilderSystem::CommitSocketTransform(){
    if(!socketTransform_.active)return false;
    auto* record=MutableSelectedPlacedRecord();if(!record||record->source.moduleId!=socketTransform_.moduleId||socketTransform_.socketIndex>=record->sockets.size()){socketTransform_={};return false;}
    const auto before=record->sockets;auto originalList=before;originalList[socketTransform_.socketIndex]=socketTransform_.original;
    record->sockets[socketTransform_.socketIndex]=socketTransform_.working;PushSocketHistory(record->source.moduleId,originalList,record->sockets);socketTransform_={};
    model_.validation=Validate();model_.status=model_.validation.valid?"Socket transform committed":"Socket transform committed - current ship needs review";return true;
}

bool ShipyardBuilderSystem::CancelSocketTransform(){
    if(!socketTransform_.active)return false;
    for(auto& record:model_.catalog)if(record.source.moduleId==socketTransform_.moduleId&&socketTransform_.socketIndex<record.sockets.size()){record.sockets[socketTransform_.socketIndex]=socketTransform_.original;break;}
    socketTransform_={};ReflowRecipeAttachments();model_.validation=Validate();model_.status="Socket transform cancelled";return true;
}


bool ShipyardBuilderSystem::CycleSelectedSemantic(int delta){
    auto* record=MutableSelectedPlacedRecord();if(!record)return false;
    constexpr int count=static_cast<int>(ShipyardModuleSemantic::Component)+1;
    int index=(static_cast<int>(record->semantic)+delta)%count;if(index<0)index+=count;
    record->semantic=static_cast<ShipyardModuleSemantic>(index);
    record->moduleClass=ClassForSemantic(record->semantic);
    record->builderCategory=ShipyardPartTaxonomySystem::CategoryFor(record->moduleClass);
    record->partRole=ShipyardPartTaxonomySystem::RoleFor(record->semantic,record->source.moduleId);
    record->primaryHull=record->partRole==ShipyardPartRole::PrimaryHull;
    record->functional=record->moduleClass==ShipyardModuleClass::Command||record->moduleClass==ShipyardModuleClass::Propulsion||record->moduleClass==ShipyardModuleClass::Hardpoint;
    record->surfaceOnly=record->semantic==ShipyardModuleSemantic::SurfaceDetail;
    record->placementRole=DefaultPlacementRole(record->semantic);
    model_.selectedClass=record->moduleClass;model_.definitionOverridesDirty=true;model_.validation=Validate();
    model_.status=std::string("Teach PCG semantic: ")+ShipyardModuleSystem::SemanticName(record->semantic);return true;
}

bool ShipyardBuilderSystem::ToggleSelectedGeneratorEligibility(){auto* r=MutableSelectedPlacedRecord();if(!r)return false;r->generatorEligible=!r->generatorEligible;model_.definitionOverridesDirty=true;model_.status=std::string("PCG eligibility ")+(r->generatorEligible?"ON":"OFF");return true;}
bool ShipyardBuilderSystem::ToggleSelectedPairedPlacement(){auto* r=MutableSelectedPlacedRecord();if(!r)return false;r->pairedPlacement=!r->pairedPlacement;r->mirrorPreferred=r->pairedPlacement;model_.definitionOverridesDirty=true;model_.status=std::string("Paired placement ")+(r->pairedPlacement?"ON":"OFF");return true;}
bool ShipyardBuilderSystem::CycleSelectedPreferredMountFace(){
    auto* r=MutableSelectedPlacedRecord();if(!r)return false;static const std::vector<std::string> faces={"","AFT","FORWARD","PORT","STARBOARD","DORSAL","VENTRAL","LATERAL"};
    auto it=std::find(faces.begin(),faces.end(),r->preferredMountFace);std::size_t i=it==faces.end()?0:(static_cast<std::size_t>(it-faces.begin())+1)%faces.size();r->preferredMountFace=faces[i];r->mountFaceConfidence=r->preferredMountFace.empty()?0.0f:1.0f;model_.definitionOverridesDirty=true;model_.status="Preferred mount face: "+(r->preferredMountFace.empty()?std::string("AUTO"):r->preferredMountFace);return true;
}
bool ShipyardBuilderSystem::ResetSelectedDefinitionOverride(){
    auto* r=MutableSelectedPlacedRecord();if(!r)return false;for(const auto& base:initialCatalog_)if(base.source.moduleId==r->source.moduleId){auto sockets=r->sockets;*r=base;r->sockets=std::move(sockets);model_.selectedClass=r->moduleClass;model_.definitionOverridesDirty=true;model_.validation=Validate();model_.status="Definition restored to certified/inferred baseline";return true;}return false;
}

bool ShipyardBuilderSystem::LoadDefinitionOverrides(const std::string& path,std::string* error,std::size_t* appliedModules){
    std::size_t applied=0;if(!ShipyardDefinitionOverrideSystem::LoadAndApply(model_.catalog,path,error,&applied))return false;if(appliedModules)*appliedModules=applied;model_.definitionOverridesDirty=false;SyncCatalogSelectionToPlaced();model_.validation=Validate();model_.status="Loaded "+std::to_string(applied)+" definition override module(s)";return true;
}
bool ShipyardBuilderSystem::SaveDefinitionOverrides(const std::string& path,std::string* error,std::size_t* changedModules) const{return ShipyardDefinitionOverrideSystem::Save(initialCatalog_,model_.catalog,path,error,changedModules);}

bool ShipyardBuilderSystem::LoadSocketOverrides(const std::string& path,std::string* error,std::size_t* appliedModules){
    std::size_t applied=0;if(!ShipyardSocketOverrideSystem::LoadAndApply(model_.catalog,path,error,&applied))return false;
    if(appliedModules)*appliedModules=applied;socketHistory_.clear();socketHistoryCursor_=0;model_.socketOverridesDirty=false;SyncSocketSelection();ReflowRecipeAttachments();model_.validation=Validate();
    model_.status="Loaded "+std::to_string(applied)+" socket override module(s)";return true;
}

bool ShipyardBuilderSystem::SaveSocketOverrides(const std::string& path,std::string* error,std::size_t* changedModules) const {
    return ShipyardSocketOverrideSystem::Save(initialCatalog_,model_.catalog,path,error,changedModules);
}

bool ShipyardBuilderSystem::IsSocketOccupied(std::size_t parentIndex,const ShipyardAssemblySocket& socket) const {
    if(parentIndex>=model_.recipe.modules.size())return false;
    const auto& parent=model_.recipe.modules[parentIndex];
    const float px=SocketWorldX(parent,socket),py=SocketWorldY(parent,socket),pz=SocketWorldZ(parent,socket);
    for(std::size_t i=0;i<model_.recipe.modules.size();++i){
        if(i==parentIndex)continue;
        const auto* otherRecord=FindRecord(model_.recipe.modules[i].moduleId);
        if(!otherRecord)continue;
        for(const auto& os:otherRecord->sockets){
            if(!ShipyardModuleSystem::CanMate(socket.type,os.type)&&!ShipyardModuleSystem::CanMate(os.type,socket.type))continue;
            if(Distance3(px,py,pz,SocketWorldX(model_.recipe.modules[i],os),SocketWorldY(model_.recipe.modules[i],os),SocketWorldZ(model_.recipe.modules[i],os))<=kSocketCoincidenceEpsilon)
                return true;
        }
    }
    return false;
}

bool ShipyardBuilderSystem::TryAttach(const ShipyardModuleRecord& child,
                                      VisualModulePlacement& outPlacement,
                                      std::size_t& outParentIndex,
                                      std::string& outParentSocket) const {
    if(model_.recipe.modules.empty())return false;
    const std::size_t preferred=std::min(model_.selectedPlacedModule,model_.recipe.modules.size()-1);
    auto socketPreference=[&](const std::string& name){
        const auto& role=child.placementRole;
        if(role=="COMMAND_DORSAL"||role=="SURFACE_HARDPOINT"||role=="SURFACE_SENSOR"||role=="SURFACE_DETAIL")return name=="dorsal"?140:(name=="ventral"?65:0);
        if(role=="AFT_DRIVE"||role=="ENGINE_HOUSING")return ShipyardModuleSystem::IsRearDriveSocketName(name)?170:(name=="aft"?50:0);
        if(role=="LATERAL_STRUCTURE"||role=="LATERAL_ATTACHMENT"||role=="CARGO_UTILITY")return (name=="port"||name=="starboard")?145:0;
        if(role=="SPINE"||role=="ADAPTER")return (name=="forward"||name=="aft")?150:0;
        return 20;
    };
    bool found=false;int bestScore=-100000;VisualModulePlacement bestPlacement;std::size_t bestParent=0;std::string bestSocket;
    for(std::size_t parentIndex=0;parentIndex<model_.recipe.modules.size();++parentIndex){
        const auto& parentPlacement=model_.recipe.modules[parentIndex];
        const auto* parentRecord=FindRecord(parentPlacement.moduleId);if(!parentRecord)continue;
        for(const auto& ps:parentRecord->sockets){
            if(IsSocketOccupied(parentIndex,ps))continue;
            for(const auto& cs:child.sockets){
                if(!ShipyardModuleSystem::CanMate(ps.type,cs.type))continue;
                const bool mainDrive=child.semantic==ShipyardModuleSemantic::MainEngine||child.semantic==ShipyardModuleSemantic::EngineNozzle;
                const bool housing=child.semantic==ShipyardModuleSemantic::EngineHousing;
                if(housing && !ShipyardModuleSystem::IsRearDriveSocketName(ps.name)) continue;
                if(mainDrive && parentRecord->semantic!=ShipyardModuleSemantic::EngineHousing &&
                   !ShipyardModuleSystem::IsRearDriveSocketName(ps.name)) continue;
                if(mainDrive && parentRecord->semantic==ShipyardModuleSemantic::EngineHousing && ps.name!="engine_cavity") continue;
                int score=socketPreference(ps.name);
                if(parentIndex==preferred)score+=90;
                if(ShipyardModuleSystem::SizeCompatible(parentRecord->size,child.size))score+=25;else score-=35;
                if(ShipyardModuleSystem::IsRearDriveSocketName(ps.name))score+=child.moduleClass==ShipyardModuleClass::Propulsion?45:0;
                if(ps.name=="dorsal")score+=(child.surfaceOnly||child.moduleClass==ShipyardModuleClass::Command)?25:0;
                if(score<=bestScore)continue;
                const float uniformScale=TargetUniformScale(child,model_.targetModuleSize);
                bestPlacement=ShipyardModuleSystem::BuildAttachmentPlacement(parentPlacement,ps,child,cs,uniformScale);
                bestParent=parentIndex;bestSocket=ps.name;bestScore=score;found=true;
            }
        }
    }
    if(!found)return false;
    outPlacement=bestPlacement;outParentIndex=bestParent;outParentSocket=bestSocket;return true;
}

bool ShipyardBuilderSystem::AddSelectedModule(){
    const auto* child=SelectedCatalogModule();if(!child)return false;
    if(model_.recipe.modules.empty()){
        VisualModulePlacement p;p.moduleId=child->source.moduleId;p.scaleX=p.scaleY=p.scaleZ=TargetUniformScale(*child,model_.targetModuleSize);
        p.material=child->moduleClass==ShipyardModuleClass::Command?SpaceMaterialKind::Canopy:SpaceMaterialKind::IndustrialHull;
        model_.recipe.modules.push_back(p);model_.selectedPlacedModule=0;RefreshForwardAuthority();InvalidateRecipeMetadata();
        model_.status="Placed root module "+CompactId(child->source.moduleId);return true;
    }
    VisualModulePlacement p;std::size_t parentIndex=0;std::string parentSocket;
    if(!TryAttach(*child,p,parentIndex,parentSocket)){
        model_.status="No compatible open socket for "+CompactId(child->source.moduleId);return false;
    }
    model_.recipe.modules.push_back(p);model_.selectedPlacedModule=model_.recipe.modules.size()-1;
    model_.recipe.attachments.push_back({parentIndex,model_.selectedPlacedModule,parentSocket,"mount",0.0f,true});

    // Pass655-674: live symmetry is an exact reflected partner, not a second
    // independent socket solve. This preserves asymmetric pipe/antenna/wing
    // handedness. If a mirrored parent/socket exists we also preserve a
    // certified attachment; otherwise the reflected module remains reviewable.
    if(model_.symmetryFrame.live){
        const auto mirrored=ConstructionSymmetrySystem::ReflectPlacement(p,model_.symmetryFrame);
        const Vector3 delta{mirrored.x-p.x,mirrored.y-p.y,mirrored.z-p.z};
        if(delta.length()>.05f){
            const std::size_t sourceIndex=model_.selectedPlacedModule;
            const std::size_t mirrorIndex=model_.recipe.modules.size();
            model_.recipe.modules.push_back(mirrored);
            bool attached=false;std::size_t mirrorParent=parentIndex;
            for(const auto& pair:model_.symmetryPairs)if(pair.linked&&pair.axis==model_.symmetryFrame.axis){if(pair.first==parentIndex){mirrorParent=pair.second;attached=true;break;}if(pair.second==parentIndex){mirrorParent=pair.first;attached=true;break;}}
            if(!attached&&parentIndex<model_.recipe.modules.size()){
                const auto rp=ConstructionSymmetrySystem::ReflectPlacement(model_.recipe.modules[parentIndex],model_.symmetryFrame);
                const auto& pp=model_.recipe.modules[parentIndex];attached=Vector3{rp.x-pp.x,rp.y-pp.y,rp.z-pp.z}.length()<.04f;
            }
            if(attached){
                model_.recipe.attachments.push_back({mirrorParent,mirrorIndex,ConstructionSymmetrySystem::ReflectSocketName(parentSocket,model_.symmetryFrame.axis),"mount",0.0f,true});
            }
            model_.symmetryPairs.push_back({sourceIndex,mirrorIndex,model_.symmetryFrame.axis,true});
        }
    }
    RefreshForwardAuthority();InvalidateRecipeMetadata();
    model_.status="Attached "+CompactId(child->source.moduleId)+" at "+parentSocket;
    return true;
}

bool ShipyardBuilderSystem::ReplaceSelectedModule(){
    if(model_.recipe.modules.empty())return false;
    const auto* replacement=SelectedCatalogModule();if(!replacement)return false;
    const std::size_t index=std::min(model_.selectedPlacedModule,model_.recipe.modules.size()-1);
    const auto* existing=FindRecord(model_.recipe.modules[index].moduleId);
    if(existing&&existing->moduleClass!=replacement->moduleClass){
        model_.status="Replace requires the same module class to preserve the socket graph";return false;
    }
    auto& p=model_.recipe.modules[index];p.moduleId=replacement->source.moduleId;
    RefreshForwardAuthority();InvalidateRecipeMetadata();model_.status="Replaced selected module with "+CompactId(replacement->source.moduleId);return true;
}

bool ShipyardBuilderSystem::RemoveSelectedModule(){
    if(model_.recipe.modules.empty())return false;
    const std::size_t index=std::min(model_.selectedPlacedModule,model_.recipe.modules.size()-1);
    const std::string id=model_.recipe.modules[index].moduleId;

    std::vector<bool> remove(model_.recipe.modules.size(),false);remove[index]=true;
    bool changed=true;
    while(changed){changed=false;for(const auto& e:model_.recipe.attachments)if(e.parentModuleIndex<remove.size()&&e.childModuleIndex<remove.size()&&remove[e.parentModuleIndex]&&!remove[e.childModuleIndex]){remove[e.childModuleIndex]=true;changed=true;}}
    std::vector<std::size_t> remap(model_.recipe.modules.size(),static_cast<std::size_t>(-1));
    std::vector<VisualModulePlacement> kept;kept.reserve(model_.recipe.modules.size());
    for(std::size_t i=0;i<model_.recipe.modules.size();++i)if(!remove[i]){remap[i]=kept.size();kept.push_back(model_.recipe.modules[i]);}
    std::vector<ShipVisualAttachment> edges;edges.reserve(model_.recipe.attachments.size());
    for(auto e:model_.recipe.attachments){
        if(e.parentModuleIndex>=remove.size()||e.childModuleIndex>=remove.size()||remove[e.parentModuleIndex]||remove[e.childModuleIndex])continue;
        e.parentModuleIndex=remap[e.parentModuleIndex];e.childModuleIndex=remap[e.childModuleIndex];edges.push_back(std::move(e));
    }
    model_.recipe.modules=std::move(kept);model_.recipe.attachments=std::move(edges);
    RefreshForwardAuthority();InvalidateRecipeMetadata();NormalizeSelections();model_.status="Removed "+CompactId(id)+" and attached descendants";return true;
}

bool ShipyardBuilderSystem::GenerateVariant(){
    ++model_.seed;
    const auto generated=ShipyardModuleSystem::BuildShowcaseRecipes(model_.catalog,model_.seed);
    std::vector<ProceduralShipVisualRecipe> candidates;
    for(const auto&r:generated)if(r.role==model_.role)candidates.push_back(r);
    if(candidates.empty()){model_.status="Generator could not resolve this role from the certified catalog";return false;}

    const auto dna=FactionShipDesignSystem::DefaultFactionDna(model_.factionId);
    const auto families=FactionShipDesignSystem::BuildClassFamilies(model_.factionId,model_.shipClass,dna);
    const std::size_t familyIndex=families.empty()?0:std::min<std::size_t>(static_cast<std::size_t>(std::max(0,model_.hullFamilyIndex)),families.size()-1);
    const auto role=RoleFromGeneratorName(model_.role);
    const FactionHullFamilyDefinition* family=families.empty()?nullptr:&families[familyIndex];

    ProceduralShipVisualRecipe* selected=nullptr;
    for(auto& candidate:candidates){
        ShipPcgRuntimeClosureSystem::RepairCandidate(model_.catalog,candidate,5);
        const auto report=ShipPcgRuntimeClosureSystem::EvaluateCandidate(model_.catalog,candidate,false,true);
        candidate.runtimePcgCertified=report.accepted;
        candidate.runtimeCertificationMessage=report.accepted?"PCG_RUNTIME_CERTIFIED":(report.messages.empty()?"PCG_RUNTIME_REVIEW":report.messages.front());
        if(family){
            const auto runtime=ShipPcgRuntimeClosureSystem::BuildHullFamilyProfile(*family);
            ShipPcgRuntimeClosureSystem::ApplyLineage(candidate,runtime,role);
            // A rejected generated candidate keeps its faction/class/hull labels
            // for authoring/debugging but is not promoted to strict lineage.
            // This preserves older/manual recipes as reviewable drafts while
            // runtime selection prefers only fully certified lineage variants.
            if(!report.accepted)candidate.lineageAuthority="FACTION_CLASS_HULL_ROLE_REVIEW";
            candidate.appearancePresetId=model_.factionId+"_SHIP_DEFAULT";
        }
        if(report.accepted&&!selected)selected=&candidate;
    }
    if(!selected)selected=&candidates[model_.seed%candidates.size()];
    model_.recipe=*selected;model_.selectedPlacedModule=0;model_.dirty=true;RefreshForwardAuthority();
    model_.status=model_.recipe.runtimePcgCertified
        ? "Generated spatially certified "+model_.role+" variant seed "+std::to_string(model_.seed)
        : "Generated REVIEW variant; inspect spatial/propulsion validation before blueprint certification";
    NormalizeSelections();return true;
}

ShipyardBuilderValidation ShipyardBuilderSystem::Validate() const {
    ShipyardBuilderValidation v;
    if(model_.recipe.modules.empty()){v.errors.push_back("No ship modules placed");return v;}
    bool hull=false,command=false,propulsion=false;std::vector<std::size_t> structural;
    for(std::size_t i=0;i<model_.recipe.modules.size();++i){
        const auto& p=model_.recipe.modules[i];const auto* r=FindRecord(p.moduleId);
        if(!r){v.errors.push_back("Unknown or uncertified module: "+p.moduleId);continue;}
        hull|=r->moduleClass==ShipyardModuleClass::Hull;command|=r->moduleClass==ShipyardModuleClass::Command;propulsion|=r->moduleClass==ShipyardModuleClass::Propulsion;
        if(IsStructuralClass(r->moduleClass))structural.push_back(i);
        if(r->moduleClass!=ShipyardModuleClass::Detail&&
           (std::fabs(p.scaleX-p.scaleY)>.0001f||std::fabs(p.scaleY-p.scaleZ)>.0001f))
            v.errors.push_back("Non-uniform structural scale: "+p.moduleId);
        const auto orientation=ShipyardOrientationConstraintSystem::Validate(*r,p);
        if(!orientation.valid)v.errors.push_back(orientation.warning+": "+CompactId(p.moduleId));
    }
    if(!hull)v.errors.push_back("Missing structural hull module");
    if(!command)v.errors.push_back("Missing command/cockpit module");
    if(!propulsion)v.errors.push_back("Missing authored propulsion module");

    // Wings are not generic surface decorations. Their certified root socket
    // must mate a lateral parent socket; otherwise a visually upright or
    // tip-attached wing can still have a formally connected graph.
    for(const auto& edge:model_.recipe.attachments){
        if(edge.childModuleIndex>=model_.recipe.modules.size())continue;
        const auto* child=FindRecord(model_.recipe.modules[edge.childModuleIndex].moduleId);
        if(!child||child->semantic!=ShipyardModuleSemantic::Wing)continue;
        const bool lateralParent=edge.parentSocket.find("port")!=std::string::npos ||
                                 edge.parentSocket.find("starboard")!=std::string::npos;
        if(!lateralParent)v.errors.push_back("Wing root must attach to a port/starboard hull socket: "+CompactId(child->source.moduleId));
        if(edge.childSocket!="mount")v.errors.push_back("Wing attachment must use the certified root face: "+CompactId(child->source.moduleId));
    }

    if(structural.size()>1){
        std::vector<std::vector<std::size_t>> edges(model_.recipe.modules.size());
        for(std::size_t ai=0;ai<structural.size();++ai)for(std::size_t bi=ai+1;bi<structural.size();++bi){
            const std::size_t a=structural[ai],b=structural[bi];const auto* ar=FindRecord(model_.recipe.modules[a].moduleId);const auto* br=FindRecord(model_.recipe.modules[b].moduleId);if(!ar||!br)continue;
            bool connected=false;
            for(const auto&as:ar->sockets)for(const auto&bs:br->sockets){
                if(!ShipyardModuleSystem::CanMate(as.type,bs.type)&&!ShipyardModuleSystem::CanMate(bs.type,as.type))continue;
                const float d=Distance3(SocketWorldX(model_.recipe.modules[a],as),SocketWorldY(model_.recipe.modules[a],as),SocketWorldZ(model_.recipe.modules[a],as),SocketWorldX(model_.recipe.modules[b],bs),SocketWorldY(model_.recipe.modules[b],bs),SocketWorldZ(model_.recipe.modules[b],bs));
                if(d<=kSocketCoincidenceEpsilon){connected=true;break;}
            }
            if(connected){edges[a].push_back(b);edges[b].push_back(a);}
        }
        const std::size_t root=structural.front();std::unordered_set<std::size_t> visited;std::queue<std::size_t> q;q.push(root);visited.insert(root);
        while(!q.empty()){const auto n=q.front();q.pop();for(auto e:edges[n])if(visited.insert(e).second)q.push(e);}
        for(const auto i:structural)if(!visited.count(i)){
            const auto* r=FindRecord(model_.recipe.modules[i].moduleId);
            v.errors.push_back("Disconnected structural module: "+std::string(r?CompactId(r->source.moduleId):model_.recipe.modules[i].moduleId));
        }
    }
    const auto canonical=ShipyardModuleSystem::ValidateAssemblyGraph(model_.catalog,model_.recipe);
    for(const auto& e:canonical.errors)if(std::find(v.errors.begin(),v.errors.end(),e)==v.errors.end())v.errors.push_back(e);
    for(const auto& w:canonical.warnings)if(std::find(v.warnings.begin(),v.warnings.end(),w)==v.warnings.end())v.warnings.push_back(w);
    const auto runtime=ShipPcgRuntimeClosureSystem::EvaluateCandidate(model_.catalog,model_.recipe,false,true);
    if(!runtime.accepted){
        // Pass615 normalization preserves existing/manual blueprints as editable
        // drafts while making newly generated faction/class/hull recipes fail
        // closed through the runtime-PCG authority. This prevents the stricter
        // whole-ship spatial rules from invalidating historical player designs.
        const bool strictPcg=model_.recipe.lineageAuthority=="FACTION_CLASS_HULL_ROLE_V1";
        auto& target=strictPcg?v.errors:v.warnings;
        for(const auto& e:runtime.messages)if(std::find(target.begin(),target.end(),e)==target.end())target.push_back(e);
    }
    const auto core=ShipFunctionalCoreSystem::Validate(model_.catalog,model_.recipe,true);
    for(const auto& e:core.messages)if(std::find(v.warnings.begin(),v.warnings.end(),e)==v.warnings.end())v.warnings.push_back(e);
    if(model_.recipe.modules.size()<5)v.warnings.push_back("Very small design; role systems may have little attachment capacity");
    v.valid=v.errors.empty();return v;
}

bool ShipyardBuilderSystem::Activate(ShipyardBuilderCommand command,int value){
    if(!model_.initialized&&command!=ShipyardBuilderCommand::Reset)return false;
    bool changed=false;
    switch(command){
        case ShipyardBuilderCommand::WorkspaceBuild:model_.workspaceMode=ShipyardWorkspaceMode::Build;model_.inspectorTab=ShipyardInspectorTab::Transform;CancelSocketTransform();model_.status="Build workspace - drag parts into the viewport or refine the selected module";return true;
        case ShipyardBuilderCommand::WorkspaceModel:if(!model_.capabilities.model){model_.status="MODEL is available in Shipyard Dev Studio / Runtime Dev Mode";return false;}model_.workspaceMode=ShipyardWorkspaceMode::Model;CancelTransform();CancelSocketTransform();model_.status="Model workspace - Add Shape, stretch, modifiers, and CanonicalAsset publishing";return true;
        case ShipyardBuilderCommand::WorkspaceInterior:if(!model_.capabilities.interior){model_.status="INTERIOR is available in Shipyard Dev Studio / Runtime Dev Mode";return false;}model_.workspaceMode=ShipyardWorkspaceMode::Interior;CancelTransform();CancelSocketTransform();model_.interiorPlan=ShipModuleInteriorLinkSystem::BuildPlan(model_.catalog,model_.recipe,model_.worldScale);model_.status=model_.interiorPlan.valid?"Interior workspace - exterior modules mapped to walkable/service interior bindings":"Interior workspace - connection plan requires review";return true;
        case ShipyardBuilderCommand::WorkspaceAppearance:model_.workspaceMode=ShipyardWorkspaceMode::Appearance;model_.inspectorTab=ShipyardInspectorTab::Appearance;CancelSocketTransform();model_.status="Appearance workspace - paint, livery, patterns, and decals";return true;
        case ShipyardBuilderCommand::WorkspaceSystems:model_.workspaceMode=ShipyardWorkspaceMode::Systems;model_.inspectorTab=ShipyardInspectorTab::Assembly;CancelSocketTransform();model_.status="Systems workspace - module function, integrity, power, heat, and fitting authority";return true;
        case ShipyardBuilderCommand::WorkspaceCharacter:if(!model_.capabilities.character){model_.status="CHARACTER is available in Shipyard Dev Studio / Runtime Dev Mode";return false;}model_.workspaceMode=ShipyardWorkspaceMode::Character;CancelTransform();CancelSocketTransform();model_.status="Character workspace - player-scale calibration and governed animation preview";return true;
        case ShipyardBuilderCommand::WorkspacePcg:if(!model_.capabilities.pcgStudio){model_.status="PCG Studio is available in Shipyard Dev Studio / Runtime Dev Mode";return false;}model_.workspaceMode=ShipyardWorkspaceMode::Pcg;CancelTransform();CancelSocketTransform();model_.status="PCG Studio - generate, reroll, explain, overlay, and teach from authored assemblies";return true;
        case ShipyardBuilderCommand::WorkspaceWorld:if(!model_.capabilities.world){model_.status="WORLD is available in Shipyard Dev Studio / Runtime Dev Mode";return false;}model_.workspaceMode=ShipyardWorkspaceMode::World;CancelTransform();CancelSocketTransform();model_.status="World workspace - deterministic terrain authoring and terraform deltas";return true;
        case ShipyardBuilderCommand::WorkspaceDevWorld:if(!model_.capabilities.devWorld){model_.status="DEV WORLD is available in Shipyard Dev Studio / Runtime Dev Mode";return false;}model_.workspaceMode=ShipyardWorkspaceMode::DevWorld;CancelTransform();CancelSocketTransform();model_.devWorld.enabled=true;model_.status=std::string("Dev World - ")+ShipyardDevWorldSystem::BackdropName(model_.devWorld.backdrop)+" backdrop / player-scale proving ground";return true;
        case ShipyardBuilderCommand::WorkspaceAuthoring:if(!model_.capabilities.rawAuthoring){model_.status="Raw AUTHORING is restricted to Dev Studio / Runtime Dev Mode";return false;}model_.workspaceMode=ShipyardWorkspaceMode::Authoring;model_.inspectorTab=ShipyardInspectorTab::Authoring;CancelTransform();CancelSocketTransform();model_.status="Advanced authoring - sockets, Teach PCG, surfaces, DesignDNA, and certification";return true;
        case ShipyardBuilderCommand::InspectorTransform:model_.workspaceMode=ShipyardWorkspaceMode::Build;model_.inspectorTab=ShipyardInspectorTab::Transform;model_.status="Transform tools";return true;
        case ShipyardBuilderCommand::InspectorAssembly:model_.workspaceMode=ShipyardWorkspaceMode::Systems;model_.inspectorTab=ShipyardInspectorTab::Assembly;CancelSocketTransform();model_.status="Systems and blueprint summary";return true;
        case ShipyardBuilderCommand::InspectorSockets:if(!model_.capabilities.sockets){model_.status="Socket authoring is restricted to Dev Studio / Runtime Dev Mode";return false;}model_.workspaceMode=ShipyardWorkspaceMode::Authoring;model_.inspectorTab=ShipyardInspectorTab::Sockets;CancelTransform();SyncSocketSelection();model_.transformTool=ShipyardTransformTool::Move;model_.status="Authoring / SOCKETS / CONNECTIONS - edit reusable mating points";return true;
        case ShipyardBuilderCommand::InspectorAuthoring:if(!model_.capabilities.rawAuthoring){model_.status="Raw authoring is restricted to Dev Studio / Runtime Dev Mode";return false;}model_.workspaceMode=ShipyardWorkspaceMode::Authoring;model_.inspectorTab=ShipyardInspectorTab::Authoring;CancelTransform();CancelSocketTransform();model_.status="Teach PCG definition authoring - reusable definition metadata";return true;
        case ShipyardBuilderCommand::InspectorAppearance:model_.workspaceMode=ShipyardWorkspaceMode::Appearance;model_.inspectorTab=ShipyardInspectorTab::Appearance;CancelSocketTransform();model_.status="Appearance and decals";return true;
        case ShipyardBuilderCommand::SelectClass:
            if(value>=0&&value<=static_cast<int>(ShipyardModuleClass::Component)){model_.selectedClass=static_cast<ShipyardModuleClass>(value);model_.selectedFilteredModule=0;model_.catalogScrollStart=0;changed=true;}break;
        case ShipyardBuilderCommand::SelectModule:{const auto filtered=FilteredCatalogIndices();if(value>=0&&static_cast<std::size_t>(value)<filtered.size()){model_.selectedFilteredModule=static_cast<std::size_t>(value);changed=true;}}break;
        case ShipyardBuilderCommand::SelectPlaced:if(value>=0&&static_cast<std::size_t>(value)<model_.recipe.modules.size()){model_.selectedPlacedModule=static_cast<std::size_t>(value);SyncCatalogSelectionToPlaced();changed=true;}break;
        case ShipyardBuilderCommand::PreviousModule:{const auto f=FilteredCatalogIndices();if(!f.empty()){model_.selectedFilteredModule=(model_.selectedFilteredModule+f.size()-1)%f.size();changed=true;}}break;
        case ShipyardBuilderCommand::NextModule:{const auto f=FilteredCatalogIndices();if(!f.empty()){model_.selectedFilteredModule=(model_.selectedFilteredModule+1)%f.size();changed=true;}}break;
        case ShipyardBuilderCommand::PreviousPlaced:if(!model_.recipe.modules.empty()){model_.selectedPlacedModule=(model_.selectedPlacedModule+model_.recipe.modules.size()-1)%model_.recipe.modules.size();SyncCatalogSelectionToPlaced();changed=true;}break;
        case ShipyardBuilderCommand::NextPlaced:if(!model_.recipe.modules.empty()){model_.selectedPlacedModule=(model_.selectedPlacedModule+1)%model_.recipe.modules.size();SyncCatalogSelectionToPlaced();changed=true;}break;
        case ShipyardBuilderCommand::AddModule:changed=AddSelectedModule();break;
        case ShipyardBuilderCommand::ReplaceModule:changed=ReplaceSelectedModule();break;
        case ShipyardBuilderCommand::RemoveModule:changed=RemoveSelectedModule();break;
        case ShipyardBuilderCommand::PreviousSocket:{const auto* r=SelectedPlacedRecord();if(r&&!r->sockets.empty()){model_.selectedSocket=(model_.selectedSocket+r->sockets.size()-1)%r->sockets.size();model_.status="Selected socket "+r->sockets[model_.selectedSocket].name;return true;}}return false;
        case ShipyardBuilderCommand::NextSocket:{const auto* r=SelectedPlacedRecord();if(r&&!r->sockets.empty()){model_.selectedSocket=(model_.selectedSocket+1)%r->sockets.size();model_.status="Selected socket "+r->sockets[model_.selectedSocket].name;return true;}}return false;
        case ShipyardBuilderCommand::AddSocket:return AddSocket();
        case ShipyardBuilderCommand::RemoveSocket:return RemoveSocket();
        case ShipyardBuilderCommand::MirrorSocketX:return MirrorSelectedSocketX();
        case ShipyardBuilderCommand::CycleSocketType:return CycleSelectedSocketType();
        case ShipyardBuilderCommand::ResetSocket:return ResetSelectedSocket();
        case ShipyardBuilderCommand::UndoSocketEdit:return UndoSocketEdit();
        case ShipyardBuilderCommand::RedoSocketEdit:return RedoSocketEdit();
        case ShipyardBuilderCommand::SaveSocketOverrides:socketOverridesSaveRequested_=true;model_.status="Socket override save requested";return true;
        case ShipyardBuilderCommand::PreviousSemantic:return CycleSelectedSemantic(-1);
        case ShipyardBuilderCommand::NextSemantic:return CycleSelectedSemantic(1);
        case ShipyardBuilderCommand::ToggleGeneratorEligible:return ToggleSelectedGeneratorEligibility();
        case ShipyardBuilderCommand::TogglePairedPlacement:return ToggleSelectedPairedPlacement();
        case ShipyardBuilderCommand::CyclePreferredMountFace:return CycleSelectedPreferredMountFace();
        case ShipyardBuilderCommand::ResetDefinitionOverride:return ResetSelectedDefinitionOverride();
        case ShipyardBuilderCommand::SaveDefinitionOverrides:definitionOverridesSaveRequested_=true;model_.status="Definition override save requested";return true;
        case ShipyardBuilderCommand::ToggleMirrorX:
        case ShipyardBuilderCommand::ToggleLiveSymmetry:model_.symmetryFrame.live=!model_.symmetryFrame.live;model_.mirrorX=model_.symmetryFrame.live&&model_.symmetryFrame.axis==ConstructionSymmetryAxis::PortStarboard;model_.status=std::string("LIVE SYMMETRY ")+(model_.symmetryFrame.live?"ON":"OFF");changed=true;break;
        case ShipyardBuilderCommand::SymmetryAxisPortStarboard:model_.symmetryFrame.axis=ConstructionSymmetryAxis::PortStarboard;model_.mirrorX=model_.symmetryFrame.live;model_.status="Symmetry plane: PORT <-> STARBOARD";return true;
        case ShipyardBuilderCommand::SymmetryAxisForeAft:model_.symmetryFrame.axis=ConstructionSymmetryAxis::ForeAft;model_.mirrorX=false;model_.status="Symmetry plane: FORE <-> AFT";return true;
        case ShipyardBuilderCommand::SymmetryAxisDorsalVentral:model_.symmetryFrame.axis=ConstructionSymmetryAxis::DorsalVentral;model_.mirrorX=false;model_.status="Symmetry plane: DORSAL <-> VENTRAL";return true;
        case ShipyardBuilderCommand::SymmetryPlaneNegative:
        case ShipyardBuilderCommand::SymmetryPlanePositive:{
            const float d=command==ShipyardBuilderCommand::SymmetryPlanePositive?.25f:-.25f;
            if(model_.symmetryFrame.axis==ConstructionSymmetryAxis::PortStarboard)model_.symmetryFrame.origin.x+=d;
            else if(model_.symmetryFrame.axis==ConstructionSymmetryAxis::ForeAft)model_.symmetryFrame.origin.y+=d;
            else model_.symmetryFrame.origin.z+=d;
            for(const auto& pair:model_.symmetryPairs)if(pair.linked)SyncSymmetryPartner(pair.first);
            model_.dirty=true;model_.validation=Validate();model_.status=std::string("Moved symmetry plane ")+(d>0?"+":"-")+"0.25";return true;}
        case ShipyardBuilderCommand::ResetSymmetryFrame:{
            Vector3 c{};if(!model_.recipe.modules.empty()){for(const auto&m:model_.recipe.modules){c.x+=m.x;c.y+=m.y;c.z+=m.z;}const float inv=1.0f/static_cast<float>(model_.recipe.modules.size());c=c*inv;}model_.symmetryFrame.origin=c;
            for(const auto& pair:model_.symmetryPairs)if(pair.linked)SyncSymmetryPartner(pair.first);
            model_.status="Symmetry center reset to assembly center";model_.dirty=true;return true;}
        case ShipyardBuilderCommand::MirrorSelectedAcrossSymmetry:return MirrorSelectedAcrossActiveSymmetry();
        case ShipyardBuilderCommand::BreakSymmetryPair:return BreakSelectedSymmetryPair();
        case ShipyardBuilderCommand::PreviousLiveryPreset:{const auto n=LiveryPresets().size();if(n){model_.liveryPreset=(model_.liveryPreset+n-1)%n;ApplyLiveryPreset();changed=true;}}break;
        case ShipyardBuilderCommand::NextLiveryPreset:{const auto n=LiveryPresets().size();if(n){model_.liveryPreset=(model_.liveryPreset+1)%n;ApplyLiveryPreset();changed=true;}}break;
        case ShipyardBuilderCommand::PreviousPrimaryPaint:model_.primaryPaintName=StepPaintLayer(model_.appearance.primary,-1);model_.liveryName="CUSTOM";model_.status="Primary paint: "+model_.primaryPaintName;model_.dirty=true;changed=true;break;
        case ShipyardBuilderCommand::NextPrimaryPaint:model_.primaryPaintName=StepPaintLayer(model_.appearance.primary,1);model_.liveryName="CUSTOM";model_.status="Primary paint: "+model_.primaryPaintName;model_.dirty=true;changed=true;break;
        case ShipyardBuilderCommand::PreviousSecondaryPaint:model_.secondaryPaintName=StepPaintLayer(model_.appearance.secondary,-1);model_.liveryName="CUSTOM";model_.status="Secondary paint: "+model_.secondaryPaintName;model_.dirty=true;changed=true;break;
        case ShipyardBuilderCommand::NextSecondaryPaint:model_.secondaryPaintName=StepPaintLayer(model_.appearance.secondary,1);model_.liveryName="CUSTOM";model_.status="Secondary paint: "+model_.secondaryPaintName;model_.dirty=true;changed=true;break;
        case ShipyardBuilderCommand::PreviousTrimPaint:model_.trimPaintName=StepPaintLayer(model_.appearance.trim,-1);model_.liveryName="CUSTOM";model_.status="Accent paint: "+model_.trimPaintName;model_.dirty=true;changed=true;break;
        case ShipyardBuilderCommand::NextTrimPaint:model_.trimPaintName=StepPaintLayer(model_.appearance.trim,1);model_.liveryName="CUSTOM";model_.status="Accent paint: "+model_.trimPaintName;model_.dirty=true;changed=true;break;
        case ShipyardBuilderCommand::PreviousDecalPreset:{const auto n=DecalPresets().size();if(n){model_.decalPreset=(model_.decalPreset+n-1)%n;model_.status="Decal preset: "+DecalPresets()[model_.decalPreset];changed=true;}}break;
        case ShipyardBuilderCommand::NextDecalPreset:{const auto n=DecalPresets().size();if(n){model_.decalPreset=(model_.decalPreset+1)%n;model_.status="Decal preset: "+DecalPresets()[model_.decalPreset];changed=true;}}break;
        case ShipyardBuilderCommand::AddDecal:changed=AddSelectedDecal();break;
        case ShipyardBuilderCommand::RemoveDecal:changed=RemoveSelectedDecal();break;
        case ShipyardBuilderCommand::ToolSelect:if(model_.inspectorTab!=ShipyardInspectorTab::Sockets){model_.workspaceMode=ShipyardWorkspaceMode::Build;model_.inspectorTab=ShipyardInspectorTab::Transform;}model_.transformTool=ShipyardTransformTool::Select;CancelTransform();CancelSocketTransform();model_.status="Select tool";return true;
        case ShipyardBuilderCommand::ToolMove:if(model_.inspectorTab!=ShipyardInspectorTab::Sockets){model_.workspaceMode=ShipyardWorkspaceMode::Build;model_.inspectorTab=ShipyardInspectorTab::Transform;}model_.transformTool=ShipyardTransformTool::Move;CancelTransform();CancelSocketTransform();model_.status=model_.inspectorTab==ShipyardInspectorTab::Sockets?"Socket MOVE tool":"Move tool";return true;
        case ShipyardBuilderCommand::ToolRotate:if(model_.inspectorTab!=ShipyardInspectorTab::Sockets){model_.workspaceMode=ShipyardWorkspaceMode::Build;model_.inspectorTab=ShipyardInspectorTab::Transform;}model_.transformTool=ShipyardTransformTool::Rotate;CancelTransform();CancelSocketTransform();model_.status=model_.inspectorTab==ShipyardInspectorTab::Sockets?"Socket ROTATE tool":"Rotate tool";return true;
        case ShipyardBuilderCommand::ToolScale:model_.workspaceMode=ShipyardWorkspaceMode::Build;model_.inspectorTab=ShipyardInspectorTab::Transform;model_.transformTool=ShipyardTransformTool::Scale;CancelTransform();model_.status="Scale tool";return true;
        case ShipyardBuilderCommand::ModelPreviousPrimitive:
        case ShipyardBuilderCommand::ModelNextPrimitive:{if(!model_.capabilities.model)return false;constexpr int count=17;int i=static_cast<int>(model_.modeling.selectedPrimitive)+(command==ShipyardBuilderCommand::ModelNextPrimitive?1:-1);if(i<0)i=count-1;if(i>=count)i=0;model_.modeling.selectedPrimitive=static_cast<ModelingPrimitiveType>(i);model_.status=std::string("Add Shape: ")+ShipyardModelingSystem::PrimitiveName(model_.modeling.selectedPrimitive);return true;}
        case ShipyardBuilderCommand::ModelAddShape:{if(!model_.capabilities.model)return false;const auto i=ShipyardModelingSystem::AddPrimitive(model_.modeling.recipe,model_.modeling.selectedPrimitive);model_.modeling.selectedPrimitiveIndex=i;model_.dirty=true;model_.status=std::string("Added modeled ")+ShipyardModelingSystem::PrimitiveName(model_.modeling.selectedPrimitive)+" shape";return true;}
        case ShipyardBuilderCommand::ModelCycleSelectionMode:{if(!model_.capabilities.model)return false;int i=(static_cast<int>(model_.modeling.selectionMode)+1)%4;model_.modeling.selectionMode=static_cast<ModelingSelectionMode>(i);model_.modeling.recipe.selectionMode=model_.modeling.selectionMode;model_.status=std::string("Model selection: ")+ShipyardModelingSystem::SelectionModeName(model_.modeling.selectionMode);return true;}
        case ShipyardBuilderCommand::ModelStretchXNegative:case ShipyardBuilderCommand::ModelStretchXPositive:case ShipyardBuilderCommand::ModelStretchYNegative:case ShipyardBuilderCommand::ModelStretchYPositive:case ShipyardBuilderCommand::ModelStretchZNegative:case ShipyardBuilderCommand::ModelStretchZPositive:{if(!model_.capabilities.model||model_.modeling.recipe.primitives.empty())return false;Vector3 d{};const float step=model_.modeling.stretchStep;const float sign=(command==ShipyardBuilderCommand::ModelStretchXNegative||command==ShipyardBuilderCommand::ModelStretchYNegative||command==ShipyardBuilderCommand::ModelStretchZNegative)?-1.0f:1.0f;if(command==ShipyardBuilderCommand::ModelStretchXNegative||command==ShipyardBuilderCommand::ModelStretchXPositive)d.x=step*sign;else if(command==ShipyardBuilderCommand::ModelStretchYNegative||command==ShipyardBuilderCommand::ModelStretchYPositive)d.y=step*sign;else d.z=step*sign;const bool ok=ShipyardModelingSystem::StretchPrimitive(model_.modeling.recipe,std::min(model_.modeling.selectedPrimitiveIndex,model_.modeling.recipe.primitives.size()-1),d,model_.modeling.symmetricStretch);if(ok){model_.dirty=true;model_.status="Modeled shape stretched; collision marked dirty";}return ok;}
        case ShipyardBuilderCommand::ModelToggleSymmetricStretch:if(!model_.capabilities.model)return false;model_.modeling.symmetricStretch=!model_.modeling.symmetricStretch;model_.status=std::string("Symmetric stretch ")+(model_.modeling.symmetricStretch?"ON":"OFF");return true;
        case ShipyardBuilderCommand::ModelValidate:{if(!model_.capabilities.model)return false;const auto v=ShipyardModelingSystem::Validate(model_.modeling.recipe);model_.status=v.valid?"Model recipe valid; collision/surface warnings may remain":"Model recipe invalid - inspect modeling errors";return true;}
        case ShipyardBuilderCommand::ModelPublishCanonical:{if(!model_.capabilities.publishCanonicalAsset)return false;const auto v=ShipyardModelingSystem::Validate(model_.modeling.recipe);if(!v.valid){model_.status="Cannot publish modeled module: model validation failed";return false;}const auto a=ShipyardModelingSystem::BakeCanonicalAsset(model_.modeling.recipe,{});model_.status="CanonicalAsset bake ready: "+a.assetId+" (save/persistence bridge follows)";return true;}
        case ShipyardBuilderCommand::PcgReroll:if(!model_.capabilities.pcgStudio)return false;model_.pcgStudio.request.seed=model_.seed;model_.seed=ShipyardPcgStudioSystem::RerollSeed(model_.pcgStudio);model_.status=model_.pcgStudio.status;return true;
        case ShipyardBuilderCommand::PcgAudit:if(!model_.capabilities.pcgStudio)return false;model_.pcgStudio.request.factionId=model_.factionId;model_.pcgStudio.request.shipClass=model_.shipClass;model_.pcgStudio.request.hullFamilyIndex=model_.hullFamilyIndex;model_.pcgStudio.request.role=model_.role;model_.pcgStudio.request.seed=model_.seed;model_.pcgStudio.lastReport=ShipyardPcgStudioSystem::AuditCandidate(model_.catalog,model_.recipe,model_.pcgStudio.request);model_.status=model_.pcgStudio.lastReport.valid?"PCG candidate accepted by Studio audit":"PCG Studio found candidate issues - inspect overlays/reasons";return true;
        case ShipyardBuilderCommand::PcgToggleOccupancy:if(!model_.capabilities.pcgStudio)return false;model_.pcgStudio.overlays.occupancy=!model_.pcgStudio.overlays.occupancy;model_.status=std::string("PCG occupancy overlay ")+(model_.pcgStudio.overlays.occupancy?"ON":"OFF");return true;
        case ShipyardBuilderCommand::PcgToggleDetailDensity:if(!model_.capabilities.pcgStudio)return false;model_.pcgStudio.overlays.detailDensity=!model_.pcgStudio.overlays.detailDensity;model_.status=std::string("PCG detail-density overlay ")+(model_.pcgStudio.overlays.detailDensity?"ON":"OFF");return true;
        case ShipyardBuilderCommand::PcgTeachFromAssembly:if(!model_.capabilities.pcgStudio)return false;model_.pcgStudio.teachFromCurrentAssembly=!model_.pcgStudio.teachFromCurrentAssembly;model_.status=model_.pcgStudio.teachFromCurrentAssembly?"Current assembly marked as PCG exemplar candidate":"PCG exemplar candidate cleared";return true;
        case ShipyardBuilderCommand::WorldPreviousBrush:case ShipyardBuilderCommand::WorldNextBrush:{if(!model_.capabilities.world)return false;constexpr int count=13;int i=static_cast<int>(model_.worldAuthoring.brush)+(command==ShipyardBuilderCommand::WorldNextBrush?1:-1);if(i<0)i=count-1;if(i>=count)i=0;model_.worldAuthoring.brush=static_cast<WorldBrushMode>(i);model_.status=std::string("World brush: ")+PlanetWorldEngineSystem::BrushName(model_.worldAuthoring.brush);return true;}
        case ShipyardBuilderCommand::WorldRadiusDown:if(!model_.capabilities.world)return false;model_.worldAuthoring.brushRadiusMeters=std::max(1.0f,model_.worldAuthoring.brushRadiusMeters*.8f);model_.status="World brush radius "+std::to_string(int(model_.worldAuthoring.brushRadiusMeters))+"m";return true;
        case ShipyardBuilderCommand::WorldRadiusUp:if(!model_.capabilities.world)return false;model_.worldAuthoring.brushRadiusMeters=std::min(100000.0f,model_.worldAuthoring.brushRadiusMeters*1.25f);model_.status="World brush radius "+std::to_string(int(model_.worldAuthoring.brushRadiusMeters))+"m";return true;
        case ShipyardBuilderCommand::WorldStrengthDown:if(!model_.capabilities.world)return false;model_.worldAuthoring.brushStrengthMeters=std::max(.05f,model_.worldAuthoring.brushStrengthMeters*.8f);model_.status="Terraform strength "+std::to_string(model_.worldAuthoring.brushStrengthMeters)+"m";return true;
        case ShipyardBuilderCommand::WorldStrengthUp:if(!model_.capabilities.world)return false;model_.worldAuthoring.brushStrengthMeters=std::min(10000.0f,model_.worldAuthoring.brushStrengthMeters*1.25f);model_.status="Terraform strength "+std::to_string(model_.worldAuthoring.brushStrengthMeters)+"m";return true;
        case ShipyardBuilderCommand::WorldApplyBrush:{if(!model_.capabilities.world)return false;TerraformOperation o;o.center={0,0,0};o.radiusMeters=model_.worldAuthoring.brushRadiusMeters;o.strengthMeters=model_.worldAuthoring.brushStrengthMeters;o.ownerFactionId=model_.factionId;o.gameplayAuthorized=false;switch(model_.worldAuthoring.brush){case WorldBrushMode::Lower:o.type=TerraformOperationType::Lower;break;case WorldBrushMode::Flatten:o.type=TerraformOperationType::Flatten;o.targetHeightMeters=PlanetWorldEngineSystem::Sample(model_.worldAuthoring.world,model_.worldAuthoring.terraform,0,0).finalHeightMeters;break;case WorldBrushMode::Terrace:o.type=TerraformOperationType::Terrace;break;case WorldBrushMode::Canal:o.type=TerraformOperationType::Canal;break;case WorldBrushMode::PaintSurface:o.type=TerraformOperationType::MaterialPaint;o.materialId="AuthoredSurface";break;case WorldBrushMode::PaintBiome:o.type=TerraformOperationType::BiomePaint;o.materialId="AuthoredBiome";break;case WorldBrushMode::PaintResource:o.type=TerraformOperationType::ResourceOverride;o.materialId="AuthoredResource";break;default:o.type=TerraformOperationType::Raise;break;}const bool ok=PlanetWorldEngineSystem::ApplyOperation(model_.worldAuthoring.terraform,o);if(ok){model_.dirty=true;model_.status="Terraform delta authored at world cursor; affected chunks must rebuild";}return ok;}
        case ShipyardBuilderCommand::WorldToggleHydrology:if(!model_.capabilities.world)return false;model_.worldAuthoring.showHydrology=!model_.worldAuthoring.showHydrology;model_.status=std::string("Hydrology overlay ")+(model_.worldAuthoring.showHydrology?"ON":"OFF");return true;
        case ShipyardBuilderCommand::WorldToggleResources:if(!model_.capabilities.world)return false;model_.worldAuthoring.showResources=!model_.worldAuthoring.showResources;model_.status=std::string("Resource geology overlay ")+(model_.worldAuthoring.showResources?"ON":"OFF");return true;
        case ShipyardBuilderCommand::DevWorldPreviousBackdrop:ShipyardDevWorldSystem::CycleBackdrop(model_.devWorld,-1);model_.status=std::string("Dev World backdrop: ")+ShipyardDevWorldSystem::BackdropName(model_.devWorld.backdrop);return true;
        case ShipyardBuilderCommand::DevWorldNextBackdrop:ShipyardDevWorldSystem::CycleBackdrop(model_.devWorld,1);model_.status=std::string("Dev World backdrop: ")+ShipyardDevWorldSystem::BackdropName(model_.devWorld.backdrop);return true;
        case ShipyardBuilderCommand::DevWorldToggleCertifiedOnly:model_.devWorld.showCertifiedOnly=!model_.devWorld.showCertifiedOnly;model_.status=std::string("Dev World certified-only catalog ")+(model_.devWorld.showCertifiedOnly?"ON":"OFF");return true;
        case ShipyardBuilderCommand::DevWorldTogglePlayerReference:model_.devWorld.showPlayerReference=!model_.devWorld.showPlayerReference;model_.status=std::string("Player scale reference ")+(model_.devWorld.showPlayerReference?"ON":"OFF");return true;
        case ShipyardBuilderCommand::ScalePlayerDown:model_.worldScale=WorldScaleAuthoritySystem::WithPlayerHeight(std::max(1.20f,model_.worldScale.referencePlayerHeightMeters-.05f));model_.devWorld=ShipyardDevWorldSystem::CreateDefault(model_.worldScale);model_.devWorld.enabled=true;model_.status="Reference player height: "+std::to_string(model_.worldScale.referencePlayerHeightMeters)+" m";return true;
        case ShipyardBuilderCommand::ScalePlayerUp:model_.worldScale=WorldScaleAuthoritySystem::WithPlayerHeight(std::min(2.40f,model_.worldScale.referencePlayerHeightMeters+.05f));model_.devWorld=ShipyardDevWorldSystem::CreateDefault(model_.worldScale);model_.devWorld.enabled=true;model_.status="Reference player height: "+std::to_string(model_.worldScale.referencePlayerHeightMeters)+" m";return true;
        case ShipyardBuilderCommand::ToggleTransformSpace:
            if(model_.transformSpace==ShipyardTransformSpace::View)model_.transformSpace=ShipyardTransformSpace::Ship;
            else if(model_.transformSpace==ShipyardTransformSpace::Ship)model_.transformSpace=ShipyardTransformSpace::Local;
            else model_.transformSpace=ShipyardTransformSpace::View;
            model_.status=model_.transformSpace==ShipyardTransformSpace::View?"TRANSFORM SPACE: CAMERA":(model_.transformSpace==ShipyardTransformSpace::Ship?"TRANSFORM SPACE: SHIP":"TRANSFORM SPACE: LOCAL");return true;
        case ShipyardBuilderCommand::ToggleTransformSnap:model_.transformSnap=!model_.transformSnap;if(model_.transform.active)model_.transform.snap=model_.transformSnap;model_.status=std::string("TRANSFORM SNAP ")+(model_.transformSnap?"ON":"OFF");return true;
        case ShipyardBuilderCommand::NudgePort:{model_.transformTool=ShipyardTransformTool::Move;if(model_.inspectorTab==ShipyardInspectorTab::Sockets){if(!TranslateSelectedSocket({-.10f,0,0}))return false;return CommitSocketTransform();}if(!TranslateSelected({-.10f,0,0}))return false;return CommitTransform();}
        case ShipyardBuilderCommand::NudgeStarboard:{model_.transformTool=ShipyardTransformTool::Move;if(model_.inspectorTab==ShipyardInspectorTab::Sockets){if(!TranslateSelectedSocket({.10f,0,0}))return false;return CommitSocketTransform();}if(!TranslateSelected({.10f,0,0}))return false;return CommitTransform();}
        case ShipyardBuilderCommand::NudgeForward:{model_.transformTool=ShipyardTransformTool::Move;if(model_.inspectorTab==ShipyardInspectorTab::Sockets){if(!TranslateSelectedSocket({0,.10f,0}))return false;return CommitSocketTransform();}if(!TranslateSelected({0,.10f,0}))return false;return CommitTransform();}
        case ShipyardBuilderCommand::NudgeAft:{model_.transformTool=ShipyardTransformTool::Move;if(model_.inspectorTab==ShipyardInspectorTab::Sockets){if(!TranslateSelectedSocket({0,-.10f,0}))return false;return CommitSocketTransform();}if(!TranslateSelected({0,-.10f,0}))return false;return CommitTransform();}
        case ShipyardBuilderCommand::NudgeDorsal:{model_.transformTool=ShipyardTransformTool::Move;if(model_.inspectorTab==ShipyardInspectorTab::Sockets){if(!TranslateSelectedSocket({0,0,.10f}))return false;return CommitSocketTransform();}if(!TranslateSelected({0,0,.10f}))return false;return CommitTransform();}
        case ShipyardBuilderCommand::NudgeVentral:{model_.transformTool=ShipyardTransformTool::Move;if(model_.inspectorTab==ShipyardInspectorTab::Sockets){if(!TranslateSelectedSocket({0,0,-.10f}))return false;return CommitSocketTransform();}if(!TranslateSelected({0,0,-.10f}))return false;return CommitTransform();}
        case ShipyardBuilderCommand::RotatePitchPositive:{model_.transformTool=ShipyardTransformTool::Rotate;if(model_.inspectorTab==ShipyardInspectorTab::Sockets){if(!RotateSelectedSocket({model_.rotationStepDegrees,0,0}))return false;return CommitSocketTransform();}if(!RotateSelected({model_.rotationStepDegrees,0,0}))return false;return CommitTransform();}
        case ShipyardBuilderCommand::RotatePitchNegative:{model_.transformTool=ShipyardTransformTool::Rotate;if(model_.inspectorTab==ShipyardInspectorTab::Sockets){if(!RotateSelectedSocket({-model_.rotationStepDegrees,0,0}))return false;return CommitSocketTransform();}if(!RotateSelected({-model_.rotationStepDegrees,0,0}))return false;return CommitTransform();}
        case ShipyardBuilderCommand::RotateYawPositive:{model_.transformTool=ShipyardTransformTool::Rotate;if(model_.inspectorTab==ShipyardInspectorTab::Sockets){if(!RotateSelectedSocket({0,model_.rotationStepDegrees,0}))return false;return CommitSocketTransform();}if(!RotateSelected({0,model_.rotationStepDegrees,0}))return false;return CommitTransform();}
        case ShipyardBuilderCommand::RotateYawNegative:{model_.transformTool=ShipyardTransformTool::Rotate;if(model_.inspectorTab==ShipyardInspectorTab::Sockets){if(!RotateSelectedSocket({0,-model_.rotationStepDegrees,0}))return false;return CommitSocketTransform();}if(!RotateSelected({0,-model_.rotationStepDegrees,0}))return false;return CommitTransform();}
        case ShipyardBuilderCommand::RotateRollPositive:{model_.transformTool=ShipyardTransformTool::Rotate;if(model_.inspectorTab==ShipyardInspectorTab::Sockets){if(!RotateSelectedSocket({0,0,model_.rotationStepDegrees}))return false;return CommitSocketTransform();}if(!RotateSelected({0,0,model_.rotationStepDegrees}))return false;return CommitTransform();}
        case ShipyardBuilderCommand::RotateRollNegative:{model_.transformTool=ShipyardTransformTool::Rotate;if(model_.inspectorTab==ShipyardInspectorTab::Sockets){if(!RotateSelectedSocket({0,0,-model_.rotationStepDegrees}))return false;return CommitSocketTransform();}if(!RotateSelected({0,0,-model_.rotationStepDegrees}))return false;return CommitTransform();}
        case ShipyardBuilderCommand::FlipPitch:{model_.transformTool=ShipyardTransformTool::Rotate;if(model_.inspectorTab==ShipyardInspectorTab::Sockets){if(!RotateSelectedSocket({180,0,0}))return false;return CommitSocketTransform();}if(!RotateSelected({180,0,0}))return false;return CommitTransform();}
        case ShipyardBuilderCommand::FlipYaw:{model_.transformTool=ShipyardTransformTool::Rotate;if(model_.inspectorTab==ShipyardInspectorTab::Sockets){if(!RotateSelectedSocket({0,180,0}))return false;return CommitSocketTransform();}if(!RotateSelected({0,180,0}))return false;return CommitTransform();}
        case ShipyardBuilderCommand::FlipRoll:{model_.transformTool=ShipyardTransformTool::Rotate;if(model_.inspectorTab==ShipyardInspectorTab::Sockets){if(!RotateSelectedSocket({0,0,180}))return false;return CommitSocketTransform();}if(!RotateSelected({0,0,180}))return false;return CommitTransform();}
        case ShipyardBuilderCommand::MirrorSelectedX:return MirrorSelectedSubtreeX();
        case ShipyardBuilderCommand::CycleRotationStep:
            model_.rotationStepDegrees=model_.rotationStepDegrees>10.0f?5.0f:(model_.rotationStepDegrees>2.0f?1.0f:15.0f);
            if(model_.transform.active)model_.transform.rotationSnapDegrees=model_.rotationStepDegrees;
            model_.status="ROTATION STEP "+std::to_string(static_cast<int>(model_.rotationStepDegrees))+" DEG";return true;
        case ShipyardBuilderCommand::ScaleUniformNegative:{model_.transformTool=ShipyardTransformTool::Scale;if(!ScaleSelected({-.05f,-.05f,-.05f}))return false;return CommitTransform();}
        case ShipyardBuilderCommand::ScaleUniformPositive:{model_.transformTool=ShipyardTransformTool::Scale;if(!ScaleSelected({.05f,.05f,.05f}))return false;return CommitTransform();}
        case ShipyardBuilderCommand::ResetScale:{
            if(model_.recipe.modules.empty())return false;
            const auto index=std::min(model_.selectedPlacedModule,model_.recipe.modules.size()-1);
            model_.transform={};auto& placement=model_.recipe.modules[index];placement.scaleX=placement.scaleY=placement.scaleZ=1.0f;
            model_.dirty=true;InvalidateRecipeMetadata();model_.validation=Validate();model_.status="Selected module scale reset to 1.00";return true;
        }
        case ShipyardBuilderCommand::ScaleAssemblyDown:return ScaleAssembly(1.0f/1.05f);
        case ShipyardBuilderCommand::ScaleAssemblyUp:return ScaleAssembly(1.05f);
        case ShipyardBuilderCommand::ResetRotation:{
            if(model_.recipe.modules.empty())return false;
            const auto index=std::min(model_.selectedPlacedModule,model_.recipe.modules.size()-1);
            model_.transform={};
            auto& placement=model_.recipe.modules[index];
            placement.yawDegrees=0.0f;placement.pitchDegrees=0.0f;placement.rollDegrees=0.0f;
            model_.dirty=true;InvalidateRecipeMetadata();model_.validation=Validate();
            model_.status=model_.validation.valid?"Rotation reset":"Rotation reset; validation reports orientation issues";return true;
        }
        case ShipyardBuilderCommand::FrameSelected:model_.status="Frame selected module";return true;
        case ShipyardBuilderCommand::FrameShip:model_.status="Frame whole ship";return true;
        case ShipyardBuilderCommand::PreviousRole:
        case ShipyardBuilderCommand::NextRole:{auto roles=Roles();auto it=std::find(roles.begin(),roles.end(),model_.role);std::size_t i=it==roles.end()?0:static_cast<std::size_t>(std::distance(roles.begin(),it));i=(command==ShipyardBuilderCommand::NextRole)?(i+1)%roles.size():(i+roles.size()-1)%roles.size();model_.role=roles[i];model_.status="Generator role: "+model_.role;changed=true;}break;
        case ShipyardBuilderCommand::PreviousShipClass:
        case ShipyardBuilderCommand::NextShipClass:{
            static const std::vector<ShipClass> kNormalClasses={ShipClass::Frigate,ShipClass::Destroyer,ShipClass::Cruiser,ShipClass::Battlecruiser,ShipClass::Battleship};
            auto it=std::find(kNormalClasses.begin(),kNormalClasses.end(),model_.shipClass);
            std::size_t i=it==kNormalClasses.end()?0u:static_cast<std::size_t>(std::distance(kNormalClasses.begin(),it));
            i=command==ShipyardBuilderCommand::NextShipClass?(i+1u)%kNormalClasses.size():(i+kNormalClasses.size()-1u)%kNormalClasses.size();
            model_.shipClass=kNormalClasses[i];
            model_.targetModuleSize=ShipClassRoleSystem::Envelope(model_.shipClass).structuralSize;
            model_.status=std::string("Ship class: ")+ShipClassRoleSystem::ClassName(model_.shipClass)+" / "+UniversalKitbashAuthority::SizeName(model_.targetModuleSize);changed=true;}break;
        case ShipyardBuilderCommand::PreviousTargetSize:
        case ShipyardBuilderCommand::NextTargetSize:{int i=static_cast<int>(model_.targetModuleSize);i+=(command==ShipyardBuilderCommand::NextTargetSize)?1:-1;if(i<0)i=4;if(i>4)i=0;model_.targetModuleSize=static_cast<UniversalSizeClass>(i);model_.status=std::string("Module target size: ")+UniversalKitbashAuthority::SizeName(model_.targetModuleSize);changed=true;}break;
        case ShipyardBuilderCommand::CycleConstructionMode:{int i=(static_cast<int>(model_.constructionMode)+1)%4;model_.constructionMode=static_cast<ConstructionWorkspaceMode>(i);model_.status=std::string("Construction mode: ")+UniversalConstructionSystem::ModeName(model_.constructionMode);changed=true;}break;
        case ShipyardBuilderCommand::GenerateVariant:changed=GenerateVariant();break;
        case ShipyardBuilderCommand::Validate:model_.validation=Validate();model_.status=model_.validation.valid?"VALID - structural socket graph passes":"INVALID - inspect errors on right";return true;
        case ShipyardBuilderCommand::SaveBlueprint:
            model_.validation=Validate();
            if(model_.recipe.modules.empty()){model_.status="Cannot save an empty blueprint";return false;}
            saveRequested_=true;
            model_.status=model_.validation.valid?"Certified blueprint save requested":"Authoring draft save requested; generator review required";
            return true;
        case ShipyardBuilderCommand::Apply:
            model_.validation=Validate();
            if(!model_.liveApplyEnabled){model_.status="Live refit requires a docked Shipyard - use SAVE BLUEPRINT here";return false;}
            if(model_.validation.valid){applyRequested_=true;model_.status="Apply requested - validated refit ready";return true;}
            model_.status="Cannot apply: design validation failed";return false;
        case ShipyardBuilderCommand::Reset:model_.recipe=initialRecipe_;model_.appearance=initialAppearance_;model_.liveryPreset=0;model_.decalPreset=0;model_.liveryName="CUSTOM";model_.primaryPaintName=PaintName(model_.appearance.primary);model_.secondaryPaintName=PaintName(model_.appearance.secondary);model_.trimPaintName=PaintName(model_.appearance.trim);model_.transform={};model_.dragPreview={};model_.dirty=false;RefreshForwardAuthority();model_.status="Restored entry ship design";NormalizeSelections();model_.validation=Validate();return true;
        case ShipyardBuilderCommand::None:break;
    }
    NormalizeSelections();if(changed&&command!=ShipyardBuilderCommand::SelectClass&&command!=ShipyardBuilderCommand::SelectModule&&command!=ShipyardBuilderCommand::SelectPlaced&&command!=ShipyardBuilderCommand::PreviousModule&&command!=ShipyardBuilderCommand::NextModule&&command!=ShipyardBuilderCommand::PreviousPlaced&&command!=ShipyardBuilderCommand::NextPlaced)model_.validation=Validate();
    return changed;
}


bool ShipyardBuilderSystem::BeginSelectedTransform(){
    if(model_.recipe.modules.empty()||model_.transformTool==ShipyardTransformTool::Select)return false;
    const auto index=std::min(model_.selectedPlacedModule,model_.recipe.modules.size()-1);
    if(!ShipyardTransformSystem::Begin(model_.transform,index,model_.recipe.modules[index],model_.transformTool,model_.transformSpace))return false;
    model_.transform.snap=model_.transformSnap;
    model_.transform.rotationSnapDegrees=model_.rotationStepDegrees;
    model_.transform.scaleSnap=.05f;
    if(model_.transformTool==ShipyardTransformTool::Move)model_.status="MOVE / arrows follow camera; SHIFT = 0.1x precision";
    else if(model_.transformTool==ShipyardTransformTool::Rotate)model_.status="ROTATE / drag yaw+pitch; CTRL-drag roll; SHIFT = 0.1x";
    else model_.status="SCALE / drag for uniform scale; SHIFT = 0.1x precision";
    return true;
}

bool ShipyardBuilderSystem::TranslateSelected(const Vector3& delta,bool fine){
    if(!model_.transform.active&& !BeginSelectedTransform())return false;if(model_.transform.tool!=ShipyardTransformTool::Move)return false;
    ShipyardTransformSystem::Translate(model_.transform,delta,fine);
    if(model_.transform.moduleIndex<model_.recipe.modules.size())model_.recipe.modules[model_.transform.moduleIndex]=model_.transform.working;
    model_.dirty=true;InvalidateRecipeMetadata();return true;
}

bool ShipyardBuilderSystem::RotateSelected(const Vector3& deltaDegrees,bool fine){
    if(!model_.transform.active&& !BeginSelectedTransform())return false;if(model_.transform.tool!=ShipyardTransformTool::Rotate)return false;
    ShipyardTransformSystem::Rotate(model_.transform,deltaDegrees,fine);
    // Manual authoring must be able to traverse every axis, including a full
    // 180-degree source-basis correction. Do not silently clamp the user's
    // working transform here; commit-time validation remains authoritative and
    // will explain invalid wing/engine orientations without stealing control.
    if(model_.transform.moduleIndex<model_.recipe.modules.size())
        model_.recipe.modules[model_.transform.moduleIndex]=model_.transform.working;
    model_.dirty=true;InvalidateRecipeMetadata();return true;
}

bool ShipyardBuilderSystem::ScaleSelected(const Vector3& deltaScale,bool fine){
    if(!model_.transform.active&& !BeginSelectedTransform())return false;if(model_.transform.tool!=ShipyardTransformTool::Scale)return false;
    ShipyardTransformSystem::Scale(model_.transform,deltaScale,fine);
    if(model_.transform.moduleIndex<model_.recipe.modules.size()){
        if(const auto* record=FindRecord(model_.recipe.modules[model_.transform.moduleIndex].moduleId))
            ClampPlacementToMorphProfile(*record,model_.transform.before,model_.transform.working);
        model_.recipe.modules[model_.transform.moduleIndex]=model_.transform.working;
    }
    model_.dirty=true;InvalidateRecipeMetadata();return true;
}

bool ShipyardBuilderSystem::ScaleAssembly(float factor){
    if(model_.recipe.modules.empty()||factor<=0.0f)return false;
    // Determine a single safe factor shared by every module. This keeps socket
    // relationships coherent; an assembly scale is rejected/clamped rather
    // than stretching fixed/discrete members independently.
    float safeFactor=factor;
    for(const auto& p:model_.recipe.modules){
        const auto* record=FindRecord(p.moduleId);if(!record)continue;
        const auto profile=UniversalKitbashAuthority::BuildProfile(*record,KitbashMaterialCertification::NormalizedFallback);
        if(profile.morph.policy==KitbashScalingPolicy::FixedReference){safeFactor=1.0f;break;}
        const float current=std::max(.001f,(p.scaleX+p.scaleY+p.scaleZ)/3.0f);
        const float lo=std::max({profile.morph.minLengthScale,profile.morph.minWidthScale,profile.morph.minHeightScale})/current;
        const float hi=std::min({profile.morph.maxLengthScale,profile.morph.maxWidthScale,profile.morph.maxHeightScale})/current;
        safeFactor=std::clamp(safeFactor,lo,std::max(lo,hi));
    }
    if(std::fabs(safeFactor-1.0f)<.0001f&&std::fabs(factor-1.0f)>.0001f){model_.status="Assembly scale blocked by fixed/certified morph limits";return false;}
    Vector3 center{};for(const auto& p:model_.recipe.modules){center.x+=p.x;center.y+=p.y;center.z+=p.z;}
    const float inv=1.0f/static_cast<float>(model_.recipe.modules.size());center.x*=inv;center.y*=inv;center.z*=inv;
    for(auto& p:model_.recipe.modules){
        p.x=center.x+(p.x-center.x)*safeFactor;p.y=center.y+(p.y-center.y)*safeFactor;p.z=center.z+(p.z-center.z)*safeFactor;
        const auto before=p;p.scaleX*=safeFactor;p.scaleY*=safeFactor;p.scaleZ*=safeFactor;
        if(const auto* record=FindRecord(p.moduleId))ClampPlacementToMorphProfile(*record,before,p);
    }
    model_.transform={};model_.dirty=true;InvalidateRecipeMetadata();model_.validation=Validate();
    std::ostringstream ss;ss.setf(std::ios::fixed);ss.precision(2);ss<<"Assembly scaled x"<<safeFactor;if(std::fabs(safeFactor-factor)>.0001f)ss<<" (morph-limited)";model_.status=ss.str();return true;
}

bool ShipyardBuilderSystem::CommitTransform(){if(!model_.transform.active)return false;const auto index=model_.transform.moduleIndex;auto p=ShipyardTransformSystem::Commit(model_.transform);if(index<model_.recipe.modules.size())model_.recipe.modules[index]=p;SyncSymmetryPartner(index);model_.validation=Validate();model_.status=model_.validation.valid?"Transform committed":"Transform committed with validation issues";return true;}

bool ShipyardBuilderSystem::CancelTransform(){if(!model_.transform.active)return false;const auto index=model_.transform.moduleIndex;auto p=ShipyardTransformSystem::Cancel(model_.transform);if(index<model_.recipe.modules.size())model_.recipe.modules[index]=p;model_.status="Transform cancelled";return true;}

bool ShipyardBuilderSystem::MirrorSelectedSubtreeX(){
    if(model_.recipe.modules.empty())return false;
    const std::size_t root=std::min(model_.selectedPlacedModule,model_.recipe.modules.size()-1);
    ConstructionSymmetryFrame frame=model_.symmetryFrame;frame.axis=ConstructionSymmetryAxis::PortStarboard;
    if(!ShipyardKitbashTransformSystem::MirrorRecipeSubtree(model_.recipe,root,frame))return false;
    model_.dirty=true;RefreshForwardAuthority();InvalidateRecipeMetadata();model_.validation=Validate();
    model_.status="Reflected selected module/subassembly in place across PORT <-> STARBOARD";
    return true;
}

bool ShipyardBuilderSystem::SyncSymmetryPartner(std::size_t changedIndex){
    if(changedIndex>=model_.recipe.modules.size())return false;
    for(auto& pair:model_.symmetryPairs){
        if(!pair.linked)continue;
        ConstructionSymmetryFrame frame=model_.symmetryFrame;frame.axis=pair.axis;
        if(pair.first==changedIndex&&pair.second<model_.recipe.modules.size()){
            model_.recipe.modules[pair.second]=ConstructionSymmetrySystem::ReflectPlacement(model_.recipe.modules[pair.first],frame);return true;
        }
        if(pair.second==changedIndex&&pair.first<model_.recipe.modules.size()){
            model_.recipe.modules[pair.first]=ConstructionSymmetrySystem::ReflectPlacement(model_.recipe.modules[pair.second],frame);return true;
        }
    }
    return false;
}

bool ShipyardBuilderSystem::MirrorSelectedAcrossActiveSymmetry(){
    if(model_.recipe.modules.empty())return false;
    const std::size_t source=std::min(model_.selectedPlacedModule,model_.recipe.modules.size()-1);
    const auto mirrored=ConstructionSymmetrySystem::ReflectPlacement(model_.recipe.modules[source],model_.symmetryFrame);
    const Vector3 a{model_.recipe.modules[source].x,model_.recipe.modules[source].y,model_.recipe.modules[source].z};
    const Vector3 b{mirrored.x,mirrored.y,mirrored.z};
    if((a-b).length()<.02f){model_.status="Selected module lies on the active symmetry plane";return false;}
    const std::size_t partner=model_.recipe.modules.size();model_.recipe.modules.push_back(mirrored);
    // Mirror the selected module's parent attachment when a corresponding
    // parent exists. If no paired parent exists, a centerline parent may host
    // both children; otherwise the mirrored copy remains a reviewable draft.
    for(const auto& att:model_.recipe.attachments){
        if(att.childModuleIndex!=source)continue;
        std::size_t parent=att.parentModuleIndex;bool parentResolved=false;
        for(const auto& pair:model_.symmetryPairs)if(pair.linked&&pair.axis==model_.symmetryFrame.axis){
            if(pair.first==parent){parent=pair.second;parentResolved=true;break;}
            if(pair.second==parent){parent=pair.first;parentResolved=true;break;}
        }
        if(!parentResolved&&att.parentModuleIndex<model_.recipe.modules.size()){
            const auto& pp=model_.recipe.modules[att.parentModuleIndex];const auto rp=ConstructionSymmetrySystem::ReflectPlacement(pp,model_.symmetryFrame);
            parentResolved=(Vector3{pp.x-rp.x,pp.y-rp.y,pp.z-rp.z}.length()<.03f);
        }
        if(parentResolved){model_.recipe.attachments.push_back({parent,partner,ConstructionSymmetrySystem::ReflectSocketName(att.parentSocket,model_.symmetryFrame.axis),ConstructionSymmetrySystem::ReflectSocketName(att.childSocket,model_.symmetryFrame.axis),0.0f,true});}
        break;
    }
    model_.symmetryPairs.push_back({source,partner,model_.symmetryFrame.axis,true});model_.selectedPlacedModule=partner;
    model_.dirty=true;RefreshForwardAuthority();InvalidateRecipeMetadata();model_.validation=Validate();
    model_.status=std::string("Created exact mirror partner across ")+ConstructionSymmetrySystem::AxisName(model_.symmetryFrame.axis);return true;
}

bool ShipyardBuilderSystem::BreakSelectedSymmetryPair(){
    if(model_.recipe.modules.empty())return false;const auto selected=std::min(model_.selectedPlacedModule,model_.recipe.modules.size()-1);
    for(auto& pair:model_.symmetryPairs)if(pair.linked&&(pair.first==selected||pair.second==selected)){pair.linked=false;model_.status="Symmetry link broken; both modules are now independent";return true;}
    model_.status="Selected module has no linked symmetry partner";return false;
}

void ShipyardBuilderSystem::RefreshDragSymmetryPreview(){
    model_.dragPreview.mirroredPreviewActive=false;model_.dragPreview.mirroredValid=false;
    if(!model_.dragPreview.active||!model_.symmetryFrame.live)return;
    const auto mirrored=ConstructionSymmetrySystem::ReflectPlacement(model_.dragPreview.ghost,model_.symmetryFrame);
    const Vector3 a{model_.dragPreview.ghost.x,model_.dragPreview.ghost.y,model_.dragPreview.ghost.z},b{mirrored.x,mirrored.y,mirrored.z};
    if((a-b).length()<.02f)return;
    model_.dragPreview.mirroredGhost=mirrored;model_.dragPreview.mirroredPreviewActive=true;model_.dragPreview.mirroredValid=model_.dragPreview.valid;
}

bool ShipyardBuilderSystem::HandleWheel(float pointerX,float pointerY,float wheelDelta,int viewportWidth,int viewportHeight){
    if(std::fabs(wheelDelta)<0.0001f)return false;
    const auto l=Layout(viewportWidth,viewportHeight);if(!l.valid)return false;
    const int steps=wheelDelta>0.0f?-1:1;
    if(pointerX>=l.left&&pointerX<=l.left+l.leftWidth&&pointerY>=l.moduleCardsY&&pointerY<=l.leftInfoY){
        const auto filtered=FilteredCatalogIndices();
        const std::size_t pageSize=5;
        if(filtered.size()<=pageSize){model_.catalogScrollStart=0;return true;}
        const std::size_t maxStart=filtered.size()-pageSize;
        if(steps<0)model_.catalogScrollStart=model_.catalogScrollStart==0?0:model_.catalogScrollStart-1;
        else model_.catalogScrollStart=std::min(maxStart,model_.catalogScrollStart+1);
        if(model_.selectedFilteredModule<model_.catalogScrollStart)model_.selectedFilteredModule=model_.catalogScrollStart;
        if(model_.selectedFilteredModule>=model_.catalogScrollStart+pageSize)model_.selectedFilteredModule=std::min(filtered.size()-1,model_.catalogScrollStart+pageSize-1);
        model_.status="Parts inventory scrolled";return true;
    }
    if(pointerX>=l.right&&pointerX<=l.right+l.rightWidth&&pointerY>=l.placedListY&&pointerY<=l.validationY){
        if(model_.recipe.modules.empty()){model_.placedScrollStart=0;return true;}
        const std::size_t pageSize=std::max<std::size_t>(1,l.placedPageSize);
        const std::size_t maxStart=model_.recipe.modules.size()>pageSize?model_.recipe.modules.size()-pageSize:0;
        if(steps<0)model_.placedScrollStart=model_.placedScrollStart==0?0:model_.placedScrollStart-1;
        else model_.placedScrollStart=std::min(maxStart,model_.placedScrollStart+1);
        model_.status="Inspector module list scrolled";return true;
    }
    return false;
}

bool ShipyardBuilderSystem::BeginCatalogDrag(int filteredIndex){
    const auto filtered=FilteredCatalogIndices();if(filteredIndex<0||static_cast<std::size_t>(filteredIndex)>=filtered.size())return false;model_.selectedFilteredModule=static_cast<std::size_t>(filteredIndex);const auto& child=model_.catalog[filtered[model_.selectedFilteredModule]];model_.dragPreview=ShipyardDragDropSystem::Begin(child,model_.catalog,model_.recipe,model_.targetModuleSize);model_.status=model_.dragPreview.status;return model_.dragPreview.active;
}

bool ShipyardBuilderSystem::UpdateCatalogDrag(const Vector3& shipLocalPointer){
    if(!model_.dragPreview.active)return false;
    float best=1.0e30f;int bestIndex=-1;
    for(std::size_t i=0;i<model_.dragPreview.candidates.size();++i){
        const auto& p=model_.dragPreview.candidates[i].placement;
        const float dx=p.x-shipLocalPointer.x,dy=p.y-shipLocalPointer.y,dz=p.z-shipLocalPointer.z;
        const float d=std::sqrt(dx*dx+dy*dy+dz*dz*.35f);
        model_.dragPreview.candidates[i].pointerDistance=d;
        if(d<best){best=d;bestIndex=static_cast<int>(i);}
    }
    const bool canSnap=bestIndex>=0&&best<=model_.dragPreview.snapRadius;
    if(canSnap){
        model_.dragPreview.selectedCandidate=bestIndex;
        model_.dragPreview.ghost=model_.dragPreview.candidates[static_cast<std::size_t>(bestIndex)].placement;
        model_.dragPreview.valid=!model_.dragPreview.candidates[static_cast<std::size_t>(bestIndex)].collisionRisk;
        model_.dragPreview.snapped=true;model_.dragPreview.freePlacement=false;
        model_.dragPreview.status=model_.dragPreview.valid?"SNAP READY / RELEASE TO ATTACH":"INVALID / COLLISION";
    }else{
        model_.dragPreview.selectedCandidate=-1;
        model_.dragPreview.ghost.x=shipLocalPointer.x;model_.dragPreview.ghost.y=shipLocalPointer.y;model_.dragPreview.ghost.z=shipLocalPointer.z;
        model_.dragPreview.valid=true;model_.dragPreview.snapped=false;model_.dragPreview.freePlacement=true;
        model_.dragPreview.status="FREE BUILD / RELEASE TO PLACE DRAFT";
    }
    RefreshDragSymmetryPreview();
    if(model_.dragPreview.mirroredPreviewActive)model_.dragPreview.status += model_.dragPreview.mirroredValid?" / MIRROR READY":" / MIRROR INVALID";
    model_.status=model_.dragPreview.status;return true;
}

bool ShipyardBuilderSystem::CommitCatalogDrag(){
    if(!model_.dragPreview.active||!model_.dragPreview.valid)return false;
    const auto preview=model_.dragPreview;
    const std::size_t childIndex=model_.recipe.modules.size();
    std::size_t primaryParent=static_cast<std::size_t>(-1);std::string primaryParentSocket,primaryChildSocket;
    if(preview.snapped&&preview.selectedCandidate>=0){
        const auto c=preview.candidates[static_cast<std::size_t>(preview.selectedCandidate)];
        model_.recipe.modules.push_back(c.placement);
        model_.recipe.attachments.push_back({c.parentModuleIndex,childIndex,c.parentSocket,c.childSocket,0.0f,true});
        primaryParent=c.parentModuleIndex;primaryParentSocket=c.parentSocket;primaryChildSocket=c.childSocket;
        model_.status="Module dropped onto compatible socket";
    }else if(preview.freePlacement){
        model_.recipe.modules.push_back(preview.ghost);
        model_.status="Free-placed module; attach/certify before production use";
    }else return false;

    if(preview.mirroredPreviewActive&&preview.mirroredValid){
        const std::size_t mirrorIndex=model_.recipe.modules.size();model_.recipe.modules.push_back(preview.mirroredGhost);
        bool mirrorAttached=false;std::size_t mirrorParent=primaryParent;
        if(primaryParent!=static_cast<std::size_t>(-1)){
            for(const auto& pair:model_.symmetryPairs)if(pair.linked&&pair.axis==model_.symmetryFrame.axis){if(pair.first==primaryParent){mirrorParent=pair.second;mirrorAttached=true;break;}if(pair.second==primaryParent){mirrorParent=pair.first;mirrorAttached=true;break;}}
            if(!mirrorAttached&&primaryParent<model_.recipe.modules.size()){
                const auto reflectedParent=ConstructionSymmetrySystem::ReflectPlacement(model_.recipe.modules[primaryParent],model_.symmetryFrame);
                const auto& pp=model_.recipe.modules[primaryParent];mirrorAttached=Vector3{reflectedParent.x-pp.x,reflectedParent.y-pp.y,reflectedParent.z-pp.z}.length()<.04f;
            }
            if(mirrorAttached)model_.recipe.attachments.push_back({mirrorParent,mirrorIndex,ConstructionSymmetrySystem::ReflectSocketName(primaryParentSocket,model_.symmetryFrame.axis),ConstructionSymmetrySystem::ReflectSocketName(primaryChildSocket,model_.symmetryFrame.axis),0.0f,true});
        }
        model_.symmetryPairs.push_back({childIndex,mirrorIndex,model_.symmetryFrame.axis,true});
        model_.status += " + exact symmetry partner";
    }
    model_.selectedPlacedModule=childIndex;model_.dragPreview={};model_.dirty=true;SyncCatalogSelectionToPlaced();RefreshForwardAuthority();InvalidateRecipeMetadata();model_.validation=Validate();return true;
}

void ShipyardBuilderSystem::CancelCatalogDrag(){model_.dragPreview={};}

bool ShipyardBuilderSystem::ConsumeApplyRequested(){const bool v=applyRequested_;applyRequested_=false;return v;}
bool ShipyardBuilderSystem::ConsumeSaveRequested(){const bool v=saveRequested_;saveRequested_=false;return v;}
bool ShipyardBuilderSystem::ConsumeSocketOverridesSaveRequested(){const bool v=socketOverridesSaveRequested_;socketOverridesSaveRequested_=false;return v;}
bool ShipyardBuilderSystem::ConsumeDefinitionOverridesSaveRequested(){const bool v=definitionOverridesSaveRequested_;definitionOverridesSaveRequested_=false;return v;}
void ShipyardBuilderSystem::MarkApplied(){model_.dirty=false;model_.status="Applied to player ship visual blueprint";}
void ShipyardBuilderSystem::MarkSaved(const std::string& path){model_.status="Blueprint saved: "+path;}
void ShipyardBuilderSystem::MarkSocketOverridesSaved(const std::string& path){model_.socketOverridesDirty=false;model_.status="Socket overrides saved: "+path;}
void ShipyardBuilderSystem::MarkDefinitionOverridesSaved(const std::string& path){model_.definitionOverridesDirty=false;model_.status="Definition overrides saved: "+path;}

ShipyardBuilderLayout ShipyardBuilderSystem::Layout(int w,int h){
    ShipyardBuilderLayout l;
    if(w<1120||h<740)return l;
    l.valid=true;
    l.uiScale=std::clamp(std::min(static_cast<float>(w)/1920.0f,static_cast<float>(h)/1080.0f),1.0f,1.60f);
    const float s=l.uiScale;
    l.compact=h<static_cast<int>(860.0f*s);

    // Pass585+: the project-wide UI authority is resolution-aware.  The old
    // fixed 390px/520px caps made the Shipyard look nearly unchanged and tiny
    // at 1440p/4K even when the internal editor architecture had improved.
    l.left=24.0f*s;
    l.top=34.0f*s;
    l.leftWidth=std::clamp(static_cast<float>(w)*.235f,400.0f*s,560.0f*s);
    l.rightWidth=std::clamp(static_cast<float>(w)*.300f,520.0f*s,700.0f*s);
    l.right=static_cast<float>(w)-l.rightWidth-16.0f*s;
    l.rowHeight=(l.compact?32.0f:36.0f)*s;
    l.rowGap=6.0f*s;

    // Left: two-row category grid followed by five large visual asset cards.
    l.libraryListY=l.top+62.0f*s;
    l.moduleCardsY=l.libraryListY+2.0f*(l.rowHeight+l.rowGap)+12.0f*s;
    l.moduleCardHeight=(l.compact?58.0f:70.0f)*s;
    l.leftActionsY=l.moduleCardsY+5.0f*(l.moduleCardHeight+l.rowGap)+12.0f*s;
    l.leftInfoY=l.leftActionsY+82.0f*s;

    // Right: explicit one-click workflow tabs. SOCKETS is intentionally a
    // first-class visible tab during development; advanced metadata remains in
    // AUTHORING.  This restores the corrective workflow lost in Pass535.
    l.tabRowY=l.top+56.0f*s;
    l.tabHeight=38.0f*s;
    // Reserve three workflow rows. Dev Mode now exposes BUILD/MODEL/INTERIOR,
    // CHARACTER, PCG, WORLD, DEV WORLD and raw AUTHORING without shrinking
    // mouse targets. Player-docked mode simply leaves unused rows empty.
    l.contentTopY=l.tabRowY+3.0f*l.tabHeight+2.0f*l.rowGap+14.0f*s;
    l.selectedSummaryY=l.contentTopY;
    l.placedListY=l.contentTopY+64.0f*s;
    l.placedPageSize=l.compact?5u:6u;

    l.editLabelY=l.contentTopY+70.0f*s;
    l.editRowY=l.editLabelY+24.0f*s;
    l.focusLabelY=l.editRowY+42.0f*s;
    l.focusRowY=l.focusLabelY+24.0f*s;
    l.moveLabelY=l.focusRowY+42.0f*s;
    l.moveRowY=l.moveLabelY+24.0f*s;
    l.moveRow2Y=l.moveRowY+42.0f*s;
    l.rotateLabelY=l.moveRow2Y+44.0f*s;
    l.rotateRowY=l.rotateLabelY+24.0f*s;
    l.yawRowY=l.rotateRowY+42.0f*s;
    l.rollRowY=l.yawRowY+42.0f*s;
    l.flipRowY=l.rollRowY+42.0f*s;

    const float placedEnd=l.placedListY+static_cast<float>(l.placedPageSize)*(l.rowHeight+l.rowGap);
    l.blueprintLabelY=placedEnd+18.0f*s;
    l.classRowY=l.blueprintLabelY+24.0f*s;
    l.sizeModeRowY=l.classRowY+44.0f*s;
    l.generateRowY=l.sizeModeRowY+44.0f*s;
    l.saveRowY=l.generateRowY+44.0f*s;
    l.liveryLabelY=l.contentTopY+82.0f*s;
    l.liveryRowY=l.liveryLabelY+28.0f*s;
    l.liverySecondaryRowY=l.liveryRowY+46.0f*s;
    l.paintSecondaryRowY=l.liverySecondaryRowY+46.0f*s;
    l.paintTrimRowY=l.paintSecondaryRowY+46.0f*s;
    l.decalRowY=l.paintTrimRowY+46.0f*s;

    const float transformEnd=l.flipRowY+36.0f*s;
    const float assemblyEnd=l.saveRowY+36.0f*s;
    const float appearanceEnd=l.decalRowY+40.0f*s;
    l.validationY=std::max({transformEnd,assemblyEnd,appearanceEnd})+18.0f*s;
    l.statusY=static_cast<float>(h)-42.0f*s;
    // If a shorter supported window cannot fit the lower validation card,
    // clamp it upward rather than letting controls collide with the status bar.
    l.validationY=std::min(l.validationY,l.statusY-92.0f*s);
    return l;
}

std::vector<ShipyardBuilderControl> ShipyardBuilderSystem::BuildControls(const ShipyardBuilderRuntimeModel& model,int w,int h){
    std::vector<ShipyardBuilderControl> out;
    const auto l=Layout(w,h);
    if(!l.valid)return out;
    const float left=l.left,libraryW=l.leftWidth,rightW=l.rightWidth,right=l.right,rowH=l.rowHeight,gap=l.rowGap,s=l.uiScale;
    auto add=[&](ShipyardBuilderCommand c,int value,float x,float y,float cw,float ch,std::string label,bool active=false,bool enabled=true){
        out.push_back({c,value,x,y,cw,ch,std::move(label),active,enabled});
    };

    // Visual inventory categories: two rows of four broad filters.  The old
    // 8-row text strip consumed the entire left pane and made the library read
    // like a debug list rather than a parts inventory.
    const float categoryGap=6.0f*s;
    const float categoryW=(libraryW-20.0f*s-categoryGap*3.0f)/4.0f;
    for(int ci=0;ci<8;++ci){
        const auto cls=static_cast<ShipyardModuleClass>(ci);
        const std::size_t count=static_cast<std::size_t>(std::count_if(model.catalog.begin(),model.catalog.end(),
            [&](const auto& r){return r.moduleClass==cls;}));
        const int col=ci%4,row=ci/4;
        add(ShipyardBuilderCommand::SelectClass,ci,left+10.0f*s+col*(categoryW+categoryGap),l.libraryListY+row*(rowH+gap),categoryW,rowH,
            std::string(ShipyardModuleSystem::ClassName(cls))+"  "+std::to_string(count),static_cast<int>(model.selectedClass)==ci);
    }

    std::vector<std::size_t> filtered;
    for(std::size_t i=0;i<model.catalog.size();++i)if(model.catalog[i].moduleClass==model.selectedClass)filtered.push_back(i);
    const std::size_t pageSize=5;
    const std::size_t selected=filtered.empty()?0:std::min(model.selectedFilteredModule,filtered.size()-1);
    const std::size_t maxPageStart=filtered.size()>pageSize?filtered.size()-pageSize:0;
    const std::size_t pageStart=filtered.empty()?0:std::min(model.catalogScrollStart,maxPageStart);
    const float moduleX=left+10.0f*s,moduleW=libraryW-20.0f*s;
    for(std::size_t cardIndex=0;cardIndex<pageSize&&pageStart+cardIndex<filtered.size();++cardIndex){
        const auto fi=pageStart+cardIndex;const auto& rec=model.catalog[filtered[fi]];
        add(ShipyardBuilderCommand::SelectModule,static_cast<int>(fi),moduleX,l.moduleCardsY+cardIndex*(l.moduleCardHeight+gap),moduleW,l.moduleCardHeight,
            FriendlyModuleLabel(rec),fi==selected);
    }

    const float actionsY=l.leftActionsY;
    const float inner=libraryW-20.0f*s;
    const float navW=52.0f*s,buttonH=34.0f*s;
    add(ShipyardBuilderCommand::PreviousModule,0,left+10.0f*s,actionsY,navW,buttonH,"<");
    add(ShipyardBuilderCommand::NextModule,0,left+68.0f*s,actionsY,navW,buttonH,">");
    add(ShipyardBuilderCommand::AddModule,0,left+126.0f*s,actionsY,96.0f*s,buttonH,"PLACE",false,!filtered.empty());
    add(ShipyardBuilderCommand::ReplaceModule,0,left+228.0f*s,actionsY,inner-218.0f*s,buttonH,"REPLACE",false,!filtered.empty()&&!model.recipe.modules.empty());
    add(ShipyardBuilderCommand::RemoveModule,0,left+10.0f*s,actionsY+40.0f*s,92.0f*s,buttonH,"REMOVE",false,!model.recipe.modules.empty());
    add(ShipyardBuilderCommand::ToggleLiveSymmetry,0,left+108.0f*s,actionsY+40.0f*s,126.0f*s,buttonH,model.symmetryFrame.live?"LIVE SYM ON":"LIVE SYM OFF",model.symmetryFrame.live);
    add(ShipyardBuilderCommand::Validate,0,left+240.0f*s,actionsY+40.0f*s,inner-230.0f*s,buttonH,"VALIDATE");

    const float rx=right+12.0f*s,rw=rightW-24.0f*s,smallGap=7.0f*s;
    auto row=[&](float y,const std::vector<std::tuple<ShipyardBuilderCommand,std::string,bool,bool>>& items,float ch=0.0f){
        if(ch<=0.0f)ch=34.0f*s;
        const float cw=(rw-smallGap*static_cast<float>(items.size()-1))/static_cast<float>(items.size());
        float x=rx;
        for(const auto& item:items){
            add(std::get<0>(item),0,x,y,cw,ch,std::get<1>(item),std::get<2>(item),std::get<3>(item));
            x+=cw+smallGap;
        }
    };
    const bool hasPlaced=!model.recipe.modules.empty();
    // Compatibility bridge for older callers/tests that set inspectorTab
    // directly. Runtime navigation now owns workspaceMode, but a directly
    // supplied legacy page still resolves to the matching modern workspace.
    ShipyardWorkspaceMode effectiveMode=model.workspaceMode;
    if(model.workspaceMode==ShipyardWorkspaceMode::Build){
        if(model.inspectorTab==ShipyardInspectorTab::Assembly)effectiveMode=ShipyardWorkspaceMode::Systems;
        else if(model.inspectorTab==ShipyardInspectorTab::Appearance)effectiveMode=ShipyardWorkspaceMode::Appearance;
        else if(model.inspectorTab==ShipyardInspectorTab::Sockets||model.inspectorTab==ShipyardInspectorTab::Authoring)effectiveMode=ShipyardWorkspaceMode::Authoring;
    }

    // Shipyard is now the in-game authoring environment. Player-docked access
    // keeps the approachable construction surface while Main Menu Dev Studio /
    // runtime Dev Mode unlock native modeling, PCG, world, socket, and raw
    // authoring capabilities over the same runtime data.
    std::vector<std::tuple<ShipyardBuilderCommand,std::string,bool,bool>> workspaceTabs;
    workspaceTabs.push_back({ShipyardBuilderCommand::WorkspaceBuild,"BUILD",effectiveMode==ShipyardWorkspaceMode::Build,true});
    if(model.capabilities.model)workspaceTabs.push_back({ShipyardBuilderCommand::WorkspaceModel,"MODEL",effectiveMode==ShipyardWorkspaceMode::Model,true});
    if(model.capabilities.interior)workspaceTabs.push_back({ShipyardBuilderCommand::WorkspaceInterior,"INTERIOR",effectiveMode==ShipyardWorkspaceMode::Interior,true});
    if(model.capabilities.sockets)workspaceTabs.push_back({ShipyardBuilderCommand::InspectorSockets,"SOCKETS",model.inspectorTab==ShipyardInspectorTab::Sockets,true});
    workspaceTabs.push_back({ShipyardBuilderCommand::WorkspaceAppearance,"APPEARANCE",effectiveMode==ShipyardWorkspaceMode::Appearance,true});
    workspaceTabs.push_back({ShipyardBuilderCommand::WorkspaceSystems,"SYSTEMS",effectiveMode==ShipyardWorkspaceMode::Systems,true});
    if(model.capabilities.character)workspaceTabs.push_back({ShipyardBuilderCommand::WorkspaceCharacter,"CHARACTER",effectiveMode==ShipyardWorkspaceMode::Character,true});
    if(model.capabilities.pcgStudio)workspaceTabs.push_back({ShipyardBuilderCommand::WorkspacePcg,"PCG",effectiveMode==ShipyardWorkspaceMode::Pcg,true});
    if(model.capabilities.world)workspaceTabs.push_back({ShipyardBuilderCommand::WorkspaceWorld,"WORLD",effectiveMode==ShipyardWorkspaceMode::World,true});
    if(model.capabilities.devWorld)workspaceTabs.push_back({ShipyardBuilderCommand::WorkspaceDevWorld,"DEV WORLD",effectiveMode==ShipyardWorkspaceMode::DevWorld,true});
    if(model.capabilities.rawAuthoring)workspaceTabs.push_back({ShipyardBuilderCommand::WorkspaceAuthoring,"AUTHOR",effectiveMode==ShipyardWorkspaceMode::Authoring&&model.inspectorTab==ShipyardInspectorTab::Authoring,true});
    // Expanded Dev Mode uses a three-row workflow strip. Keep every tab large
    // enough for mouse-first use rather than compressing the complete authoring
    // environment into unreadable tiny tabs.
    const std::size_t tabsPerRow=4;
    for(std::size_t start=0;start<workspaceTabs.size();start+=tabsPerRow){
        const auto end=std::min(start+tabsPerRow,workspaceTabs.size());
        std::vector<std::tuple<ShipyardBuilderCommand,std::string,bool,bool>> tabRow(workspaceTabs.begin()+static_cast<std::ptrdiff_t>(start),workspaceTabs.begin()+static_cast<std::ptrdiff_t>(end));
        const float tabY=l.tabRowY+static_cast<float>(start/tabsPerRow)*(l.tabHeight+l.rowGap);
        row(tabY,tabRow,l.tabHeight);
    }
    // Zero-size legacy navigation sentinels preserve source/API compatibility
    // without reintroducing the old five-peer-tab visual clutter.
    add(ShipyardBuilderCommand::InspectorTransform,0,-100,-100,0,0,"",false,false);
    add(ShipyardBuilderCommand::InspectorAssembly,0,-100,-100,0,0,"",false,false);
    add(ShipyardBuilderCommand::InspectorAppearance,0,-100,-100,0,0,"",false,false);

    if(effectiveMode==ShipyardWorkspaceMode::Build){
        const std::string spaceLabel=model.transformSpace==ShipyardTransformSpace::View?"CAMERA":(model.transformSpace==ShipyardTransformSpace::Ship?"SHIP":"LOCAL");
        row(l.editRowY,{
            {ShipyardBuilderCommand::ToolSelect,"[Q] SELECT",model.transformTool==ShipyardTransformTool::Select,true},
            {ShipyardBuilderCommand::ToolMove,"[W] MOVE",model.transformTool==ShipyardTransformTool::Move,hasPlaced},
            {ShipyardBuilderCommand::ToolRotate,"[E] ROTATE",model.transformTool==ShipyardTransformTool::Rotate,hasPlaced},
            {ShipyardBuilderCommand::ToolScale,"[R] SCALE",model.transformTool==ShipyardTransformTool::Scale,hasPlaced},
            {ShipyardBuilderCommand::ToggleTransformSpace,spaceLabel,false,hasPlaced},
            {ShipyardBuilderCommand::ToggleTransformSnap,model.transformSnap?"SNAP":"FREE",model.transformSnap,hasPlaced}
        });
        row(l.focusRowY,{
            {ShipyardBuilderCommand::PreviousPlaced,"PREV",false,hasPlaced},
            {ShipyardBuilderCommand::NextPlaced,"NEXT",false,hasPlaced},
            {ShipyardBuilderCommand::FrameSelected,"[F] FRAME PART",false,hasPlaced},
            {ShipyardBuilderCommand::FrameShip,"[HOME] FRAME SHIP",false,hasPlaced}
        });
        if(model.transformTool==ShipyardTransformTool::Move){
            row(l.moveRowY,{
                {ShipyardBuilderCommand::NudgePort,"PORT",false,hasPlaced},
                {ShipyardBuilderCommand::NudgeStarboard,"STARBOARD",false,hasPlaced},
                {ShipyardBuilderCommand::NudgeForward,"FORWARD",false,hasPlaced}
            });
            row(l.moveRow2Y,{
                {ShipyardBuilderCommand::NudgeAft,"AFT",false,hasPlaced},
                {ShipyardBuilderCommand::NudgeDorsal,"UP",false,hasPlaced},
                {ShipyardBuilderCommand::NudgeVentral,"DOWN",false,hasPlaced}
            });
        }else if(model.transformTool==ShipyardTransformTool::Rotate){
            row(l.moveRowY,{
                {ShipyardBuilderCommand::RotatePitchNegative,"PITCH -",false,hasPlaced},
                {ShipyardBuilderCommand::RotatePitchPositive,"PITCH +",false,hasPlaced},
                {ShipyardBuilderCommand::FlipPitch,"FLIP PITCH",false,hasPlaced}
            });
            row(l.moveRow2Y,{
                {ShipyardBuilderCommand::RotateYawNegative,"YAW -",false,hasPlaced},
                {ShipyardBuilderCommand::RotateYawPositive,"YAW +",false,hasPlaced},
                {ShipyardBuilderCommand::FlipYaw,"FLIP YAW",false,hasPlaced}
            });
            row(l.rotateRowY,{
                {ShipyardBuilderCommand::RotateRollNegative,"ROLL -",false,hasPlaced},
                {ShipyardBuilderCommand::RotateRollPositive,"ROLL +",false,hasPlaced},
                {ShipyardBuilderCommand::FlipRoll,"FLIP ROLL",false,hasPlaced}
            });
            row(l.yawRowY,{
                {ShipyardBuilderCommand::CycleRotationStep,"STEP  "+std::to_string(static_cast<int>(model.rotationStepDegrees))+" DEG",false,hasPlaced},
                {ShipyardBuilderCommand::MirrorSelectedX,"MIRROR X",false,hasPlaced},
                {ShipyardBuilderCommand::ResetRotation,"RESET ROTATION",false,hasPlaced}
            });
        }else if(model.transformTool==ShipyardTransformTool::Scale){
            row(l.moveRowY,{
                {ShipyardBuilderCommand::ScaleUniformNegative,"PART  -5%",false,hasPlaced},
                {ShipyardBuilderCommand::ScaleUniformPositive,"PART  +5%",false,hasPlaced},
                {ShipyardBuilderCommand::ResetScale,"RESET PART",false,hasPlaced}
            });
            row(l.moveRow2Y,{
                {ShipyardBuilderCommand::ScaleAssemblyDown,"ASSEMBLY  -5%",false,hasPlaced},
                {ShipyardBuilderCommand::ScaleAssemblyUp,"ASSEMBLY  +5%",false,hasPlaced}
            });
        }else{ // Select tool: symmetry is a first-class construction workflow.
            row(l.moveRowY,{
                {ShipyardBuilderCommand::SymmetryAxisPortStarboard,"PORT <-> STARBOARD",model.symmetryFrame.axis==ConstructionSymmetryAxis::PortStarboard,true},
                {ShipyardBuilderCommand::SymmetryAxisForeAft,"FORE <-> AFT",model.symmetryFrame.axis==ConstructionSymmetryAxis::ForeAft,true},
                {ShipyardBuilderCommand::SymmetryAxisDorsalVentral,"DORSAL <-> VENTRAL",model.symmetryFrame.axis==ConstructionSymmetryAxis::DorsalVentral,true}
            });
            row(l.moveRow2Y,{
                {ShipyardBuilderCommand::SymmetryPlaneNegative,"PLANE -",false,true},
                {ShipyardBuilderCommand::SymmetryPlanePositive,"PLANE +",false,true},
                {ShipyardBuilderCommand::ResetSymmetryFrame,"CENTER",false,true},
                {ShipyardBuilderCommand::ToggleLiveSymmetry,model.symmetryFrame.live?"LIVE ON":"LIVE OFF",model.symmetryFrame.live,true}
            });
            row(l.rotateRowY,{
                {ShipyardBuilderCommand::MirrorSelectedAcrossSymmetry,"MIRROR COPY",false,hasPlaced},
                {ShipyardBuilderCommand::BreakSymmetryPair,"BREAK PAIR",false,hasPlaced}
            });
        }
    }
    else if(effectiveMode==ShipyardWorkspaceMode::Model){
        const auto primitive=ShipyardModelingSystem::PrimitiveName(model.modeling.selectedPrimitive);
        const auto selection=ShipyardModelingSystem::SelectionModeName(model.modeling.selectionMode);
        row(l.editRowY,{
            {ShipyardBuilderCommand::ModelPreviousPrimitive,"< SHAPE",false,true},
            {ShipyardBuilderCommand::ModelAddShape,std::string("ADD ")+primitive,true,true},
            {ShipyardBuilderCommand::ModelNextPrimitive,"SHAPE >",false,true}
        });
        row(l.focusRowY,{
            {ShipyardBuilderCommand::ModelCycleSelectionMode,std::string("SELECT ")+selection,true,true},
            {ShipyardBuilderCommand::ModelToggleSymmetricStretch,model.modeling.symmetricStretch?"STRETCH SYM":"STRETCH ONE SIDE",model.modeling.symmetricStretch,true}
        });
        const bool hasShape=!model.modeling.recipe.primitives.empty();
        row(l.moveRowY,{
            {ShipyardBuilderCommand::ModelStretchXNegative,"WIDTH -",false,hasShape},
            {ShipyardBuilderCommand::ModelStretchXPositive,"WIDTH +",false,hasShape},
            {ShipyardBuilderCommand::ModelStretchYNegative,"LENGTH -",false,hasShape},
            {ShipyardBuilderCommand::ModelStretchYPositive,"LENGTH +",false,hasShape}
        });
        row(l.moveRow2Y,{
            {ShipyardBuilderCommand::ModelStretchZNegative,"HEIGHT -",false,hasShape},
            {ShipyardBuilderCommand::ModelStretchZPositive,"HEIGHT +",false,hasShape}
        });
        row(l.rotateRowY,{
            {ShipyardBuilderCommand::ModelValidate,"VALIDATE MODEL",false,true},
            {ShipyardBuilderCommand::ModelPublishCanonical,"PUBLISH MODULE",false,hasShape&&model.capabilities.publishCanonicalAsset}
        });
    }
    else if(effectiveMode==ShipyardWorkspaceMode::Pcg){
        row(l.editRowY,{
            {ShipyardBuilderCommand::PcgReroll,"REROLL SEED",false,true},
            {ShipyardBuilderCommand::GenerateVariant,"GENERATE",false,true},
            {ShipyardBuilderCommand::PcgAudit,"AUDIT / EXPLAIN",false,true}
        });
        row(l.focusRowY,{
            {ShipyardBuilderCommand::PcgToggleOccupancy,model.pcgStudio.overlays.occupancy?"OCCUPANCY ON":"OCCUPANCY OFF",model.pcgStudio.overlays.occupancy,true},
            {ShipyardBuilderCommand::PcgToggleDetailDensity,model.pcgStudio.overlays.detailDensity?"DENSITY ON":"DENSITY OFF",model.pcgStudio.overlays.detailDensity,true}
        });
        row(l.moveRowY,{
            {ShipyardBuilderCommand::PreviousRole,"< ROLE",false,true},
            {ShipyardBuilderCommand::NextRole,model.role+" >",true,true},
            {ShipyardBuilderCommand::PcgTeachFromAssembly,model.pcgStudio.teachFromCurrentAssembly?"EXEMPLAR ON":"TEACH FROM THIS",model.pcgStudio.teachFromCurrentAssembly,true}
        });
        row(l.moveRow2Y,{
            {ShipyardBuilderCommand::PreviousShipClass,"< CLASS",false,true},
            {ShipyardBuilderCommand::NextShipClass,ShipClassRoleSystem::ClassName(model.shipClass),true,true}
        });
    }
    else if(effectiveMode==ShipyardWorkspaceMode::World){
        row(l.editRowY,{
            {ShipyardBuilderCommand::WorldPreviousBrush,"< BRUSH",false,true},
            {ShipyardBuilderCommand::WorldNextBrush,PlanetWorldEngineSystem::BrushName(model.worldAuthoring.brush),true,true},
            {ShipyardBuilderCommand::WorldApplyBrush,"APPLY",false,model.worldAuthoring.world.terraformable}
        });
        row(l.focusRowY,{
            {ShipyardBuilderCommand::WorldRadiusDown,"RADIUS -",false,true},
            {ShipyardBuilderCommand::WorldRadiusUp,std::string("RADIUS ")+std::to_string(int(model.worldAuthoring.brushRadiusMeters))+"m +",true,true}
        });
        row(l.moveRowY,{
            {ShipyardBuilderCommand::WorldStrengthDown,"STRENGTH -",false,true},
            {ShipyardBuilderCommand::WorldStrengthUp,std::string("STRENGTH ")+std::to_string(model.worldAuthoring.brushStrengthMeters)+"m +",true,true}
        });
        row(l.moveRow2Y,{
            {ShipyardBuilderCommand::WorldToggleHydrology,model.worldAuthoring.showHydrology?"HYDROLOGY ON":"HYDROLOGY OFF",model.worldAuthoring.showHydrology,true},
            {ShipyardBuilderCommand::WorldToggleResources,model.worldAuthoring.showResources?"GEOLOGY ON":"GEOLOGY OFF",model.worldAuthoring.showResources,true}
        });
    }
    else if(effectiveMode==ShipyardWorkspaceMode::Interior){
        row(l.editRowY,{{ShipyardBuilderCommand::Validate,"REBUILD PLAN",false,true},{ShipyardBuilderCommand::FrameShip,"FRAME SHIP",false,hasPlaced}});
    }
    else if(effectiveMode==ShipyardWorkspaceMode::Character){
        row(l.editRowY,{{ShipyardBuilderCommand::ScalePlayerDown,"PLAYER -",false,true},{ShipyardBuilderCommand::ScalePlayerUp,"PLAYER +",false,true}});
    }
    else if(effectiveMode==ShipyardWorkspaceMode::DevWorld){
        row(l.editRowY,{{ShipyardBuilderCommand::DevWorldPreviousBackdrop,"BACKDROP <",false,true},{ShipyardBuilderCommand::DevWorldNextBackdrop,"BACKDROP >",false,true}});
        row(l.focusRowY,{{ShipyardBuilderCommand::DevWorldToggleCertifiedOnly,model.devWorld.showCertifiedOnly?"CERTIFIED ONLY":"ALL ASSETS",model.devWorld.showCertifiedOnly,true},{ShipyardBuilderCommand::DevWorldTogglePlayerReference,model.devWorld.showPlayerReference?"PLAYER REF ON":"PLAYER REF OFF",model.devWorld.showPlayerReference,true}});
    }
    else if(effectiveMode==ShipyardWorkspaceMode::Authoring&&model.inspectorTab==ShipyardInspectorTab::Sockets){
        const ShipyardModuleRecord* placedRecord=nullptr;
        if(!model.recipe.modules.empty()){
            const auto pi=std::min(model.selectedPlacedModule,model.recipe.modules.size()-1);
            const auto& id=model.recipe.modules[pi].moduleId;
            for(const auto& record:model.catalog)if(record.source.moduleId==id){placedRecord=&record;break;}
        }
        const bool hasSocket=placedRecord&&!placedRecord->sockets.empty();
        const auto selectedSocket=hasSocket?std::min(model.selectedSocket,placedRecord->sockets.size()-1):0u;
        const std::string socketName=hasSocket?placedRecord->sockets[selectedSocket].name:"NO SOCKET";
        row(l.editRowY,{
            {ShipyardBuilderCommand::InspectorAuthoring,"OPEN TEACH PCG / DEFINITION AUTHORING",false,true}
        });
        row(l.focusRowY,{
            {ShipyardBuilderCommand::PreviousSocket,"PREV",false,hasSocket},
            {ShipyardBuilderCommand::NextSocket,"NEXT",false,hasSocket},
            {ShipyardBuilderCommand::AddSocket,"ADD",false,hasPlaced},
            {ShipyardBuilderCommand::RemoveSocket,"REMOVE",false,hasSocket}
        });
        row(l.moveRowY,{
            {ShipyardBuilderCommand::ToolMove,"MOVE",model.transformTool==ShipyardTransformTool::Move,hasSocket},
            {ShipyardBuilderCommand::ToolRotate,"ROTATE",model.transformTool==ShipyardTransformTool::Rotate,hasSocket},
            {ShipyardBuilderCommand::ToggleTransformSpace,model.transformSpace==ShipyardTransformSpace::View?"CAMERA":(model.transformSpace==ShipyardTransformSpace::Ship?"SHIP":"LOCAL"),false,hasSocket},
            {ShipyardBuilderCommand::ToggleTransformSnap,model.transformSnap?"SNAP":"FREE",model.transformSnap,hasSocket}
        });
        row(l.moveRow2Y,{
            {ShipyardBuilderCommand::MirrorSocketX,"MIRROR SOCKET",false,hasSocket},
            {ShipyardBuilderCommand::CycleSocketType,"TYPE",false,hasSocket},
            {ShipyardBuilderCommand::ResetSocket,"RESET",false,hasSocket}
        });
        row(l.rotateRowY,{
            {ShipyardBuilderCommand::UndoSocketEdit,"UNDO",false,true},
            {ShipyardBuilderCommand::RedoSocketEdit,"REDO",false,true},
            {ShipyardBuilderCommand::SaveSocketOverrides,model.socketOverridesDirty?"SAVE SOCKETS *":"SAVE SOCKETS",model.socketOverridesDirty,true}
        });
        (void)socketName;
    }
    else if(effectiveMode==ShipyardWorkspaceMode::Authoring&&model.inspectorTab==ShipyardInspectorTab::Authoring){
        // Compatibility sentinels keep older authoring regression tests/API
        // callers valid while the visible SOCKETS entry is now top-level.
        add(ShipyardBuilderCommand::InspectorAuthoring,0,-100,-100,0,0,"",true,false);
        add(ShipyardBuilderCommand::AddSocket,0,-100,-100,0,0,"",false,false);
        const ShipyardModuleRecord* record=nullptr;if(!model.recipe.modules.empty()){const auto pi=std::min(model.selectedPlacedModule,model.recipe.modules.size()-1);for(const auto& r:model.catalog)if(r.source.moduleId==model.recipe.modules[pi].moduleId){record=&r;break;}}
        const bool has=record!=nullptr;
        row(l.editRowY,{{ShipyardBuilderCommand::InspectorSockets,"OPEN SOCKET EDITOR",false,true}});
        row(l.focusRowY,{{ShipyardBuilderCommand::PreviousSemantic,"SEMANTIC <",false,has},{ShipyardBuilderCommand::NextSemantic,"> SEMANTIC",false,has}});
        row(l.moveRowY,{{ShipyardBuilderCommand::ToggleGeneratorEligible,record&&record->generatorEligible?"PCG ON":"PCG OFF",record&&record->generatorEligible,has},{ShipyardBuilderCommand::TogglePairedPlacement,record&&record->pairedPlacement?"PAIRED":"SINGLE",record&&record->pairedPlacement,has}});
        row(l.moveRow2Y,{{ShipyardBuilderCommand::CyclePreferredMountFace,"MOUNT FACE",false,has},{ShipyardBuilderCommand::ResetDefinitionOverride,"RESET DEFINITION",false,has}});
        row(l.rotateRowY,{{ShipyardBuilderCommand::SaveDefinitionOverrides,model.definitionOverridesDirty?"SAVE TEACH PCG *":"SAVE TEACH PCG",model.definitionOverridesDirty,true}});
    }
    else if(effectiveMode==ShipyardWorkspaceMode::Systems){
        const std::size_t placedPage=l.placedPageSize;
        const std::size_t psel=model.recipe.modules.empty()?0:std::min(model.selectedPlacedModule,model.recipe.modules.size()-1);
        const std::size_t maxPlacedStart=model.recipe.modules.size()>placedPage?model.recipe.modules.size()-placedPage:0;
        const std::size_t pstart=model.recipe.modules.empty()?0:std::min(model.placedScrollStart,maxPlacedStart);
        auto placedLabel=[&](std::size_t pi){
            for(const auto& rec:model.catalog)if(rec.source.moduleId==model.recipe.modules[pi].moduleId)
                return std::to_string(pi+1)+".  "+FriendlyModuleLabel(rec);
            return std::to_string(pi+1)+".  "+CompactId(model.recipe.modules[pi].moduleId,37);
        };
        for(std::size_t r=0;r<placedPage&&pstart+r<model.recipe.modules.size();++r){
            const auto pi=pstart+r;
            add(ShipyardBuilderCommand::SelectPlaced,static_cast<int>(pi),rx,l.placedListY+r*(rowH+gap),rw,rowH,
                placedLabel(pi),pi==psel);
        }
        const std::string className=ShipClassRoleSystem::ClassName(model.shipClass);
        const std::string sizeName=UniversalKitbashAuthority::SizeName(model.targetModuleSize);
        row(l.classRowY,{
            {ShipyardBuilderCommand::PreviousShipClass,"< CLASS",false,true},
            {ShipyardBuilderCommand::NextShipClass,className+" >",true,true}
        });
        row(l.sizeModeRowY,{
            {ShipyardBuilderCommand::PreviousTargetSize,"< SIZE",false,true},
            {ShipyardBuilderCommand::NextTargetSize,sizeName+" >",true,true},
            {ShipyardBuilderCommand::CycleConstructionMode,std::string("MODE  ")+UniversalConstructionSystem::ModeName(model.constructionMode),true,true}
        });
        row(l.generateRowY,{
            {ShipyardBuilderCommand::PreviousRole,"PREV ROLE",false,true},
            {ShipyardBuilderCommand::NextRole,"NEXT ROLE",false,true},
            {ShipyardBuilderCommand::GenerateVariant,"GENERATE",false,true},
            {ShipyardBuilderCommand::Validate,"VALIDATE",false,true}
        });
        row(l.saveRowY,{
            {ShipyardBuilderCommand::Reset,"RESET SHIP",false,true},
            {ShipyardBuilderCommand::SaveBlueprint,model.validation.valid?"SAVE BLUEPRINT":"SAVE DRAFT",false,hasPlaced},
            {ShipyardBuilderCommand::Apply,model.liveApplyEnabled?"APPLY REFIT":"APPLY WHEN DOCKED",model.liveApplyEnabled&&model.validation.valid,model.liveApplyEnabled&&model.validation.valid}
        });
    }
    else { // Appearance
        row(l.liveryRowY,{
            {ShipyardBuilderCommand::PreviousLiveryPreset,"PRESET  <",false,true},
            {ShipyardBuilderCommand::NextLiveryPreset,">  PRESET",false,true}
        });
        row(l.liverySecondaryRowY,{
            {ShipyardBuilderCommand::PreviousPrimaryPaint,"PRIMARY  <",false,true},
            {ShipyardBuilderCommand::NextPrimaryPaint,">  PRIMARY",false,true}
        });
        row(l.paintSecondaryRowY,{
            {ShipyardBuilderCommand::PreviousSecondaryPaint,"SECONDARY  <",false,true},
            {ShipyardBuilderCommand::NextSecondaryPaint,">  SECONDARY",false,true}
        });
        row(l.paintTrimRowY,{
            {ShipyardBuilderCommand::PreviousTrimPaint,"ACCENT  <",false,true},
            {ShipyardBuilderCommand::NextTrimPaint,">  ACCENT",false,true}
        });
        row(l.decalRowY,{
            {ShipyardBuilderCommand::PreviousDecalPreset,"DECAL  <",false,true},
            {ShipyardBuilderCommand::NextDecalPreset,">  DECAL",false,true},
            {ShipyardBuilderCommand::AddDecal,"ADD DECAL",false,hasPlaced},
            {ShipyardBuilderCommand::RemoveDecal,"REMOVE",false,!model.appearance.decals.empty()}
        });
    }

    return out;
}

ShipyardBuilderControl ShipyardBuilderSystem::HitTest(const ShipyardBuilderRuntimeModel& model,int w,int h,float x,float y){
    const auto controls=BuildControls(model,w,h);
    for(auto it=controls.rbegin();it!=controls.rend();++it)if(it->Contains(x,y))return *it;
    return {};
}

} // namespace subspace
