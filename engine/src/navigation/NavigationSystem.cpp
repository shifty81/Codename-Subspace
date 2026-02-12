#include "navigation/NavigationSystem.h"

#include <cmath>

namespace subspace {

// ---------------------------------------------------------------------------
// SectorCoordinate
// ---------------------------------------------------------------------------

float SectorCoordinate::DistanceTo(const SectorCoordinate& other) const
{
    float dx = static_cast<float>(x - other.x);
    float dy = static_cast<float>(y - other.y);
    float dz = static_cast<float>(z - other.z);
    return std::sqrt(dx * dx + dy * dy + dz * dz);
}

bool SectorCoordinate::IsInRangeOf(const SectorCoordinate& other, float range) const
{
    return DistanceTo(other) <= range;
}

float SectorCoordinate::DistanceFromCenter() const
{
    return DistanceTo(SectorCoordinate(0, 0, 0));
}

int SectorCoordinate::GetTechLevel() const
{
    float dist = DistanceFromCenter();
    if (dist < 2.0f)  return 7;
    if (dist < 5.0f)  return 6;
    if (dist < 10.0f) return 5;
    if (dist < 20.0f) return 4;
    if (dist < 40.0f) return 3;
    if (dist < 80.0f) return 2;
    return 1;
}

SecurityLevel SectorCoordinate::GetSecurityLevel() const
{
    float dist = DistanceFromCenter();
    if (dist < 10.0f) return SecurityLevel::HighSec;
    if (dist < 30.0f) return SecurityLevel::LowSec;
    return SecurityLevel::NullSec;
}

// ---------------------------------------------------------------------------
// HyperdriveComponent
// ---------------------------------------------------------------------------

bool HyperdriveComponent::CanJump() const
{
    return !isCharging && timeSinceLastJump >= jumpCooldown && IsFullyCharged();
}

bool HyperdriveComponent::IsFullyCharged() const
{
    return currentCharge >= chargeTime;
}

void HyperdriveComponent::StartCharge(const SectorCoordinate& target)
{
    targetSector  = target;
    hasTarget     = true;
    isCharging    = true;
    currentCharge = 0.0f;
}

void HyperdriveComponent::CancelCharge()
{
    isCharging    = false;
    hasTarget     = false;
    currentCharge = 0.0f;
}

// ---------------------------------------------------------------------------
// NavigationSystem
// ---------------------------------------------------------------------------

NavigationSystem::NavigationSystem()
    : SystemBase("NavigationSystem")
{
}

void NavigationSystem::Update(float /*deltaTime*/)
{
    // TODO: iterate over all HyperdriveComponent instances via a component
    // registry and update charging / cooldown timers each frame.
}

bool NavigationSystem::StartJumpCharge(HyperdriveComponent& drive, const SectorCoordinate& target)
{
    if (drive.isCharging) return false;
    drive.StartCharge(target);
    return true;
}

bool NavigationSystem::ExecuteJump(HyperdriveComponent& drive, SectorLocationComponent& location)
{
    if (!drive.CanJump() || !drive.hasTarget) return false;

    location.currentSector      = drive.targetSector;
    drive.timeSinceLastJump     = 0.0f;
    drive.currentCharge         = 0.0f;
    drive.isCharging            = false;
    drive.hasTarget             = false;
    return true;
}

void NavigationSystem::CancelJump(HyperdriveComponent& drive)
{
    drive.CancelCharge();
}

float NavigationSystem::CalculateJumpFuelCost(const SectorCoordinate& from, const SectorCoordinate& to) const
{
    return from.DistanceTo(to) * 10.0f;
}

bool NavigationSystem::IsInJumpRange(const HyperdriveComponent& drive, const SectorCoordinate& from, const SectorCoordinate& to) const
{
    return from.DistanceTo(to) <= drive.jumpRange;
}

} // namespace subspace
