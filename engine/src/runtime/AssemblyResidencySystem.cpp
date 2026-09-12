#include "runtime/AssemblyResidencySystem.h"
namespace subspace {
AssemblyResidencyPolicy AssemblyResidencySystem::Resolve(const AssemblyResidencyRequest&r){
    if(r.playerInside)return {AssemblySimulationFidelity::Full,AssemblyRenderFidelity::Interior,AssemblyResidencyState::InteriorActive,AssemblyNetworkInterest::Authoritative};
    if(r.playerDocked)return {AssemblySimulationFidelity::Full,AssemblyRenderFidelity::Modules,AssemblyResidencyState::InteriorWarm,AssemblyNetworkInterest::Authoritative};
    if(r.combatRelevant||r.distanceMeters<5000.0)return {AssemblySimulationFidelity::Full,AssemblyRenderFidelity::Modules,AssemblyResidencyState::ExteriorFull,AssemblyNetworkInterest::Relevant};
    if(r.playerTargeted||r.distanceMeters<50000.0)return {AssemblySimulationFidelity::Regional,AssemblyRenderFidelity::Hlod,AssemblyResidencyState::ExteriorProxy,AssemblyNetworkInterest::Relevant};
    if(r.distanceMeters<5000000.0)return {AssemblySimulationFidelity::Aggregate,AssemblyRenderFidelity::StrategicIcon,AssemblyResidencyState::Metadata,AssemblyNetworkInterest::Aggregate};
    return {};
}
} // namespace subspace
