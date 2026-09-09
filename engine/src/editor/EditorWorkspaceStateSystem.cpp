#include "editor/EditorWorkspaceStateSystem.h"
#include <algorithm>
namespace subspace {
void EditorWorkspaceStateSystem::Set(EditorWorkspacePreferences p){p.leftPanelWidth=std::clamp(p.leftPanelWidth,180.0f,640.0f);p.rightPanelWidth=std::clamp(p.rightPanelWidth,220.0f,720.0f);p.bottomPanelHeight=std::clamp(p.bottomPanelHeight,100.0f,420.0f);state_[static_cast<int>(p.workspace)]=std::move(p);}
EditorWorkspacePreferences EditorWorkspaceStateSystem::Get(EditorWorkspaceKind w)const{auto it=state_.find(static_cast<int>(w));if(it!=state_.end())return it->second;EditorWorkspacePreferences p;p.workspace=w;return p;}
void EditorWorkspaceStateSystem::Reset(EditorWorkspaceKind w){state_.erase(static_cast<int>(w));}
} // namespace subspace
