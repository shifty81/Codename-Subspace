#pragma once
#include "interior/ShipInteriorSystem.h"
#include <string>
#include <vector>
namespace subspace {
struct InteriorInteractionOption { std::string label; bool enabled=true; std::string reason; };
enum class InteriorFixtureKind { Door, Hatch, Airlock, Console, EngineeringPanel, CargoTerminal };
struct InteriorFixtureState { InteriorFixtureKind kind=InteriorFixtureKind::Door; bool open=false; bool locked=false; bool powered=true; bool pressurized=true; bool cycling=false; };
struct InteriorInteractionResult { bool success=false; std::string status; };
class InteriorInteractionSystem {
public:
    std::vector<InteriorInteractionOption> ActionsFor(const InteriorRoom& room,bool damaged,bool powered) const;
    std::vector<InteriorInteractionOption> ActionsFor(const InteriorFixtureState& fixture) const;
    InteriorInteractionResult Execute(InteriorFixtureState& fixture,const std::string& action) const;
};
}
