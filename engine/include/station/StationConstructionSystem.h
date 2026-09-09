#pragma once

#include "navigation/AstronomicalScaleSystem.h"
#include "station/StationModuleRole.h"
#include <cstdint>
#include <string>
#include <vector>

namespace subspace {

enum class StationLocationKind { PlanetOrbit, MoonOrbit, Belt, Ring, SalvageField, TradeLane, SubspaceTear, DeepSpace };
struct StationPlacementRequest { StationLocationKind kind=StationLocationKind::DeepSpace; AstronomicalPosition position; double nearestBodyRadiusMeters=0; double altitudeMeters=0; bool restrictedTerritory=false; bool hasPermission=true; };
struct StationModuleNative { StationModuleType type=StationModuleType::Core; std::string id; double structuralMaterial=0; double powerNet=0; double storage=0; };
struct PlayerStationDesign { std::string name; std::vector<StationModuleNative> modules; AstronomicalPosition position; StationLocationKind location=StationLocationKind::DeepSpace; };
struct StationPlacementResult { bool valid=false; std::string reason; };

class StationConstructionSystem {
public:
    StationPlacementResult ValidatePlacement(const StationPlacementRequest& request) const;
    bool HasOperationalCore(const PlayerStationDesign& station) const;
    std::vector<std::string> EmergentRoles(const PlayerStationDesign& station) const;
    double StructuralMaterialRequired(const PlayerStationDesign& station) const;
};

} // namespace subspace
