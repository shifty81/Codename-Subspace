#pragma once

#include "interior/ModularInteriorKitSystem.h"
#include "interior/ShipInteriorSystem.h"

#include <cstdint>
#include <string>
#include <vector>

namespace subspace {

struct InteriorRoomRequest {
    InteriorRoomType roomType = InteriorRoomType::Corridor;
    int widthCells = 2;
    int heightCells = 2;
    int crewCapacity = 0;
    double powerDemand = 0.0;
};

struct InteriorModulePlacement {
    std::string moduleAssetId;
    InteriorModuleKind moduleKind = InteriorModuleKind::Floor;
    int gridX = 0;
    int gridY = 0;
    int deck = 0;
    std::size_t roomIndex = 0;
};

struct InteriorConstructionPlan {
    std::uint64_t shipId = 0;
    std::vector<InteriorRoomRequest> rooms;
    std::vector<InteriorModulePlacement> placements;
    bool connected = false;
    int totalWidthCells = 0;
};

struct InteriorConstructionResult {
    bool applied = false;
    std::vector<std::uint64_t> gameplayRoomIds;
    std::vector<std::string> errors;
};

/// Pass320 turns normalized modular art into a deterministic gameplay layout.
/// Visual modules are placements; room health/power/crew authority remains in
/// ShipInteriorSystem.
class InteriorConstructionSystem {
public:
    InteriorConstructionPlan BuildLinearPlan(
        std::uint64_t shipId,
        const InteriorModuleKit& kit,
        const std::vector<InteriorRoomRequest>& roomRequests,
        int deck = 0) const;

    InteriorConstructionResult ApplyToGameplay(
        const InteriorConstructionPlan& plan,
        ShipInteriorSystem& interiors) const;
};

} // namespace subspace
