#pragma once

#include "input/ControlIntentRouterSystem.h"

#include <cstdint>
#include <string>
#include <utility>

namespace subspace {

/// Runtime-facing player control facade.  It owns identity + active control
/// domain, but delegates actual ship physics, FPS collision, and fleet order
/// execution to their canonical systems.
class PlayerController {
public:
    void SetControlledEntity(std::string entityId) { _controlledEntityId = std::move(entityId); }
    const std::string& GetControlledEntity() const { return _controlledEntityId; }
    bool HasControlledEntity() const { return !_controlledEntityId.empty(); }

    void SetActorId(std::uint64_t actorId) { _actorId = actorId; }
    std::uint64_t GetActorId() const { return _actorId; }

    void SetControlDomain(ControlDomain domain) { _controlDomain = domain; }
    ControlDomain GetControlDomain() const { return _controlDomain; }

    void RouteInput(const InputState& input) { _intent = ControlIntentRouterSystem::Build(input, _controlDomain); }
    const ControlIntent& Intent() const { return _intent; }

    bool CanDirectlyPilot() const { return _controlDomain == ControlDomain::Pilot && HasControlledEntity(); }
    bool CanControlAvatar() const { return _controlDomain == ControlDomain::FirstPerson || _controlDomain == ControlDomain::DockedService; }
    bool CanIssueFleetOrders() const { return _controlDomain == ControlDomain::FleetStrategy; }

private:
    std::string _controlledEntityId;
    std::uint64_t _actorId = 0;
    ControlDomain _controlDomain = ControlDomain::Pilot;
    ControlIntent _intent{};
};

} // namespace subspace
