#include "ships/ThrusterLayoutSystem.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <set>

namespace subspace {


namespace {

const ShipyardModuleRecord* FindModuleRecord(const std::vector<ShipyardModuleRecord>& catalog,
                                             const std::string& moduleId)
{
    for (const auto& record : catalog) if (record.source.moduleId == moduleId) return &record;
    return nullptr;
}

const ShipyardAssemblySocket* FindSocket(const ShipyardModuleRecord& record, const char* name)
{
    for (const auto& socket : record.sockets) if (socket.name == name) return &socket;
    return nullptr;
}

Vector3 RotateZ(const Vector3& v, float degrees)
{
    const float a = degrees * 3.14159265358979323846f / 180.0f;
    const float c = std::cos(a), s = std::sin(a);
    return {v.x*c - v.y*s, v.x*s + v.y*c, v.z};
}

Vector3 TransformSocket(const VisualModulePlacement& placement, const ShipyardAssemblySocket& socket)
{
    const Vector3 scaled{socket.x*placement.scaleX, socket.y*placement.scaleY, socket.z*placement.scaleZ};
    const Vector3 rotated = RotateZ(scaled, placement.yawDegrees);
    return {placement.x + rotated.x, placement.y + rotated.y, placement.z + rotated.z};
}

Vector3 TransformNormal(const VisualModulePlacement& placement, const ShipyardAssemblySocket& socket)
{
    Vector3 n = RotateZ({socket.dirX, socket.dirY, socket.dirZ}, placement.yawDegrees);
    return n.length() > 1.0e-6f ? n.normalized() : Vector3{0,-1,0};
}

std::uint32_t StableHash(const std::string& value, std::uint32_t seed)
{
    std::uint32_t h = 2166136261u ^ seed;
    for (unsigned char c : value) { h ^= c; h *= 16777619u; }
    return h ? h : 1u;
}

float MajorHalfExtent(const ShipyardModuleRecord& r)
{
    return std::max({r.source.halfWidth, r.source.halfLength, r.source.halfHeight, 0.001f});
}

float YawToOpposeSurface(const ShipyardAssemblySocket& mount, const Vector3& outward)
{
    const float childAngle = std::atan2(mount.dirY, mount.dirX);
    const float desiredAngle = std::atan2(-outward.y, -outward.x);
    return (desiredAngle - childAngle) * 180.0f / 3.14159265358979323846f;
}

} // namespace

bool ShipThrusterLayout::HasRole(ThrusterRole role) const
{
    return CountRole(role) > 0;
}

std::size_t ShipThrusterLayout::CountRole(ThrusterRole role) const
{
    std::size_t count = 0;
    for (const auto& socket : sockets) if (socket.role == role) ++count;
    return count;
}

bool ShipThrusterLayout::IsCompleteSixAxisPlanarLayout() const
{
    return HasRole(ThrusterRole::Forward) && HasRole(ThrusterRole::Reverse) &&
           HasRole(ThrusterRole::StrafeLeft) && HasRole(ThrusterRole::StrafeRight) &&
           HasRole(ThrusterRole::YawLeft) && HasRole(ThrusterRole::YawRight);
}

bool ShipThrusterLayout::HasExternallyMountedRole(ThrusterRole role) const
{
    for (const auto& socket : sockets) if (socket.role == role && socket.externalMount) return true;
    return false;
}

std::size_t ShipThrusterLayout::CountDistinctModuleShapes() const
{
    std::set<int> shapes;
    for (const auto& socket : sockets) shapes.insert(static_cast<int>(socket.moduleShape));
    return shapes.size();
}

bool ShipThrusterLayout::HasClearPlumeDirections() const
{
    for (const auto& socket : sockets) {
        const float d2 = socket.plumeDirection.x*socket.plumeDirection.x +
                         socket.plumeDirection.y*socket.plumeDirection.y +
                         socket.plumeDirection.z*socket.plumeDirection.z;
        if (d2 < 0.80f || socket.plumeLength <= 0.20f || socket.nozzleRadius <= 0.02f) return false;
    }
    return true;
}

const char* ThrusterLayoutSystem::ModuleShapeName(ThrusterModuleShape shape)
{
    switch (shape) {
        case ThrusterModuleShape::MainBell: return "MAIN BELL";
        case ThrusterModuleShape::TwinPod: return "TWIN POD";
        case ThrusterModuleShape::BrakeWedge: return "BRAKE WEDGE";
        case ThrusterModuleShape::AngledPod: return "ANGLED POD";
        case ThrusterModuleShape::RcsBlock: return "RCS BLOCK";
        case ThrusterModuleShape::VectorEmitter: return "VECTOR EMITTER";
    }
    return "UNKNOWN";
}

ShipThrusterLayout ThrusterLayoutSystem::StarterIndustrial()
{
    ShipThrusterLayout layout;
    layout.layoutId = "industrial_exposed_thrusters_v3";

    // The drive pods sit wider and farther aft than the old buried sockets.
    layout.sockets.push_back({"main_port", ThrusterRole::Forward, {-2.05f,-10.15f,0.22f}, {0,-1,0}, 0.44f, 5.8f, 1.0f,
                              ThrusterModuleShape::TwinPod,"engine_small",180.0f,0.48f,0.58f,0.48f,true});
    layout.sockets.push_back({"main_starboard", ThrusterRole::Forward, {2.05f,-10.15f,0.22f}, {0,-1,0}, 0.44f, 5.8f, 1.0f,
                              ThrusterModuleShape::TwinPod,"engine_small",180.0f,0.48f,0.58f,0.48f,true});

    // Retro pods are visibly mounted on the forward shoulders, not tucked
    // under the cockpit where the hull hides them from normal cameras.
    layout.sockets.push_back({"retro_port", ThrusterRole::Reverse, {-2.05f,6.55f,0.18f}, {0,1,0}, 0.26f, 2.6f, 0.78f,
                              ThrusterModuleShape::BrakeWedge,"thruster",0.0f,0.42f,0.52f,0.42f,true});
    layout.sockets.push_back({"retro_starboard", ThrusterRole::Reverse, {2.05f,6.55f,0.18f}, {0,1,0}, 0.26f, 2.6f, 0.78f,
                              ThrusterModuleShape::BrakeWedge,"thruster",0.0f,0.42f,0.52f,0.42f,true});

    layout.sockets.push_back({"strafe_left_fore", ThrusterRole::StrafeLeft, {3.95f,2.35f,0.16f}, {1,0,0}, 0.19f, 1.65f, 0.58f,
                              ThrusterModuleShape::AngledPod,"thruster_small",-90.0f,0.48f,0.62f,0.48f,true});
    layout.sockets.push_back({"strafe_left_aft", ThrusterRole::StrafeLeft, {4.10f,-3.65f,0.16f}, {1,0,0}, 0.19f, 1.65f, 0.58f,
                              ThrusterModuleShape::AngledPod,"thruster_small",-90.0f,0.48f,0.62f,0.48f,true});
    layout.sockets.push_back({"strafe_right_fore", ThrusterRole::StrafeRight, {-3.95f,2.35f,0.16f}, {-1,0,0}, 0.19f, 1.65f, 0.58f,
                              ThrusterModuleShape::AngledPod,"thruster_small",90.0f,0.48f,0.62f,0.48f,true});
    layout.sockets.push_back({"strafe_right_aft", ThrusterRole::StrafeRight, {-4.10f,-3.65f,0.16f}, {-1,0,0}, 0.19f, 1.65f, 0.58f,
                              ThrusterModuleShape::AngledPod,"thruster_small",90.0f,0.48f,0.62f,0.48f,true});

    layout.sockets.push_back({"yaw_left_fore", ThrusterRole::YawLeft, {3.72f,3.65f,0.28f}, {1,0,0}, 0.15f, 1.30f, 0.48f,
                              ThrusterModuleShape::RcsBlock,"thruster_small",-78.0f,0.34f,0.44f,0.34f,true});
    layout.sockets.push_back({"yaw_left_aft", ThrusterRole::YawLeft, {-3.85f,-4.05f,0.28f}, {-1,0,0}, 0.15f, 1.30f, 0.48f,
                              ThrusterModuleShape::RcsBlock,"thruster_small",102.0f,0.34f,0.44f,0.34f,true});
    layout.sockets.push_back({"yaw_right_fore", ThrusterRole::YawRight, {-3.72f,3.65f,0.28f}, {-1,0,0}, 0.15f, 1.30f, 0.48f,
                              ThrusterModuleShape::RcsBlock,"thruster_small",78.0f,0.34f,0.44f,0.34f,true});
    layout.sockets.push_back({"yaw_right_aft", ThrusterRole::YawRight, {3.85f,-4.05f,0.28f}, {1,0,0}, 0.15f, 1.30f, 0.48f,
                              ThrusterModuleShape::RcsBlock,"thruster_small",-102.0f,0.34f,0.44f,0.34f,true});

    return layout;
}

ShipThrusterLayout ThrusterLayoutSystem::CompactUtility()
{
    ShipThrusterLayout layout;
    layout.layoutId = "compact_exposed_thrusters_v2";
    layout.sockets.push_back({"main_compact", ThrusterRole::Forward, {0.0f,-5.60f,0.16f}, {0,-1,0}, 0.34f, 3.8f, 1.0f,
                              ThrusterModuleShape::MainBell,"engine_small",180.0f,0.50f,0.62f,0.50f,true});
    layout.sockets.push_back({"retro_compact_port", ThrusterRole::Reverse, {-1.25f,4.75f,0.12f}, {0,1,0}, 0.18f, 1.75f, 0.72f,
                              ThrusterModuleShape::BrakeWedge,"thruster_small",0.0f,0.40f,0.46f,0.40f,true});
    layout.sockets.push_back({"retro_compact_starboard", ThrusterRole::Reverse, {1.25f,4.75f,0.12f}, {0,1,0}, 0.18f, 1.75f, 0.72f,
                              ThrusterModuleShape::BrakeWedge,"thruster_small",0.0f,0.40f,0.46f,0.40f,true});
    layout.sockets.push_back({"strafe_left_compact", ThrusterRole::StrafeLeft, {2.45f,-0.10f,0.18f}, {1,0,0}, 0.14f, 1.18f, 0.50f,
                              ThrusterModuleShape::AngledPod,"thruster_small",-90.0f,0.34f,0.44f,0.34f,true});
    layout.sockets.push_back({"strafe_right_compact", ThrusterRole::StrafeRight, {-2.45f,-0.10f,0.18f}, {-1,0,0}, 0.14f, 1.18f, 0.50f,
                              ThrusterModuleShape::AngledPod,"thruster_small",90.0f,0.34f,0.44f,0.34f,true});
    layout.sockets.push_back({"yaw_left_fore_compact", ThrusterRole::YawLeft, {2.15f,2.45f,0.20f}, {1,0,0}, 0.12f, 1.00f, 0.46f,
                              ThrusterModuleShape::RcsBlock,"thruster_small",-78.0f,0.29f,0.38f,0.29f,true});
    layout.sockets.push_back({"yaw_left_aft_compact", ThrusterRole::YawLeft, {-2.15f,-2.55f,0.20f}, {-1,0,0}, 0.12f, 1.00f, 0.46f,
                              ThrusterModuleShape::RcsBlock,"thruster_small",102.0f,0.29f,0.38f,0.29f,true});
    layout.sockets.push_back({"yaw_right_fore_compact", ThrusterRole::YawRight, {-2.15f,2.45f,0.20f}, {-1,0,0}, 0.12f, 1.00f, 0.46f,
                              ThrusterModuleShape::RcsBlock,"thruster_small",78.0f,0.29f,0.38f,0.29f,true});
    layout.sockets.push_back({"yaw_right_aft_compact", ThrusterRole::YawRight, {2.15f,-2.55f,0.20f}, {1,0,0}, 0.12f, 1.00f, 0.46f,
                              ThrusterModuleShape::RcsBlock,"thruster_small",-102.0f,0.29f,0.38f,0.29f,true});
    return layout;
}

ShipThrusterLayout ThrusterLayoutSystem::CombatInterceptor()
{
    auto layout = CompactUtility();
    layout.layoutId = "combat_vector_thrusters_v1";
    layout.sockets[0] = {"main_combat_port",ThrusterRole::Forward,{-1.10f,-5.85f,0.20f},{0,-1,0},0.30f,4.4f,1.0f,
                         ThrusterModuleShape::AngledPod,"thruster",192.0f,0.42f,0.60f,0.42f,true};
    layout.sockets.insert(layout.sockets.begin()+1,{"main_combat_starboard",ThrusterRole::Forward,{1.10f,-5.85f,0.20f},{0,-1,0},0.30f,4.4f,1.0f,
                                                     ThrusterModuleShape::AngledPod,"thruster",168.0f,0.42f,0.60f,0.42f,true});
    return layout;
}

ShipThrusterLayout ThrusterLayoutSystem::HeavyHauler()
{
    auto layout = StarterIndustrial();
    layout.layoutId = "heavy_hauler_thrusters_v1";
    for (auto& socket : layout.sockets) {
        if (socket.role == ThrusterRole::Forward) {
            socket.moduleShape = ThrusterModuleShape::MainBell;
            socket.moduleMesh = "engine_main";
            socket.visualScaleX = 0.42f;
            socket.visualScaleY = 0.50f;
            socket.visualScaleZ = 0.42f;
            socket.nozzleRadius *= 1.15f;
            socket.plumeLength *= 1.18f;
        }
    }
    return layout;
}

ShipThrusterLayout ThrusterLayoutSystem::ExplorationScout()
{
    auto layout = CompactUtility();
    layout.layoutId = "exploration_canted_thrusters_v1";
    for (auto& socket : layout.sockets) {
        if (socket.role == ThrusterRole::Forward) {
            socket.moduleShape = ThrusterModuleShape::VectorEmitter;
            socket.moduleMesh = "thruster";
            socket.visualScaleX = 0.36f;
            socket.visualScaleY = 0.52f;
            socket.visualScaleZ = 0.36f;
        }
    }
    return layout;
}

ShipThrusterLayout ThrusterLayoutSystem::ForShipRole(const std::string& shipRole)
{
    std::string upper=shipRole;
    for(char& c:upper) if(c>='a'&&c<='z') c=static_cast<char>(c-'a'+'A');
    // Preserve the certified Pass197 generic-role routing while richer role
    // names opt into the new external modular layouts.
    if(upper=="HAULER") return StarterIndustrial();
    if(upper=="ESCORT") return CompactUtility();
    if(upper.find("HEAVY HAUL")!=std::string::npos || upper.find("CARRIER")!=std::string::npos) return HeavyHauler();
    if(upper.find("COMBAT")!=std::string::npos || upper.find("FIGHT")!=std::string::npos ||
       upper.find("INTERCEPT")!=std::string::npos || upper.find("PATROL")!=std::string::npos) return CombatInterceptor();
    if(upper.find("EXPLO")!=std::string::npos || upper.find("SCOUT")!=std::string::npos || upper.find("PATH")!=std::string::npos) return ExplorationScout();
    if(upper.find("INDUSTR")!=std::string::npos || upper.find("MIN")!=std::string::npos ||
       upper.find("SALV")!=std::string::npos) return StarterIndustrial();
    return CompactUtility();
}


ShipThrusterLayout ThrusterLayoutSystem::ForShipRecipe(const ProceduralShipVisualRecipe& recipe,
                                                       const std::vector<ShipyardModuleRecord>& catalog,
                                                       const std::string& shipRole)
{
    if (recipe.modules.empty() || catalog.empty()) return ForShipRole(shipRole);

    const auto* root = FindModuleRecord(catalog, recipe.modules.front().moduleId);
    if (!root || !root->primaryHull) return ForShipRole(shipRole);

    ShipThrusterLayout layout;
    layout.layoutId = "recipe_bound_kitbash_propulsion_v1";

    // Existing terminal main engines are already rendered as part of the
    // certified assembly graph.  Reuse their authored exhaust sockets for
    // thrust/plume placement but do not draw a second floating engine mesh.
    for (std::size_t i=0; i<recipe.modules.size(); ++i) {
        const auto& placement = recipe.modules[i];
        const auto* record = FindModuleRecord(catalog, placement.moduleId);
        if (!record || record->partRole != ShipyardPartRole::MainEngine) continue;
        const auto* exhaust = FindSocket(*record, "exhaust");
        if (!exhaust) continue;

        ShipThrusterSocket socket;
        socket.id = "recipe_main_" + std::to_string(i);
        socket.role = ThrusterRole::Forward;
        socket.localPosition = TransformSocket(placement, *exhaust);
        socket.plumeDirection = TransformNormal(placement, *exhaust);
        socket.nozzleRadius = std::clamp(MajorHalfExtent(*record) * placement.scaleX * 0.20f, 0.18f, 0.58f);
        socket.plumeLength = std::clamp(root->source.halfLength * recipe.modules.front().scaleY * 1.15f, 3.2f, 7.5f);
        socket.power = 1.0f;
        socket.moduleShape = ThrusterModuleShape::MainBell;
        socket.moduleMesh = record->source.moduleId;
        socket.externalMount = false;
        socket.renderModule = false;
        socket.renderNozzle = false;
        layout.sockets.push_back(std::move(socket));
    }

    // The Greyoxide corpus currently has many terminal propulsion bodies but
    // no dedicated certified RCS family.  Use the real MainEngine variants as
    // compact attitude pods.  They retain authored proportions and are mounted
    // by their real engine_mount socket against the actual selected hull
    // envelope.  Different slots deterministically sample the propulsion
    // family so one ship no longer repeats the same generic thruster mesh.
    std::vector<const ShipyardModuleRecord*> podCandidates;
    for (const auto& record : catalog) {
        if (record.partRole == ShipyardPartRole::MainEngine && record.functional)
            podCandidates.push_back(&record);
    }
    std::sort(podCandidates.begin(), podCandidates.end(), [](const auto* a, const auto* b) {
        const float am = MajorHalfExtent(*a), bm = MajorHalfExtent(*b);
        if (am != bm) return am < bm;
        return a->source.moduleId < b->source.moduleId;
    });
    if (podCandidates.empty()) return layout.sockets.empty() ? ForShipRole(shipRole) : layout;

    const auto& hp = recipe.modules.front();
    const float hw = std::max(0.35f, root->source.halfWidth  * hp.scaleX);
    const float hl = std::max(0.55f, root->source.halfLength * hp.scaleY);
    const float hh = std::max(0.20f, root->source.halfHeight * hp.scaleZ);
    const float cx = hp.x, cy = hp.y, cz = hp.z + hh*0.08f;

    struct Target {
        const char* id;
        ThrusterRole role;
        Vector3 point;
        Vector3 outward;
        ThrusterModuleShape shape;
        float power;
    };
    const Target targets[] = {
        {"retro_port",       ThrusterRole::Reverse,     {cx-hw*.46f,cy+hl*.76f,cz}, {0, 1,0}, ThrusterModuleShape::BrakeWedge,.78f},
        {"retro_starboard",  ThrusterRole::Reverse,     {cx+hw*.46f,cy+hl*.76f,cz}, {0, 1,0}, ThrusterModuleShape::BrakeWedge,.78f},
        {"strafe_left_fore", ThrusterRole::StrafeLeft,  {cx+hw*.98f,cy+hl*.27f,cz}, {1, 0,0}, ThrusterModuleShape::AngledPod,.62f},
        {"strafe_left_aft",  ThrusterRole::StrafeLeft,  {cx+hw*.98f,cy-hl*.31f,cz}, {1, 0,0}, ThrusterModuleShape::AngledPod,.62f},
        {"strafe_right_fore",ThrusterRole::StrafeRight, {cx-hw*.98f,cy+hl*.27f,cz}, {-1,0,0}, ThrusterModuleShape::AngledPod,.62f},
        {"strafe_right_aft", ThrusterRole::StrafeRight, {cx-hw*.98f,cy-hl*.31f,cz}, {-1,0,0}, ThrusterModuleShape::AngledPod,.62f},
        {"yaw_left_fore",    ThrusterRole::YawLeft,     {cx+hw*.98f,cy+hl*.58f,cz+hh*.08f}, {1,0,0}, ThrusterModuleShape::RcsBlock,.52f},
        {"yaw_left_aft",     ThrusterRole::YawLeft,     {cx-hw*.98f,cy-hl*.58f,cz+hh*.08f}, {-1,0,0}, ThrusterModuleShape::RcsBlock,.52f},
        {"yaw_right_fore",   ThrusterRole::YawRight,    {cx-hw*.98f,cy+hl*.58f,cz+hh*.08f}, {-1,0,0}, ThrusterModuleShape::RcsBlock,.52f},
        {"yaw_right_aft",    ThrusterRole::YawRight,    {cx+hw*.98f,cy-hl*.58f,cz+hh*.08f}, {1,0,0}, ThrusterModuleShape::RcsBlock,.52f},
    };

    const float targetHalf = std::clamp(std::min(hw,hl)*0.12f, 0.20f, 0.48f);
    const std::uint32_t baseHash = StableHash(recipe.recipeId, recipe.seed);

    for (std::size_t i=0; i<sizeof(targets)/sizeof(targets[0]); ++i) {
        const auto* pod = podCandidates[(baseHash + static_cast<std::uint32_t>(i*7u)) % podCandidates.size()];
        const auto* mount = FindSocket(*pod, "mount");
        const auto* exhaust = FindSocket(*pod, "exhaust");
        if (!mount || !exhaust) continue;

        const float scale = std::clamp(targetHalf / MajorHalfExtent(*pod), 0.12f, 0.42f);
        const float yaw = YawToOpposeSurface(*mount, targets[i].outward);
        const Vector3 mountLocal = RotateZ({mount->x*scale,mount->y*scale,mount->z*scale}, yaw);
        const Vector3 outward = targets[i].outward.normalized();
        const Vector3 origin = targets[i].point - mountLocal - outward*0.025f;

        const Vector3 exhaustLocal = RotateZ({exhaust->x*scale,exhaust->y*scale,exhaust->z*scale}, yaw);
        Vector3 exhaustDir = RotateZ({exhaust->dirX,exhaust->dirY,exhaust->dirZ}, yaw);
        if (exhaustDir.length() < 1.0e-6f) exhaustDir = outward;
        else exhaustDir = exhaustDir.normalized();

        ShipThrusterSocket socket;
        socket.id = targets[i].id;
        socket.role = targets[i].role;
        socket.localPosition = origin;
        socket.exhaustOffset = exhaustLocal;
        socket.plumeDirection = exhaustDir;
        socket.nozzleRadius = std::clamp(targetHalf*.34f, 0.065f, 0.17f);
        socket.plumeLength = (targets[i].role==ThrusterRole::Reverse ? 2.3f : 1.45f);
        socket.power = targets[i].power;
        socket.moduleShape = targets[i].shape;
        socket.moduleMesh = pod->source.moduleId;
        socket.mountYawDegrees = yaw;
        socket.visualScaleX = socket.visualScaleY = socket.visualScaleZ = scale;
        socket.externalMount = true;
        layout.sockets.push_back(std::move(socket));
    }

    // Fail closed only if the recipe could not supply a usable propulsion
    // presentation.  Otherwise preserve the recipe-bound layout even if a
    // damaged/incomplete future ship intentionally lacks one axis.
    return layout.sockets.empty() ? ForShipRole(shipRole) : layout;
}

float ThrusterLayoutSystem::RoleActivity(ThrusterRole role, const InputState& input)
{
    switch (role) {
        case ThrusterRole::Forward: return input.GetValue(InputAction::ThrustForward);
        case ThrusterRole::Reverse: return input.GetValue(InputAction::ThrustReverse);
        case ThrusterRole::StrafeLeft: return input.GetValue(InputAction::StrafeLeft);
        case ThrusterRole::StrafeRight: return input.GetValue(InputAction::StrafeRight);
        case ThrusterRole::YawLeft: return input.GetValue(InputAction::TurnLeft);
        case ThrusterRole::YawRight: return input.GetValue(InputAction::TurnRight);
    }
    return 0.0f;
}

} // namespace subspace
