#pragma once

#include <cstdint>
#include <string>
#include <unordered_set>
#include <vector>

namespace subspace {

enum class GamePanel {
    Inventory, Fitting, Shipyard, GalaxyMap, Contracts, Logistics, Fleet,
    Crew, Subsystems, QuestLog, Builder, StationServices, Settings
};

class GameUIState {
public:
    void Open(GamePanel panel);
    void Close(GamePanel panel);
    void Toggle(GamePanel panel);
    bool IsOpen(GamePanel panel) const;
    void CloseAll();

    void PushModal(const std::string& modalId);
    bool PopModal();
    std::string ActiveModal() const;
    std::size_t ModalDepth() const { return modals_.size(); }

    void SetSelectedEntity(std::uint64_t entityId) { selectedEntity_ = entityId; }
    std::uint64_t GetSelectedEntity() const { return selectedEntity_; }

private:
    std::unordered_set<int> openPanels_;
    std::vector<std::string> modals_;
    std::uint64_t selectedEntity_ = 0;
};

} // namespace subspace
