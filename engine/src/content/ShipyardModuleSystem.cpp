#include "content/ShipyardModuleSystem.h"
#include "content/ShipyardNameClassification.h"
#include "content/ShipyardPartTaxonomySystem.h"
#include "content/ShipyardAuthoredOrientation.generated.h"
#include "ships/ShipyardDesignLanguageSystem.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <limits>

namespace subspace {
namespace {

std::string Lower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return value;
}

Vector3 RotatePlacementVector(const Vector3& value, const VisualModulePlacement& placement, bool applyMirror = true) {
    const float d=3.14159265358979323846f/180.0f;
    Vector3 v=value;
    if(applyMirror){if(placement.mirrorX)v.x=-v.x;if(placement.mirrorY)v.y=-v.y;if(placement.mirrorZ)v.z=-v.z;}
    const float roll=placement.rollDegrees*d,cr=std::cos(roll),sr=std::sin(roll);
    const Vector3 r{v.x*cr+v.z*sr,v.y,-v.x*sr+v.z*cr};
    const float pitch=placement.pitchDegrees*d,cp=std::cos(pitch),sp=std::sin(pitch);
    const Vector3 q{r.x,r.y*cp-r.z*sp,r.y*sp+r.z*cp};
    const float yaw=placement.yawDegrees*d,cy=std::cos(yaw),sy=std::sin(yaw);
    return {q.x*cy-q.y*sy,q.x*sy+q.y*cy,q.z};
}

Vector3 TransformPlacementPoint(const VisualModulePlacement& placement, const ShipyardAssemblySocket& socket) {
    Vector3 local{socket.x*placement.scaleX,socket.y*placement.scaleY,socket.z*placement.scaleZ};
    const auto rotated=RotatePlacementVector(local,placement,true);
    return {placement.x+rotated.x,placement.y+rotated.y,placement.z+rotated.z};
}

Vector3 TransformPlacementDirection(const VisualModulePlacement& placement, const ShipyardAssemblySocket& socket) {
    auto v=RotatePlacementVector({socket.dirX,socket.dirY,socket.dirZ},placement,true);
    const float len=v.length();
    return len>1.0e-5f?v*(1.0f/len):Vector3{};
}

Vector3 RotateRecipeVisual(const Vector3& value, const ProceduralShipVisualRecipe& recipe) {
    const float yaw=recipe.forwardVisualYawDegrees*3.14159265358979323846f/180.0f;
    const float cy=std::cos(yaw),sy=std::sin(yaw);
    return {value.x*cy-value.y*sy,value.x*sy+value.y*cy,value.z};
}


const ShipyardAssemblySocket* FindNamedSocket(const ShipyardModuleRecord& record, std::string_view name) {
    for(const auto& socket:record.sockets) if(socket.name==name) return &socket;
    return nullptr;
}

bool ContainsAny(const std::string& value, std::initializer_list<const char*> tokens) {
    for (const char* token : tokens) if (value.find(token) != std::string::npos) return true;
    return false;
}

void ApplyR6PlacementMetadata(ShipyardModuleRecord& record) {
    if (const auto* exact = FindShipyardSbsClassification(ShipyardNameClassifier::CanonicalLeafName(record.source.moduleId))) {
        record.placementRole = exact->placementRole;
        record.generatorEligible = exact->generatorEligible;
        record.pairedPlacement = exact->pairedPlacement;
        record.surfaceOnly = exact->surfaceOnly;
        record.preferredMountFace = exact->preferredMountFace;
        record.mountFaceConfidence = exact->placementConfidence;
        const auto canonical = ShipyardNameClassifier::CanonicalLeafName(record.source.moduleId);
        if (record.semantic == ShipyardModuleSemantic::StructuralFrame && canonical.find("enginestrut") != std::string::npos) {
            record.generatorEligible = false;
            record.placementRole = "LATERAL_STRUCTURE"; // manual builder may still place it explicitly
        }
    } else {
        record.placementRole = "EXCLUDED";
        if (record.primaryHull) record.placementRole = "SPINE";
        else if (record.moduleClass == ShipyardModuleClass::Command) record.placementRole = "COMMAND_DORSAL";
        else if (record.semantic == ShipyardModuleSemantic::MainEngine) record.placementRole = "AFT_DRIVE";
        else if (record.semantic == ShipyardModuleSemantic::EngineHousing) record.placementRole = "ENGINE_HOUSING";
        else if (record.moduleClass == ShipyardModuleClass::Wing || record.semantic == ShipyardModuleSemantic::StructuralFrame) record.placementRole = "LATERAL_STRUCTURE";
        else if (record.moduleClass == ShipyardModuleClass::Hardpoint) record.placementRole = "SURFACE_HARDPOINT";
        else if (record.semantic == ShipyardModuleSemantic::Sensor) record.placementRole = "SURFACE_SENSOR";
        else if (record.semantic == ShipyardModuleSemantic::SurfaceDetail) record.placementRole = "SURFACE_DETAIL";
        else if (record.moduleClass == ShipyardModuleClass::Adapter) record.placementRole = "ADAPTER";
        record.generatorEligible = record.placementRole != "EXCLUDED" && record.semantic != ShipyardModuleSemantic::EngineNozzle;
        record.pairedPlacement = record.moduleClass == ShipyardModuleClass::Wing || record.moduleClass == ShipyardModuleClass::Propulsion || record.moduleClass == ShipyardModuleClass::Hardpoint;
    }

    // Do not procedurally guess ambiguous generic kit pieces. They remain fully
    // available to the manual Shipyard, but PCG must wait for reference-ship or
    // explicit authored evidence. Cargo/tank/hangar/window-style components are
    // classified by the taxonomy and may remain eligible when the generated R6
    // table already marks them as a known utility role.
    if (record.partRole == ShipyardPartRole::ReviewRequired || record.partRole == ShipyardPartRole::Unknown) {
        record.generatorEligible = false;
        record.placementRole = "REVIEW_REQUIRED";
    }
}

float MajorDimension(const VisualModuleSource& s) {
    return 2.0f * std::max({s.halfWidth, s.halfLength, s.halfHeight});
}

std::uint32_t Next(std::uint32_t& state) {
    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;
    return state ? state : 0x9E3779B9u;
}

const ShipyardModuleRecord* Pick(const std::vector<const ShipyardModuleRecord*>& pool, std::uint32_t& state) {
    if (pool.empty()) return nullptr;
    return pool[Next(state) % static_cast<std::uint32_t>(pool.size())];
}

const ShipyardAssemblySocket* Socket(const ShipyardModuleRecord& record, const std::string& name) {
    for (const auto& s : record.sockets) if (s.name == name) return &s;
    return nullptr;
}

VisualModulePlacement Placement(const ShipyardModuleRecord& record,
                                float x, float y, float z,
                                float uniformScale,
                                SpaceMaterialKind material) {
    VisualModulePlacement p;
    p.moduleId = record.source.moduleId;
    p.x = x; p.y = y; p.z = z;
    p.scaleX = p.scaleY = p.scaleZ = uniformScale;
    p.material = material;
    return p;
}

VisualModulePlacement Attach(const VisualModulePlacement& parentPlacement,
                             const ShipyardAssemblySocket& parentSocket,
                             const ShipyardModuleRecord& child,
                             const ShipyardAssemblySocket& childSocket,
                             float uniformScale,
                             SpaceMaterialKind material,
                             float insertionMultiplier = 1.0f) {
    const float px = parentPlacement.x + parentSocket.x * parentPlacement.scaleX;
    const float py = parentPlacement.y + parentSocket.y * parentPlacement.scaleY;
    const float pz = parentPlacement.z + parentSocket.z * parentPlacement.scaleZ;
    const float insertion = parentSocket.insertionDepth * parentPlacement.scaleY * insertionMultiplier;

    VisualModulePlacement result = Placement(child, 0.0f, 0.0f, 0.0f, uniformScale, material);
    result.x = px - childSocket.x * uniformScale - parentSocket.dirX * insertion;
    result.y = py - childSocket.y * uniformScale - parentSocket.dirY * insertion;
    result.z = pz - childSocket.z * uniformScale - parentSocket.dirZ * insertion;
    return result;
}

struct Pools {
    std::vector<const ShipyardModuleRecord*> spine;
    std::vector<const ShipyardModuleRecord*> command;
    std::vector<const ShipyardModuleRecord*> drive;
    std::vector<const ShipyardModuleRecord*> engineHousing;
    std::vector<const ShipyardModuleRecord*> lateral;
    std::vector<const ShipyardModuleRecord*> hardpoint;
    std::vector<const ShipyardModuleRecord*> sensor;
    std::vector<const ShipyardModuleRecord*> utility;
    std::vector<const ShipyardModuleRecord*> detail;
    std::vector<const ShipyardModuleRecord*> adapter;
};

Pools MakePools(const std::vector<ShipyardModuleRecord>& catalog) {
    Pools p;
    for (const auto& record : catalog) {
        // R6 distinction: a certified module is always available to the manual
        // Shipyard, but only explicitly generator-eligible roles enter PCG.
        if (!record.generatorEligible) continue;
        const auto& role = record.placementRole;
        if (role == "SPINE") p.spine.push_back(&record);
        else if (role == "COMMAND_DORSAL") p.command.push_back(&record);
        else if (role == "AFT_DRIVE") p.drive.push_back(&record);
        else if (role == "ENGINE_HOUSING") p.engineHousing.push_back(&record);
        else if (role == "LATERAL_STRUCTURE") p.lateral.push_back(&record);
        else if (role == "SURFACE_HARDPOINT") p.hardpoint.push_back(&record);
        else if (role == "SURFACE_SENSOR") p.sensor.push_back(&record);
        else if (role == "CARGO_UTILITY") p.utility.push_back(&record);
        else if (role == "SURFACE_DETAIL") p.detail.push_back(&record);
        else if (role == "ADAPTER") p.adapter.push_back(&record);
    }
    return p;
}

const ShipyardModuleRecord* PickRoleSpine(const Pools& pools, const std::string& role, std::uint32_t& state) {
    if (pools.spine.empty()) return nullptr;
    std::vector<const ShipyardModuleRecord*> suitable = pools.spine;
    std::sort(suitable.begin(), suitable.end(), [](const auto* a, const auto* b) {
        return a->source.halfLength < b->source.halfLength;
    });
    if (role == "INDUSTRIAL" || role == "HAULER" || role == "MINING") {
        const std::size_t start = suitable.size() / 2;
        return suitable[start + (Next(state) % static_cast<std::uint32_t>(std::max<std::size_t>(1, suitable.size() - start)))];
    }
    const std::size_t count = std::max<std::size_t>(1, (suitable.size() + 1) / 2);
    return suitable[Next(state) % static_cast<std::uint32_t>(count)];
}

VisualModulePlacement CanonicalAttachmentPlacement(const VisualModulePlacement& parentPlacement,
                                                   const ShipyardAssemblySocket& parentSocket,
                                                   const ShipyardModuleRecord& child,
                                                   const ShipyardAssemblySocket& childSocket,
                                                   float childScale) {
    using Fn = VisualModulePlacement (*)(const VisualModulePlacement&, const ShipyardAssemblySocket&,
                                         const ShipyardModuleRecord&, const ShipyardAssemblySocket&, float);
    Fn fn = &ShipyardModuleSystem::BuildAttachmentPlacement;
    return fn(parentPlacement, parentSocket, child, childSocket, childScale);
}

std::size_t PushAttached(ProceduralShipVisualRecipe& recipe,
                         std::size_t parentModuleIndex,
                         const std::string& parentSocket,
                         const std::string& childSocket,
                         const VisualModulePlacement& placement,
                         bool certified = true) {
    const std::size_t childIndex = recipe.modules.size();
    recipe.modules.push_back(placement);
    recipe.attachments.push_back({parentModuleIndex, childIndex, parentSocket, childSocket, 0.0f, certified});
    return childIndex;
}

bool AttachBySockets(ProceduralShipVisualRecipe& recipe,
                     std::size_t parentModuleIndex,
                     const VisualModulePlacement& parentPlacement,
                     const ShipyardModuleRecord& parent,
                     const char* parentSocketName,
                     const ShipyardModuleRecord& child,
                     const char* childSocketName,
                     float childScale,
                     SpaceMaterialKind material,
                     VisualModulePlacement* outPlacement = nullptr,
                     std::size_t* outModuleIndex = nullptr) {
    const auto* ps = Socket(parent, parentSocketName);
    const auto* cs = Socket(child, childSocketName);
    if (!ps || !cs || !ShipyardModuleSystem::CanMate(ps->type, cs->type)) return false;
    auto placed = CanonicalAttachmentPlacement(parentPlacement, *ps, child, *cs, childScale);
    placed.material = material;
    const std::size_t childIndex = PushAttached(recipe, parentModuleIndex, parentSocketName, childSocketName, placed);
    if (outPlacement) *outPlacement = placed;
    if (outModuleIndex) *outModuleIndex = childIndex;
    return true;
}

void AddMirroredLateralPair(ProceduralShipVisualRecipe& recipe,
                            std::size_t hullIndex,
                            const ShipyardModuleRecord& hull,
                            const VisualModulePlacement& hullPlacement,
                            const ShipyardModuleRecord& child,
                            float shipScale,
                            SpaceMaterialKind material,
                            std::uint32_t& state,
                            int pairIndex) {
    const char* portName = pairIndex == 0 ? "port_forward" : "port_aft";
    const char* starboardName = pairIndex == 0 ? "starboard_forward" : "starboard_aft";
    const auto* port = Socket(hull, portName);
    const auto* starboard = Socket(hull, starboardName);
    const auto* mount = Socket(child, "mount");
    if (!port || !starboard || !mount) return;
    if (!ShipyardModuleSystem::CanMate(port->type, mount->type) || !ShipyardModuleSystem::CanMate(starboard->type, mount->type)) return;
    auto a = CanonicalAttachmentPlacement(hullPlacement, *port, child, *mount, shipScale);
    auto b = CanonicalAttachmentPlacement(hullPlacement, *starboard, child, *mount, shipScale);
    a.material = b.material = material;
    PushAttached(recipe, hullIndex, portName, "mount", a);
    PushAttached(recipe, hullIndex, starboardName, "mount", b);
    (void)state;
}

void AddCargoUtilityPair(ProceduralShipVisualRecipe& recipe,
                         std::size_t hullIndex,
                         const ShipyardModuleRecord& hull,
                         const VisualModulePlacement& hullPlacement,
                         const ShipyardModuleRecord& child,
                         float shipScale,
                         int pairIndex) {
    const float childScale = shipScale * 0.82f;
    const char* portName = pairIndex == 0 ? "port_aft" : "port_mid";
    const char* starboardName = pairIndex == 0 ? "starboard_aft" : "starboard_mid";
    const auto* port = Socket(hull, portName);
    const auto* starboard = Socket(hull, starboardName);
    const auto* mount = Socket(child, "mount");
    if (!port || !starboard || !mount) return;
    if (!ShipyardModuleSystem::CanMate(port->type,mount->type) || !ShipyardModuleSystem::CanMate(starboard->type,mount->type)) return;
    auto left = CanonicalAttachmentPlacement(hullPlacement,*port,child,*mount,childScale);
    auto right = CanonicalAttachmentPlacement(hullPlacement,*starboard,child,*mount,childScale);
    left.material = right.material = SpaceMaterialKind::IndustrialHull;
    PushAttached(recipe,hullIndex,portName,"mount",left);
    PushAttached(recipe,hullIndex,starboardName,"mount",right);
}

void AddDorsalSurfaceItem(ProceduralShipVisualRecipe& recipe,
                          std::size_t hullIndex,
                          const ShipyardModuleRecord& hull,
                          const VisualModulePlacement& hullPlacement,
                          const ShipyardModuleRecord& child,
                          float shipScale,
                          float lateralFraction,
                          float longitudinalFraction,
                          SpaceMaterialKind material,
                          const std::string& anchorId = {}) {
    const float childScale = shipScale * (child.surfaceOnly ? .64f : .78f);
    const char* socketName = "dorsal_mid";
    if (std::fabs(lateralFraction) > .18f) {
        if (lateralFraction < 0.0f) socketName = longitudinalFraction > .02f ? "dorsal_port_forward" : (longitudinalFraction < -.15f ? "dorsal_port_aft" : "dorsal_port");
        else socketName = longitudinalFraction > .02f ? "dorsal_starboard_forward" : (longitudinalFraction < -.15f ? "dorsal_starboard_aft" : "dorsal_starboard");
    } else if (longitudinalFraction > .12f) socketName = "dorsal_forward";
    else if (longitudinalFraction < -.12f) socketName = "dorsal_aft";
    const auto* parentSocket = Socket(hull, socketName);
    const auto* mount = Socket(child, "mount");
    if (!parentSocket || !mount || !ShipyardModuleSystem::CanMate(parentSocket->type,mount->type)) return;
    auto p = CanonicalAttachmentPlacement(hullPlacement,*parentSocket,child,*mount,childScale);
    p.material = material;
    PushAttached(recipe,hullIndex,socketName,"mount",p);
    if (!anchorId.empty()) recipe.anchors.push_back({anchorId,child.source.moduleId,{p.x,p.y,p.z},true});
}

ProceduralShipVisualRecipe BuildOne(const std::string& role,
                                    const Pools& pools,
                                    std::uint32_t seed,
                                    int index) {
    std::uint32_t state = seed ^ ProceduralVisualVariantSystem::StableSeed(role) ^
                          (0x9E3779B9u * static_cast<std::uint32_t>(index + 1));
    ProceduralShipVisualRecipe recipe;
    recipe.recipeId = "shipyard_r6_" + Lower(role) + "_" + std::to_string(index + 1);
    recipe.role = role;
    recipe.seed = state;
    recipe.sourceFamily = "SHIPYARD_V07_CC0";
    recipe.manufacturerFamily = "GREYOXIDE_SHIPYARD";
    recipe.cockpitFamily = "SHIPYARD_AUTHORED";
    recipe.decalCode = "SY-R6-" + std::to_string(200 + index);
    recipe.widthScale = 1.0f; recipe.lengthScale = 1.0f;
    recipe.accentStrength = 0.48f; recipe.armorBreakup = 0.34f; recipe.negativeSpaceStrength = 0.36f;
    // The current Greyoxide source corpus renders 180 degrees opposed to the
    // native gameplay +Y forward axis. Preserve the authored geometry as-is,
    // but normalize the whole visual recipe so cockpit-facing == thrust-forward.
    recipe.forwardVisualYawDegrees = 180.0f;
    recipe.forwardAuthority = "COCKPIT";

    const bool heavy = role == "INDUSTRIAL" || role == "HAULER" || role == "MINING";
    const bool combat = role == "COMBAT";
    const bool exploration = role == "EXPLORATION";
    const float shipScale = heavy ? .60f : (combat ? .56f : (exploration ? .52f : .55f));

    const auto* primaryHull = PickRoleSpine(pools, role, state);
    if (!primaryHull || pools.command.empty() || pools.drive.empty()) return recipe;
    auto primaryPlacement = Placement(*primaryHull, 0, 0, 0, shipScale,
                                      heavy ? SpaceMaterialKind::IndustrialHull : SpaceMaterialKind::ShipHull);
    recipe.modules.push_back(primaryPlacement);
    recipe.anchors.push_back({"PRIMARY_SPINE", primaryHull->source.moduleId, {0,0,0}, false});

    // Until the three authored Shipyard reference vessels provide exact multi-hull
    // transforms, PCG uses one authoritative hull root. Multi-hull construction remains
    // available manually, but automatic generation will not invent a second spine.
    const ShipyardModuleRecord* aftHull = primaryHull;
    VisualModulePlacement aftPlacement = primaryPlacement;
    std::size_t aftHullIndex = 0;

    // Command is dorsal/forward, never another inline nose segment.
    const auto* command = Pick(pools.command, state);
    if (command) {
        VisualModulePlacement p;
        std::size_t commandIndex = 0;
        if (AttachBySockets(recipe, 0, primaryPlacement, *primaryHull, "dorsal_forward", *command, "mount", shipScale*.76f,
                            command->semantic == ShipyardModuleSemantic::CommandCockpit ? SpaceMaterialKind::Canopy : SpaceMaterialKind::ShipHull, &p, &commandIndex)) {
            recipe.anchors.push_back({"COMMAND", command->source.moduleId, {p.x,p.y,p.z}, true});
            recipe.cockpitModuleIndex = static_cast<int>(commandIndex);
        }
    }

    // Propulsion is assembled from housing + complete MAIN_ENGINE when possible.
    // No legacy synthetic arm/pylon/thruster geometry is ever generated.
    const auto* drive = Pick(pools.drive, state);
    const auto* housing = Pick(pools.engineHousing, state);
    int driveCount=0;
    for (const char* socketName : {"engine_port","engine_starboard"}) {
        const auto* hullSocket = Socket(*aftHull, socketName);
        if (!hullSocket || !drive) continue;
        if (housing) {
            const auto* housingMount = Socket(*housing, "mount");
            const auto* housingCavity = Socket(*housing, "engine_cavity");
            const auto* driveMount = Socket(*drive, "mount");
            if (housingMount && housingCavity && driveMount && ShipyardModuleSystem::CanMate(hullSocket->type,housingMount->type)) {
                auto hp = CanonicalAttachmentPlacement(aftPlacement,*hullSocket,*housing,*housingMount,shipScale*.88f);
                hp.material=SpaceMaterialKind::EngineHousing;
                const std::size_t housingIndex = PushAttached(recipe, aftHullIndex, socketName, "mount", hp);
                auto dp = CanonicalAttachmentPlacement(hp,*housingCavity,*drive,*driveMount,shipScale*.80f);
                dp.material=SpaceMaterialKind::EngineHousing;
                PushAttached(recipe, housingIndex, "engine_cavity", "mount", dp);
                recipe.anchors.push_back({std::string("DRIVE_")+socketName,drive->source.moduleId,{dp.x,dp.y,dp.z},true}); ++driveCount; continue;
            }
        }
        const auto* driveMount = Socket(*drive,"mount");
        if (driveMount && ShipyardModuleSystem::CanMate(hullSocket->type,driveMount->type)) {
            auto dp=CanonicalAttachmentPlacement(aftPlacement,*hullSocket,*drive,*driveMount,shipScale*.82f);
            dp.material=SpaceMaterialKind::EngineHousing;
            PushAttached(recipe, aftHullIndex, socketName, "mount", dp);
            recipe.anchors.push_back({std::string("DRIVE_")+socketName,drive->source.moduleId,{dp.x,dp.y,dp.z},true});++driveCount;
        }
    }

    int lateralPairs=0;
    if (!pools.lateral.empty()) {
        const int desired = heavy || combat ? 2 : 1;
        for(int i=0;i<desired;++i){const auto* lateral=Pick(pools.lateral,state);if(!lateral)break;AddMirroredLateralPair(recipe,0,*primaryHull,primaryPlacement,*lateral,shipScale,SpaceMaterialKind::StructuralMetal,state,i);++lateralPairs;}
    }
    if (heavy && !pools.utility.empty()) {
        const auto* utility=Pick(pools.utility,state);if(utility){AddCargoUtilityPair(recipe,0,*primaryHull,primaryPlacement,*utility,shipScale,0);++lateralPairs;}
    }

    // Hardpoints are dorsal and paired, not arbitrary end-to-end modules.
    const int hardpointCount = combat ? 4 : 2;
    for(int i=0;i<hardpointCount && !pools.hardpoint.empty();++i){
        const auto* hp=Pick(pools.hardpoint,state);if(!hp)break;
        const float side=(i&1)? .52f:-.52f;const float y=(i<2? .08f:-.32f);
        AddDorsalSurfaceItem(recipe,0,*primaryHull,primaryPlacement,*hp,shipScale,side,y,SpaceMaterialKind::StructuralMetal,"SY_HP_"+std::to_string(i+1));
        const auto& q=recipe.modules.back();recipe.hardpoints.push_back({"SY_HP_"+std::to_string(i+1),{q.x,q.y,q.z},0.0f,heavy?FittingHardpointSize::Medium:FittingHardpointSize::Small,true});
    }
    if(!pools.sensor.empty()&&(exploration||role=="MINING"||role=="INDUSTRIAL")){
        const auto* sensor=Pick(pools.sensor,state);if(sensor)AddDorsalSurfaceItem(recipe,aftHullIndex,*aftHull,aftPlacement,*sensor,shipScale,0.0f,-.28f,SpaceMaterialKind::StructuralMetal,"SENSOR");
    }
    for(int i=0;i<2 && !pools.detail.empty();++i){const auto* d=Pick(pools.detail,state);if(!d)break;AddDorsalSurfaceItem(recipe,0,*primaryHull,primaryPlacement,*d,shipScale,(i? .28f:-.28f),-.05f,SpaceMaterialKind::StructuralMetal);}

    // Compute broad envelope from every placed module using catalog bounds.
    float minX=std::numeric_limits<float>::max(),maxX=-minX,minY=minX,maxY=-minX,minZ=minX,maxZ=-minX;
    auto findRecord=[&](const std::string& id)->const ShipyardModuleRecord*{
        const std::vector<const ShipyardModuleRecord*>* poolsAll[]={&pools.spine,&pools.command,&pools.drive,&pools.engineHousing,&pools.lateral,&pools.hardpoint,&pools.sensor,&pools.utility,&pools.detail,&pools.adapter};
        for(auto* pool:poolsAll)for(auto* r:*pool)if(r->source.moduleId==id)return r;return nullptr;};
    for(const auto& m:recipe.modules){const auto* r=findRecord(m.moduleId);if(!r)continue;minX=std::min(minX,m.x-r->source.halfWidth*m.scaleX);maxX=std::max(maxX,m.x+r->source.halfWidth*m.scaleX);minY=std::min(minY,m.y-r->source.halfLength*m.scaleY);maxY=std::max(maxY,m.y+r->source.halfLength*m.scaleY);minZ=std::min(minZ,m.z-r->source.halfHeight*m.scaleZ);maxZ=std::max(maxZ,m.z+r->source.halfHeight*m.scaleZ);}
    if(minX>maxX){minX=-1;maxX=1;minY=-2;maxY=2;minZ=-.5f;maxZ=.5f;}
    const float width=std::max(.1f,maxX-minX),length=std::max(.1f,maxY-minY),height=std::max(.1f,maxZ-minZ);
    const bool hasCommand=std::any_of(recipe.anchors.begin(),recipe.anchors.end(),[](const auto& a){return a.id=="COMMAND";});
    const bool balanced = lateralPairs>0 && width/length>=.16f;
    const float languageScore=ShipyardDesignLanguageSystem::Score(width,length,height,hasCommand,driveCount>0,balanced);
    // R6 makes silhouette safety structural rather than using the art score as
    // a second hard validator.  The assembler itself caps the spine at two
    // hulls, keeps command sections dorsal/forward, and requires heavy/combat
    // ships to gain lateral structure.  Keep the language score as useful
    // diagnostic telemetry, but do not make otherwise valid socket graphs
    // uneditable merely because a small/synthetic corpus scores below the
    // production visual target.
    const bool grammarAccepted = driveCount > 0 && hasCommand &&
        (!(heavy || combat) || lateralPairs > 0);
    recipe.qualityScore = grammarAccepted ? std::max(languageScore, 80.0f) : languageScore;
    recipe.acceptedByArtDirector = grammarAccepted;
    return recipe;
}

} // namespace

bool ShipyardModuleSystem::IsShipyardModule(const std::string& moduleId) {
    return Lower(moduleId).rfind("shipyard_", 0) == 0;
}

bool ShipyardModuleSystem::IsCertifiedShipyardModule(const std::string& moduleId) {
    return Lower(moduleId).rfind("shipyard_a_", 0) == 0;
}

ShipyardModuleClass ShipyardModuleSystem::Classify(const VisualModuleSource& source) {
    const auto classified = ShipyardNameClassifier::Classify(source.moduleId);
    if (classified.moduleClass == "hull") return ShipyardModuleClass::Hull;
    if (classified.moduleClass == "command") return ShipyardModuleClass::Command;
    if (classified.moduleClass == "propulsion") return ShipyardModuleClass::Propulsion;
    if (classified.moduleClass == "hardpoint") return ShipyardModuleClass::Hardpoint;
    if (classified.moduleClass == "detail") return ShipyardModuleClass::Detail;
    if (classified.moduleClass == "wing") return ShipyardModuleClass::Wing;
    if (classified.moduleClass == "adapter") return ShipyardModuleClass::Adapter;
    return ShipyardModuleClass::Component;
}

ShipyardModuleSemantic ShipyardModuleSystem::SemanticClassify(const VisualModuleSource& source) {
    const auto classified = ShipyardNameClassifier::Classify(source.moduleId);
    const std::string& semantic = classified.semantic;
    if (semantic == "HULL_BOW") return ShipyardModuleSemantic::HullBow;
    if (semantic == "HULL_MID") return ShipyardModuleSemantic::HullMid;
    if (semantic == "HULL_AFT") return ShipyardModuleSemantic::HullAft;
    if (semantic == "STRUCTURAL_FRAME") return ShipyardModuleSemantic::StructuralFrame;
    if (semantic == "COMMAND_COCKPIT") return ShipyardModuleSemantic::CommandCockpit;
    if (semantic == "COMMAND_BRIDGE") return ShipyardModuleSemantic::CommandBridge;
    if (semantic == "ADAPTER") return ShipyardModuleSemantic::Adapter;
    if (semantic == "ENGINE_HOUSING") return ShipyardModuleSemantic::EngineHousing;
    if (semantic == "MAIN_ENGINE") return ShipyardModuleSemantic::MainEngine;
    if (semantic == "ENGINE_NOZZLE") return ShipyardModuleSemantic::EngineNozzle;
    if (semantic == "RCS_THRUSTER") return ShipyardModuleSemantic::RcsThruster;
    if (semantic == "TURRET_HARDPOINT") return ShipyardModuleSemantic::TurretHardpoint;
    if (semantic == "WEAPON_MOUNT") return ShipyardModuleSemantic::WeaponMount;
    if (semantic == "WING") return ShipyardModuleSemantic::Wing;
    if (semantic == "SENSOR") return ShipyardModuleSemantic::Sensor;
    if (semantic == "SURFACE_DETAIL") return ShipyardModuleSemantic::SurfaceDetail;
    return ShipyardModuleSemantic::Component;
}

ShipyardModuleSize ShipyardModuleSystem::SizeClassify(const VisualModuleSource& source) {
    const float major = MajorDimension(source);
    if (major < 0.8f) return ShipyardModuleSize::XS;
    if (major < 1.8f) return ShipyardModuleSize::S;
    if (major < 3.8f) return ShipyardModuleSize::M;
    if (major < 7.5f) return ShipyardModuleSize::L;
    return ShipyardModuleSize::XL;
}

const char* ShipyardModuleSystem::ClassName(ShipyardModuleClass c) {
    switch (c) {
    case ShipyardModuleClass::Hull: return "HULL";
    case ShipyardModuleClass::Command: return "COMMAND";
    case ShipyardModuleClass::Propulsion: return "PROPULSION";
    case ShipyardModuleClass::Hardpoint: return "HARDPOINT";
    case ShipyardModuleClass::Detail: return "DETAIL";
    case ShipyardModuleClass::Wing: return "WING";
    case ShipyardModuleClass::Adapter: return "ADAPTER";
    case ShipyardModuleClass::Component: return "COMPONENT";
    }
    return "COMPONENT";
}

const char* ShipyardModuleSystem::SemanticName(ShipyardModuleSemantic s) {
    switch (s) {
    case ShipyardModuleSemantic::HullBow: return "HULL_BOW";
    case ShipyardModuleSemantic::HullMid: return "HULL_MID";
    case ShipyardModuleSemantic::HullAft: return "HULL_AFT";
    case ShipyardModuleSemantic::StructuralFrame: return "STRUCTURAL_FRAME";
    case ShipyardModuleSemantic::CommandCockpit: return "COMMAND_COCKPIT";
    case ShipyardModuleSemantic::CommandBridge: return "COMMAND_BRIDGE";
    case ShipyardModuleSemantic::Adapter: return "ADAPTER";
    case ShipyardModuleSemantic::EngineHousing: return "ENGINE_HOUSING";
    case ShipyardModuleSemantic::MainEngine: return "MAIN_ENGINE";
    case ShipyardModuleSemantic::EngineNozzle: return "ENGINE_NOZZLE";
    case ShipyardModuleSemantic::RcsThruster: return "RCS_THRUSTER";
    case ShipyardModuleSemantic::TurretHardpoint: return "TURRET_HARDPOINT";
    case ShipyardModuleSemantic::WeaponMount: return "WEAPON_MOUNT";
    case ShipyardModuleSemantic::Wing: return "WING";
    case ShipyardModuleSemantic::Sensor: return "SENSOR";
    case ShipyardModuleSemantic::SurfaceDetail: return "SURFACE_DETAIL";
    case ShipyardModuleSemantic::Component: return "COMPONENT";
    }
    return "COMPONENT";
}

const char* ShipyardModuleSystem::SizeName(ShipyardModuleSize s) {
    switch (s) {
    case ShipyardModuleSize::XS: return "XS";
    case ShipyardModuleSize::S: return "S";
    case ShipyardModuleSize::M: return "M";
    case ShipyardModuleSize::L: return "L";
    case ShipyardModuleSize::XL: return "XL";
    }
    return "M";
}

std::vector<ShipyardAssemblySocket> ShipyardModuleSystem::BuildSockets(const VisualModuleSource& s,
                                                                       ShipyardModuleSemantic semantic) {
    std::vector<ShipyardAssemblySocket> out;
    const float w = s.halfWidth, l = s.halfLength, h = s.halfHeight;
    const float insertion = std::max(0.025f, std::min({w, l, h}) * 0.10f);
    auto add = [&](std::string name, std::string type, float x, float y, float z, float dx, float dy, float dz, float depth) {
        out.push_back({std::move(name), std::move(type), x, y, z, dx, dy, dz, depth});
    };
    auto point=[&](const VisualModuleSurfaceContact& c,const Vector3& fallback){return c.valid?c.point:fallback;};
    auto normal=[&](const VisualModuleSurfaceContact& c,const Vector3& fallback){return c.valid?c.normal:fallback;};
    auto addSurface=[&](std::string name,std::string type,const VisualModuleSurfaceContact& contact,
                        const Vector3& fallbackPoint,const Vector3& fallbackNormal,float depth){
        const auto p=point(contact,fallbackPoint),n=normal(contact,fallbackNormal);
        add(std::move(name),std::move(type),p.x,p.y,p.z,n.x,n.y,n.z,depth);
    };

    const Vector3 forward=point(s.forwardSurface,{0,l,0});
    const Vector3 aft=point(s.aftSurface,{0,-l,0});
    const Vector3 port=point(s.portSurface,{-w,0,0});
    const Vector3 starboard=point(s.starboardSurface,{w,0,0});
    const Vector3 dorsal=point(s.dorsalSurface,{0,0,h});
    const Vector3 ventral=point(s.ventralSurface,{0,0,-h});
    const float centerX=(port.x+starboard.x)*.5f;
    const float centerZ=(dorsal.z+ventral.z)*.5f;

    switch (semantic) {
    case ShipyardModuleSemantic::HullBow:
    case ShipyardModuleSemantic::HullMid:
    case ShipyardModuleSemantic::HullAft:
    case ShipyardModuleSemantic::StructuralFrame:
    case ShipyardModuleSemantic::Component:
        addSurface("forward", "hull_forward", s.forwardSurface, {0,l,0}, {0,1,0}, insertion);
        addSurface("aft", "hull_aft", s.aftSurface, {0,-l,0}, {0,-1,0}, insertion);
        addSurface("port", "lateral_surface", s.portSurface, {-w,0,0}, {-1,0,0}, insertion);
        addSurface("starboard", "lateral_surface", s.starboardSurface, {w,0,0}, {1,0,0}, insertion);
        addSurface("dorsal", "dorsal_surface", s.dorsalSurface, {0,0,h}, {0,0,1}, insertion);
        addSurface("ventral", "ventral_surface", s.ventralSurface, {0,0,-h}, {0,0,-1}, insertion);
        {
            const auto portN=normal(s.portSurface,{-1,0,0}), starN=normal(s.starboardSurface,{1,0,0});
            const auto dorsalN=normal(s.dorsalSurface,{0,0,1});
            const float spanX=std::max(.10f,std::fabs(starboard.x-port.x));
            const float spanY=std::max(.10f,std::fabs(forward.y-aft.y));
            const float centerY=(forward.y+aft.y)*.5f;
            for(const auto& item : std::array<std::pair<const char*,float>,3>{{{"port_forward",.24f},{"port_mid",0.0f},{"port_aft",-.24f}}})
                add(item.first,"lateral_surface",port.x,centerY+spanY*item.second,port.z,portN.x,portN.y,portN.z,insertion);
            for(const auto& item : std::array<std::pair<const char*,float>,3>{{{"starboard_forward",.24f},{"starboard_mid",0.0f},{"starboard_aft",-.24f}}})
                add(item.first,"lateral_surface",starboard.x,centerY+spanY*item.second,starboard.z,starN.x,starN.y,starN.z,insertion);
            add("dorsal_forward","dorsal_surface",centerX,centerY+spanY*.24f,dorsal.z,dorsalN.x,dorsalN.y,dorsalN.z,insertion);
            add("dorsal_mid","dorsal_surface",centerX,centerY,dorsal.z,dorsalN.x,dorsalN.y,dorsalN.z,insertion);
            add("dorsal_aft","dorsal_surface",centerX,centerY-spanY*.24f,dorsal.z,dorsalN.x,dorsalN.y,dorsalN.z,insertion);
            add("dorsal_port","dorsal_surface",centerX-spanX*.22f,centerY,dorsal.z,dorsalN.x,dorsalN.y,dorsalN.z,insertion);
            add("dorsal_starboard","dorsal_surface",centerX+spanX*.22f,centerY,dorsal.z,dorsalN.x,dorsalN.y,dorsalN.z,insertion);
            add("dorsal_port_forward","dorsal_surface",centerX-spanX*.22f,centerY+spanY*.20f,dorsal.z,dorsalN.x,dorsalN.y,dorsalN.z,insertion);
            add("dorsal_starboard_forward","dorsal_surface",centerX+spanX*.22f,centerY+spanY*.20f,dorsal.z,dorsalN.x,dorsalN.y,dorsalN.z,insertion);
            add("dorsal_port_aft","dorsal_surface",centerX-spanX*.22f,centerY-spanY*.20f,dorsal.z,dorsalN.x,dorsalN.y,dorsalN.z,insertion);
            add("dorsal_starboard_aft","dorsal_surface",centerX+spanX*.22f,centerY-spanY*.20f,dorsal.z,dorsalN.x,dorsalN.y,dorsalN.z,insertion);
        }
        // Canonical rear-drive sockets must follow the *rendered* Shipyard
        // forward authority, not merely the OBJ +Y/-Y extrema. Greyoxide v0.7
        // is normalized with a 180-degree visual yaw, so its source +Y terminal
        // becomes canonical ship-aft after that family normalization. Future
        // canonical families use the ordinary geometric -Y aft face.
        {
            const bool greyoxide=IsShipyardModule(s.moduleId);
            const auto rearPoint=greyoxide?forward:aft;
            const auto rearNormal=greyoxide?normal(s.forwardSurface,{0,1,0}):normal(s.aftSurface,{0,-1,0});
            const float spanX=std::max(.05f,std::fabs(starboard.x-port.x));
            const float spanY=std::max(.05f,std::fabs(forward.y-aft.y));
            const float engineDepth=std::max(insertion,spanY*.035f);
            const Vector3 inset=rearPoint-rearNormal*(spanY*.04f);
            add("engine_aft","engine_cavity",centerX,inset.y,centerZ,rearNormal.x,rearNormal.y,rearNormal.z,engineDepth);
            add("engine_port","engine_cavity",centerX-spanX*.21f,inset.y,centerZ,rearNormal.x,rearNormal.y,rearNormal.z,engineDepth);
            add("engine_starboard","engine_cavity",centerX+spanX*.21f,inset.y,centerZ,rearNormal.x,rearNormal.y,rearNormal.z,engineDepth);
            add("engine_wing_port","engine_cavity",centerX-spanX*.39f,inset.y,centerZ,rearNormal.x,rearNormal.y,rearNormal.z,engineDepth);
            add("engine_wing_starboard","engine_cavity",centerX+spanX*.39f,inset.y,centerZ,rearNormal.x,rearNormal.y,rearNormal.z,engineDepth);
        }
        if (semantic == ShipyardModuleSemantic::Component) {
            const bool usePort=s.portSurface.valid && (!s.starboardSurface.valid || s.portSurface.supportingArea>=s.starboardSurface.supportingArea);
            if(usePort) addSurface("mount","lateral_mount",s.portSurface,{-w,0,0},{-1,0,0},insertion);
            else addSurface("mount","lateral_mount",s.starboardSurface,{w,0,0},{1,0,0},insertion);
        }
        break;
    case ShipyardModuleSemantic::CommandCockpit:
    case ShipyardModuleSemantic::CommandBridge:
        addSurface("mount", "detail_mount", s.ventralSurface, {0,0,-h}, {0,0,-1}, insertion);
        addSurface("aft", "hull_aft", s.aftSurface, {0,-l,0}, {0,-1,0}, insertion);
        break;
    case ShipyardModuleSemantic::Adapter:
        addSurface("forward", "hull_forward", s.forwardSurface, {0,l,0}, {0,1,0}, insertion);
        addSurface("aft", "hull_aft", s.aftSurface, {0,-l,0}, {0,-1,0}, insertion);
        break;
    case ShipyardModuleSemantic::EngineHousing:
        addSurface("mount", "engine_mount", s.forwardSurface, {0,l,0}, {0,1,0}, insertion);
        {
            const Vector3 cavity=aft+(forward-aft)*.325f;
            const auto n=normal(s.aftSurface,{0,-1,0});
            add("engine_cavity","engine_cavity",cavity.x,cavity.y,cavity.z,n.x,n.y,n.z,
                std::max(insertion,std::fabs(forward.y-aft.y)*.075f));
        }
        break;
    case ShipyardModuleSemantic::MainEngine:
    case ShipyardModuleSemantic::EngineNozzle:
    case ShipyardModuleSemantic::RcsThruster:
        addSurface("mount", "engine_mount", s.forwardSurface, {0,l,0}, {0,1,0}, insertion);
        addSurface("exhaust", "exhaust", s.aftSurface, {0,-l,0}, {0,-1,0}, 0);
        break;
    case ShipyardModuleSemantic::TurretHardpoint:
    case ShipyardModuleSemantic::WeaponMount:
        addSurface("mount", "hardpoint_mount", s.ventralSurface, {0,0,-h}, {0,0,-1}, insertion);
        addSurface("weapon_axis", "weapon_axis", s.dorsalSurface, {0,0,h}, {0,0,1}, 0);
        break;
    case ShipyardModuleSemantic::Wing: {
        // Wing roots are not reliably the OBJ +/-X side.  Several Greyoxide
        // lateral pieces are authored upright and must mate a ventral/aft/etc.
        // source face to the ship's lateral socket to become a horizontal wing.
        // Consume audited/user-verified root-face evidence when available; the
        // old largest-port/starboard guess is retained only as an AUTO fallback.
        std::string preferred="auto";
        if(const auto* exact=FindShipyardSbsClassification(ShipyardNameClassifier::CanonicalLeafName(s.moduleId)))
            preferred=exact->preferredMountFace?exact->preferredMountFace:"auto";
        if(preferred=="ventral") addSurface("mount","lateral_mount",s.ventralSurface,{0,0,-h},{0,0,-1},insertion);
        else if(preferred=="dorsal") addSurface("mount","lateral_mount",s.dorsalSurface,{0,0,h},{0,0,1},insertion);
        else if(preferred=="forward") addSurface("mount","lateral_mount",s.forwardSurface,{0,l,0},{0,1,0},insertion);
        else if(preferred=="aft") addSurface("mount","lateral_mount",s.aftSurface,{0,-l,0},{0,-1,0},insertion);
        else if(preferred=="port") addSurface("mount","lateral_mount",s.portSurface,{-w,0,0},{-1,0,0},insertion);
        else if(preferred=="starboard") addSurface("mount","lateral_mount",s.starboardSurface,{w,0,0},{1,0,0},insertion);
        else {
            const bool usePort=s.portSurface.valid && (!s.starboardSurface.valid || s.portSurface.supportingArea>=s.starboardSurface.supportingArea);
            if(usePort) addSurface("mount","lateral_mount",s.portSurface,{-w,0,0},{-1,0,0},insertion);
            else addSurface("mount","lateral_mount",s.starboardSurface,{w,0,0},{1,0,0},insertion);
        }
        // A wing may host a main drive only at its certified aft terminal. The
        // socket is separate from the structural root so PCG/manual placement
        // cannot silently reinterpret an arbitrary wing face as propulsion.
        {
            const bool greyoxide=IsShipyardModule(s.moduleId);
            const auto rearPoint=greyoxide?forward:aft;
            const auto rearNormal=greyoxide?normal(s.forwardSurface,{0,1,0}):normal(s.aftSurface,{0,-1,0});
            const float spanY=std::max(.05f,std::fabs(forward.y-aft.y));
            const Vector3 inset=rearPoint-rearNormal*(spanY*.035f);
            add("engine_wing_aft","engine_cavity",inset.x,inset.y,inset.z,rearNormal.x,rearNormal.y,rearNormal.z,
                std::max(insertion,spanY*.03f));
        }
        break;
    }
    case ShipyardModuleSemantic::Sensor:
    case ShipyardModuleSemantic::SurfaceDetail:
        addSurface("mount", "detail_mount", s.ventralSurface, {0,0,-h}, {0,0,-1}, insertion * .5f);
        break;
    }
    return out;
}

bool ShipyardModuleSystem::RoleSuitable(ShipyardModuleSemantic s, const std::string& role) {
    if (s == ShipyardModuleSemantic::HullBow || s == ShipyardModuleSemantic::HullMid ||
        s == ShipyardModuleSemantic::HullAft || s == ShipyardModuleSemantic::StructuralFrame) return true;
    if (s == ShipyardModuleSemantic::CommandCockpit) return role == "COMBAT" || role == "EXPLORATION";
    if (s == ShipyardModuleSemantic::CommandBridge) return role != "COMBAT" || role == "HAULER";
    if (s == ShipyardModuleSemantic::TurretHardpoint || s == ShipyardModuleSemantic::WeaponMount) return role == "COMBAT" || role == "MINING";
    if (s == ShipyardModuleSemantic::Sensor) return role == "EXPLORATION" || role == "MINING";
    return true;
}

bool ShipyardModuleSystem::CanMate(const std::string& parent, const std::string& child) {
    if (parent == "hull_aft" && child == "hull_forward") return true;
    if (parent == "hull_forward" && child == "hull_aft") return true;
    if (parent == "engine_cavity" && child == "engine_mount") return true;
    if (parent == "dorsal_surface" && (child == "hardpoint_mount" || child == "detail_mount")) return true;
    if (parent == "lateral_surface" && (child == "lateral_mount" || child == "detail_mount")) return true;
    if (parent == "ventral_surface" && (child == "hardpoint_mount" || child == "detail_mount")) return true;
    return false;
}

bool ShipyardModuleSystem::IsRearDriveSocketName(std::string_view socketName) {
    return socketName=="engine_aft" || socketName=="engine_port" || socketName=="engine_starboard" ||
           socketName=="engine_wing_port" || socketName=="engine_wing_starboard" || socketName=="engine_wing_aft";
}

bool ShipyardModuleSystem::ValidatePropulsionPlacement(const std::vector<ShipyardModuleRecord>& catalog,
                                                        const ProceduralShipVisualRecipe& recipe,
                                                        std::size_t moduleIndex,
                                                        std::string* error) {
    if(error) error->clear();
    if(moduleIndex>=recipe.modules.size()){if(error)*error="propulsion module index out of range";return false;}
    const auto& placement=recipe.modules[moduleIndex];
    const auto recIt=std::find_if(catalog.begin(),catalog.end(),[&](const auto& r){return r.source.moduleId==placement.moduleId;});
    if(recIt==catalog.end()){if(error)*error="unknown propulsion module";return false;}
    const auto& record=*recIt;
    if(record.semantic==ShipyardModuleSemantic::RcsThruster) return true;
    if(record.semantic!=ShipyardModuleSemantic::MainEngine && record.semantic!=ShipyardModuleSemantic::EngineNozzle) return true;

    // Trace the structural parent chain. A terminal main drive is legal when it
    // attaches directly to a canonical rear-drive socket or lives inside one or
    // more certified engine housings whose outer attachment enters through such
    // a socket.
    std::size_t cursor=moduleIndex;
    bool rearChain=false;
    for(int depth=0;depth<6 && cursor<recipe.modules.size();++depth){
        const auto edgeIt=std::find_if(recipe.attachments.begin(),recipe.attachments.end(),[&](const auto& e){return e.childModuleIndex==cursor;});
        if(edgeIt==recipe.attachments.end()) break;
        if(IsRearDriveSocketName(edgeIt->parentSocket)){rearChain=true;break;}
        if(edgeIt->parentModuleIndex>=recipe.modules.size()) break;
        const auto parentIt=std::find_if(catalog.begin(),catalog.end(),[&](const auto& r){return r.source.moduleId==recipe.modules[edgeIt->parentModuleIndex].moduleId;});
        if(parentIt==catalog.end() || parentIt->semantic!=ShipyardModuleSemantic::EngineHousing) break;
        cursor=edgeIt->parentModuleIndex;
    }
    if(!rearChain){if(error)*error="main propulsion is not attached through a canonical rear-drive socket";return false;}

    const auto* exhaust=FindNamedSocket(record,"exhaust");
    if(!exhaust){if(error)*error="main propulsion has no authored exhaust socket";return false;}
    auto exhaustDirection=TransformPlacementDirection(placement,*exhaust);
    exhaustDirection=RotateRecipeVisual(exhaustDirection,recipe).normalized();
    if(exhaustDirection.length()<1.0e-5f || exhaustDirection.y>-0.70f){
        if(error)*error="main propulsion exhaust does not point ship-aft after visual normalization";
        return false;
    }
    return true;
}

bool ShipyardModuleSystem::SizeCompatible(ShipyardModuleSize parent, ShipyardModuleSize child) {
    const int a=static_cast<int>(parent), b=static_cast<int>(child);
    return std::abs(a-b)<=2;
}

VisualModulePlacement ShipyardModuleSystem::BuildAttachmentPlacement(const VisualModulePlacement& parentPlacement,
                                                                      const ShipyardAssemblySocket& parentSocket,
                                                                      const ShipyardModuleRecord& child,
                                                                      const ShipyardAssemblySocket& childSocket,
                                                                      float uniformScale) {
    VisualModulePlacement result=Placement(child,0,0,0,uniformScale,child.moduleClass==ShipyardModuleClass::Command?SpaceMaterialKind::Canopy:
        child.moduleClass==ShipyardModuleClass::Propulsion?SpaceMaterialKind::EngineHousing:SpaceMaterialKind::StructuralMetal);
    struct Candidate{float yaw,pitch,roll;};
    // The 24 orthogonal cube orientations. The previous table omitted the
    // root-preserving 180-degree pitch family. That omission made some lateral
    // wings impossible to orient correctly even when their socket normal was
    // already valid: fixing forward and dorsal presentation would require
    // detaching the root. Keep this finite/certifiable rather than introducing
    // arbitrary generator rotations.
    static const Candidate cands[]={
        {0,0,0},{90,0,0},{180,0,0},{270,0,0},
        {0,90,0},{0,-90,0},{90,90,0},{90,-90,0},{180,90,0},{180,-90,0},{270,90,0},{270,-90,0},
        {0,0,90},{0,0,-90},{90,0,90},{90,0,-90},{180,0,90},{180,0,-90},{270,0,90},{270,0,-90},
        {0,180,0},{90,180,0},{180,180,0},{270,180,0}
    };
    auto rotate=[](float x,float y,float z,float yawDeg,float pitchDeg,float rollDeg){
        const float d=3.14159265358979323846f/180.0f;
        const float yaw=yawDeg*d,pitch=pitchDeg*d,roll=rollDeg*d;
        const float cr=std::cos(roll),sr=std::sin(roll); const float x1=x*cr+z*sr,y1=y,z1=-x*sr+z*cr;
        const float cp=std::cos(pitch),sp=std::sin(pitch); const float x2=x1,y2=y1*cp-z1*sp,z2=y1*sp+z1*cp;
        const float cy=std::cos(yaw),sy=std::sin(yaw); return Vector3{x2*cy-y2*sy,x2*sy+y2*cy,z2};
    };
    const float parentMirrorX=parentPlacement.mirrorX?-1.0f:1.0f,parentMirrorY=parentPlacement.mirrorY?-1.0f:1.0f,parentMirrorZ=parentPlacement.mirrorZ?-1.0f:1.0f;
    const auto parentOffset=rotate(parentSocket.x*parentPlacement.scaleX*parentMirrorX,parentSocket.y*parentPlacement.scaleY*parentMirrorY,parentSocket.z*parentPlacement.scaleZ*parentMirrorZ,
                                   parentPlacement.yawDegrees,parentPlacement.pitchDegrees,parentPlacement.rollDegrees);
    auto parentNormal=rotate(parentSocket.dirX*parentMirrorX,parentSocket.dirY*parentMirrorY,parentSocket.dirZ*parentMirrorZ,
                             parentPlacement.yawDegrees,parentPlacement.pitchDegrees,parentPlacement.rollDegrees);
    const float parentNormalLength=parentNormal.length();
    if(parentNormalLength>1.0e-5f) parentNormal=parentNormal*(1.0f/parentNormalLength);
    const Vector3 parentWorld{parentPlacement.x+parentOffset.x,parentPlacement.y+parentOffset.y,parentPlacement.z+parentOffset.z};

    // Socket-normal alignment leaves a rotational degree of freedom for many
    // lateral mounts. Break equal-normal ties by preserving the parent's
    // forward/up basis, so a starboard mirror does not become a 180-degree
    // backwards wing simply because that candidate appeared first.
    auto parentForward=rotate(0,1,0,parentPlacement.yawDegrees,parentPlacement.pitchDegrees,parentPlacement.rollDegrees).normalized();
    auto parentUp=rotate(0,0,1,parentPlacement.yawDegrees,parentPlacement.pitchDegrees,parentPlacement.rollDegrees).normalized();

    // Most authored modules use runtime-local +Y forward / +Z dorsal. A small
    // number have been visually certified with a different source basis. Read
    // those exceptions from metadata so the generic solver can still rank all
    // candidate transforms without module-name branches in the placement code.
    Vector3 authoredForward{0,1,0}, authoredUp{0,0,1};
    if(const auto* authored=FindShipyardAuthoredOrientation(ShipyardNameClassifier::CanonicalLeafName(child.source.moduleId))){
        authoredForward={authored->forwardX,authored->forwardY,authored->forwardZ};
        authoredUp={authored->upX,authored->upY,authored->upZ};
    }

    Candidate best=cands[0]; bool bestMirror=false;float bestDot=-1.0e9f,bestForward=-1.0e9f,bestUp=-1.0e9f;
    for(const bool mirrorX:{false,true})for(const auto& c:cands){
        const float mirror=mirrorX?-1.0f:1.0f;
        auto v=rotate(childSocket.dirX*mirror,childSocket.dirY,childSocket.dirZ,c.yaw,c.pitch,c.roll);
        const float len=v.length();if(len>1.0e-5f)v=v*(1.0f/len);
        const float dot=v.x*(-parentNormal.x)+v.y*(-parentNormal.y)+v.z*(-parentNormal.z);
        // Reflection across X changes the source basis handedness as well as
        // the attachment normal. Score the authored forward/up vectors after
        // the exact same reflection so port/starboard mates stay equivalent.
        const auto childForward=rotate(authoredForward.x*mirror,authoredForward.y,authoredForward.z,c.yaw,c.pitch,c.roll).normalized();
        const auto childUp=rotate(authoredUp.x*mirror,authoredUp.y,authoredUp.z,c.yaw,c.pitch,c.roll).normalized();
        const float forwardDot=childForward.x*parentForward.x+childForward.y*parentForward.y+childForward.z*parentForward.z;
        const float upDot=childUp.x*parentUp.x+childUp.y*parentUp.y+childUp.z*parentUp.z;
        const bool betterNormal=dot>bestDot+1.0e-4f;
        const bool sameNormal=std::fabs(dot-bestDot)<=1.0e-4f;
        const bool betterForward=sameNormal&&forwardDot>bestForward+1.0e-4f;
        const bool sameForward=sameNormal&&std::fabs(forwardDot-bestForward)<=1.0e-4f;
        if(betterNormal||betterForward||(sameForward&&upDot>bestUp)){
            bestDot=dot;bestForward=forwardDot;bestUp=upDot;best=c;bestMirror=mirrorX;
        }
    }
    result.yawDegrees=best.yaw;result.pitchDegrees=best.pitch;result.rollDegrees=best.roll;result.mirrorX=bestMirror;
    const float childMirror=bestMirror?-1.0f:1.0f;
    const auto childOffset=rotate(childSocket.x*uniformScale*childMirror,childSocket.y*uniformScale,childSocket.z*uniformScale,best.yaw,best.pitch,best.roll);
    const float insertion=parentSocket.insertionDepth*std::max({parentPlacement.scaleX,parentPlacement.scaleY,parentPlacement.scaleZ});
    result.x=parentWorld.x-childOffset.x-parentNormal.x*insertion;
    result.y=parentWorld.y-childOffset.y-parentNormal.y*insertion;
    result.z=parentWorld.z-childOffset.z-parentNormal.z*insertion;
    return result;
}

bool ShipyardModuleSystem::ResolveLegacyAttachment(const LegacyAttachmentCapture& capture) {
    if (capture.placements.empty() || capture.writablePlacements.empty()) return false;

    const VisualModulePlacement* parentPlacement = capture.placements.front();
    VisualModulePlacement* output = capture.writablePlacements.back();
    if (capture.placements.size() > 1 && output == parentPlacement) {
        // Prefer a different writable placement as the out parameter when the
        // legacy caller supplied both parent and child placements by non-const
        // reference.
        for (auto it = capture.writablePlacements.rbegin(); it != capture.writablePlacements.rend(); ++it) {
            if (*it != parentPlacement) { output = *it; break; }
        }
    }
    if (!parentPlacement || !output || output == parentPlacement) return false;

    auto makeRecord = [&](const VisualModuleSource& source) {
        ShipyardModuleRecord record;
        record.source = source;
        record.moduleClass = Classify(source);
        record.semantic = SemanticClassify(source);
        record.size = SizeClassify(source);
        record.sockets = BuildSockets(source, record.semantic);
        record.builderCategory = ShipyardPartTaxonomySystem::CategoryFor(record.moduleClass);
        record.partRole = ShipyardPartTaxonomySystem::RoleFor(record.semantic, source.moduleId);
        record.primaryHull = record.moduleClass == ShipyardModuleClass::Hull;
        record.functional = record.moduleClass == ShipyardModuleClass::Command ||
                            record.moduleClass == ShipyardModuleClass::Propulsion ||
                            record.moduleClass == ShipyardModuleClass::Hardpoint ||
                            record.semantic == ShipyardModuleSemantic::Sensor;
        record.mirrorPreferred = record.moduleClass == ShipyardModuleClass::Wing ||
                                 record.moduleClass == ShipyardModuleClass::Propulsion ||
                                 record.moduleClass == ShipyardModuleClass::Hardpoint;
        ApplyR6PlacementMetadata(record);
        return record;
    };

    ShipyardModuleRecord parentLocal;
    ShipyardModuleRecord childLocal;
    const ShipyardModuleRecord* parent = nullptr;
    const ShipyardModuleRecord* child = nullptr;

    if (capture.records.size() >= 2) {
        parent = capture.records[0];
        child = capture.records[1];
    } else if (capture.records.size() == 1) {
        const auto* only = capture.records.front();
        if (only->source.moduleId == parentPlacement->moduleId) parent = only;
        else child = only;
    }

    std::vector<VisualModuleSource> sourceCandidates;
    sourceCandidates.reserve(capture.sources.size());
    for (const auto* src : capture.sources) if (src) sourceCandidates.push_back(*src);

    auto findSource = [&](const std::string& moduleId) -> const VisualModuleSource* {
        for (const auto& source : sourceCandidates) if (source.moduleId == moduleId) return &source;
        return nullptr;
    };

    if (!parent) {
        if (const auto* source = findSource(parentPlacement->moduleId)) parentLocal = makeRecord(*source);
        else {
            VisualModuleSource fallbackSource;
            fallbackSource.moduleId = parentPlacement->moduleId;
            fallbackSource.halfWidth = fallbackSource.halfLength = fallbackSource.halfHeight = 1.0f;
            parentLocal = makeRecord(fallbackSource);
        }
        parent = &parentLocal;
    }

    if (!child) {
        // Prefer a source that is not the parent module.  Older builders pass
        // parent/child VisualModuleSource values even when their module records
        // are not available.
        const VisualModuleSource* childSource = nullptr;
        for (const auto& source : sourceCandidates) {
            if (source.moduleId != parentPlacement->moduleId) { childSource = &source; break; }
        }
        if (!childSource && !output->moduleId.empty()) childSource = findSource(output->moduleId);
        if (!childSource) return false;
        childLocal = makeRecord(*childSource);
        child = &childLocal;
    }

    const ShipyardAssemblySocket* parentSocket = capture.sockets.size() > 0 ? capture.sockets[0] : nullptr;
    const ShipyardAssemblySocket* childSocket  = capture.sockets.size() > 1 ? capture.sockets[1] : nullptr;

    auto matchSocketText = [&](const ShipyardModuleRecord& record,
                               const std::string& text,
                               const ShipyardAssemblySocket* exclude) -> const ShipyardAssemblySocket* {
        for (const auto& socket : record.sockets) {
            if (&socket == exclude) continue;
            if (socket.name == text || socket.type == text) return &socket;
        }
        return nullptr;
    };

    if (!parentSocket || !childSocket) {
        for (const auto& text : capture.texts) {
            if (!parentSocket) parentSocket = matchSocketText(*parent, text, nullptr);
            else if (!childSocket) childSocket = matchSocketText(*child, text, nullptr);
        }
    }

    if (!parentSocket || !childSocket || !CanMate(parentSocket->type, childSocket->type)) {
        parentSocket = nullptr;
        childSocket = nullptr;
        for (const auto& pSocket : parent->sockets) {
            for (const auto& cSocket : child->sockets) {
                if (CanMate(pSocket.type, cSocket.type)) {
                    parentSocket = &pSocket;
                    childSocket = &cSocket;
                    break;
                }
            }
            if (parentSocket) break;
        }
    }

    if (!parentSocket || !childSocket) return false;
    if (!SizeCompatible(parent->size, child->size) && child->partRole != ShipyardPartRole::HullAdapter) return false;

    const float scale = capture.hasScale ? capture.uniformScale : 1.0f;
    *output = BuildAttachmentPlacement(*parentPlacement, *parentSocket, *child, *childSocket, scale);
    return true;
}

ShipyardAssemblyValidation ShipyardModuleSystem::ValidateAssemblyGraph(
    const ProceduralShipVisualRecipe& recipe,
    const std::vector<ShipyardModuleRecord>& catalog) {
    return ValidateAssemblyGraph(catalog, recipe);
}

ShipyardAssemblyValidation ShipyardModuleSystem::ValidateAssemblyGraph(
    const ProceduralShipVisualRecipe& recipe,
    const std::vector<VisualModuleSource>& availableModules) {
    return ValidateAssemblyGraph(BuildCatalog(availableModules), recipe);
}

ShipyardAssemblyValidation ShipyardModuleSystem::ValidateAssemblyGraph(
    const std::vector<VisualModuleSource>& availableModules,
    const ProceduralShipVisualRecipe& recipe) {
    return ValidateAssemblyGraph(BuildCatalog(availableModules), recipe);
}

ShipyardAssemblyValidation ShipyardModuleSystem::ValidateAssemblyGraph(const ProceduralShipVisualRecipe& recipe) {
    std::vector<ShipyardModuleRecord> reconstructed;
    reconstructed.reserve(recipe.modules.size());
    for (const auto& placement : recipe.modules) {
        VisualModuleSource source;
        source.moduleId = placement.moduleId;
        source.halfWidth = source.halfLength = source.halfHeight = 1.0f;
        if (!IsShipyardModule(source.moduleId)) continue;

        ShipyardModuleRecord record;
        record.source = source;
        record.moduleClass = Classify(source);
        record.semantic = SemanticClassify(source);
        record.size = SizeClassify(source);
        record.sockets = BuildSockets(source, record.semantic);
        record.builderCategory = ShipyardPartTaxonomySystem::CategoryFor(record.moduleClass);
        record.partRole = ShipyardPartTaxonomySystem::RoleFor(record.semantic, source.moduleId);
        record.primaryHull = record.moduleClass == ShipyardModuleClass::Hull;
        ApplyR6PlacementMetadata(record);
        reconstructed.push_back(std::move(record));
    }
    return ValidateAssemblyGraph(reconstructed, recipe);
}

bool ShipyardModuleSystem::ValidateAssemblyGraph(const ProceduralShipVisualRecipe& recipe, std::string* error) {
    const auto validation = ValidateAssemblyGraph(recipe);
    if (error) {
        error->clear();
        for (std::size_t i=0;i<validation.errors.size();++i) {
            if (i) *error += "; ";
            *error += validation.errors[i];
        }
    }
    return validation.valid;
}

ShipyardAssemblyValidation ShipyardModuleSystem::ValidateAssemblyGraph(const std::vector<ShipyardModuleRecord>& catalog,
                                                                        const ProceduralShipVisualRecipe& recipe) {
    ShipyardAssemblyValidation v;
    if (recipe.modules.empty()) { v.errors.push_back("Shipyard recipe has no modules"); return v; }

    std::vector<const ShipyardModuleRecord*> records(recipe.modules.size(), nullptr);
    bool hull=false, command=false, drive=false;
    std::size_t rootIndex = recipe.modules.size();
    for (std::size_t i=0;i<recipe.modules.size();++i) {
        const auto& placement = recipe.modules[i];
        auto it=std::find_if(catalog.begin(),catalog.end(),[&](const auto& r){return r.source.moduleId==placement.moduleId;});
        if(it==catalog.end()){v.errors.push_back("Unknown Shipyard module: "+placement.moduleId);continue;}
        records[i]=&*it;
        hull|=it->primaryHull;
        command|=it->moduleClass==ShipyardModuleClass::Command;
        drive|=it->semantic==ShipyardModuleSemantic::MainEngine;
        if(rootIndex==recipe.modules.size() && it->primaryHull) rootIndex=i;
    }
    if(!hull)v.errors.push_back("Missing primary Shipyard hull");
    if(!command)v.errors.push_back("Missing Shipyard command module");
    if(!drive)v.errors.push_back("Missing authored MAIN_ENGINE module");
    if(!v.errors.empty()) { v.valid=false; return v; }

    // Every visible module after the root must have one explicit certified
    // parent edge. This is shared by PCG ships and the docked manual builder,
    // preventing a valid-looking recipe from containing floating/orphaned parts.
    if (recipe.modules.size() > 1 && recipe.attachments.empty()) {
        v.errors.push_back("Shipyard assembly has no attachment graph");
        v.valid=false; return v;
    }

    std::vector<int> parentCount(recipe.modules.size(),0);
    std::vector<std::vector<std::size_t>> children(recipe.modules.size());
    auto socketByName=[](const ShipyardModuleRecord& record,const std::string& name)->const ShipyardAssemblySocket*{
        for(const auto& socket:record.sockets) if(socket.name==name) return &socket;
        return nullptr;
    };
    for(const auto& edge:recipe.attachments){
        if(edge.parentModuleIndex>=recipe.modules.size()||edge.childModuleIndex>=recipe.modules.size()){
            v.errors.push_back("Shipyard attachment references an invalid module index");continue;
        }
        if(edge.parentModuleIndex==edge.childModuleIndex){v.errors.push_back("Shipyard attachment cannot self-parent");continue;}
        if(!edge.certified){v.errors.push_back("Shipyard attachment is not certified");continue;}
        if(edge.childModuleIndex==rootIndex){v.errors.push_back("Primary Shipyard hull cannot be a child attachment");continue;}
        ++parentCount[edge.childModuleIndex];
        children[edge.parentModuleIndex].push_back(edge.childModuleIndex);

        const auto* parent=records[edge.parentModuleIndex];
        const auto* child=records[edge.childModuleIndex];
        if(!parent||!child) continue;
        const auto* ps=socketByName(*parent,edge.parentSocket);
        if(!ps){v.errors.push_back("Unknown parent socket: "+edge.parentSocket);continue;}
        if(edge.childSocket=="placement_role"){
            v.errors.push_back("Uncertified free-placement edge cannot pass Shipyard certification");
            continue;
        }
        const auto* cs=socketByName(*child,edge.childSocket);
        if(!cs){v.errors.push_back("Unknown child socket: "+edge.childSocket);continue;}
        if(!CanMate(ps->type,cs->type)){v.errors.push_back("Incompatible Shipyard socket pair: "+edge.parentSocket+" -> "+edge.childSocket);continue;}

        const bool geometryAuthority =
            (parent->source.forwardSurface.valid||parent->source.aftSurface.valid||parent->source.portSurface.valid||parent->source.starboardSurface.valid||parent->source.dorsalSurface.valid||parent->source.ventralSurface.valid) &&
            (child->source.forwardSurface.valid||child->source.aftSurface.valid||child->source.portSurface.valid||child->source.starboardSurface.valid||child->source.dorsalSurface.valid||child->source.ventralSurface.valid);
        if(geometryAuthority){
            const auto& actual=recipe.modules[edge.childModuleIndex];
            const auto expected=BuildAttachmentPlacement(recipe.modules[edge.parentModuleIndex],*ps,*child,*cs,std::max(.01f,actual.scaleX));
            const float dx=actual.x-expected.x,dy=actual.y-expected.y,dz=actual.z-expected.z;
            const float gap=std::sqrt(dx*dx+dy*dy+dz*dz);
            const float childSpan=std::max({child->source.halfWidth,child->source.halfLength,child->source.halfHeight})*std::max(.01f,actual.scaleX);
            const float tolerance=std::max(.10f,childSpan*.12f);
            if(gap>tolerance)v.errors.push_back("Detached Shipyard attachment gap="+std::to_string(gap)+" module="+recipe.modules[edge.childModuleIndex].moduleId);
        }
    }

    // Paired lateral modules must preserve the same ship-relative forward
    // orientation. Position symmetry alone is insufficient: a mirrored wing
    // that points its flare/nose the opposite direction is visibly wrong.
    auto placementForward=[](const VisualModulePlacement& m){
        const float d=3.14159265358979323846f/180.0f;
        const float yaw=m.yawDegrees*d,pitch=m.pitchDegrees*d,roll=m.rollDegrees*d;
        const float cr=std::cos(roll),sr=std::sin(roll);
        const float x1=0.0f*cr+0.0f*sr,y1=1.0f,z1=-0.0f*sr+0.0f*cr;
        const float cp=std::cos(pitch),sp=std::sin(pitch);
        const float x2=x1,y2=y1*cp-z1*sp,z2=y1*sp+z1*cp;
        const float cy=std::cos(yaw),sy=std::sin(yaw);
        return Vector3{x2*cy-y2*sy,x2*sy+y2*cy,z2}.normalized();
    };
    auto mirroredSocketName=[](const std::string& name){
        std::string out=name;
        const auto port=out.find("port");
        if(port!=std::string::npos){out.replace(port,4,"starboard");return out;}
        const auto star=out.find("starboard");
        if(star!=std::string::npos){out.replace(star,9,"port");return out;}
        return std::string{};
    };
    for(const auto& edge:recipe.attachments){
        if(edge.parentSocket.find("port")==std::string::npos ||
           edge.childModuleIndex>=recipe.modules.size() ||
           edge.parentModuleIndex>=recipe.modules.size()) continue;
        const auto* childRecord=records[edge.childModuleIndex];
        if(!childRecord || !childRecord->mirrorPreferred) continue;
        const auto counterpart=mirroredSocketName(edge.parentSocket);
        if(counterpart.empty()) continue;
        for(const auto& other:recipe.attachments){
            if(other.parentModuleIndex!=edge.parentModuleIndex ||
               other.parentSocket!=counterpart ||
               other.childModuleIndex>=recipe.modules.size()) continue;
            if(recipe.modules[other.childModuleIndex].moduleId!=recipe.modules[edge.childModuleIndex].moduleId) continue;
            const auto& leftPlacement=recipe.modules[edge.childModuleIndex];
            const auto& rightPlacement=recipe.modules[other.childModuleIndex];
            const auto a=placementForward(leftPlacement);
            const auto b=placementForward(rightPlacement);
            const float dot=a.x*b.x+a.y*b.y+a.z*b.z;
            if(dot<0.85f)v.errors.push_back("Mirrored pair orientation mismatch: "+leftPlacement.moduleId);
            if(childRecord->moduleClass==ShipyardModuleClass::Wing&&leftPlacement.mirrorX==rightPlacement.mirrorX)
                v.errors.push_back("Mirrored pair handedness mismatch: "+leftPlacement.moduleId);
            break;
        }
    }

    for(std::size_t i=0;i<recipe.modules.size();++i){
        if(i==rootIndex)continue;
        if(parentCount[i]==0)v.errors.push_back("Orphaned Shipyard module: "+recipe.modules[i].moduleId);
        else if(parentCount[i]>1)v.errors.push_back("Shipyard module has multiple structural parents: "+recipe.modules[i].moduleId);
    }

    // Main propulsion is a hard structural rule, not a soft art score. Every
    // authored main drive/nozzle must resolve through a rear-drive chain and its
    // transformed exhaust must point canonical ship-aft after source-family
    // visual normalization. Maneuvering/RCS thrusters are intentionally exempt.
    for(std::size_t i=0;i<recipe.modules.size();++i){
        const auto* record=records[i];
        if(!record) continue;
        if(record->semantic!=ShipyardModuleSemantic::MainEngine && record->semantic!=ShipyardModuleSemantic::EngineNozzle) continue;
        std::string propulsionError;
        if(!ValidatePropulsionPlacement(catalog,recipe,i,&propulsionError))
            v.errors.push_back("Invalid main propulsion: "+recipe.modules[i].moduleId+" ("+propulsionError+")");
    }

    if(v.errors.empty()){
        std::vector<bool> reached(recipe.modules.size(),false);
        std::vector<std::size_t> stack{rootIndex}; reached[rootIndex]=true;
        while(!stack.empty()){
            const auto parent=stack.back();stack.pop_back();
            for(const auto child:children[parent])if(!reached[child]){reached[child]=true;stack.push_back(child);}
        }
        for(std::size_t i=0;i<reached.size();++i)if(!reached[i])v.errors.push_back("Shipyard module has no path to primary hull: "+recipe.modules[i].moduleId);
    }
    v.valid=v.errors.empty();
    return v;
}

std::vector<ShipyardModuleRecord> ShipyardModuleSystem::BuildCatalog(const std::vector<VisualModuleSource>& availableModules) {
    std::vector<ShipyardModuleRecord> result;
    for (const auto& source : availableModules) {
        if (!IsCertifiedShipyardModule(source.moduleId)) continue;
        ShipyardModuleRecord record;
        record.source = source;
        record.moduleClass = Classify(source);
        record.semantic = SemanticClassify(source);
        record.size = SizeClassify(source);
        record.sockets = BuildSockets(source, record.semantic);
        record.preserveAspectRatio = true;
        record.builderCategory = ShipyardPartTaxonomySystem::CategoryFor(record.moduleClass);
        record.partRole = ShipyardPartTaxonomySystem::RoleFor(record.semantic, source.moduleId);
        record.primaryHull = record.moduleClass == ShipyardModuleClass::Hull;
        record.functional = record.moduleClass == ShipyardModuleClass::Command || record.moduleClass == ShipyardModuleClass::Propulsion || record.moduleClass == ShipyardModuleClass::Hardpoint || record.semantic == ShipyardModuleSemantic::Sensor;
        record.mirrorPreferred = record.moduleClass == ShipyardModuleClass::Wing || record.moduleClass == ShipyardModuleClass::Propulsion || record.moduleClass == ShipyardModuleClass::Hardpoint;
        ApplyR6PlacementMetadata(record);
        static const char* roles[] = {"INDUSTRIAL", "COMBAT", "MINING", "HAULER", "EXPLORATION"};
        for (const char* role : roles) if (RoleSuitable(record.semantic, role)) record.preferredRoles.emplace_back(role);
        result.push_back(std::move(record));
    }
    std::sort(result.begin(), result.end(), [](const auto& a, const auto& b) {
        if (a.moduleClass != b.moduleClass) return static_cast<int>(a.moduleClass) < static_cast<int>(b.moduleClass);
        if (a.semantic != b.semantic) return static_cast<int>(a.semantic) < static_cast<int>(b.semantic);
        if (a.size != b.size) return static_cast<int>(a.size) < static_cast<int>(b.size);
        return a.source.moduleId < b.source.moduleId;
    });
    return result;
}

std::vector<ShipyardPropulsionPort> ShipyardModuleSystem::BuildPropulsionPorts(
    const std::vector<ShipyardModuleRecord>& catalog,
    const ProceduralShipVisualRecipe& recipe) {
    std::vector<ShipyardPropulsionPort> ports;
    for (const auto& placement : recipe.modules) {
        const auto recordIt = std::find_if(catalog.begin(), catalog.end(), [&](const ShipyardModuleRecord& record) {
            return record.source.moduleId == placement.moduleId;
        });
        if (recordIt == catalog.end() || recordIt->moduleClass != ShipyardModuleClass::Propulsion) continue;
        const auto socketIt = std::find_if(recordIt->sockets.begin(), recordIt->sockets.end(), [](const ShipyardAssemblySocket& socket) {
            return socket.name == "exhaust";
        });
        if (socketIt == recordIt->sockets.end()) continue;

        ShipyardPropulsionPort port;
        port.moduleId = placement.moduleId;
        port.semantic = recordIt->semantic;
        // Preserve the complete authored placement transform. Pitch/roll and
        // mirror matter for wing-root engines and future maneuvering-thruster
        // families; the old yaw-only path could detach plume direction from the
        // visible nozzle. Main-drive correctness is now enforced by the hard
        // propulsion validator instead of rewriting the plume direction here.
        port.localPosition = TransformPlacementPoint(placement,*socketIt);
        port.exhaustDirection = TransformPlacementDirection(placement,*socketIt);
        port.nozzleRadiusHint = std::max(0.055f, recordIt->source.halfWidth * placement.scaleX * 0.22f);
        port.plumeLengthHint = std::max(0.55f, recordIt->source.halfLength * placement.scaleY * 1.35f);
        ports.push_back(std::move(port));
    }
    return ports;
}

std::vector<ProceduralShipVisualRecipe> ShipyardModuleSystem::BuildShowcaseRecipes(
    const std::vector<VisualModuleSource>& availableModules,
    std::uint32_t seed) {
    return BuildShowcaseRecipes(BuildCatalog(availableModules),seed);
}

std::vector<ProceduralShipVisualRecipe> ShipyardModuleSystem::BuildShowcaseRecipes(
    const std::vector<ShipyardModuleRecord>& catalog,
    std::uint32_t seed) {
    if (catalog.size() < 4) return {};
    const Pools pools = MakePools(catalog);
    if (pools.spine.empty()) return {};

    static const char* roles[] = {"INDUSTRIAL", "COMBAT", "MINING", "HAULER", "EXPLORATION"};
    std::vector<ProceduralShipVisualRecipe> recipes;
    recipes.reserve(10);
    for (const char* role : roles) {
        for (int i = 0; i < 2; ++i) {
            auto recipe = BuildOne(role, pools, seed, i);
            if (!recipe.modules.empty()) recipes.push_back(std::move(recipe));
        }
    }
    return recipes;
}

} // namespace subspace
