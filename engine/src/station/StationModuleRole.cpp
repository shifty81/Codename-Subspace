#include "station/StationModuleRole.h"

namespace subspace {
const char* StationModuleRoleName(StationModuleRole role) {
    switch (role) {
        case StationModuleRole::Core: return "CORE";
        case StationModuleRole::Dock: return "DOCK";
        case StationModuleRole::Storage: return "STORAGE";
        case StationModuleRole::Power: return "POWER";
        case StationModuleRole::Refinery: return "REFINERY";
        case StationModuleRole::Manufacturing: return "MANUFACTURING";
        case StationModuleRole::Shipyard: return "SHIPYARD";
        case StationModuleRole::Repair: return "REPAIR";
        case StationModuleRole::Market: return "MARKET";
        case StationModuleRole::Habitation: return "HABITATION";
        case StationModuleRole::Research: return "RESEARCH";
        case StationModuleRole::Logistics: return "LOGISTICS";
        case StationModuleRole::DroneControl: return "DRONE CONTROL";
        case StationModuleRole::Sensor: return "SENSOR";
        case StationModuleRole::Defense: return "DEFENSE";
        case StationModuleRole::ElevatorInterface: return "ELEVATOR INTERFACE";
    }
    return "CORE";
}
} // namespace subspace
