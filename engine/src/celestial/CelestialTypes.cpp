#include "celestial/CelestialTypes.h"

#include <sstream>

namespace subspace {

std::string CelestialBodyTypeName(CelestialBodyType type) {
    switch (type) {
        case CelestialBodyType::Star: return "Star";
        case CelestialBodyType::RockyPlanet: return "RockyPlanet";
        case CelestialBodyType::DryTerranPlanet: return "DryTerranPlanet";
        case CelestialBodyType::TerranPlanet: return "TerranPlanet";
        case CelestialBodyType::RiverWorld: return "RiverWorld";
        case CelestialBodyType::IceWorld: return "IceWorld";
        case CelestialBodyType::LavaWorld: return "LavaWorld";
        case CelestialBodyType::GasGiant: return "GasGiant";
        case CelestialBodyType::RingedGasGiant: return "RingedGasGiant";
        case CelestialBodyType::AsteroidBelt: return "AsteroidBelt";
        case CelestialBodyType::BlackHole: return "BlackHole";
        case CelestialBodyType::GalaxyBackdrop: return "GalaxyBackdrop";
        default: return "Unknown";
    }
}

std::string CelestialOrbitRoleName(CelestialOrbitRole role) {
    switch (role) {
        case CelestialOrbitRole::Primary: return "Primary";
        case CelestialOrbitRole::InnerSystem: return "InnerSystem";
        case CelestialOrbitRole::HabitableBand: return "HabitableBand";
        case CelestialOrbitRole::OuterSystem: return "OuterSystem";
        case CelestialOrbitRole::Belt: return "Belt";
        case CelestialOrbitRole::DeepSpace: return "DeepSpace";
        default: return "Unknown";
    }
}

std::string CelestialBodySummary(const CelestialBodyDefinition& body) {
    std::ostringstream stream;
    stream << body.displayName << " [" << CelestialBodyTypeName(body.type) << "] "
           << "orbit=" << body.orbitRadius
           << " radius=" << body.visualRadius
           << " resources=" << body.resourceRichness;
    if (body.hasRings) {
        stream << " rings";
    }
    if (body.hasAtmosphere) {
        stream << " atmosphere";
    }
    if (body.isHazardous) {
        stream << " hazardous";
    }
    return stream.str();
}

std::string StarSystemSummary(const StarSystemDefinition& system) {
    std::ostringstream stream;
    stream << system.displayName << " seed=" << system.seed
           << " primary=" << CelestialBodyTypeName(system.primary.type)
           << " bodies=" << system.bodies.size();
    return stream.str();
}

} // namespace subspace
