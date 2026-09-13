#include "ui/SubspaceUiFramework.h"

#include <algorithm>
#include <unordered_set>

namespace subspace {

SubspaceUiTheme SubspaceUiTheme::Dark(){
    SubspaceUiTheme t;
    t.canvas={.006f,.009f,.012f,1};
    t.panel={.017f,.022f,.027f,.92f};
    t.raised={.027f,.034f,.041f,.96f};
    t.border={.12f,.17f,.20f,.82f};
    t.textPrimary={.86f,.89f,.91f,1};
    t.textSecondary={.53f,.60f,.64f,.94f};
    t.accent={.18f,.58f,.70f,1};
    t.valid={.28f,.82f,.48f,1};
    t.warning={.91f,.64f,.22f,1};
    t.error={.90f,.27f,.25f,1};
    t.authoring={.61f,.42f,.88f,1};
    t.input={.012f,.016f,.020f,.96f};
    t.header={.020f,.027f,.033f,.98f};
    t.scrollTrack={.010f,.014f,.018f,.70f};
    t.scrollThumb={.16f,.21f,.24f,.92f};
    t.overlay={.003f,.006f,.009f,.72f};
    t.popup={.018f,.023f,.028f,.98f};
    t.focus={.24f,.68f,.78f,1};
    return t;
}

SubspaceUiColor SubspaceUiTheme::StateColor(SubspaceUiState s)const{
    switch(s){
        case SubspaceUiState::Hover:return{accent.r*.78f,accent.g*.78f,accent.b*.78f,1};
        case SubspaceUiState::Pressed:return{accent.r*.56f,accent.g*.56f,accent.b*.56f,1};
        case SubspaceUiState::Selected:return accent;
        case SubspaceUiState::Disabled:return{textSecondary.r*.45f,textSecondary.g*.45f,textSecondary.b*.45f,.65f};
        case SubspaceUiState::Valid:return valid;
        case SubspaceUiState::Warning:return warning;
        case SubspaceUiState::Error:return error;
        case SubspaceUiState::Authoring:return authoring;
        default:return raised;
    }
}

void EditorHelpRegistry::Register(EditorHelpEntry e){entries_[e.id]=std::move(e);}
const EditorHelpEntry*EditorHelpRegistry::Find(const std::string&id)const{auto i=entries_.find(id);return i==entries_.end()?nullptr:&i->second;}
std::string EditorHelpRegistry::Tooltip(const std::string&id,bool d,const std::string&reason)const{const auto*e=Find(id);if(!e)return reason;std::string t=e->displayName;if(!e->shortDescription.empty())t+="\n"+e->shortDescription;if(d&&!e->detailedDescription.empty())t+="\n"+e->detailedDescription;if(!e->shortcut.empty())t+="\nShortcut: "+e->shortcut;if(!reason.empty())t+="\nUnavailable: "+reason;return t;}

namespace {
bool Contains(const std::vector<std::string>& values,const std::string& value){return std::find(values.begin(),values.end(),value)!=values.end();}
void RemovePanelHosts(SubspaceDockWorkspace& w,const std::string& panelId){
    for(auto& n:w.nodes){if(n.split)continue;n.tabs.erase(std::remove(n.tabs.begin(),n.tabs.end(),panelId),n.tabs.end());if(n.activeTabId==panelId)n.activeTabId=n.tabs.empty()?std::string{}:n.tabs.front();}
    w.floatingPanels.erase(std::remove_if(w.floatingPanels.begin(),w.floatingPanels.end(),[&](const auto& f){return f.panelId==panelId;}),w.floatingPanels.end());
}
void LayoutNode(const SubspaceDockWorkspace& w,const std::string& id,const SubspaceUiRect& rect,std::vector<SubspaceDockLayout>& out){
    const auto* n=SubspaceDockSystem::FindNode(w,id);if(!n||n->collapsed)return;
    if(n->split){const float ratio=std::clamp(n->ratio,.08f,.92f);SubspaceUiRect a=rect,b=rect;if(n->axis==SubspaceDockSplitAxis::Horizontal){a.width=rect.width*ratio;b.x=rect.x+a.width;b.width=std::max(0.0f,rect.width-a.width);}else{a.height=rect.height*ratio;b.y=rect.y+a.height;b.height=std::max(0.0f,rect.height-a.height);}LayoutNode(w,n->firstChildId,a,out);LayoutNode(w,n->secondChildId,b,out);return;}
    for(const auto& panelId:n->tabs){const auto* p=SubspaceDockSystem::FindPanel(w,panelId);if(!p||!p->visible)continue;out.push_back({panelId,n->id,rect,p->opacity,true,n->activeTabId==panelId,false});}
}
}

SubspaceDockWorkspace SubspaceDockSystem::CreateMinimalWorkspace(std::string id){
    SubspaceDockWorkspace w;w.id=std::move(id);w.rootNodeId="root";
    w.nodes.push_back({"root",true,SubspaceDockSplitAxis::Vertical,.80f,"main","bottom",{},{},false});
    w.nodes.push_back({"main",true,SubspaceDockSplitAxis::Horizontal,.20f,"left","center_right",{},{},false});
    w.nodes.push_back({"center_right",true,SubspaceDockSplitAxis::Horizontal,.76f,"center","right",{},{},false});
    SubspaceDockNode left; left.id="left"; w.nodes.push_back(left);
    SubspaceDockNode center; center.id="center"; w.nodes.push_back(center);
    SubspaceDockNode right; right.id="right"; w.nodes.push_back(right);
    SubspaceDockNode bottom; bottom.id="bottom"; w.nodes.push_back(bottom);
    return w;
}
SubspaceDockPanel* SubspaceDockSystem::FindPanel(SubspaceDockWorkspace& w,const std::string& id){for(auto& p:w.panels)if(p.id==id)return &p;return nullptr;}
const SubspaceDockPanel* SubspaceDockSystem::FindPanel(const SubspaceDockWorkspace& w,const std::string& id){for(const auto& p:w.panels)if(p.id==id)return &p;return nullptr;}
SubspaceDockNode* SubspaceDockSystem::FindNode(SubspaceDockWorkspace& w,const std::string& id){for(auto& n:w.nodes)if(n.id==id)return &n;return nullptr;}
const SubspaceDockNode* SubspaceDockSystem::FindNode(const SubspaceDockWorkspace& w,const std::string& id){for(const auto& n:w.nodes)if(n.id==id)return &n;return nullptr;}

bool SubspaceDockSystem::RegisterPanel(SubspaceDockWorkspace& w,SubspaceDockPanel p){
    if(p.id.empty()||FindPanel(w,p.id))return false;
    if(p.defaultLeafId.empty())p.defaultLeafId="bottom";
    auto* leaf=FindNode(w,p.defaultLeafId);if(!leaf||leaf->split)return false;
    p.opacity=std::clamp(p.opacity,.28f,1.0f);w.panels.push_back(p);leaf=FindNode(w,p.defaultLeafId);leaf->tabs.push_back(p.id);if(leaf->activeTabId.empty()&&p.visible)leaf->activeTabId=p.id;return true;
}
bool SubspaceDockSystem::OpenPanel(SubspaceDockWorkspace& w,const std::string& id){auto* p=FindPanel(w,id);if(!p)return false;p->visible=true;for(const auto& f:w.floatingPanels)if(f.panelId==id)return true;for(auto& n:w.nodes)if(!n.split&&Contains(n.tabs,id)){n.collapsed=false;n.activeTabId=id;return true;}auto* leaf=FindNode(w,p->defaultLeafId);if(!leaf||leaf->split)return false;leaf->tabs.push_back(id);leaf->activeTabId=id;leaf->collapsed=false;return true;}
bool SubspaceDockSystem::ClosePanel(SubspaceDockWorkspace& w,const std::string& id){auto* p=FindPanel(w,id);if(!p||!p->closable)return false;p->visible=false;return true;}
bool SubspaceDockSystem::ActivatePanel(SubspaceDockWorkspace& w,const std::string& id){auto* p=FindPanel(w,id);if(!p)return false;if(!p->visible&&!OpenPanel(w,id))return false;for(auto& n:w.nodes)if(!n.split&&Contains(n.tabs,id)){n.activeTabId=id;n.collapsed=false;return true;}for(const auto& f:w.floatingPanels)if(f.panelId==id)return true;return false;}
bool SubspaceDockSystem::MovePanel(SubspaceDockWorkspace& w,const std::string& id,const std::string& leafId,bool activate){auto* p=FindPanel(w,id);auto* leaf=FindNode(w,leafId);if(!p||!leaf||leaf->split)return false;RemovePanelHosts(w,id);leaf=FindNode(w,leafId);leaf->tabs.push_back(id);leaf->collapsed=false;p=FindPanel(w,id);p->visible=true;if(activate)leaf->activeTabId=id;return true;}
bool SubspaceDockSystem::FloatPanel(SubspaceDockWorkspace& w,const std::string& id,SubspaceUiRect r){auto* p=FindPanel(w,id);if(!p||!p->floatable)return false;RemovePanelHosts(w,id);p=FindPanel(w,id);p->visible=true;r.width=std::clamp(r.width,p->minWidth,p->maxWidth);r.height=std::clamp(r.height,p->minHeight,p->maxHeight);w.floatingPanels.push_back({id,r});return true;}
bool SubspaceDockSystem::DockPanel(SubspaceDockWorkspace& w,const std::string& id,const std::string& leafId,bool activate){return MovePanel(w,id,leafId,activate);}
bool SubspaceDockSystem::SetPanelOpacity(SubspaceDockWorkspace& w,const std::string& id,float opacity,const SubspaceUiTheme& theme){auto* p=FindPanel(w,id);if(!p)return false;p->opacity=std::clamp(opacity,theme.panelOpacityMinimum,theme.panelOpacityMaximum);return true;}
bool SubspaceDockSystem::ResizeSplit(SubspaceDockWorkspace& w,const std::string& id,float ratio){auto* n=FindNode(w,id);if(!n||!n->split)return false;n->ratio=std::clamp(ratio,.08f,.92f);return true;}
bool SubspaceDockSystem::ResizeFloating(SubspaceDockWorkspace& w,const std::string& id,SubspaceUiRect r){auto* p=FindPanel(w,id);if(!p||!p->resizable)return false;for(auto& f:w.floatingPanels)if(f.panelId==id){r.width=std::clamp(r.width,p->minWidth,p->maxWidth);r.height=std::clamp(r.height,p->minHeight,p->maxHeight);f.rect=r;return true;}return false;}
std::vector<SubspaceDockLayout> SubspaceDockSystem::Materialize(const SubspaceDockWorkspace& w,int width,int height,float topInset){std::vector<SubspaceDockLayout> out;if(width<=0||height<=0||topInset<0||topInset>=height)return out;LayoutNode(w,w.rootNodeId,{0,topInset,static_cast<float>(width),static_cast<float>(height)-topInset},out);for(const auto& f:w.floatingPanels){const auto* p=FindPanel(w,f.panelId);if(!p||!p->visible)continue;SubspaceUiRect r=f.rect;r.width=std::clamp(r.width,p->minWidth,std::min(p->maxWidth,static_cast<float>(width)));r.height=std::clamp(r.height,p->minHeight,std::min(p->maxHeight,static_cast<float>(height)-topInset));r.x=std::clamp(r.x,0.0f,std::max(0.0f,static_cast<float>(width)-r.width));r.y=std::clamp(r.y,topInset,std::max(topInset,static_cast<float>(height)-r.height));out.push_back({p->id,{},r,p->opacity,true,true,true});}return out;}
bool SubspaceDockSystem::Validate(const SubspaceDockWorkspace& w,std::string* error){
    if(w.rootNodeId.empty()||!FindNode(w,w.rootNodeId)){if(error)*error="dock workspace missing root";return false;}
    std::unordered_set<std::string> ids,owned;for(const auto& p:w.panels){if(p.id.empty()||!ids.insert(p.id).second){if(error)*error="duplicate/empty panel id";return false;}if(p.opacity<.0f||p.opacity>1.0f||p.minWidth<=0||p.minHeight<=0){if(error)*error="invalid panel geometry/style";return false;}}
    std::unordered_set<std::string> nodeIds;for(const auto& n:w.nodes){if(n.id.empty()||!nodeIds.insert(n.id).second){if(error)*error="duplicate/empty dock node id";return false;}if(n.split){if(n.firstChildId.empty()||n.secondChildId.empty()||n.ratio<=0||n.ratio>=1){if(error)*error="invalid dock split";return false;}}else for(const auto& id:n.tabs){if(!FindPanel(w,id)||!owned.insert(id).second){if(error)*error="panel is unknown or hosted twice";return false;}}}
    for(const auto& n:w.nodes)if(n.split&&(!FindNode(w,n.firstChildId)||!FindNode(w,n.secondChildId))){if(error)*error="split references missing child";return false;}
    for(const auto& f:w.floatingPanels){const auto* p=FindPanel(w,f.panelId);if(!p||!p->floatable||!owned.insert(f.panelId).second){if(error)*error="invalid floating panel";return false;}}
    return true;
}

} // namespace subspace
