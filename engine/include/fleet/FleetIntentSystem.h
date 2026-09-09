#pragma once

#include "formation/FormationSystem.h"
#include "flight/StrategicFlightSystem.h"
#include <cstdint>
#include <string>
#include <vector>

namespace subspace {

enum class FleetShipRole { Leader, Combat, Mining, Salvage, Support };
struct FleetWingShip { std::uint64_t id=0; FleetShipRole role=FleetShipRole::Combat; bool operational=true; };
struct FleetIntentAssignment { std::uint64_t shipId=0; StrategicOrderKind order=StrategicOrderKind::Follow; std::string behavior; };
struct FleetFlightConfig { FormationType travelFormation=FormationType::V; FormationType combatFormation=FormationType::Wedge; FormationType miningFormation=FormationType::Line; float spacing=18.0f; bool mirrorTarget=true; bool autoFormation=true; };
class FleetIntentSystem {
public:
    std::vector<FleetIntentAssignment> Mirror(const std::vector<FleetWingShip>& wing,StrategicOrderKind playerOrder) const;
    FormationType FormationFor(const FleetFlightConfig& config,StrategicOrderKind order) const;
};

} // namespace subspace
