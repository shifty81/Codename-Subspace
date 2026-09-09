#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace subspace {

enum class RailTravelRouteRisk {
    Safe,
    Industrial,
    Hazardous,
    PirateWatched,
    Anomalous
};

enum class RailTravelState {
    Idle,
    Preparing,
    Traveling,
    ExtractionAvailable,
    Completed,
    Aborted,
    Failed
};

struct RailTravelRouteOption {
    std::string routeId;
    std::string displayName;
    std::string originSystemId;
    std::string destinationSystemId;
    RailTravelRouteRisk risk = RailTravelRouteRisk::Safe;
    int requiredDriveTier = 1;
    int recommendedDefense = 0;
    int recommendedScanner = 0;
    int recommendedCargoCapacity = 0;
    float distanceUnits = 1.0f;
    float baseDurationSeconds = 30.0f;
    float baseFuelCost = 10.0f;
    float cargoOpportunity = 10.0f;
    float salvageOpportunity = 0.0f;
    float hazardPressure = 0.0f;
    std::vector<std::string> routeTags;
};

struct ShipRailTravelFit {
    int driveTier = 1;
    int defenseRating = 0;
    int scannerRating = 0;
    int cargoCapacity = 0;
    float fuelAvailable = 0.0f;
    float fuelCapacity = 0.0f;
    float mass = 1.0f;
    float thrustRating = 1.0f;
    bool hasRailDrive = true;
};

struct RailTravelFitReport {
    bool canLaunch = false;
    float fitScore = 0.0f;
    float projectedFuelRemaining = 0.0f;
    float projectedCargoYield = 0.0f;
    float projectedDamageRisk = 0.0f;
    std::vector<std::string> warnings;
};

struct RailTravelStateSnapshot {
    RailTravelState state = RailTravelState::Idle;
    RailTravelRouteOption route;
    ShipRailTravelFit fit;
    float elapsedSeconds = 0.0f;
    float progress01 = 0.0f;
    float fuelRemaining = 0.0f;
    float cargoCollected = 0.0f;
    float salvageCollected = 0.0f;
    float damageRiskAccumulated = 0.0f;
    std::vector<std::string> eventLog;
};

const char* RailTravelRouteRiskName(RailTravelRouteRisk risk);
const char* RailTravelStateName(RailTravelState state);

std::vector<RailTravelRouteOption> CreateStarterRailTravelRoutes(const std::string& originSystemId,
                                                                 const std::string& destinationPrefix,
                                                                 std::uint32_t seed);

RailTravelFitReport EvaluateRailTravelFit(const RailTravelRouteOption& route, const ShipRailTravelFit& fit);
RailTravelStateSnapshot StartRailTravel(const RailTravelRouteOption& route, const ShipRailTravelFit& fit);
void TickRailTravel(RailTravelStateSnapshot& travel, float dtSeconds);
void AbortRailTravel(RailTravelStateSnapshot& travel);

std::string RailTravelRouteSummary(const RailTravelRouteOption& route);
std::string RailTravelFitSummary(const RailTravelFitReport& report);
std::string RailTravelStateSummary(const RailTravelStateSnapshot& travel);

} // namespace subspace
