#include "ui/SubspaceUiFramework.h"

#include <algorithm>
#include <unordered_set>
#include <sstream>

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
    // PASS1507: a leaf is a tab stack, not a pile of simultaneous panels.
    // A closed/auto-hidden active tab yields to the first available sibling.
    auto usable=[&](const std::string& id){
        const auto* panel=SubspaceDockSystem::FindPanel(w,id);
        return panel&&panel->visible&&(!panel->autoHide||panel->pinned||panel->hoverReveal);
    };
    std::string active=Contains(n->tabs,n->activeTabId)&&usable(n->activeTabId)
        ?n->activeTabId:std::string{};
    if(active.empty())for(const auto& candidate:n->tabs){
        if(usable(candidate)){active=candidate;break;}
    }
    for(const auto& panelId:n->tabs){
        if(panelId!=active)continue;
        const auto* p=SubspaceDockSystem::FindPanel(w,panelId);
        if(!p||!p->visible||(p->autoHide&&!p->pinned&&!p->hoverReveal))continue;
        SubspaceUiRect panelRect=rect;
        if(p->collapsed){
            if(n->id=="left"||n->id=="right"||n->id=="tool_left")panelRect.width=std::min(rect.width,34.0f);
            else panelRect.height=std::min(rect.height,28.0f);
        }
        out.push_back({panelId,n->id,panelRect,p->opacity,true,n->activeTabId==panelId,false});
    }
}
}

// A Shipyard dock leaf is a tool anchor. It never partitions the 3D canvas.
// All callers (renderer, controls and pointer) receive these same materialized
// rectangles. The previous permanent split tree remains only as tab ownership
// metadata for the serialized workspace; no renderer may use its ratios.
void LayoutShipyardOverlays(const SubspaceDockWorkspace& w,int width,int height,
                           float topInset,std::vector<SubspaceDockLayout>& out){
    const float x=static_cast<float>(width), bottom=static_cast<float>(height);
    const float available=std::max(1.0f,bottom-topInset);
    const auto rectFor=[&](const std::string& id)->SubspaceUiRect{
        if(id=="center")return {0,topInset,x,available};
        if(id=="tool_left")return {4,topInset+4,48.0f,std::min(available-8.0f,420.0f)};
        if(id=="right_top")return {std::max(0.0f,x-346.0f),topInset+12.0f,
            std::min(x,340.0f),std::min(available-20.0f,240.0f)};
        if(id=="right_bottom")return {std::max(0.0f,x-386.0f),topInset+262.0f,
            std::min(x,380.0f),std::max(36.0f,available-274.0f)};
        if(id=="bottom"){
            // Dock anchors are non-partitioning overlays, but their *default*
            // placements must not cover another docked tool's hit targets.
            // The former full-width asset shelf covered the Properties header
            // and SHIELD control at 1280x768. Reserve the right column only
            // when its dock leaf actually has an eligible active panel; a
            // floating/hidden Properties panel releases the width immediately.
            const auto* right=SubspaceDockSystem::FindNode(w,"right_bottom");
            bool rightOccupied=false;
            if(right&&!right->split&&!right->collapsed){
                for(const auto& panelId:right->tabs){
                    const auto* panel=SubspaceDockSystem::FindPanel(w,panelId);
                    if(panel&&panel->visible&&(!panel->autoHide||panel->pinned||panel->hoverReveal)){
                        rightOccupied=true;
                        break;
                    }
                }
            }
            const float reserved=rightOccupied?386.0f:0.0f;
            return {56.0f,std::max(topInset,bottom-222.0f),
                std::max(1.0f,x-62.0f-reserved),std::min(available,216.0f)};
        }
        return {0,topInset,x,available};
    };
    // The viewport is always first and full-size. It is the canvas, not a tool.
    if(const auto* panel=SubspaceDockSystem::FindPanel(w,"viewport"))
        if(panel->visible)out.push_back({"viewport","center",rectFor("center"),1.0f,true,true,false});
    for(const char* id:{"tool_left","bottom","right_top","right_bottom"}){
        const auto* node=SubspaceDockSystem::FindNode(w,id);
        if(!node||node->split||node->collapsed)continue;
        const auto eligible=[&](const std::string& panelId){
            const auto* p=SubspaceDockSystem::FindPanel(w,panelId);
            return p&&p->visible&&(!p->autoHide||p->pinned||p->hoverReveal);
        };
        std::string active=Contains(node->tabs,node->activeTabId)&&eligible(node->activeTabId)
            ?node->activeTabId:std::string{};
        if(active.empty())for(const auto& candidate:node->tabs)
            if(eligible(candidate)){active=candidate;break;}
        if(active.empty())continue; // empty anchor does not leave a viewport hole
        const auto* panel=SubspaceDockSystem::FindPanel(w,active);
        auto r=rectFor(id);
        if(panel->collapsed){if(id==std::string("tool_left"))r.width=34.0f;
            else r.height=28.0f;}
        if(r.width>0&&r.height>0)
            out.push_back({active,id,r,panel->opacity,true,node->activeTabId==active,false});
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
bool SubspaceDockSystem::ClosePanel(SubspaceDockWorkspace& w,const std::string& id){
    auto* p=FindPanel(w,id);
    if(!p||!p->closable)return false;
    p->visible=false;
    // Do not leave the dock stack pointing at a hidden tab after a close.
    for(auto& node:w.nodes){
        if(node.split||node.activeTabId!=id)continue;
        node.activeTabId.clear();
        for(const auto& candidate:node.tabs){
            const auto* next=FindPanel(w,candidate);
            if(next&&next->visible&&(!next->autoHide||next->pinned||next->hoverReveal)){
                node.activeTabId=candidate;
                break;
            }
        }
    }
    return true;
}
bool SubspaceDockSystem::ActivatePanel(SubspaceDockWorkspace& w,const std::string& id){auto* p=FindPanel(w,id);if(!p)return false;if(!p->visible&&!OpenPanel(w,id))return false;for(auto& n:w.nodes)if(!n.split&&Contains(n.tabs,id)){n.activeTabId=id;n.collapsed=false;return true;}for(const auto& f:w.floatingPanels)if(f.panelId==id)return true;return false;}
bool SubspaceDockSystem::MovePanel(SubspaceDockWorkspace& w,const std::string& id,const std::string& leafId,bool activate){auto* p=FindPanel(w,id);auto* leaf=FindNode(w,leafId);if(!p||!leaf||leaf->split)return false;RemovePanelHosts(w,id);leaf=FindNode(w,leafId);leaf->tabs.push_back(id);leaf->collapsed=false;p=FindPanel(w,id);p->visible=true;if(activate)leaf->activeTabId=id;return true;}
bool SubspaceDockSystem::FloatPanel(SubspaceDockWorkspace& w,const std::string& id,SubspaceUiRect r){auto* p=FindPanel(w,id);if(!p||!p->floatable)return false;RemovePanelHosts(w,id);p=FindPanel(w,id);p->visible=true;r.width=std::clamp(r.width,p->minWidth,p->maxWidth);r.height=std::clamp(r.height,p->minHeight,p->maxHeight);w.floatingPanels.push_back({id,r});return true;}
bool SubspaceDockSystem::RaiseFloatingPanel(SubspaceDockWorkspace& w,const std::string& id){
    const auto it=std::find_if(w.floatingPanels.begin(),w.floatingPanels.end(),
        [&](const auto& f){return f.panelId==id;});
    if(it==w.floatingPanels.end())return false;
    std::rotate(it,it+1,w.floatingPanels.end());
    return true;
}
bool SubspaceDockSystem::DockPanel(SubspaceDockWorkspace& w,const std::string& id,const std::string& leafId,bool activate){return MovePanel(w,id,leafId,activate);}
bool SubspaceDockSystem::SetPanelOpacity(SubspaceDockWorkspace& w,const std::string& id,float opacity,const SubspaceUiTheme& theme){auto* p=FindPanel(w,id);if(!p)return false;p->opacity=std::clamp(opacity,theme.panelOpacityMinimum,theme.panelOpacityMaximum);return true;}
bool SubspaceDockSystem::ResizeSplit(SubspaceDockWorkspace& w,const std::string& id,float ratio){auto* n=FindNode(w,id);if(!n||!n->split)return false;n->ratio=std::clamp(ratio,.08f,.92f);return true;}
bool SubspaceDockSystem::ResizeFloating(SubspaceDockWorkspace& w,const std::string& id,SubspaceUiRect r){auto* p=FindPanel(w,id);if(!p||!p->resizable)return false;for(auto& f:w.floatingPanels)if(f.panelId==id){r.width=std::clamp(r.width,p->minWidth,p->maxWidth);r.height=std::clamp(r.height,p->minHeight,p->maxHeight);f.rect=r;return true;}return false;}
bool SubspaceDockSystem::ToggleCollapsed(SubspaceDockWorkspace& w,const std::string& id){auto* p=FindPanel(w,id);if(!p)return false;p->collapsed=!p->collapsed;return true;}
bool SubspaceDockSystem::TogglePinned(SubspaceDockWorkspace& w,const std::string& id){auto* p=FindPanel(w,id);if(!p)return false;p->pinned=!p->pinned;if(p->pinned)p->hoverReveal=true;return true;}
bool SubspaceDockSystem::SetAutoHide(SubspaceDockWorkspace& w,const std::string& id,bool autoHide){auto* p=FindPanel(w,id);if(!p)return false;p->autoHide=autoHide;if(!autoHide)p->hoverReveal=true;return true;}
bool SubspaceDockSystem::SetHoverReveal(SubspaceDockWorkspace& w,const std::string& id,bool reveal){auto* p=FindPanel(w,id);if(!p)return false;p->hoverReveal=reveal||p->pinned;return true;}
std::vector<SubspaceDockLayout> SubspaceDockSystem::Materialize(const SubspaceDockWorkspace& w,int width,int height,float topInset){std::vector<SubspaceDockLayout> out;if(width<=0||height<=0||topInset<0||topInset>=height)return out;if(w.id=="shipyard")LayoutShipyardOverlays(w,width,height,topInset,out);else LayoutNode(w,w.rootNodeId,{0,topInset,static_cast<float>(width),static_cast<float>(height)-topInset},out);for(const auto& f:w.floatingPanels){const auto* p=FindPanel(w,f.panelId);if(!p||!p->visible)continue;SubspaceUiRect r=f.rect;r.width=std::clamp(r.width,p->minWidth,std::min(p->maxWidth,static_cast<float>(width)));r.height=std::clamp(r.height,p->minHeight,std::min(p->maxHeight,static_cast<float>(height)-topInset));r.x=std::clamp(r.x,0.0f,std::max(0.0f,static_cast<float>(width)-r.width));r.y=std::clamp(r.y,topInset,std::max(topInset,static_cast<float>(height)-r.height));out.push_back({p->id,{},r,p->opacity,true,true,true});}return out;}

std::string SubspaceDockSystem::Serialize(const SubspaceDockWorkspace& w){
    std::ostringstream o;o<<"SUBSPACE_DOCK_V1\n"<<"W|"<<w.id<<"|"<<w.rootNodeId<<"\n";
    for(const auto& p:w.panels)o<<"P|"<<p.id<<"|"<<p.title<<"|"<<p.defaultLeafId<<"|"<<(p.visible?1:0)<<"|"<<(p.closable?1:0)<<"|"<<(p.floatable?1:0)<<"|"<<(p.resizable?1:0)<<"|"<<(p.pinned?1:0)<<"|"<<(p.collapsed?1:0)<<"|"<<(p.autoHide?1:0)<<"|"<<(p.hoverReveal?1:0)<<"|"<<p.opacity<<"|"<<p.minWidth<<"|"<<p.minHeight<<"|"<<p.preferredWidth<<"|"<<p.preferredHeight<<"\n";
    for(const auto& n:w.nodes){o<<"N|"<<n.id<<"|"<<(n.split?1:0)<<"|"<<(n.axis==SubspaceDockSplitAxis::Horizontal?0:1)<<"|"<<n.ratio<<"|"<<n.firstChildId<<"|"<<n.secondChildId<<"|"<<n.activeTabId<<"|"<<(n.collapsed?1:0)<<"|";for(std::size_t i=0;i<n.tabs.size();++i){if(i)o<<",";o<<n.tabs[i];}o<<"\n";}
    for(const auto& f:w.floatingPanels)o<<"F|"<<f.panelId<<"|"<<f.rect.x<<"|"<<f.rect.y<<"|"<<f.rect.width<<"|"<<f.rect.height<<"\n";
    return o.str();
}

bool SubspaceDockSystem::Deserialize(const std::string& text,SubspaceDockWorkspace& out,std::string* error){
    auto fail=[&](const std::string&m){if(error)*error=m;return false;};std::istringstream in(text);std::string line;if(!std::getline(in,line)||line!="SUBSPACE_DOCK_V1")return fail("Unsupported dock-layout format");
    SubspaceDockWorkspace w;w.panels.clear();w.nodes.clear();w.floatingPanels.clear();
    auto split=[](const std::string& v,char delim){std::vector<std::string> r;std::string x;std::istringstream s(v);while(std::getline(s,x,delim))r.push_back(x);if(!v.empty()&&v.back()==delim)r.push_back({});return r;};
    try{while(std::getline(in,line)){if(line.empty())continue;const auto f=split(line,'|');if(f.empty())continue;if(f[0]=="W"){if(f.size()<3)return fail("Malformed workspace record");w.id=f[1];w.rootNodeId=f[2];}
        else if(f[0]=="P"){if(f.size()<17)return fail("Malformed panel record");SubspaceDockPanel p;p.id=f[1];p.title=f[2];p.defaultLeafId=f[3];p.visible=std::stoi(f[4])!=0;p.closable=std::stoi(f[5])!=0;p.floatable=std::stoi(f[6])!=0;p.resizable=std::stoi(f[7])!=0;p.pinned=std::stoi(f[8])!=0;p.collapsed=std::stoi(f[9])!=0;p.autoHide=std::stoi(f[10])!=0;p.hoverReveal=std::stoi(f[11])!=0;p.opacity=std::stof(f[12]);p.minWidth=std::stof(f[13]);p.minHeight=std::stof(f[14]);p.preferredWidth=std::stof(f[15]);p.preferredHeight=std::stof(f[16]);w.panels.push_back(std::move(p));}
        else if(f[0]=="N"){if(f.size()<10)return fail("Malformed node record");SubspaceDockNode n;n.id=f[1];n.split=std::stoi(f[2])!=0;n.axis=std::stoi(f[3])==0?SubspaceDockSplitAxis::Horizontal:SubspaceDockSplitAxis::Vertical;n.ratio=std::stof(f[4]);n.firstChildId=f[5];n.secondChildId=f[6];n.activeTabId=f[7];n.collapsed=std::stoi(f[8])!=0;if(!f[9].empty())n.tabs=split(f[9],',');w.nodes.push_back(std::move(n));}
        else if(f[0]=="F"){if(f.size()<6)return fail("Malformed floating-panel record");SubspaceFloatingPanel p;p.panelId=f[1];p.rect={std::stof(f[2]),std::stof(f[3]),std::stof(f[4]),std::stof(f[5])};w.floatingPanels.push_back(std::move(p));}}}
    catch(...){return fail("Invalid numeric value in dock-layout data");}
    std::string validation;if(!Validate(w,&validation))return fail(validation);out=std::move(w);return true;
}

bool SubspaceDockSystem::Validate(const SubspaceDockWorkspace& w,std::string* error){
    if(w.rootNodeId.empty()||!FindNode(w,w.rootNodeId)){if(error)*error="dock workspace missing root";return false;}
    std::unordered_set<std::string> ids,owned;for(const auto& p:w.panels){if(p.id.empty()||!ids.insert(p.id).second){if(error)*error="duplicate/empty panel id";return false;}if(p.opacity<.0f||p.opacity>1.0f||p.minWidth<=0||p.minHeight<=0||p.preferredWidth<p.minWidth||p.preferredHeight<p.minHeight){if(error)*error="invalid panel geometry/style";return false;}}
    std::unordered_set<std::string> nodeIds;for(const auto& n:w.nodes){if(n.id.empty()||!nodeIds.insert(n.id).second){if(error)*error="duplicate/empty dock node id";return false;}if(n.split){if(n.firstChildId.empty()||n.secondChildId.empty()||n.ratio<=0||n.ratio>=1){if(error)*error="invalid dock split";return false;}}else for(const auto& id:n.tabs){if(!FindPanel(w,id)||!owned.insert(id).second){if(error)*error="panel is unknown or hosted twice";return false;}}}
    for(const auto& n:w.nodes)if(n.split&&(!FindNode(w,n.firstChildId)||!FindNode(w,n.secondChildId))){if(error)*error="split references missing child";return false;}
    for(const auto& f:w.floatingPanels){const auto* p=FindPanel(w,f.panelId);if(!p||!p->floatable||!owned.insert(f.panelId).second){if(error)*error="invalid floating panel";return false;}}
    return true;
}

} // namespace subspace
