#pragma once

#include "core/Math.h"
#include "world/WorldScaleAuthoritySystem.h"
#include <string>
#include <vector>

namespace subspace {

enum class ApertureKind { Door, Window, Airlock, DockingCollar, HangarDoor, ForceField };
enum class VehicleEnvelopeKind { Hoverbike, Mech, Rover, Shuttle };

struct ArticulatedRegionDefinition {
    std::string id;
    std::string sourceModuleId;
    Vector3 pivotLocal{};
    Vector3 axisLocal{1.0f,0.0f,0.0f};
    float closedDegrees=0.0f;
    float openDegrees=90.0f;
    bool detachablePanel=false;
    bool pressureBoundary=false;
};

struct ApertureDefinition {
    std::string id;
    ApertureKind kind=ApertureKind::Door;
    float openingWidthMeters=1.05f;
    float openingHeightMeters=2.25f;
    float clearanceDepthMeters=0.8f;
    float supportedWallThicknessMeters=0.5f;
    bool pressureSeal=true;
    bool animated=true;
    bool playerPassable=true;
    bool vehiclePassable=false;
    bool usesForceField=false;
    ArticulatedRegionDefinition articulation{};
};

struct VehicleTransportEnvelope {
    VehicleEnvelopeKind kind=VehicleEnvelopeKind::Rover;
    Vector3 vehicleSizeMeters{2.1f,4.2f,1.8f};
    Vector3 requiredBaySizeMeters{3.4f,6.0f,2.8f};
    float approachClearanceMeters=1.0f;
};

struct HangarDefinition {
    std::string id;
    ApertureDefinition aperture{};
    std::vector<VehicleTransportEnvelope> supportedVehicles;
    unsigned capacity=1;
    bool internalPressurized=true;
    bool serviceAccessRequired=true;
};

struct ApertureValidationResult { bool valid=false; std::vector<std::string> errors; std::vector<std::string> warnings; };

/// Shared authoring contract for doors, windows, airlocks, docking collars and
/// hangar/ramp cuts. Exterior geometry only declares a legal cut/animation;
/// actual ship capability is compiled from the connected interior + systems.
class ApertureHangarAuthoringSystem {
public:
    static ApertureDefinition StandardDoor(const WorldScaleProfile& scale=WorldScaleAuthoritySystem::DefaultProfile());
    static ApertureDefinition StandardAirlock(const WorldScaleProfile& scale=WorldScaleAuthoritySystem::DefaultProfile());
    static ApertureDefinition StandardHangarDoor(const VehicleTransportEnvelope& vehicle,const WorldScaleProfile& scale=WorldScaleAuthoritySystem::DefaultProfile());
    static VehicleTransportEnvelope BaselineVehicle(VehicleEnvelopeKind kind);
    static ApertureValidationResult Validate(const ApertureDefinition& aperture,const WorldScaleProfile& scale=WorldScaleAuthoritySystem::DefaultProfile());
    static ApertureValidationResult Validate(const HangarDefinition& hangar,const WorldScaleProfile& scale=WorldScaleAuthoritySystem::DefaultProfile());
};

} // namespace subspace
