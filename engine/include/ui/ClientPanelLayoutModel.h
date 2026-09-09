#pragma once

#include <string>
#include <vector>

namespace subspace {

enum class ClientPanelAnchor { TopLeft, TopRight, BottomLeft, BottomRight, Center, Hidden };

struct ClientPanelDescriptor {
    std::string panelId;
    std::string title;
    ClientPanelAnchor anchor = ClientPanelAnchor::TopLeft;
    int width = 220;
    int height = 120;
    bool visible = true;
};

struct ClientPanelLayoutModel {
    std::vector<ClientPanelDescriptor> panels;
};

const char* ClientPanelAnchorName(ClientPanelAnchor anchor);
ClientPanelLayoutModel CreateDefaultClientPanelLayout(bool homeMode, bool travelMode, bool debugVisible);
int CountVisibleClientPanels(const ClientPanelLayoutModel& model);

} // namespace subspace
