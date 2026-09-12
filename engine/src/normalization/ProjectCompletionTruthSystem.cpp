#include "normalization/ProjectCompletionTruthSystem.h"
#include <utility>

namespace subspace {

CompletionTruthState ProjectCompletionTruthSystem::Evaluate(const CompletionTruthEvidence& e){
    if(e.superseded)return CompletionTruthState::Superseded;
    if(e.legacy)return CompletionTruthState::Legacy;
    if(e.plannedOnly&&!e.sourceExists)return CompletionTruthState::Planned;
    if(e.containsPlaceholder)return e.runtimeWired?CompletionTruthState::Partial:CompletionTruthState::Placeholder;
    if(!e.sourceExists)return CompletionTruthState::Planned;
    if(!e.compiles)return CompletionTruthState::Scaffold;
    if(!e.unitBehavior)return CompletionTruthState::Scaffold;
    if(!e.runtimeWired)return CompletionTruthState::ImplementedUncertified;
    if(!e.runtimeSmoke)return CompletionTruthState::Partial;
    return CompletionTruthState::CertifiedRuntime;
}

CompletionTruthRecord ProjectCompletionTruthSystem::Build(std::string id,CompletionTruthEvidence e){
    CompletionTruthRecord r;r.systemId=std::move(id);r.state=Evaluate(e);r.evidence=std::move(e);
    switch(r.state){
    case CompletionTruthState::CertifiedRuntime:r.reason="runtime-wired behavioral evidence and smoke certification";break;
    case CompletionTruthState::ImplementedUncertified:r.reason="implementation exists but runtime wiring/certification is incomplete";break;
    case CompletionTruthState::Partial:r.reason="runtime path exists but behavioral/runtime evidence is incomplete";break;
    case CompletionTruthState::Scaffold:r.reason="API/source scaffold exists without complete behavior";break;
    case CompletionTruthState::Placeholder:r.reason="placeholder behavior remains";break;
    case CompletionTruthState::Legacy:r.reason="legacy donor/reference only";break;
    case CompletionTruthState::Superseded:r.reason="superseded by newer authority";break;
    case CompletionTruthState::Planned:r.reason="planned; no implementation authority";break;
    }
    return r;
}

const char* ProjectCompletionTruthSystem::Name(CompletionTruthState s){
    switch(s){
    case CompletionTruthState::CertifiedRuntime:return "CERTIFIED_RUNTIME";
    case CompletionTruthState::ImplementedUncertified:return "IMPLEMENTED_UNCERTIFIED";
    case CompletionTruthState::Partial:return "PARTIAL";
    case CompletionTruthState::Scaffold:return "SCAFFOLD";
    case CompletionTruthState::Placeholder:return "PLACEHOLDER";
    case CompletionTruthState::Legacy:return "LEGACY";
    case CompletionTruthState::Superseded:return "SUPERSEDED";
    case CompletionTruthState::Planned:return "PLANNED";
    }
    return "PLANNED";
}
bool ProjectCompletionTruthSystem::MayClaimDone(CompletionTruthState s){return s==CompletionTruthState::CertifiedRuntime;}

} // namespace subspace
