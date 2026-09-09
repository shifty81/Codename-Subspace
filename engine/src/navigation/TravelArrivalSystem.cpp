#include "navigation/TravelArrivalSystem.h"

#include "rendering/CelestialEnvironmentSystem.h"
#include "celestial/OrbitalDynamicsSystem.h"

#include <algorithm>
#include <cmath>

namespace subspace {
namespace {
const PlanetData* FindPlanet(const GalaxySector& sector, std::uint64_t id) {
    for (const auto& p : sector.planets) {
        std::uint64_t parsed = 0;
        try { parsed = static_cast<std::uint64_t>(std::stoull(p.planetId)); } catch (...) {}
        if (parsed == id || std::hash<std::string>{}(p.planetId) == id) return &p;
    }
    return nullptr;
}
}

double TravelArrivalSystem::DefaultStandOff(SystemDestinationType type, bool discovered) {
    switch (type) {
        case SystemDestinationType::Planet: return 120.0;
        case SystemDestinationType::Moon: return 75.0;
        case SystemDestinationType::Station: return 90.0;
        case SystemDestinationType::BeltRegion: return 180.0;
        case SystemDestinationType::RingRegion: return 160.0;
        case SystemDestinationType::SalvageSite: return 140.0;
        case SystemDestinationType::Signature: return discovered ? 180.0 : 260.0;
        case SystemDestinationType::FleetMember: return 80.0;
        case SystemDestinationType::Bookmark: return 100.0;
        default: return 120.0;
    }
}

float TravelArrivalSystem::YawToward(const Vector3& from, const Vector3& to) {
    const float dx = to.x - from.x;
    const float dy = to.y - from.y;
    if (std::fabs(dx) + std::fabs(dy) < 0.0001f) return 0.0f;
    // Ship forward is {-sin(yaw), cos(yaw)}.
    return std::atan2(-dx, dy);
}

TravelArrivalEnvelope TravelArrivalSystem::Resolve(const SystemDestination& d,
                                                    const GalaxySector& sector,
                                                    float localCruiseSpeed) const {
    TravelArrivalEnvelope out;
    if (d.id == 0 || d.name.empty()) return out;
    out.valid = true;
    out.destinationId = d.id;
    out.standOffWorld = DefaultStandOff(d.type, d.discovered);
    out.localSceneId = "system:" + std::to_string(sector.x) + ":" + std::to_string(sector.y) + ":" + d.name;

    // Navigation registration stores system-local coordinates in metres. The
    // corresponding generated scene position uses the same 1e6 conversion.
    out.focalWorld = {static_cast<float>(d.position.localX / 1000000.0) * 0.0120f,
                      static_cast<float>(d.position.localY / 1000000.0) * 0.0120f, 0.0f};

    switch (d.type) {
        case SystemDestinationType::Planet: {
            out.environment = ArrivalEnvironment::PlanetOrbit;
            // Match by nearest generated planet world position when an opaque
            // navigation id cannot be reversed into a planetId.
            const PlanetData* nearest = nullptr;
            float best = 1e30f;
            for (const auto& p : sector.planets) {
                const Vector3 pw{p.position.x*0.0120f,p.position.y*0.0120f,0.0f};
                const float dx = pw.x - out.focalWorld.x, dy = pw.y - out.focalWorld.y;
                const float ds = dx*dx + dy*dy;
                if (ds < best) { best = ds; nearest = &p; out.focalWorld = pw; }
            }
            if (nearest) {
                CelestialEnvironmentSystem celestial;
                const float radius = celestial.WorldRadius(*nearest);
                out.standOffWorld = std::max<double>(out.standOffWorld, radius * 1.18 + 90.0);
                const auto profile = celestial.ProfileFor(*nearest);
                if (profile.supportsDenseRingField) {
                    // Visual rings can be enormous, but routine orbital arrival
                    // should still land in a useful 20-60 second local-approach
                    // envelope rather than several minutes from the planet.
                    const double ringSafe = std::min<double>(1100.0, radius * 2.20 + 140.0);
                    out.standOffWorld = std::max<double>(out.standOffWorld, ringSafe);
                }
            }
            break;
        }
        case SystemDestinationType::Moon: out.environment = ArrivalEnvironment::MoonOrbit; break;
        case SystemDestinationType::Station: out.environment = ArrivalEnvironment::StationApproach; break;
        case SystemDestinationType::BeltRegion: out.environment = ArrivalEnvironment::BeltField; break;
        case SystemDestinationType::RingRegion: out.environment = ArrivalEnvironment::RingField; break;
        case SystemDestinationType::SalvageSite: out.environment = ArrivalEnvironment::SalvageField; break;
        case SystemDestinationType::Signature: out.environment = ArrivalEnvironment::SignatureSite; break;
        case SystemDestinationType::FleetMember: out.environment = ArrivalEnvironment::FleetRendezvous; break;
        default: out.environment = ArrivalEnvironment::DeepSpace; break;
    }

    // Deterministic, off-axis arrival avoids every destination presenting from
    // the exact same direction and guarantees a real normal-flight approach.
    const float side = (d.id & 1u) ? 1.0f : -1.0f;
    const float xBias = static_cast<float>(out.standOffWorld * 0.38 * side);
    const float yBias = static_cast<float>(out.standOffWorld * 0.925);
    out.arrivalWorld = {out.focalWorld.x + xBias, out.focalWorld.y - yBias, 0.0f};

    // Pass515: final local-scene certification. The target standoff calculation
    // is not enough when another large/ringed planet overlaps the chosen arrival
    // vector. Project against every rendered celestial envelope and the stellar
    // safety radius before the tunnel is allowed to reveal the destination.
    CelestialEnvironmentSystem celestial;
    for(int pass=0;pass<4;++pass){
        bool moved=false;
        for(const auto& planet:sector.planets){
            const Vector3 pw{planet.position.x*0.0120f,planet.position.y*0.0120f,0.0f};
            const Vector3 repaired=celestial.ProjectOutsideLocalOrbit(out.arrivalWorld,pw,planet,28.0f);
            if((repaired-out.arrivalWorld).length()>0.001f){out.arrivalWorld=repaired;moved=true;}
        }
        if(!moved)break;
    }
    if(sector.hasStar){
        OrbitalDynamicsSystem dynamics;
        const Vector3 sw{sector.star.position.x*0.0120f,sector.star.position.y*0.0120f,0.0f};
        const auto safety=dynamics.StarSafety(sector.star);
        Vector3 delta=out.arrivalWorld-sw;float dist=delta.length();const float safe=safety.spawnRadius+120.0f;
        if(dist<safe){if(dist<0.001f){delta={1,0,0};dist=1;}out.arrivalWorld=sw+delta*(safe/dist);}
    }
    out.arrivalYaw = YawToward(out.arrivalWorld, out.focalWorld);
    out.finalApproachSeconds = std::clamp(out.standOffWorld / std::max(1.0f, localCruiseSpeed), 12.0, 60.0);
    out.safeFromPlanetSurface = out.environment != ArrivalEnvironment::PlanetOrbit ||
        ([&](){
            float nearestRadius=0.0f;
            for(const auto& p:sector.planets){
                const Vector3 pw{p.position.x*0.0120f,p.position.y*0.0120f,0.0f};
                const float dx=pw.x-out.focalWorld.x,dy=pw.y-out.focalWorld.y;
                if(dx*dx+dy*dy<4.0f){ CelestialEnvironmentSystem c; nearestRadius=c.WorldRadius(p); break; }
            }
            return out.standOffWorld >= nearestRadius*1.05f;
        })();
    out.safeFromDenseRing = out.environment != ArrivalEnvironment::RingField || out.standOffWorld >= 120.0;
    out.streamLeadSeconds = std::clamp(out.finalApproachSeconds * 0.28, 1.25, 8.0);
    out.sensorContactSeconds = std::clamp(out.finalApproachSeconds * 0.18, 0.75, 5.0);
    out.localSceneReadyAtExit = true;
    out.targetVisibleAtExit = out.environment != ArrivalEnvironment::SignatureSite || d.discovered;
    return out;
}

} // namespace subspace
