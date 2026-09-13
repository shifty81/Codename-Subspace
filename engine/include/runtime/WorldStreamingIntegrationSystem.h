#pragma once

#include "runtime/WorldSimulationAuthority.h"

#include <cstdint>
#include <string>

namespace subspace {

enum class StreamingHandoffState : std::uint8_t { Requested, WaitingForResidency, Ready, Activated, Failed };

struct StreamingHandoff {
    std::uint64_t ticket=0;
    PersistentEntityId destinationId{};
    SimulationRepresentation target=SimulationRepresentation::Proxy;
    int priority=0;
    std::string reason;
    StreamingHandoffState state=StreamingHandoffState::Requested;
    std::string diagnostic;
};

/// Converts abstract WorldSimulationAuthority prefetch requests into explicit
/// streaming handoffs.  Platform/system/interior/planet streamers can perform
/// I/O asynchronously, then call CompleteResidency to atomically publish the
/// persistent destination as resident at the requested representation.
class WorldStreamingIntegrationSystem {
public:
    WorldPrefetchRequest Queue(WorldSimulationAuthority& world,PersistentEntityId destination,
                               SimulationRepresentation target,int priority,std::string reason) const;
    bool AcquireNext(WorldSimulationAuthority& world,StreamingHandoff& out) const;
    bool CompleteResidency(WorldSimulationAuthority& world,StreamingHandoff& handoff,bool success,
                           std::string diagnostic={}) const;
};

} // namespace subspace
