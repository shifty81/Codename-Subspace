#pragma once

#include "content/ShipyardModuleSystem.h"
#include "ship_editor/ShipyardDocumentSystem.h"
#include "ship_editor/ShipyardMountProfileSystem.h"
#include "ship_editor/ShipyardSelectionSystem.h"
#include "ui/SubspaceUiFramework.h"

#include <string>
#include <vector>

namespace subspace {

struct ShipyardOutlinerNode {
    ShipyardObjectId id{};
    ShipyardObjectId parentId{};
    std::string label;
    std::string kind;
    int depth=0;
    bool selectable=true;
    bool expanded=true;
};

struct ShipyardPanelModel {
    std::vector<ShipyardOutlinerNode> outliner;
    std::vector<EditorPropertySection> properties;
    std::vector<EditorContextAction> actions;
    std::string selectedLabel;
    std::string selectedInstanceId;
    std::string selectedDefinitionId;
    bool definitionEditable = false;
};

class ShipyardPanelModelSystem {
public:
    static ShipyardPanelModel Build(const ShipyardDocument& document,
                                    const std::vector<ShipyardModuleRecord>& catalog,
                                    const ShipyardSelectionState& selection);
    static std::vector<ShipyardOutlinerNode> BuildOutliner(const ShipyardDocument& document,
                                                           const std::vector<ShipyardModuleRecord>& catalog);
    static std::vector<EditorPropertySection> BuildProperties(const ShipyardDocument& document,
                                                              const std::vector<ShipyardModuleRecord>& catalog,
                                                              const ShipyardSelectionState& selection);
};

} // namespace subspace
