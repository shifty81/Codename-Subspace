#pragma once

#include "ship_editor/ShipyardAssetBrowserSystem.h"
#include "ship_editor/ShipyardCommandSystem.h"
#include "ship_editor/ShipyardPanelModelSystem.h"
#include "ship_editor/ShipyardProfessionalUiSystem.h"
#include "ui/SubspaceUiFramework.h"

#include <string>
#include <vector>

namespace subspace {

enum class ShipyardSearchGroup { Command, Asset, ShipObject, Property, Panel, Validation, Developer };

struct ShipyardSearchResult {
    ShipyardSearchGroup group = ShipyardSearchGroup::Command;
    std::string id;
    std::string label;
    std::string context;
    float score = 0.0f;
    bool advanced = false;
};

class ShipyardUniversalSearchSystem {
public:
    static std::vector<ShipyardSearchResult> Search(std::string query,
                                                    const ShipyardCommandSystem& commands,
                                                    const std::vector<ShipyardModuleRecord>& catalog,
                                                    const ShipyardPanelModel& panelModel,
                                                    const std::vector<EditorValidationMessage>& validation,
                                                    bool includeAdvanced,
                                                    std::size_t limit = 40);
    static const char* GroupName(ShipyardSearchGroup group);
};

} // namespace subspace
