#pragma once

#include <string>
#include <vector>

namespace subspace {

enum class ShipyardActivityTab { Activity, Validation, History, Console, Search, Build, Pcc };

struct ShipyardActivityTabDescriptor {
    ShipyardActivityTab tab = ShipyardActivityTab::Activity;
    std::string id;
    std::string label;
    bool advanced = false;
};

struct ShipyardActivityDockState {
    ShipyardActivityTab active = ShipyardActivityTab::Activity;
    bool collapsed = true;
    float expandedHeight = 220.0f;
    bool autoRevealErrors = true;
};

class ShipyardActivityDockSystem {
public:
    static std::vector<ShipyardActivityTabDescriptor> Tabs();
    static ShipyardActivityDockState DefaultState();
    static bool Activate(ShipyardActivityDockState& state, ShipyardActivityTab tab, bool expand = true);
    static void ToggleCollapsed(ShipyardActivityDockState& state);
    static void RevealValidationFailure(ShipyardActivityDockState& state);
    static float ClampExpandedHeight(float value);
};

} // namespace subspace
