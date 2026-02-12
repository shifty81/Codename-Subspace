#pragma once

#include "core/ecs/IComponent.h"
#include "core/ecs/SystemBase.h"
#include "core/Math.h"

#include <cmath>

namespace subspace {

/// Security classification for sectors based on distance from center.
enum class SecurityLevel { HighSec, LowSec, NullSec, WormholeSpace };

/// Grid coordinate identifying a sector in the galaxy map.
struct SectorCoordinate {
    int x = 0;
    int y = 0;
    int z = 0;

    constexpr SectorCoordinate() = default;
    constexpr SectorCoordinate(int x, int y, int z) : x(x), y(y), z(z) {}

    /// Euclidean distance to another sector.
    float DistanceTo(const SectorCoordinate& other) const;

    /// Check whether another sector is within the given range.
    bool IsInRangeOf(const SectorCoordinate& other, float range) const;

    /// Distance from the galactic center (origin).
    float DistanceFromCenter() const;

    /// Technology level (1-7) based on distance from center.
    int GetTechLevel() const;

    /// Security classification based on distance from center.
    SecurityLevel GetSecurityLevel() const;

    constexpr bool operator==(const SectorCoordinate& o) const { return x == o.x && y == o.y && z == o.z; }
    constexpr bool operator!=(const SectorCoordinate& o) const { return !(*this == o); }
};

/// Component for hyperdrive jump capabilities (port of C# HyperdriveComponent).
struct HyperdriveComponent : IComponent {
    float jumpRange        = 5.0f;
    float jumpCooldown     = 10.0f;
    float chargeTime       = 5.0f;
    float currentCharge    = 0.0f;
    float timeSinceLastJump = 0.0f;
    bool  isCharging       = false;
    bool  hasTarget        = false;
    SectorCoordinate targetSector;

    /// Whether the drive is ready to execute a jump.
    bool CanJump() const;

    /// Whether the drive has accumulated enough charge.
    bool IsFullyCharged() const;

    /// Begin charging toward a target sector.
    void StartCharge(const SectorCoordinate& target);

    /// Cancel the current charge sequence.
    void CancelCharge();
};

/// Component that tracks an entity's current sector (port of C# SectorLocationComponent).
struct SectorLocationComponent : IComponent {
    SectorCoordinate currentSector;
};

/// System that manages hyperdrive navigation and sector transitions (port of C# NavigationSystem).
class NavigationSystem : public SystemBase {
public:
    NavigationSystem();

    void Update(float deltaTime) override;

    /// Initiate jump charging toward a target sector.
    bool StartJumpCharge(HyperdriveComponent& drive, const SectorCoordinate& target);

    /// Execute a charged jump, moving the entity to the target sector.
    bool ExecuteJump(HyperdriveComponent& drive, SectorLocationComponent& location);

    /// Cancel an in-progress jump charge.
    void CancelJump(HyperdriveComponent& drive);

    /// Calculate fuel cost for a jump between two sectors.
    float CalculateJumpFuelCost(const SectorCoordinate& from, const SectorCoordinate& to) const;

    /// Check whether a jump between two sectors is within drive range.
    bool IsInJumpRange(const HyperdriveComponent& drive, const SectorCoordinate& from, const SectorCoordinate& to) const;
};

} // namespace subspace
