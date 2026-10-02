#pragma once
#include "interior/ShipInteriorSystem.h"
#include <cstdint>
#include <string>
#include <vector>
namespace subspace {
struct InteriorInteractionOption { std::string label; bool enabled=true; std::string reason; std::string actionId; };
enum class InteriorFixtureKind {
    Door, Hatch, Airlock, Console, EngineeringPanel, CargoTerminal,
    Elevator, FleetCommandTerminal, NavigationConsole, MiningConsole,
    SalvageConsole, RefineryConsole, ManufacturingConsole, StorageContainer,
    MedicalStation, HelmSeat, RepairPanel
};
struct InteriorFixtureState {
    InteriorFixtureKind kind=InteriorFixtureKind::Door;
    bool open=false; bool locked=false; bool powered=true; bool pressurized=true; bool cycling=false;
    std::uint64_t fixtureId=0;
    std::string linkedSystemId;
    std::string destinationId;
    bool accessAllowed=true;
    float interactionRangeMeters=2.25f;
};
struct InteriorInteractionContext {
    std::uint64_t actorId=0;
    float distanceMeters=0.0f;
    bool hasPressureSuit=false;
    bool hasAccess=true;
};
struct InteriorInteractionResult { bool success=false; std::string status; std::string actionId; std::uint64_t fixtureId=0; std::string linkedSystemId; std::string destinationId; };
class InteriorInteractionSystem {
public:
    std::vector<InteriorInteractionOption> ActionsFor(const InteriorRoom& room,bool damaged,bool powered) const;
    std::vector<InteriorInteractionOption> ActionsFor(const InteriorFixtureState& fixture) const;
    std::vector<InteriorInteractionOption> ActionsFor(const InteriorFixtureState& fixture,const InteriorInteractionContext& context) const;
    InteriorInteractionResult Execute(InteriorFixtureState& fixture,const std::string& action) const;
    InteriorInteractionResult Execute(InteriorFixtureState& fixture,const std::string& action,const InteriorInteractionContext& context) const;
};
}
