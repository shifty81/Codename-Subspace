#include "ship_editor/ShipyardProfessionalShellSystem.h"

#include <algorithm>

namespace subspace {
ShipyardProfessionalShellState ShipyardProfessionalShellSystem::Create(){
    ShipyardProfessionalShellState out;
    out.layout=ShipyardWorkspaceLayoutSystem::Default("BUILD");
    out.activityDock=ShipyardActivityDockSystem::DefaultState();
    out.assetBrowser=ShipyardAssetBrowserSystem::DefaultState();
    out.quickActions=ShipyardProfessionalUiSystem::QuickActions();
    out.primaryWorkspaces=ShipyardProfessionalUiSystem::PrimaryWorkspaceStrip();
    out.advancedWorkspaces=ShipyardProfessionalUiSystem::AdvancedWorkspaceMenu();
    return out;
}

void ShipyardProfessionalShellSystem::Rebuild(ShipyardProfessionalShellState& shell,
                                               const ShipyardDocument& document,
                                               const ShipyardSession& session,
                                               const ShipyardCommandSystem& commands,
                                               const std::vector<ShipyardModuleRecord>& catalog,
                                               const ShipyardModuleRecord* selectedParent,
                                               const std::vector<EditorValidationMessage>& validation){
    if(shell.layout.panels.empty())shell.layout=ShipyardWorkspaceLayoutSystem::Default(session.layoutPreset);
    shell.panelModel=ShipyardPanelModelSystem::Build(document,catalog,session.selection);
    shell.assetItems=ShipyardAssetBrowserSystem::Query(catalog,shell.assetBrowser,selectedParent);
    shell.searchResults=session.universalSearchOpen
        ?ShipyardUniversalSearchSystem::Search(session.commandSearch,commands,catalog,shell.panelModel,validation,session.advancedVisible)
        :std::vector<ShipyardSearchResult>{};
    shell.visiblePanels.clear();
    for(const auto& descriptor:ShipyardProfessionalUiSystem::Panels()){
        const auto state=std::find_if(shell.layout.panels.begin(),shell.layout.panels.end(),[&](const auto& p){return p.panelId==descriptor.id;});
        if(state==shell.layout.panels.end()||!state->visible)continue;
        if(!ShipyardProfessionalUiSystem::PanelAppliesTo(descriptor,session.workspace))continue;
        if(descriptor.advanced&&!session.advancedVisible)continue;
        shell.visiblePanels.push_back(descriptor);
    }
    shell.quickActions=ShipyardProfessionalUiSystem::QuickActions();
    shell.primaryWorkspaces=ShipyardProfessionalUiSystem::PrimaryWorkspaceStrip();
    shell.advancedWorkspaces=session.advancedVisible?ShipyardProfessionalUiSystem::AdvancedWorkspaceMenu():std::vector<ShipyardWorkspaceMode>{};
}
}
