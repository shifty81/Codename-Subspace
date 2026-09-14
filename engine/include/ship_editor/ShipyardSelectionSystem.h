#pragma once

#include "ship_editor/ShipyardStableIdSystem.h"

#include <vector>

namespace subspace {

struct ShipyardSelectionState {
    ShipyardObjectId primary{};
    std::vector<ShipyardObjectId> ordered;
};

class ShipyardSelectionSystem {
public:
    static void Clear(ShipyardSelectionState& state);
    static void SelectSingle(ShipyardSelectionState& state, ShipyardObjectId id);
    static void Add(ShipyardSelectionState& state, ShipyardObjectId id, bool makePrimary = true);
    static void Toggle(ShipyardSelectionState& state, ShipyardObjectId id);
    static bool Remove(ShipyardSelectionState& state, ShipyardObjectId id);
    static bool Contains(const ShipyardSelectionState& state, ShipyardObjectId id);
    static void PruneInvalid(ShipyardSelectionState& state,
                             const std::vector<ShipyardObjectId>& validIds);
};

} // namespace subspace
