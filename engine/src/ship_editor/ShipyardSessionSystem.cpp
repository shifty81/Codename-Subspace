#include "ship_editor/ShipyardSessionSystem.h"

namespace subspace {

ShipyardSession ShipyardSessionSystem::Create(ShipyardObjectId documentId) {
    ShipyardSession out;
    out.documentId = documentId;
    return out;
}

void ShipyardSessionSystem::BindDocument(ShipyardSession& session, ShipyardObjectId documentId) {
    session.documentId = documentId;
    ResetTransientState(session);
    session.status = "Shipyard document bound";
}

void ShipyardSessionSystem::SetWorkspace(ShipyardSession& session, ShipyardWorkspaceMode workspace) {
    session.workspace = workspace;
    session.status = std::string("Workspace: ") + ShipyardWorkspaceSystem::WorkspaceName(workspace);
}

void ShipyardSessionSystem::SetTool(ShipyardSession& session, ShipyardTransformTool tool) {
    session.activeTool = tool;
    switch (tool) {
        case ShipyardTransformTool::Move: session.status = "Move tool"; break;
        case ShipyardTransformTool::Rotate: session.status = "Rotate tool"; break;
        case ShipyardTransformTool::Scale: session.status = "Scale tool"; break;
        default: session.status = "Select tool"; break;
    }
}

void ShipyardSessionSystem::ResetTransientState(ShipyardSession& session) {
    ShipyardSelectionSystem::Clear(session.selection);
    session.candidateState = ShipyardCandidateState::None;
    session.commandSearch.clear();
    session.universalSearchOpen = false;
    session.maximizedPanelId.clear();
    session.hoveredPanelId.clear();
}

const char* ShipyardSessionSystem::CandidateStateName(ShipyardCandidateState state) {
    switch (state) {
        case ShipyardCandidateState::Preview: return "PREVIEW";
        case ShipyardCandidateState::Draft: return "DRAFT";
        case ShipyardCandidateState::Validated: return "VALIDATED";
        case ShipyardCandidateState::Certified: return "CERTIFIED";
        default: return "NONE";
    }
}

} // namespace subspace
