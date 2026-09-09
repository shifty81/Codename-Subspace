#pragma once

#include <string>
#include <vector>

namespace subspace {

enum class GameUiAnchor { TopLeft, TopCenter, TopRight, Left, Center, Right, BottomLeft, BottomCenter, BottomRight };
enum class GameUiPanelKind { CommandRail, Overview, Target, Fleet, ShipStatus, Navigation, Chat, Notifications, Hotbar, Inspector, Services, Map, Fitting, Market, Industry };

struct GameUiTheme {
    std::string name="Subspace Command";
    float panelOpacity=.88f;
    float cornerRadius=4.0f;
    float spacing=8.0f;
    float textScale=1.0f;
    std::string fontFamily="Segoe UI";
    float bodyTextPx=16.0f;
    float smallTextPx=14.0f;
    float headingTextPx=19.0f;
    float minimumContrast=0.72f;
    float clickableRowHeight=38.0f;
    bool compact=true;
};

struct GameUiPanelSpec {
    GameUiPanelKind kind=GameUiPanelKind::Overview;
    GameUiAnchor anchor=GameUiAnchor::Left;
    float width=280.0f;
    float height=220.0f;
    bool collapsible=true;
    bool movable=true;
    bool visible=true;
};

class GameUiFramework {
public:
    GameUiTheme DefaultTheme() const;
    std::vector<GameUiPanelSpec> DefaultFlightWorkspace() const;
    bool Validate(const std::vector<GameUiPanelSpec>& panels) const;
};

} // namespace subspace
