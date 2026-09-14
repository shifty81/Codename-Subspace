#pragma once

#include "ship_editor/ShipyardSelectionSystem.h"
#include "ship_editor/ShipyardTransformSystem.h"
#include "ship_editor/ShipyardWorkspaceSystem.h"

#include <string>

namespace subspace {

enum class ShipyardCandidateState {
    None,
    Preview,
    Draft,
    Validated,
    Certified
};

struct ShipyardSession {
    ShipyardObjectId documentId{};
    ShipyardWorkspaceMode workspace = ShipyardWorkspaceMode::Build;
    ShipyardTransformTool activeTool = ShipyardTransformTool::Select;
    ShipyardTransformSpace transformSpace = ShipyardTransformSpace::View;
    bool transformSnap = true;
    bool advancedVisible = false;
    bool layoutLocked = false;
    bool universalSearchOpen = false;
    bool auxiliaryDocksCollapsed = false;
    std::string maximizedPanelId;
    std::string hoveredPanelId;
    std::string layoutPreset = "BUILD";
    std::string commandSearch;
    ShipyardSelectionState selection{};
    ShipyardCandidateState candidateState = ShipyardCandidateState::None;
    std::string status = "Shipyard session ready";
};

class ShipyardSessionSystem {
public:
    static ShipyardSession Create(ShipyardObjectId documentId);
    static void BindDocument(ShipyardSession& session, ShipyardObjectId documentId);
    static void SetWorkspace(ShipyardSession& session, ShipyardWorkspaceMode workspace);
    static void SetTool(ShipyardSession& session, ShipyardTransformTool tool);
    static void ResetTransientState(ShipyardSession& session);
    static const char* CandidateStateName(ShipyardCandidateState state);
};

} // namespace subspace
