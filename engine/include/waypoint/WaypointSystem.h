#pragma once

#include "core/ecs/Entity.h"
#include "core/ecs/IComponent.h"
#include "core/ecs/SystemBase.h"
#include "core/ecs/EntityManager.h"
#include "core/persistence/SaveGameManager.h"

#include <string>
#include <vector>
#include <cstdint>

namespace subspace {

/// Types of waypoints the player or AI can place.
enum class WaypointType {
    Generic,       // General purpose marker
    Navigation,    // Route waypoint
    PointOfInterest,// Notable location
    Danger,        // Hazard marker
    Mining,        // Resource site
    Trading,       // Trade station marker
    Rally,         // Fleet rally point
    Custom         // User-defined
};

/// Visual marker style for a waypoint.
enum class WaypointIcon {
    Circle,
    Diamond,
    Triangle,
    Star,
    Crosshair,
    Flag,
    Anchor,
    Skull
};

/// A single waypoint placed in the game world.
struct Waypoint {
    uint64_t waypointId = 0;
    std::string label;
    WaypointType type = WaypointType::Generic;
    WaypointIcon icon = WaypointIcon::Circle;
    float posX = 0.0f, posY = 0.0f, posZ = 0.0f;
    uint64_t sectorId = 0;
    bool isVisible = true;
    float expiryTime = -1.0f;   // <0 means permanent
    float elapsedTime = 0.0f;

    /// Distance to a point.
    float DistanceTo(float x, float y, float z) const;

    /// Has the waypoint expired?
    bool IsExpired() const;

    /// Get the display name for a waypoint type.
    static std::string GetTypeName(WaypointType type);

    /// Get the display name for an icon.
    static std::string GetIconName(WaypointIcon icon);
};

/// ECS component that stores waypoints for an entity (e.g. a player).
class WaypointComponent : public IComponent {
public:
    WaypointComponent() = default;

    /// Add a waypoint. Returns false if at capacity.
    bool AddWaypoint(const Waypoint& wp);

    /// Remove a waypoint by its ID. Returns false if not found.
    bool RemoveWaypoint(uint64_t waypointId);

    /// Get a waypoint by ID. Returns nullptr if not found.
    const Waypoint* GetWaypoint(uint64_t waypointId) const;

    /// Get all waypoints.
    const std::vector<Waypoint>& GetAllWaypoints() const;

    /// Get waypoints of a specific type.
    std::vector<const Waypoint*> GetWaypointsByType(WaypointType type) const;

    /// Get waypoints within a sector.
    std::vector<const Waypoint*> GetWaypointsInSector(uint64_t sectorId) const;

    /// Get the nearest waypoint to a position.
    const Waypoint* GetNearest(float x, float y, float z) const;

    /// Get the number of waypoints.
    size_t GetCount() const;

    /// Remove all expired waypoints.
    void ClearExpired();

    /// Remove all waypoints.
    void ClearAll();

    /// Toggle visibility of a waypoint.
    bool ToggleVisibility(uint64_t waypointId);

    /// Next available waypoint ID.
    uint64_t GetNextId() const;

    /// Maximum waypoints per entity.
    static constexpr size_t kMaxWaypoints = 50;

    /// Serialize for save-game persistence.
    ComponentData Serialize() const;

    /// Restore from previously serialized data.
    void Deserialize(const ComponentData& data);

private:
    std::vector<Waypoint> _waypoints;
    uint64_t _nextId = 1;

    Waypoint* FindMutable(uint64_t waypointId);

    friend class WaypointSystem;
};

/// System that updates waypoint timers and cleans up expired entries.
class WaypointSystem : public SystemBase {
public:
    WaypointSystem();
    explicit WaypointSystem(EntityManager& entityManager);

    void Update(float deltaTime) override;

    void SetEntityManager(EntityManager* em);

private:
    EntityManager* _entityManager = nullptr;
};

} // namespace subspace
