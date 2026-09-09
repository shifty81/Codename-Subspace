#include "interior/InteriorConstructionSystem.h"

#include <algorithm>

namespace subspace {
namespace {
const InteriorModuleAssetDef* PickTraversal(const ModularInteriorKitSystem& systems, const InteriorModuleKit& kit) {
    if (const auto* door = systems.FindByKind(kit, InteriorModuleKind::Door)) return door;
    if (const auto* corridor = systems.FindByKind(kit, InteriorModuleKind::CorridorStraight)) return corridor;
    return nullptr;
}
}

InteriorConstructionPlan InteriorConstructionSystem::BuildLinearPlan(
    std::uint64_t shipId,
    const InteriorModuleKit& kit,
    const std::vector<InteriorRoomRequest>& roomRequests,
    int deck) const
{
    InteriorConstructionPlan plan;
    plan.shipId = shipId;
    if (shipId == 0 || roomRequests.empty()) return plan;

    ModularInteriorKitSystem kitSystem;
    if (!kitSystem.Validate(kit).valid) return plan;
    const auto* floor = kitSystem.FindByKind(kit, InteriorModuleKind::Floor);
    const auto* wall = kitSystem.FindByKind(kit, InteriorModuleKind::Wall);
    const auto* traversal = PickTraversal(kitSystem, kit);
    if (!floor || !wall || !traversal) return plan;

    int cursorX = 0;
    plan.rooms.reserve(roomRequests.size());
    for (std::size_t roomIndex = 0; roomIndex < roomRequests.size(); ++roomIndex) {
        auto room = roomRequests[roomIndex];
        room.widthCells = std::max(1, room.widthCells);
        room.heightCells = std::max(1, room.heightCells);
        room.crewCapacity = std::max(0, room.crewCapacity);
        room.powerDemand = std::max(0.0, room.powerDemand);
        plan.rooms.push_back(room);

        for (int y = 0; y < room.heightCells; ++y) {
            for (int x = 0; x < room.widthCells; ++x) {
                plan.placements.push_back({floor->assetId, InteriorModuleKind::Floor, cursorX + x, y, deck, roomIndex});
            }
        }
        for (int x = 0; x < room.widthCells; ++x) {
            plan.placements.push_back({wall->assetId, InteriorModuleKind::Wall, cursorX + x, 0, deck, roomIndex});
            plan.placements.push_back({wall->assetId, InteriorModuleKind::Wall, cursorX + x, room.heightCells, deck, roomIndex});
        }
        for (int y = 1; y < room.heightCells; ++y) {
            plan.placements.push_back({wall->assetId, InteriorModuleKind::Wall, cursorX, y, deck, roomIndex});
            plan.placements.push_back({wall->assetId, InteriorModuleKind::Wall, cursorX + room.widthCells, y, deck, roomIndex});
        }

        cursorX += room.widthCells;
        if (roomIndex + 1 < roomRequests.size()) {
            plan.placements.push_back({traversal->assetId, traversal->kind, cursorX, std::max(1, room.heightCells / 2), deck, roomIndex});
            ++cursorX;
        }
    }
    plan.totalWidthCells = cursorX;
    plan.connected = roomRequests.size() == 1 || std::count_if(plan.placements.begin(), plan.placements.end(), [](const auto& p) {
        return p.moduleKind == InteriorModuleKind::Door || p.moduleKind == InteriorModuleKind::CorridorStraight;
    }) >= static_cast<int>(roomRequests.size() - 1);
    return plan;
}

InteriorConstructionResult InteriorConstructionSystem::ApplyToGameplay(
    const InteriorConstructionPlan& plan,
    ShipInteriorSystem& interiors) const
{
    InteriorConstructionResult out;
    if (plan.shipId == 0) out.errors.emplace_back("shipId is required");
    if (plan.rooms.empty()) out.errors.emplace_back("construction plan has no rooms");
    if (!plan.connected) out.errors.emplace_back("construction plan is not connected");
    if (!out.errors.empty()) return out;

    out.gameplayRoomIds.reserve(plan.rooms.size());
    for (const auto& request : plan.rooms) {
        auto& room = interiors.AddRoom(plan.shipId, request.roomType, 0, request.crewCapacity, request.powerDemand);
        out.gameplayRoomIds.push_back(room.id);
    }
    out.applied = true;
    return out;
}

} // namespace subspace
