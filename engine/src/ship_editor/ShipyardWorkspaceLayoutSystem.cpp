#include "ship_editor/ShipyardWorkspaceLayoutSystem.h"

#include <algorithm>
#include <unordered_set>

namespace subspace {
namespace {
const ShipyardLayoutPreset* FindPreset(const std::string& id){
    static const auto presets=ShipyardProfessionalUiSystem::LayoutPresets();
    const auto it=std::find_if(presets.begin(),presets.end(),[&](const auto& p){return p.id==id;});
    return it==presets.end()?nullptr:&*it;
}
}
ShipyardWorkspaceLayoutState ShipyardWorkspaceLayoutSystem::Default(const std::string& presetId){ShipyardWorkspaceLayoutState out;ApplyPreset(out,presetId);out.dirty=false;return out;}
bool ShipyardWorkspaceLayoutSystem::ApplyPreset(ShipyardWorkspaceLayoutState& state,const std::string& presetId){
    const auto* preset=FindPreset(presetId);if(!preset)return false;
    state.panels.clear();
    for(const auto& panel:ShipyardProfessionalUiSystem::Panels()){
        const bool visible=std::find(preset->visiblePanels.begin(),preset->visiblePanels.end(),panel.id)!=preset->visiblePanels.end();
        state.panels.push_back({panel.id,panel.defaultDock,visible,false,panel.minimumWidth,panel.minimumHeight});
    }
    state.presetId=presetId;state.maximizedPanelId.clear();state.auxiliaryDocksCollapsed=false;state.dirty=true;return true;
}
bool ShipyardWorkspaceLayoutSystem::SetPanelVisible(ShipyardWorkspaceLayoutState& state,const std::string& panelId,bool visible){for(auto& p:state.panels)if(p.panelId==panelId){p.visible=visible;state.dirty=true;return true;}return false;}
bool ShipyardWorkspaceLayoutSystem::ToggleMaximized(ShipyardWorkspaceLayoutState& state,const std::string& panelId){if(panelId.empty())return false;state.maximizedPanelId=state.maximizedPanelId==panelId?std::string{}:panelId;state.dirty=true;return true;}
void ShipyardWorkspaceLayoutSystem::ToggleAuxiliaryDocks(ShipyardWorkspaceLayoutState& state){state.auxiliaryDocksCollapsed=!state.auxiliaryDocksCollapsed;state.dirty=true;}
bool ShipyardWorkspaceLayoutSystem::ResetCurrent(ShipyardWorkspaceLayoutState& state){return ApplyPreset(state,state.presetId.empty()?"BUILD":state.presetId);}
bool ShipyardWorkspaceLayoutSystem::Validate(const ShipyardWorkspaceLayoutState& state,std::string* error){
    std::unordered_set<std::string> known;for(const auto& p:ShipyardProfessionalUiSystem::Panels())known.insert(p.id);
    for(const auto& p:state.panels)if(!known.count(p.panelId)){if(error)*error="Unknown panel in layout: "+p.panelId;return false;}
    if(!state.maximizedPanelId.empty()&&!known.count(state.maximizedPanelId)){if(error)*error="Unknown maximized panel: "+state.maximizedPanelId;return false;}
    return true;
}
}
