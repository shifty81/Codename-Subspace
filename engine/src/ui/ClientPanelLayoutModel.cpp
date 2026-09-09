#include "ui/ClientPanelLayoutModel.h"

namespace subspace {

const char* ClientPanelAnchorName(ClientPanelAnchor anchor) {
    switch (anchor) {
    case ClientPanelAnchor::TopLeft: return "TopLeft";
    case ClientPanelAnchor::TopRight: return "TopRight";
    case ClientPanelAnchor::BottomLeft: return "BottomLeft";
    case ClientPanelAnchor::BottomRight: return "BottomRight";
    case ClientPanelAnchor::Center: return "Center";
    case ClientPanelAnchor::Hidden: return "Hidden";
    }
    return "Unknown";
}

ClientPanelLayoutModel CreateDefaultClientPanelLayout(bool homeMode, bool travelMode, bool debugVisible) {
    ClientPanelLayoutModel model;
    model.panels.push_back({"hud", "Ship HUD", ClientPanelAnchor::TopLeft, 280, 120, !homeMode});
    model.panels.push_back({"target", "Target", ClientPanelAnchor::TopRight, 260, 100, !homeMode});
    model.panels.push_back({"home_surface", "Home Surface", ClientPanelAnchor::TopLeft, 320, 180, homeMode});
    model.panels.push_back({"home_factory", "Factory", ClientPanelAnchor::BottomLeft, 340, 160, homeMode});
    model.panels.push_back({"travel", "Rail Travel", ClientPanelAnchor::Center, 420, 220, travelMode});
    model.panels.push_back({"debug", "Debug", ClientPanelAnchor::BottomRight, 380, 200, debugVisible});
    return model;
}

int CountVisibleClientPanels(const ClientPanelLayoutModel& model) {
    int count = 0;
    for (const auto& panel : model.panels) if (panel.visible && panel.anchor != ClientPanelAnchor::Hidden) ++count;
    return count;
}

} // namespace subspace
