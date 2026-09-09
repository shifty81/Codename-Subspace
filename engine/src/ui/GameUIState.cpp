#include "ui/GameUIState.h"

namespace subspace {
void GameUIState::Open(GamePanel panel){openPanels_.insert(static_cast<int>(panel));}
void GameUIState::Close(GamePanel panel){openPanels_.erase(static_cast<int>(panel));}
void GameUIState::Toggle(GamePanel panel){IsOpen(panel)?Close(panel):Open(panel);}
bool GameUIState::IsOpen(GamePanel panel) const{return openPanels_.find(static_cast<int>(panel))!=openPanels_.end();}
void GameUIState::CloseAll(){openPanels_.clear();modals_.clear();}
void GameUIState::PushModal(const std::string& modalId){if(!modalId.empty())modals_.push_back(modalId);}
bool GameUIState::PopModal(){if(modals_.empty())return false;modals_.pop_back();return true;}
std::string GameUIState::ActiveModal() const{return modals_.empty()?std::string{}:modals_.back();}
} // namespace subspace
