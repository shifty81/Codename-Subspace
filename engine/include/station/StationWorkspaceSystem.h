#pragma once

#include "editor/EditorAssetBrowser.h"
#include "editor/SubspaceEditorCore.h"
#include "station/StationDesignDnaSystem.h"
#include "station/StationKitbashCatalogSystem.h"
#include "ui/SubspaceUiFramework.h"

#include <vector>

namespace subspace {

enum class StationWorkspaceMode { Build, Operations, Appearance, Authoring };

class StationWorkspaceSystem {
public:
    static EditorWorkspaceDescriptor Descriptor();
    static std::vector<EditorAssetCard> BuildAssetCards(const std::vector<StationKitbashPiece>& pieces);
    static std::vector<EditorOutlinerNode> BuildOutliner(const StationKitbashVisualRecipe& recipe);
    static std::vector<EditorPropertySection> BuildInspector(const StationKitbashVisualRecipe& recipe,
                                                             std::size_t selectedIndex,
                                                             StationWorkspaceMode mode,
                                                             bool advanced);
    static EditorHelpRegistry BuildHelpRegistry();
};

} // namespace subspace
