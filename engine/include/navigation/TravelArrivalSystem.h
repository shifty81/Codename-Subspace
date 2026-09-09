#pragma once

#include "core/Math.h"
#include "navigation/SystemNavigationSystem.h"
#include "procedural/GalaxyGenerator.h"

#include <cstdint>
#include <string>

namespace subspace {

enum class ArrivalEnvironment {
    DeepSpace,
    PlanetOrbit,
    MoonOrbit,
    StationApproach,
    BeltField,
    RingField,
    SalvageField,
    SignatureSite,
    FleetRendezvous
};

struct TravelArrivalEnvelope {
    bool valid = false;
    std::uint64_t destinationId = 0;
    ArrivalEnvironment environment = ArrivalEnvironment::DeepSpace;
    std::string localSceneId;
    Vector3 focalWorld{};
    Vector3 arrivalWorld{};
    float arrivalYaw = 0.0f;
    double standOffWorld = 0.0;
    double finalApproachSeconds = 0.0;
    bool safeFromPlanetSurface = true;
    bool safeFromDenseRing = true;
    double streamLeadSeconds = 0.0;
    double sensorContactSeconds = 0.0;
    bool localSceneReadyAtExit = false;
    bool targetVisibleAtExit = false;
};

/// Pass301/304/305 travel authority. Warp destinations remain real locations
/// in the solar system but resolve to a safe local-scene arrival envelope,
/// never directly onto the object being visited.
class TravelArrivalSystem {
public:
    TravelArrivalEnvelope Resolve(const SystemDestination& destination,
                                  const GalaxySector& sector,
                                  float localCruiseSpeed = 18.0f) const;

    static double DefaultStandOff(SystemDestinationType type, bool discovered = true);
    static float YawToward(const Vector3& from, const Vector3& to);
};

} // namespace subspace
