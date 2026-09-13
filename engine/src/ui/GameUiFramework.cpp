#include "ui/GameUiFramework.h"

#include <algorithm>
#include <set>

namespace subspace {
namespace {
const char* PanelId(GameUiPanelKind k){switch(k){case GameUiPanelKind::CommandRail:return"command_rail";case GameUiPanelKind::Overview:return"overview";case GameUiPanelKind::Target:return"target";case GameUiPanelKind::Fleet:return"fleet";case GameUiPanelKind::ShipStatus:return"ship_status";case GameUiPanelKind::Navigation:return"navigation";case GameUiPanelKind::Chat:return"chat";case GameUiPanelKind::Notifications:return"notifications";case GameUiPanelKind::Hotbar:return"hotbar";case GameUiPanelKind::Inspector:return"inspector";case GameUiPanelKind::Services:return"services";case GameUiPanelKind::Map:return"map";case GameUiPanelKind::Fitting:return"fitting";case GameUiPanelKind::Market:return"market";case GameUiPanelKind::Industry:return"industry";}return"panel";}
const char* PanelTitle(GameUiPanelKind k){switch(k){case GameUiPanelKind::CommandRail:return"Commands";case GameUiPanelKind::Overview:return"Overview";case GameUiPanelKind::Target:return"Target";case GameUiPanelKind::Fleet:return"Fleet";case GameUiPanelKind::ShipStatus:return"Ship";case GameUiPanelKind::Navigation:return"Navigation";case GameUiPanelKind::Chat:return"Comms";case GameUiPanelKind::Notifications:return"Notifications";case GameUiPanelKind::Hotbar:return"Actions";case GameUiPanelKind::Inspector:return"Inspector";case GameUiPanelKind::Services:return"Services";case GameUiPanelKind::Map:return"Map";case GameUiPanelKind::Fitting:return"Fitting";case GameUiPanelKind::Market:return"Market";case GameUiPanelKind::Industry:return"Industry";}return"Panel";}
}

GameUiTheme GameUiFramework::DefaultTheme() const{
    const auto tokens=SubspaceUiTheme::Dark();GameUiTheme t;t.panelOpacity=tokens.panelOpacityDefault;t.cornerRadius=tokens.radiusPanel;t.spacing=tokens.spacingS;t.bodyTextPx=tokens.fontBody;t.smallTextPx=tokens.fontCaption;t.headingTextPx=tokens.fontTitle;t.clickableRowHeight=tokens.minControlHeight;return t;
}
std::vector<GameUiPanelSpec> GameUiFramework::DefaultFlightWorkspace() const{return {
{GameUiPanelKind::Overview,GameUiAnchor::Left,260,420,true,true,true,true,.82f},
{GameUiPanelKind::Target,GameUiAnchor::Right,300,260,true,true,true,true,.88f},
{GameUiPanelKind::Fleet,GameUiAnchor::Right,300,210,true,true,true,true,.82f},
{GameUiPanelKind::Chat,GameUiAnchor::BottomLeft,380,180,true,true,true,true,.78f},
{GameUiPanelKind::Hotbar,GameUiAnchor::BottomCenter,620,92,false,false,false,true,.76f},
{GameUiPanelKind::ShipStatus,GameUiAnchor::BottomCenter,320,70,true,false,true,true,.82f},
{GameUiPanelKind::Notifications,GameUiAnchor::BottomRight,320,170,true,true,true,true,.80f},
{GameUiPanelKind::CommandRail,GameUiAnchor::Left,88,720,false,false,false,true,.88f}};}

SubspaceDockWorkspace GameUiFramework::DefaultFlightDockWorkspace() const{
    auto w=SubspaceDockSystem::CreateMinimalWorkspace("flight");
    auto add=[&](GameUiPanelKind kind,const char* leaf,bool visible,float opacity,bool closable=true,bool floatable=true,bool resizable=true){SubspaceDockPanel p;p.id=PanelId(kind);p.title=PanelTitle(kind);p.defaultLeafId=leaf;p.visible=visible;p.opacity=opacity;p.closable=closable;p.floatable=floatable;p.resizable=resizable;SubspaceDockSystem::RegisterPanel(w,p);};
    add(GameUiPanelKind::Overview,"left",true,.78f);add(GameUiPanelKind::Navigation,"left",false,.82f);add(GameUiPanelKind::Target,"right",true,.86f);add(GameUiPanelKind::Fleet,"right",true,.78f);add(GameUiPanelKind::ShipStatus,"right",true,.82f);add(GameUiPanelKind::Chat,"bottom",false,.76f);add(GameUiPanelKind::Notifications,"bottom",false,.80f);add(GameUiPanelKind::Hotbar,"bottom",true,.70f,false,false,false);add(GameUiPanelKind::CommandRail,"left",true,.84f,false,false,false);return w;
}

SubspaceDockWorkspace GameUiFramework::DefaultStrategicDockWorkspace() const{
    auto w=SubspaceDockSystem::CreateMinimalWorkspace("remote_fleet");
    auto add=[&](GameUiPanelKind kind,const char* leaf,bool visible,float opacity){SubspaceDockPanel p;p.id=PanelId(kind);p.title=PanelTitle(kind);p.defaultLeafId=leaf;p.visible=visible;p.opacity=opacity;SubspaceDockSystem::RegisterPanel(w,p);};
    add(GameUiPanelKind::Fleet,"left",true,.88f);add(GameUiPanelKind::Overview,"left",true,.80f);add(GameUiPanelKind::Map,"center",true,.45f);add(GameUiPanelKind::Target,"right",true,.86f);add(GameUiPanelKind::Inspector,"right",true,.88f);add(GameUiPanelKind::Notifications,"bottom",false,.80f);add(GameUiPanelKind::Chat,"bottom",false,.76f);return w;
}

bool GameUiFramework::Validate(const std::vector<GameUiPanelSpec>& p) const {std::set<GameUiPanelKind> seen;for(const auto&x:p){if(x.width<80||x.height<40||x.opacity<.20f||x.opacity>1.0f)return false;if(!seen.insert(x.kind).second)return false;}return !p.empty();}

} // namespace subspace
