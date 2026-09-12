#pragma once
#include <cstdint>
namespace subspace {
enum class AssemblySimulationFidelity { Dormant, Aggregate, Regional, Full };
enum class AssemblyRenderFidelity { None, StrategicIcon, Impostor, Hlod, Modules, Interior };
enum class AssemblyResidencyState { Unloaded, Metadata, ExteriorProxy, ExteriorFull, InteriorWarm, InteriorActive };
enum class AssemblyNetworkInterest { None, Aggregate, Relevant, Authoritative };
struct AssemblyResidencyRequest {
    double distanceMeters=0.0;
    bool playerTargeted=false;
    bool playerDocked=false;
    bool playerInside=false;
    bool combatRelevant=false;
};
struct AssemblyResidencyPolicy {
    AssemblySimulationFidelity simulation=AssemblySimulationFidelity::Dormant;
    AssemblyRenderFidelity render=AssemblyRenderFidelity::None;
    AssemblyResidencyState residency=AssemblyResidencyState::Unloaded;
    AssemblyNetworkInterest network=AssemblyNetworkInterest::None;
};
class AssemblyResidencySystem {
public:
    static AssemblyResidencyPolicy Resolve(const AssemblyResidencyRequest& request);
};
} // namespace subspace
