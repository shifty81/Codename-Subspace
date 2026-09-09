#pragma once

namespace subspace {

// Pass555 canonical station module taxonomy.  Historical StationModuleType and
// StationModuleFunction APIs are aliases of this one authority so construction,
// modular assembly, editor tooling, runtime services and PCG cannot drift.
enum class StationModuleRole {
    Core = 0,
    Dock,
    Storage,
    Power,
    Refinery,
    Manufacturing,
    Factory = Manufacturing,
    Shipyard,
    Repair,
    Market,
    Habitation,
    Research,
    Logistics,
    DroneControl,
    Sensor,
    Defense,
    ElevatorInterface
};

using StationModuleType = StationModuleRole;
using StationModuleFunction = StationModuleRole;

const char* StationModuleRoleName(StationModuleRole role);

} // namespace subspace
