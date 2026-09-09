#include "editor/SubspaceEditorCore.h"
#include <algorithm>
namespace subspace {
void EditorWorkspaceRegistry::Register(EditorWorkspaceDescriptor d){for(auto&x:workspaces_)if(x.kind==d.kind){x=std::move(d);return;}workspaces_.push_back(std::move(d));}
const EditorWorkspaceDescriptor* EditorWorkspaceRegistry::Find(EditorWorkspaceKind k)const{for(const auto&x:workspaces_)if(x.kind==k)return &x;return nullptr;}
void EditorSelectionService::Select(EditorSelectionItem i,bool a){if(!a)items_.clear();for(auto&x:items_)if(x.id==i.id){x=std::move(i);return;}items_.push_back(std::move(i));}
void EditorSelectionService::Clear(){items_.clear();}bool EditorSelectionService::Contains(const std::string&id)const{return std::any_of(items_.begin(),items_.end(),[&](const auto&i){return i.id==id;});}const EditorSelectionItem* EditorSelectionService::Primary()const{return items_.empty()?nullptr:&items_.front();}
std::vector<EditorOutlinerNode> EditorOutlinerModel::ChildrenOf(const std::string&p)const{std::vector<EditorOutlinerNode>o;for(const auto&n:nodes_)if(n.parentId==p)o.push_back(n);return o;}const EditorOutlinerNode* EditorOutlinerModel::Find(const std::string&id)const{for(const auto&n:nodes_)if(n.id==id)return &n;return nullptr;}
bool EditorCommandStack::Execute(std::unique_ptr<EditorCommand>c){if(!c||!c->Execute())return false;if(cursor_<history_.size())history_.erase(history_.begin()+static_cast<std::ptrdiff_t>(cursor_),history_.end());history_.push_back(std::move(c));cursor_=history_.size();return true;}bool EditorCommandStack::Undo(){if(!CanUndo())return false;--cursor_;history_[cursor_]->Undo();return true;}bool EditorCommandStack::Redo(){if(!CanRedo())return false;if(!history_[cursor_]->Execute())return false;++cursor_;return true;}std::string EditorCommandStack::UndoDescription()const{return CanUndo()?history_[cursor_-1]->Description():std::string{};}std::string EditorCommandStack::RedoDescription()const{return CanRedo()?history_[cursor_]->Description():std::string{};}
}
