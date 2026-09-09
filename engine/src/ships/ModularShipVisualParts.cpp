#include "ships/ModularShipVisualParts.h"

namespace subspace {

const char* ShipVisualPartRoleName(ShipVisualPartRole role) {
    switch (role) {
    case ShipVisualPartRole::Cockpit: return "Cockpit";
    case ShipVisualPartRole::Hull: return "Hull";
    case ShipVisualPartRole::Engine: return "Engine";
    case ShipVisualPartRole::RcsThruster: return "RCS Thruster";
    case ShipVisualPartRole::Cargo: return "Cargo";
    case ShipVisualPartRole::Weapon: return "Weapon";
    case ShipVisualPartRole::Shield: return "Shield";
    case ShipVisualPartRole::Scanner: return "Scanner";
    case ShipVisualPartRole::Utility: return "Utility";
    }
    return "Unknown";
}

std::vector<ShipVisualPartDefinition> CreateStarterVisualShipPartCatalog() {
    return {
        {"cockpit_compact_t1", ShipVisualPartRole::Cockpit, "Compact Command Pod", 0x8FD7FFu, 3.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1, {{"nose", 0.0f, -2.0f, 0.0f}}},
        {"hull_plate_t1", ShipVisualPartRole::Hull, "Framed Hull Plate", 0x8C98A6u, 2.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1, {}},
        {"engine_burner_t1", ShipVisualPartRole::Engine, "Burner Main Drive", 0xFFAA55u, 4.0f, 2.0f, 24.0f, 0.0f, 0.0f, 1, {{"aft_exhaust", 0.0f, 2.5f, 180.0f}}},
        {"rcs_quad_t1", ShipVisualPartRole::RcsThruster, "Quad RCS Pack", 0x66D9FFu, 1.2f, 0.8f, 0.0f, 14.0f, 0.0f, 1, {{"port", -1.7f, 0.0f, 90.0f}, {"starboard", 1.7f, 0.0f, -90.0f}}},
        {"cargo_pod_t1", ShipVisualPartRole::Cargo, "Clamp-On Cargo Pod", 0x9AD17Bu, 3.5f, 0.1f, 0.0f, 0.0f, 60.0f, 1, {}},
        {"scanner_t1", ShipVisualPartRole::Scanner, "Shortwave Scanner", 0xB388FFu, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1, {}},
    };
}

float SumVisualPartThrust(const std::vector<ShipVisualPartDefinition>& parts) {
    float total = 0.0f;
    for (const auto& part : parts) total += part.thrust;
    return total;
}

float SumVisualPartCargo(const std::vector<ShipVisualPartDefinition>& parts) {
    float total = 0.0f;
    for (const auto& part : parts) total += part.cargo;
    return total;
}

} // namespace subspace
