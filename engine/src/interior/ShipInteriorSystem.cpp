#include "interior/ShipInteriorSystem.h"

#include <algorithm>

namespace subspace {

InteriorRoom& ShipInteriorSystem::AddRoom(std::uint64_t shipId, InteriorRoomType type, int deck, int crewCapacity, double powerDemand) {
    auto& layout = layouts_[shipId];
    layout.shipId = shipId;
    InteriorRoom room;
    room.id = nextRoomId_++;
    room.type = type;
    room.deck = deck;
    room.crewCapacity = std::max(0, crewCapacity);
    room.powerDemand = std::max(0.0, powerDemand);
    layout.rooms.push_back(room);
    return layout.rooms.back();
}

static InteriorRoom* FindRoom(ShipInteriorLayout& layout, std::uint64_t roomId) {
    for (auto& room : layout.rooms) if (room.id == roomId) return &room;
    return nullptr;
}

bool ShipInteriorSystem::DamageRoom(std::uint64_t shipId, std::uint64_t roomId, double damage) {
    auto it = layouts_.find(shipId); if (it == layouts_.end() || damage <= 0.0) return false;
    auto* room = FindRoom(it->second, roomId); if (!room) return false;
    room->health = std::max(0.0, room->health - damage);
    if (room->health <= 0.0) room->pressurized = false;
    return true;
}

bool ShipInteriorSystem::RepairRoom(std::uint64_t shipId, std::uint64_t roomId, double amount) {
    auto it = layouts_.find(shipId); if (it == layouts_.end() || amount <= 0.0) return false;
    auto* room = FindRoom(it->second, roomId); if (!room) return false;
    room->health = std::min(room->maxHealth, room->health + amount);
    if (room->health > room->maxHealth * 0.25) room->pressurized = true;
    return true;
}

bool ShipInteriorSystem::SetCrewOccupancy(std::uint64_t shipId, std::uint64_t roomId, int occupancy) {
    auto it = layouts_.find(shipId); if (it == layouts_.end()) return false;
    auto* room = FindRoom(it->second, roomId); if (!room || occupancy < 0 || occupancy > room->crewCapacity) return false;
    room->crewOccupancy = occupancy; return true;
}

const ShipInteriorLayout* ShipInteriorSystem::GetLayout(std::uint64_t shipId) const {
    auto it = layouts_.find(shipId); return it == layouts_.end() ? nullptr : &it->second;
}

double ShipInteriorSystem::GetIntegrity(std::uint64_t shipId) const {
    const auto* layout = GetLayout(shipId); if (!layout || layout->rooms.empty()) return 1.0;
    double current=0.0, maximum=0.0;
    for (const auto& room : layout->rooms) { current += room.health; maximum += room.maxHealth; }
    return maximum > 0.0 ? current / maximum : 1.0;
}

int ShipInteriorSystem::GetCrewCapacity(std::uint64_t shipId) const {
    const auto* layout = GetLayout(shipId); if (!layout) return 0;
    int total=0; for (const auto& room : layout->rooms) total += room.crewCapacity; return total;
}

int ShipInteriorSystem::GetCrewOccupancy(std::uint64_t shipId) const {
    const auto* layout = GetLayout(shipId); if (!layout) return 0;
    int total=0; for (const auto& room : layout->rooms) total += room.crewOccupancy; return total;
}

bool ShipInteriorSystem::ClearLayout(std::uint64_t shipId) { return layouts_.erase(shipId) > 0; }

} // namespace subspace
