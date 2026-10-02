#pragma once

#include "core/Math.h"
#include "fleet/FleetCommandSystem.h"
#include "formation/FormationSystem.h"
#include "input/ControlIntentRouterSystem.h"

#include <array>
#include <cstdint>
#include <vector>

namespace subspace {

enum class FleetSelectionMode { Replace, Add, Toggle };

struct FleetStrategyCameraState {
    Vector3 focus{};
    float yawDegrees = 0.0f;
    float tiltDegrees = 55.0f;
    float distance = 120.0f;
    float panSpeed = 60.0f;
    float zoomSpeed = 8.0f;
};

struct FleetSelectionState {
    std::vector<std::uint64_t> selected;
    std::array<std::vector<std::uint64_t>, 10> commandGroups;
};

struct FleetStrategyOrderDraft {
    FleetOrderType type = FleetOrderType::Move;
    std::uint64_t targetEntityId = 0;
    Vector3 targetPosition{};
    bool queue = false;
    int priority = 0;
    float acceptanceRadius = 5.0f;
    float orbitRadius = 0.0f;
};

/// Strategy-game interaction state for fleet command.  It never mutates ship
/// physics directly; it selects actors, moves the command camera, and emits
/// FleetOrderRequest objects for FleetCommandSystem/domain executors.
class FleetStrategyControlSystem {
public:
    const FleetSelectionState& Selection() const { return _selection; }
    const FleetStrategyCameraState& Camera() const { return _camera; }
    FleetStrategyCameraState& Camera() { return _camera; }

    void ClearSelection();
    void Select(std::uint64_t entityId, FleetSelectionMode mode = FleetSelectionMode::Replace);
    void SelectMany(const std::vector<std::uint64_t>& entityIds, FleetSelectionMode mode = FleetSelectionMode::Replace);
    bool IsSelected(std::uint64_t entityId) const;

    bool AssignCommandGroup(int index);
    bool RecallCommandGroup(int index, bool addToSelection = false);

    void TickCamera(const ControlIntent& intent, float deltaSeconds);
    void Zoom(float wheelDelta);
    void Orbit(float yawDeltaDegrees, float tiltDeltaDegrees);
    void Focus(Vector3 worldPosition) { _camera.focus = worldPosition; }

    FleetOrderRequest BuildRequest(const FleetStrategyOrderDraft& draft) const;
    bool Issue(FleetCommandComponent& fleet, const FleetStrategyOrderDraft& draft) const;

    void SetFormation(FormationType type, float spacingMeters);
    FormationType Formation() const { return _formation; }
    float FormationSpacing() const { return _formationSpacing; }

private:
    static void NormalizeUnique(std::vector<std::uint64_t>& ids);

    FleetSelectionState _selection{};
    FleetStrategyCameraState _camera{};
    FormationType _formation = FormationType::Wedge;
    float _formationSpacing = 25.0f;
};

} // namespace subspace
