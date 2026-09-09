#include "expedition/ExpeditionRun.h"

#include "celestial/CelestialSystemGenerator.h"

#include <algorithm>
#include <sstream>

namespace subspace {

ExpeditionRunStateSnapshot CreateExpeditionRun(const ExpeditionRunConfig& config) {
    ExpeditionRunStateSnapshot run;
    run.config = config;
    run.state = ExpeditionRunState::Planning;

    CelestialSystemGeneratorOptions options;
    options.minPlanets = std::max(3, 2 + config.depth);
    options.maxPlanets = std::max(options.minPlanets, 4 + config.depth);
    options.includeAsteroidBelt = true;
    options.allowBlackHolePrimary = config.depth >= 5;
    CelestialSystemGenerator generator(options);
    const std::uint32_t seed = config.seed == 0 ? 0xE7700001u + static_cast<std::uint32_t>(config.depth) : config.seed;
    run.generatedSystem = generator.GenerateSystem(config.runId.empty() ? "expedition" : config.runId, seed);
    run.generatedSystem.tags.push_back("expedition");
    run.generatedSystem.tags.push_back("roguelite-run");
    return run;
}

void StartExpeditionRun(ExpeditionRunStateSnapshot& run) {
    if (run.state == ExpeditionRunState::Planning) {
        run.state = ExpeditionRunState::Active;
        run.elapsedSeconds = 0.0f;
        run.extractionProgress = 0.0f;
    }
}

void TickExpeditionRun(ExpeditionRunStateSnapshot& run, float deltaSeconds) {
    if (run.state != ExpeditionRunState::Active && run.state != ExpeditionRunState::ExtractionAvailable) return;
    run.elapsedSeconds += std::max(0.0f, deltaSeconds);
    const float targetSeconds = static_cast<float>(std::max(1, run.config.targetMinutes)) * 60.0f;
    if (run.elapsedSeconds >= targetSeconds * 0.5f && run.state == ExpeditionRunState::Active) {
        run.state = ExpeditionRunState::ExtractionAvailable;
    }
}

void AddExpeditionCargoReward(ExpeditionRunStateSnapshot& run, const CargoYieldItem& cargo) {
    if (cargo.units <= 0 || cargo.commodity.empty()) return;
    run.pendingRewards.cargo.push_back(cargo);
    run.pendingRewards.credits += cargo.creditValue * cargo.units;
}

void MarkExpeditionExtracted(ExpeditionRunStateSnapshot& run) {
    run.state = ExpeditionRunState::Extracted;
    run.extractionProgress = 1.0f;
}

void MarkExpeditionFailed(ExpeditionRunStateSnapshot& run, const std::string& reason) {
    run.state = ExpeditionRunState::Failed;
    run.pendingRewards.discoveries.push_back(reason.empty() ? "failure:black-box-recovered" : "failure:" + reason);
    if (run.config.cargoLossOnFailure) {
        run.pendingRewards.cargo.clear();
        run.pendingRewards.credits = 0;
    }
}

float EstimateExpeditionRisk(const ExpeditionRunConfig& config) {
    float risk = 1.0f + static_cast<float>(std::max(0, config.depth - 1)) * 0.25f;
    for (const auto& modifier : config.modifiers) {
        risk *= std::max(0.1f, modifier.riskMultiplier);
    }
    return risk;
}

float EstimateExpeditionRewardMultiplier(const ExpeditionRunConfig& config) {
    float reward = 1.0f + static_cast<float>(std::max(0, config.depth - 1)) * 0.15f;
    for (const auto& modifier : config.modifiers) {
        reward *= std::max(0.1f, modifier.rewardMultiplier);
    }
    return reward;
}

std::string ExpeditionRunStateName(ExpeditionRunState state) {
    switch (state) {
        case ExpeditionRunState::Planning: return "Planning";
        case ExpeditionRunState::Active: return "Active";
        case ExpeditionRunState::ExtractionAvailable: return "ExtractionAvailable";
        case ExpeditionRunState::Extracted: return "Extracted";
        case ExpeditionRunState::Failed: return "Failed";
        case ExpeditionRunState::Abandoned: return "Abandoned";
    }
    return "Unknown";
}

std::string ExpeditionObjectiveTypeName(ExpeditionObjectiveType objective) {
    switch (objective) {
        case ExpeditionObjectiveType::MiningSurvey: return "MiningSurvey";
        case ExpeditionObjectiveType::SalvageRecovery: return "SalvageRecovery";
        case ExpeditionObjectiveType::CombatPatrol: return "CombatPatrol";
        case ExpeditionObjectiveType::AnomalyScan: return "AnomalyScan";
        case ExpeditionObjectiveType::FuelHarvest: return "FuelHarvest";
        case ExpeditionObjectiveType::BlueprintHunt: return "BlueprintHunt";
        case ExpeditionObjectiveType::FreeRoam: return "FreeRoam";
    }
    return "Unknown";
}

std::string ExpeditionRunSummary(const ExpeditionRunStateSnapshot& run) {
    std::ostringstream out;
    out << run.config.runId
        << " state=" << ExpeditionRunStateName(run.state)
        << " objective=" << ExpeditionObjectiveTypeName(run.config.objective)
        << " depth=" << run.config.depth
        << " risk=" << EstimateExpeditionRisk(run.config)
        << " rewards=" << run.pendingRewards.credits;
    return out.str();
}

} // namespace subspace
