#include "ship_editor/ShipyardSelectionSystem.h"

#include <algorithm>

namespace subspace {

void ShipyardSelectionSystem::Clear(ShipyardSelectionState& state) {
    state.primary = {};
    state.ordered.clear();
}

void ShipyardSelectionSystem::SelectSingle(ShipyardSelectionState& state, ShipyardObjectId id) {
    Clear(state);
    if (!id.Valid()) return;
    state.primary = id;
    state.ordered.push_back(id);
}

void ShipyardSelectionSystem::Add(ShipyardSelectionState& state, ShipyardObjectId id, bool makePrimary) {
    if (!id.Valid()) return;
    if (!Contains(state, id)) state.ordered.push_back(id);
    if (makePrimary || !state.primary.Valid()) state.primary = id;
}

void ShipyardSelectionSystem::Toggle(ShipyardSelectionState& state, ShipyardObjectId id) {
    if (!id.Valid()) return;
    if (Contains(state, id)) {
        Remove(state, id);
        return;
    }
    Add(state, id, true);
}

bool ShipyardSelectionSystem::Remove(ShipyardSelectionState& state, ShipyardObjectId id) {
    const auto before = state.ordered.size();
    state.ordered.erase(std::remove(state.ordered.begin(), state.ordered.end(), id), state.ordered.end());
    if (state.primary == id) state.primary = state.ordered.empty() ? ShipyardObjectId{} : state.ordered.back();
    return state.ordered.size() != before;
}

bool ShipyardSelectionSystem::Contains(const ShipyardSelectionState& state, ShipyardObjectId id) {
    return std::find(state.ordered.begin(), state.ordered.end(), id) != state.ordered.end();
}

void ShipyardSelectionSystem::PruneInvalid(ShipyardSelectionState& state,
                                            const std::vector<ShipyardObjectId>& validIds) {
    state.ordered.erase(
        std::remove_if(state.ordered.begin(), state.ordered.end(), [&](ShipyardObjectId id) {
            return std::find(validIds.begin(), validIds.end(), id) == validIds.end();
        }), state.ordered.end());
    if (std::find(state.ordered.begin(), state.ordered.end(), state.primary) == state.ordered.end())
        state.primary = state.ordered.empty() ? ShipyardObjectId{} : state.ordered.back();
}

} // namespace subspace
