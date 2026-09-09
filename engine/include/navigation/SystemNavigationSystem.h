#pragma once

#include "navigation/AstronomicalScaleSystem.h"
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace subspace {

enum class SystemDestinationType { Planet, Moon, Station, BeltRegion, RingRegion, SalvageSite, Signature, FleetMember, Bookmark, DeepSpace };

struct SystemDestination {
    std::uint64_t id = 0;
    std::string name;
    SystemDestinationType type = SystemDestinationType::DeepSpace;
    AstronomicalPosition position;
    bool discovered = true;
    bool warpable = true;
    double hazardRating = 0.0;
};

struct WarpPlan {
    bool valid = false;
    std::uint64_t destinationId = 0;
    double distanceMeters = 0.0;
    double chargeSeconds = 0.0;
    double cruiseSeconds = 0.0;
    double topSpeedMetersPerSecond = 0.0;
    double fuelCost = 0.0;
    std::string reason;
};

class SystemNavigationSystem {
public:
    bool RegisterDestination(const SystemDestination& destination);
    bool AddBookmark(std::uint64_t destinationId);
    std::vector<SystemDestination> GetBookmarks() const;
    std::vector<SystemDestination> GetDiscovered() const;
    const SystemDestination* GetDestination(std::uint64_t destinationId) const;
    WarpPlan PlanWarp(const AstronomicalPosition& from, std::uint64_t destinationId, double shipVectorTopSpeedMetersPerSecond, double availableFuel) const;
private:
    AstronomicalScaleSystem scale_;
    std::unordered_map<std::uint64_t,SystemDestination> destinations_;
    std::vector<std::uint64_t> bookmarks_;
};

} // namespace subspace
