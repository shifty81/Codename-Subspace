#pragma once

#include "core/ecs/Entity.h"
#include "core/ecs/IComponent.h"
#include "core/ecs/SystemBase.h"
#include "core/ecs/EntityManager.h"
#include "core/persistence/SaveGameManager.h"

#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>

namespace subspace {

/// States a trade route entity can be in.
enum class TradeRouteState { Idle, Traveling, Buying, Selling, WaitingForCargo, Completed };

/// A single stop on a trade route.
struct TradeWaypoint {
    std::string stationId;
    std::string stationName;
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    std::vector<std::string> buyGoods;
    std::vector<std::string> sellGoods;
    float waitTime = 5.0f;

    /// Get the display name for a trade route state.
    static std::string GetStateName(TradeRouteState state);
};

/// Defines a complete trade route with ordered waypoints.
struct TradeRoute {
    std::string routeId;
    std::string routeName;
    std::vector<TradeWaypoint> waypoints;
    bool isLoop = true;
    float totalDistance = 0.0f;
    int completedRuns = 0;
    float totalProfit = 0.0f;

    /// Calculate total distance from waypoints using 3D Euclidean distances.
    float CalculateDistance() const;

    /// Returns true if the route has at least 2 waypoints.
    bool IsValid() const;
};

/// ECS component that gives an entity automated trade route capabilities.
struct TradeRouteComponent : public IComponent {
    /// Set the trade route to follow.
    void SetRoute(const TradeRoute& route);

    /// Get the current trade route.
    const TradeRoute& GetRoute() const;

    /// Get the current state.
    TradeRouteState GetState() const;

    /// Get the index of the current waypoint.
    int GetCurrentWaypointIndex() const;

    /// Get a pointer to the current waypoint, or nullptr if invalid.
    const TradeWaypoint* GetCurrentWaypoint() const;

    /// Get a pointer to the next waypoint (with loop handling), or nullptr.
    const TradeWaypoint* GetNextWaypoint() const;

    /// Get travel progress between waypoints (0 to 1).
    float GetTravelProgress() const;

    /// Get travel speed in units per second.
    float GetTravelSpeed() const;

    /// Set travel speed in units per second.
    void SetTravelSpeed(float speed);

    /// Check if the route is actively being traversed.
    bool IsActive() const;

    /// Begin traversing the route.
    void StartRoute();

    /// Stop traversing and reset to Idle.
    void StopRoute();

    /// Pause traversal without resetting progress.
    void PauseRoute();

    /// Advance to the next waypoint. Returns false if completed (non-loop route).
    bool AdvanceToNextWaypoint();

    /// Get profit accumulated during the current run.
    float GetCurrentRunProfit() const;

    /// Add profit to the current run.
    void AddProfit(float amount);

    /// Get the list of currently carried goods.
    const std::vector<std::string>& GetCargoManifest() const;

    /// Add a good to the cargo manifest.
    void AddCargo(const std::string& goodId);

    /// Remove a good from the cargo manifest.
    void RemoveCargo(const std::string& goodId);

    /// Clear all cargo.
    void ClearCargo();

    /// Get the total number of completed route runs.
    int GetTotalCompletedRuns() const;

    /// Get the cumulative profit from all runs.
    float GetTotalProfit() const;

    /// Serialize for save-game persistence.
    ComponentData Serialize() const;

    /// Restore from previously serialized data.
    void Deserialize(const ComponentData& data);

private:
    friend class TradeRouteSystem;

    TradeRoute _route;
    TradeRouteState _state = TradeRouteState::Idle;
    int _currentWaypointIndex = 0;
    float _waitTimer = 0.0f;
    float _travelProgress = 0.0f;
    float _travelSpeed = 10.0f;
    bool _isActive = false;
    float _currentRunProfit = 0.0f;
    std::vector<std::string> _cargoManifest;
};

/// System that updates trade route components each frame.
class TradeRouteSystem : public SystemBase {
public:
    TradeRouteSystem();
    explicit TradeRouteSystem(EntityManager& entityManager);

    void Update(float deltaTime) override;

    /// Set the entity manager used to query components.
    void SetEntityManager(EntityManager* em);

private:
    EntityManager* _entityManager = nullptr;
};

} // namespace subspace
