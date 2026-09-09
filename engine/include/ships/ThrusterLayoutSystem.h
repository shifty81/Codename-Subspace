#pragma once

#include "core/Math.h"
#include "input/InputState.h"
#include "rendering/ProceduralVisualVariantSystem.h"
#include "content/ShipyardModuleSystem.h"

#include <string>
#include <vector>

namespace subspace {

enum class ThrusterRole {
    Forward,
    Reverse,
    StrafeLeft,
    StrafeRight,
    YawLeft,
    YawRight
};

enum class ThrusterModuleShape {
    MainBell,
    TwinPod,
    BrakeWedge,
    AngledPod,
    RcsBlock,
    VectorEmitter
};

struct ShipThrusterSocket {
    std::string id;
    ThrusterRole role = ThrusterRole::Forward;
    Vector3 localPosition{};   // same local coordinates as native module assembly
    Vector3 plumeDirection{};  // exhaust direction; reaction force is opposite
    float nozzleRadius = 0.22f;
    float plumeLength = 2.0f;
    float power = 1.0f;

    // Pass341: visible thrusters are first-class modules.  These fields bind a
    // socket to an authored mesh already present in the ship module library.
    // The renderer may fall back to generated housing geometry if the mesh is
    // absent, but the socket remains authoritative for both visuals and force.
    ThrusterModuleShape moduleShape = ThrusterModuleShape::RcsBlock;
    std::string moduleMesh = "thruster_small";
    float mountYawDegrees = 0.0f;
    float visualScaleX = 1.0f;
    float visualScaleY = 1.0f;
    float visualScaleZ = 1.0f;
    bool externalMount = true;

    // Pass451: module origin and exhaust origin are distinct.  Kitbash
    // propulsion pieces are mounted flush to the resolved hull surface while
    // plume/nozzle effects originate at the authored exhaust socket.
    Vector3 exhaustOffset{};
    bool renderModule = true;
    bool renderNozzle = true;
};

struct ShipThrusterLayout {
    std::string layoutId;
    std::vector<ShipThrusterSocket> sockets;

    bool HasRole(ThrusterRole role) const;
    std::size_t CountRole(ThrusterRole role) const;
    bool IsCompleteSixAxisPlanarLayout() const;
    bool HasExternallyMountedRole(ThrusterRole role) const;
    std::size_t CountDistinctModuleShapes() const;
    bool HasClearPlumeDirections() const;
};

/// Canonical visual/physics thruster layout authority. Socket coordinates are
/// authored against the same local ship-module representation used by the
/// native renderer so exhaust can no longer float independently of the hull.
class ThrusterLayoutSystem {
public:
    static ShipThrusterLayout StarterIndustrial();
    static ShipThrusterLayout CompactUtility();
    static ShipThrusterLayout CombatInterceptor();
    static ShipThrusterLayout HeavyHauler();
    static ShipThrusterLayout ExplorationScout();
    static ShipThrusterLayout ForShipRole(const std::string& shipRole);

    /// Bind the six-axis propulsion presentation to the same assembled
    /// Shipyard recipe that is being rendered.  Existing main engines become
    /// plume-only sockets (the engine mesh is already part of the recipe);
    /// reverse/strafe/yaw pods use real certified MainEngine kitbash pieces
    /// mounted against the actual primary-hull envelope instead of fixed
    /// coordinates that can float far away from differently shaped hulls.
    static ShipThrusterLayout ForShipRecipe(const ProceduralShipVisualRecipe& recipe,
                                            const std::vector<ShipyardModuleRecord>& catalog,
                                            const std::string& shipRole);

    static float RoleActivity(ThrusterRole role, const InputState& input);
    static const char* ModuleShapeName(ThrusterModuleShape shape);
};

} // namespace subspace
