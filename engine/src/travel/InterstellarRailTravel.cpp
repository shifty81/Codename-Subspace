#include "travel/InterstellarRailTravel.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <random>
#include <sstream>

namespace subspace {

namespace {

float Clamp01(float value)
{
    return std::max(0.0f, std::min(1.0f, value));
}

float RiskMultiplier(RailTravelRouteRisk risk)
{
    switch (risk) {
    case RailTravelRouteRisk::Safe: return 0.75f;
    case RailTravelRouteRisk::Industrial: return 0.95f;
    case RailTravelRouteRisk::Hazardous: return 1.2f;
    case RailTravelRouteRisk::PirateWatched: return 1.35f;
    case RailTravelRouteRisk::Anomalous: return 1.65f;
    }
    return 1.0f;
}

RailTravelRouteOption MakeRoute(const std::string& id,
                                const std::string& name,
                                const std::string& origin,
                                const std::string& destination,
                                RailTravelRouteRisk risk,
                                int driveTier,
                                float duration,
                                float fuel,
                                float cargo,
                                float salvage,
                                float hazard,
                                std::vector<std::string> tags)
{
    RailTravelRouteOption route;
    route.routeId = id;
    route.displayName = name;
    route.originSystemId = origin;
    route.destinationSystemId = destination;
    route.risk = risk;
    route.requiredDriveTier = driveTier;
    route.recommendedDefense = risk == RailTravelRouteRisk::PirateWatched ? 4 : (risk == RailTravelRouteRisk::Anomalous ? 5 : 1);
    route.recommendedScanner = risk == RailTravelRouteRisk::Anomalous ? 5 : 2;
    route.recommendedCargoCapacity = static_cast<int>(std::ceil(cargo * 0.75f));
    route.distanceUnits = std::max(1.0f, duration / 12.0f);
    route.baseDurationSeconds = duration;
    route.baseFuelCost = fuel;
    route.cargoOpportunity = cargo;
    route.salvageOpportunity = salvage;
    route.hazardPressure = hazard;
    route.routeTags = std::move(tags);
    return route;
}

} // namespace

const char* RailTravelRouteRiskName(RailTravelRouteRisk risk)
{
    switch (risk) {
    case RailTravelRouteRisk::Safe: return "Safe";
    case RailTravelRouteRisk::Industrial: return "Industrial";
    case RailTravelRouteRisk::Hazardous: return "Hazardous";
    case RailTravelRouteRisk::PirateWatched: return "Pirate-Watched";
    case RailTravelRouteRisk::Anomalous: return "Anomalous";
    }
    return "Unknown";
}

const char* RailTravelStateName(RailTravelState state)
{
    switch (state) {
    case RailTravelState::Idle: return "Idle";
    case RailTravelState::Preparing: return "Preparing";
    case RailTravelState::Traveling: return "Traveling";
    case RailTravelState::ExtractionAvailable: return "Extraction Available";
    case RailTravelState::Completed: return "Completed";
    case RailTravelState::Aborted: return "Aborted";
    case RailTravelState::Failed: return "Failed";
    }
    return "Unknown";
}

std::vector<RailTravelRouteOption> CreateStarterRailTravelRoutes(const std::string& originSystemId,
                                                                 const std::string& destinationPrefix,
                                                                 std::uint32_t seed)
{
    std::mt19937 rng(seed == 0 ? 0x5A5A51u : seed);
    std::uniform_real_distribution<float> nudge(-0.12f, 0.16f);
    const float jitter = 1.0f + nudge(rng);

    return {
        MakeRoute("inner-safe-lane", "Inner Safe Lane", originSystemId, destinationPrefix + "-A",
                  RailTravelRouteRisk::Safe, 1, 34.0f * jitter, 18.0f * jitter, 10.0f, 2.0f, 0.10f,
                  {"safe", "fuel-light", "starter"}),
        MakeRoute("belt-harvest-lane", "Belt Harvest Lane", originSystemId, destinationPrefix + "-B",
                  RailTravelRouteRisk::Industrial, 1, 46.0f * jitter, 26.0f * jitter, 26.0f, 8.0f, 0.24f,
                  {"ore", "scrap", "industrial"}),
        MakeRoute("pirate-wreck-line", "Pirate Wreck Line", originSystemId, destinationPrefix + "-C",
                  RailTravelRouteRisk::PirateWatched, 2, 58.0f * jitter, 38.0f * jitter, 36.0f, 20.0f, 0.52f,
                  {"pirates", "salvage", "high-yield"}),
        MakeRoute("subspace-anomaly-arc", "Subspace Anomaly Arc", originSystemId, destinationPrefix + "-D",
                  RailTravelRouteRisk::Anomalous, 3, 74.0f * jitter, 52.0f * jitter, 42.0f, 38.0f, 0.78f,
                  {"anomaly", "rare", "deep"})
    };
}

RailTravelFitReport EvaluateRailTravelFit(const RailTravelRouteOption& route, const ShipRailTravelFit& fit)
{
    RailTravelFitReport report;
    report.projectedFuelRemaining = fit.fuelAvailable - route.baseFuelCost * RiskMultiplier(route.risk);

    if (!fit.hasRailDrive) {
        report.warnings.push_back("No rail/subspace drive installed.");
    }
    if (fit.driveTier < route.requiredDriveTier) {
        report.warnings.push_back("Drive tier below route requirement.");
    }
    if (report.projectedFuelRemaining < 0.0f) {
        report.warnings.push_back("Insufficient fuel for selected rail route.");
    }
    if (fit.cargoCapacity < route.recommendedCargoCapacity) {
        report.warnings.push_back("Cargo capacity below route opportunity; yield will be capped.");
    }
    if (fit.defenseRating < route.recommendedDefense) {
        report.warnings.push_back("Defense rating below recommended route pressure.");
    }
    if (fit.scannerRating < route.recommendedScanner) {
        report.warnings.push_back("Scanner rating below recommended route complexity.");
    }

    const float driveScore = Clamp01(static_cast<float>(fit.driveTier) / std::max(1, route.requiredDriveTier));
    const float fuelScore = Clamp01((report.projectedFuelRemaining + 8.0f) / std::max(8.0f, route.baseFuelCost));
    const float cargoScore = Clamp01(static_cast<float>(fit.cargoCapacity) / std::max(1, route.recommendedCargoCapacity));
    const float defenseScore = Clamp01(static_cast<float>(fit.defenseRating + 1) / std::max(1, route.recommendedDefense + 1));
    const float scannerScore = Clamp01(static_cast<float>(fit.scannerRating + 1) / std::max(1, route.recommendedScanner + 1));
    report.fitScore = (driveScore * 0.30f) + (fuelScore * 0.25f) + (cargoScore * 0.15f) +
                      (defenseScore * 0.15f) + (scannerScore * 0.15f);
    report.projectedCargoYield = route.cargoOpportunity * std::min(1.0f, cargoScore + 0.15f * scannerScore);
    report.projectedDamageRisk = Clamp01(route.hazardPressure * (1.0f - 0.45f * defenseScore - 0.20f * scannerScore));
    report.canLaunch = fit.hasRailDrive && fit.driveTier >= route.requiredDriveTier && report.projectedFuelRemaining >= 0.0f;
    return report;
}

RailTravelStateSnapshot StartRailTravel(const RailTravelRouteOption& route, const ShipRailTravelFit& fit)
{
    RailTravelStateSnapshot travel;
    travel.route = route;
    travel.fit = fit;
    const auto report = EvaluateRailTravelFit(route, fit);
    travel.fuelRemaining = fit.fuelAvailable;
    if (!report.canLaunch) {
        travel.state = RailTravelState::Failed;
        travel.eventLog.push_back("Launch rejected: " + RailTravelFitSummary(report));
        return travel;
    }
    travel.state = RailTravelState::Traveling;
    travel.eventLog.push_back("Rail travel started: " + RailTravelRouteSummary(route));
    if (!report.warnings.empty()) {
        travel.eventLog.push_back(report.warnings.front());
    }
    return travel;
}

void TickRailTravel(RailTravelStateSnapshot& travel, float dtSeconds)
{
    if (travel.state != RailTravelState::Traveling) {
        return;
    }
    const float dt = std::max(0.0f, dtSeconds);
    travel.elapsedSeconds += dt;
    travel.progress01 = Clamp01(travel.elapsedSeconds / std::max(1.0f, travel.route.baseDurationSeconds));

    const float burnPerSecond = travel.route.baseFuelCost * RiskMultiplier(travel.route.risk) /
                                std::max(1.0f, travel.route.baseDurationSeconds);
    travel.fuelRemaining = std::max(0.0f, travel.fuelRemaining - burnPerSecond * dt);

    const float cargoPerSecond = travel.route.cargoOpportunity / std::max(1.0f, travel.route.baseDurationSeconds);
    const float salvagePerSecond = travel.route.salvageOpportunity / std::max(1.0f, travel.route.baseDurationSeconds);
    travel.cargoCollected = std::min(static_cast<float>(travel.fit.cargoCapacity), travel.cargoCollected + cargoPerSecond * dt);
    travel.salvageCollected += salvagePerSecond * dt;
    travel.damageRiskAccumulated = Clamp01(travel.damageRiskAccumulated + travel.route.hazardPressure * dt * 0.0025f);

    if (travel.fuelRemaining <= 0.01f && travel.progress01 < 0.98f) {
        travel.state = RailTravelState::Failed;
        travel.eventLog.push_back("Rail travel failed: fuel exhausted before destination.");
        return;
    }

    if (travel.progress01 >= 1.0f) {
        travel.state = RailTravelState::Completed;
        travel.eventLog.push_back("Rail travel completed: destination reached.");
    }
}

void AbortRailTravel(RailTravelStateSnapshot& travel)
{
    if (travel.state == RailTravelState::Traveling || travel.state == RailTravelState::Preparing) {
        travel.state = RailTravelState::Aborted;
        travel.eventLog.push_back("Rail travel aborted; emergency return requested.");
    }
}

std::string RailTravelRouteSummary(const RailTravelRouteOption& route)
{
    std::ostringstream out;
    out << route.displayName << " -> " << route.destinationSystemId
        << " risk=" << RailTravelRouteRiskName(route.risk)
        << " fuel=" << std::fixed << std::setprecision(1) << route.baseFuelCost
        << " cargo=" << std::setprecision(0) << route.cargoOpportunity
        << " drive T" << route.requiredDriveTier;
    return out.str();
}

std::string RailTravelFitSummary(const RailTravelFitReport& report)
{
    std::ostringstream out;
    out << (report.canLaunch ? "launch-ready" : "not-ready")
        << " score=" << std::fixed << std::setprecision(0) << (report.fitScore * 100.0f) << "%"
        << " fuelRemaining=" << std::setprecision(1) << report.projectedFuelRemaining
        << " cargoYield=" << std::setprecision(0) << report.projectedCargoYield
        << " risk=" << std::setprecision(0) << (report.projectedDamageRisk * 100.0f) << "%";
    if (!report.warnings.empty()) {
        out << " warn=" << report.warnings.front();
    }
    return out.str();
}

std::string RailTravelStateSummary(const RailTravelStateSnapshot& travel)
{
    std::ostringstream out;
    out << RailTravelStateName(travel.state) << " " << travel.route.displayName
        << " progress=" << std::fixed << std::setprecision(0) << (travel.progress01 * 100.0f) << "%"
        << " fuel=" << std::setprecision(1) << travel.fuelRemaining
        << " cargo=" << std::setprecision(0) << travel.cargoCollected
        << " salvage=" << std::setprecision(0) << travel.salvageCollected;
    return out.str();
}

} // namespace subspace
