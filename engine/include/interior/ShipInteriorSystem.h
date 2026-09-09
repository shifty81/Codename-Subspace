#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace subspace {

enum class InteriorRoomType { Cockpit, Engineering, Reactor, Cargo, CrewQuarters, Medbay, Workshop, Airlock, Corridor };

struct InteriorRoom {
    std::uint64_t id = 0;
    InteriorRoomType type = InteriorRoomType::Corridor;
    int deck = 0;
    double maxHealth = 100.0;
    double health = 100.0;
    double powerDemand = 0.0;
    int crewCapacity = 0;
    int crewOccupancy = 0;
    bool pressurized = true;
};

struct ShipInteriorLayout {
    std::uint64_t shipId = 0;
    std::vector<InteriorRoom> rooms;
};

class ShipInteriorSystem {
public:
    InteriorRoom& AddRoom(std::uint64_t shipId, InteriorRoomType type, int deck, int crewCapacity = 0, double powerDemand = 0.0);
    bool DamageRoom(std::uint64_t shipId, std::uint64_t roomId, double damage);
    bool RepairRoom(std::uint64_t shipId, std::uint64_t roomId, double amount);
    bool SetCrewOccupancy(std::uint64_t shipId, std::uint64_t roomId, int occupancy);
    const ShipInteriorLayout* GetLayout(std::uint64_t shipId) const;
    double GetIntegrity(std::uint64_t shipId) const;
    int GetCrewCapacity(std::uint64_t shipId) const;
    int GetCrewOccupancy(std::uint64_t shipId) const;
    bool ClearLayout(std::uint64_t shipId);

private:
    std::uint64_t nextRoomId_ = 1;
    std::unordered_map<std::uint64_t, ShipInteriorLayout> layouts_;
};

} // namespace subspace
