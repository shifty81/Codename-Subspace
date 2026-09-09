#include "editor/EditorActionRegistry.h"

#include <algorithm>

namespace subspace {
void EditorActionRegistry::Register(RegisteredEditorAction a){if(a.action.id.empty())return;actions_[a.action.id]=std::move(a);}
const RegisteredEditorAction* EditorActionRegistry::Find(const std::string&id)const{auto it=actions_.find(id);return it==actions_.end()?nullptr:&it->second;}
std::vector<EditorContextAction> EditorActionRegistry::ActionsForCategory(const std::string&category)const{std::vector<EditorContextAction>out;for(const auto&kv:actions_)if(category.empty()||kv.second.category==category)out.push_back(kv.second.action);std::sort(out.begin(),out.end(),[](const auto&a,const auto&b){return a.label<b.label;});return out;}
bool EditorActionRegistry::Execute(const std::string&id,std::string*reason)const{const auto*a=Find(id);if(!a){if(reason)*reason="unknown editor action";return false;}if(!a->action.enabled){if(reason)*reason=a->action.disabledReason.empty()?"editor action is disabled":a->action.disabledReason;return false;}if(!a->execute){if(reason)*reason="editor action has no execution handler";return false;}return a->execute();}
} // namespace subspace
