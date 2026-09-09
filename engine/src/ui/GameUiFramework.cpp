#include "ui/GameUiFramework.h"
#include "ui/SubspaceUiFramework.h"
#include <set>
#include <algorithm>

namespace subspace {
GameUiTheme GameUiFramework::DefaultTheme() const{
    const auto tokens=SubspaceUiTheme::Dark();
    GameUiTheme t;
    t.name="Subspace Command";
    t.panelOpacity=tokens.panel.a;
    t.cornerRadius=tokens.radiusPanel;
    t.spacing=tokens.spacingS;
    t.textScale=1.0f;
    t.fontFamily="Segoe UI";
    t.bodyTextPx=tokens.fontBody;
    t.smallTextPx=tokens.fontCaption;
    t.headingTextPx=tokens.fontTitle;
    t.minimumContrast=0.72f;
    t.clickableRowHeight=tokens.minControlHeight;
    t.compact=true;
    return t;
}
std::vector<GameUiPanelSpec> GameUiFramework::DefaultFlightWorkspace() const{return {
{GameUiPanelKind::Overview,GameUiAnchor::Left,260,420,true,true,true},
{GameUiPanelKind::Target,GameUiAnchor::Right,300,260,true,true,true},
{GameUiPanelKind::Fleet,GameUiAnchor::Right,300,210,true,true,true},
{GameUiPanelKind::Chat,GameUiAnchor::BottomLeft,380,180,true,true,true},
{GameUiPanelKind::Hotbar,GameUiAnchor::BottomCenter,620,92,false,false,true},
{GameUiPanelKind::ShipStatus,GameUiAnchor::BottomCenter,320,70,true,false,true},
{GameUiPanelKind::Notifications,GameUiAnchor::BottomRight,320,170,true,true,true},
{GameUiPanelKind::CommandRail,GameUiAnchor::Left,88,720,false,false,true}};}
bool GameUiFramework::Validate(const std::vector<GameUiPanelSpec>& p) const {std::set<GameUiPanelKind> seen;for(const auto&x:p){if(x.width<80||x.height<40)return false;if(!seen.insert(x.kind).second)return false;}return !p.empty();}
} // namespace subspace
