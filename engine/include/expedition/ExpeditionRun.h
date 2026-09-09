#pragma once

#include "celestial/CelestialTypes.h"
#include "mining/MiningSalvageModel.h"

#include <cstdint>
#include <string>
#include <vector>

namespace subspace {

enum class ExpeditionRunState {
    Planning,
    Active,
    ExtractionAvailable,
    Extracted,
    Failed,
    Abandoned
};

enum class ExpeditionObjectiveType {
    MiningSurvey,
    SalvageRecovery,
    CombatPatrol,
    AnomalyScan,
    FuelHarvest,
    BlueprintHunt,
    FreeRoam
};

struct ExpeditionRunModifier {
    std::string id;
    std::string displayName;
    float riskMultiplier = 1.0f;
    float rewardMultiplier = 1.0f;
};

struct ExpeditionRunConfig {
    std::string runId = "dev-run";
    std::uint32_t seed = 0;
    ExpeditionObjectiveType objective = ExpeditionObjectiveType::FreeRoam;
    int depth = 1;
    int targetMinutes = 30;
    bool shipLossEnabled = true;
    bool cargoLossOnFailure = true;
    std::vector<ExpeditionRunModifier> modifiers;
};

struct ExpeditionRewardPackage {
    int credits = 0;
    int blueprintProgress = 0;
    int researchData = 0;
    std::vector<CargoYieldItem> cargo;
    std::vector<std::string> discoveries;
};

struct ExpeditionRunStateSnapshot {
    ExpeditionRunConfig config;
    ExpeditionRunState state = ExpeditionRunState::Planning;
    StarSystemDefinition generatedSystem;
    float elapsedSeconds = 0.0f;
    float extractionProgress = 0.0f;
    int hullDamageTaken = 0;
    ExpeditionRewardPackage pendingRewards;
};

ExpeditionRunStateSnapshot CreateExpeditionRun(const ExpeditionRunConfig& config);
void StartExpeditionRun(ExpeditionRunStateSnapshot& run);
void TickExpeditionRun(ExpeditionRunStateSnapshot& run, float deltaSeconds);
void AddExpeditionCargoReward(ExpeditionRunStateSnapshot& run, const CargoYieldItem& cargo);
void MarkExpeditionExtracted(ExpeditionRunStateSnapshot& run);
void MarkExpeditionFailed(ExpeditionRunStateSnapshot& run, const std::string& reason);
float EstimateExpeditionRisk(const ExpeditionRunConfig& config);
float EstimateExpeditionRewardMultiplier(const ExpeditionRunConfig& config);
std::string ExpeditionRunStateName(ExpeditionRunState state);
std::string ExpeditionObjectiveTypeName(ExpeditionObjectiveType objective);
std::string ExpeditionRunSummary(const ExpeditionRunStateSnapshot& run);

} // namespace subspace
