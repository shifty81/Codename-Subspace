#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>

namespace subspace {

enum class CrisisContribution { Combat, Mining, Manufacturing, Logistics, Exploration, Salvage };
enum class SectorCrisisType { PirateArmada, MachineOutbreak, PlanetarySiege, LogisticsCollapse, AncientAwakening, StellarEmergency };
struct SectorCrisis { std::uint64_t id=0; SectorCrisisType type=SectorCrisisType::PirateArmada; std::string name; double requiredProgress=100; double progress=0; bool completed=false; std::unordered_map<int,double> contribution; };
class SectorCrisisSystem {
public:
    std::uint64_t Create(SectorCrisisType type,const std::string& name,double requiredProgress=100);
    bool Contribute(std::uint64_t id,CrisisContribution channel,double amount);
    const SectorCrisis* Get(std::uint64_t id) const;
private:
    std::uint64_t nextId_=1;std::unordered_map<std::uint64_t,SectorCrisis> crises_;
};

} // namespace subspace
