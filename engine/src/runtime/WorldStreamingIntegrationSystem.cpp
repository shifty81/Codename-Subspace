#include "runtime/WorldStreamingIntegrationSystem.h"

namespace subspace {
WorldPrefetchRequest WorldStreamingIntegrationSystem::Queue(WorldSimulationAuthority& world,PersistentEntityId destination,
                                                             SimulationRepresentation target,int priority,std::string reason) const{
    return world.RequestPrefetch(destination,target,priority,std::move(reason));
}

bool WorldStreamingIntegrationSystem::AcquireNext(WorldSimulationAuthority& world,StreamingHandoff& out) const{
    const auto req=world.PopNextPrefetch();if(!req)return false;
    out.ticket=req->ticket;out.destinationId=req->destinationId;out.target=req->minimumRepresentation;out.priority=req->priority;out.reason=req->reason;
    const auto* entity=world.Find(out.destinationId);
    if(!entity){out.state=StreamingHandoffState::Failed;out.diagnostic="persistent destination disappeared before streaming handoff";return true;}
    if(entity->resident&&static_cast<unsigned>(entity->representation)>=static_cast<unsigned>(out.target))out.state=StreamingHandoffState::Ready;
    else out.state=StreamingHandoffState::WaitingForResidency;
    return true;
}

bool WorldStreamingIntegrationSystem::CompleteResidency(WorldSimulationAuthority& world,StreamingHandoff& handoff,bool success,std::string diagnostic) const{
    if(handoff.state==StreamingHandoffState::Failed||!handoff.destinationId.IsValid())return false;
    if(!success){handoff.state=StreamingHandoffState::Failed;handoff.diagnostic=std::move(diagnostic);return false;}
    if(!world.SetResident(handoff.destinationId,true)){handoff.state=StreamingHandoffState::Failed;handoff.diagnostic="failed to mark destination resident";return false;}
    if(!world.TransitionRepresentation(handoff.destinationId,handoff.target)){handoff.state=StreamingHandoffState::Failed;handoff.diagnostic="resident destination rejected requested representation";return false;}
    handoff.state=StreamingHandoffState::Activated;handoff.diagnostic=std::move(diagnostic);return true;
}
} // namespace subspace
