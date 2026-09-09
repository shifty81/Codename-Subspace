#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace subspace {

enum class FleetWingRole { Combat, Mining, Salvage, Logistics, Exploration, Support };
enum class FleetStandingOrder { Hold, Escort, MineArea, SalvageArea, HaulRoute, Defend, EngageHostiles, RepairFleet };
struct FleetWingNative { std::uint64_t id=0;std::string name;FleetWingRole role=FleetWingRole::Combat;std::vector<std::uint64_t> ships;FleetStandingOrder order=FleetStandingOrder::Hold;std::uint64_t target=0; };
struct CorporationAssetLedger { double credits=0;std::vector<std::uint64_t> ships;std::vector<std::uint64_t> stations;std::vector<std::uint64_t> planetaryOperations; };

class CorporationOperationsSystem {
public:
    std::uint64_t CreateWing(const std::string& name,FleetWingRole role);
    bool AssignShip(std::uint64_t wingId,std::uint64_t shipId);
    bool IssueStandingOrder(std::uint64_t wingId,FleetStandingOrder order,std::uint64_t target=0);
    const FleetWingNative* GetWing(std::uint64_t wingId) const;
    double LogisticsReadiness(const CorporationAssetLedger& assets) const;
private:
    std::uint64_t nextWing_=1;std::unordered_map<std::uint64_t,FleetWingNative> wings_;
};

} // namespace subspace
