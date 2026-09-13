#pragma once

#include "ui/SubspaceUiFramework.h"

#include <string>
#include <vector>

namespace subspace {

enum class GameUiAnchor { TopLeft,TopCenter,TopRight,Left,Center,Right,BottomLeft,BottomCenter,BottomRight };
enum class GameUiPanelKind { CommandRail,Overview,Target,Fleet,ShipStatus,Navigation,Chat,Notifications,Hotbar,Inspector,Services,Map,Fitting,Market,Industry };

struct GameUiTheme {
    std::string name="Subspace Dark";
    float panelOpacity=.92f;
    float cornerRadius=8.0f;
    float spacing=8.0f;
    float textScale=1.0f;
    std::string fontFamily="Segoe UI";
    float bodyTextPx=16.0f;
    float smallTextPx=14.0f;
    float headingTextPx=20.0f;
    float minimumContrast=0.72f;
    float clickableRowHeight=34.0f;
    bool compact=true;
};

struct GameUiPanelSpec {
    GameUiPanelKind kind=GameUiPanelKind::Overview;
    GameUiAnchor anchor=GameUiAnchor::Left;
    float width=280.0f;
    float height=220.0f;
    bool collapsible=true;
    bool movable=true;
    bool resizable=true;
    bool visible=true;
    float opacity=.92f;
};

class GameUiFramework {
public:
    GameUiTheme DefaultTheme() const;
    std::vector<GameUiPanelSpec> DefaultFlightWorkspace() const;
    SubspaceDockWorkspace DefaultFlightDockWorkspace() const;
    SubspaceDockWorkspace DefaultStrategicDockWorkspace() const;
    bool Validate(const std::vector<GameUiPanelSpec>& panels) const;
};

} // namespace subspace
