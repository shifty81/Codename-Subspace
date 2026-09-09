#pragma once
#include "runtime/SimulationLodSystem.h"
#include <cstdint>
#include <string>
#include <vector>
namespace subspace {
enum class SectorEventType { Convoy, MiningFleet, Patrol, PirateRaid, DistressCall, Construction, Shortage, Discovery };
struct SectorEvent { SectorEventType type=SectorEventType::Convoy; std::string description; double severity=0; };
struct DynamicSectorState { std::string id; SimulationLod lod=SimulationLod::Aggregate; double security=0.7; double prosperity=0.5; double resourcePressure=0.2; double piratePressure=0.1; int npcStations=1; std::uint64_t tick=0; };
class SectorSimulationSystem {
public:
 std::vector<SectorEvent> Advance(DynamicSectorState& state,double hours) const;
};
}
