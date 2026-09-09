#pragma once

#include <string>

namespace subspace {

enum class DeathLossPreset { Relaxed, Standard, Hardcore };
enum class RespawnAnchorType { CurrentShipRescue, OrbitalHub, FriendlyStation, EmergencyContract };
enum class LossCondition { None, RecoveryDebt, CampaignCollapse };

struct DeathLossRules {
    DeathLossPreset preset = DeathLossPreset::Standard;
    float insuredHullPayoutFraction = 0.70f;
    float uninsuredModuleRecoveryFraction = 0.25f;
    float carriedCargoRecoveryFraction = 0.75f;
    float creditRecoveryFeeFraction = 0.05f;
    float maximumRecoveryFee = 250000.0f;
    float emergencyDebtLimit = 150000.0f;
    bool infrastructureSurvivesPlayerDeath = true;
    bool spawnRecoverableWreck = true;
    bool allowEmergencyRecoveryContract = true;

    static DeathLossRules ForPreset(DeathLossPreset preset);
};

struct DeathContext {
    float liquidCredits = 0.0f;
    float currentHullValue = 0.0f;
    float insuredHullValue = 0.0f;
    float uninsuredModuleValue = 0.0f;
    float carriedCargoValue = 0.0f;
    bool shipDestroyed = true;
    bool hasOwnedOrbitalHub = false;
    bool hasFriendlyStation = true;
    bool hasSurvivingIndustry = false;
};

struct DeathOutcome {
    RespawnAnchorType respawnAnchor = RespawnAnchorType::FriendlyStation;
    LossCondition lossCondition = LossCondition::None;
    float insurancePayout = 0.0f;
    float recoveryFee = 0.0f;
    float recoverableCargoValue = 0.0f;
    float recoverableModuleValue = 0.0f;
    float resultingCredits = 0.0f;
    bool wreckSpawned = false;
    bool industryRetained = true;
    bool emergencyShipGranted = false;
    std::string summary;
};

class DeathLossSystem {
public:
    static DeathOutcome Resolve(const DeathContext& context,
                                const DeathLossRules& rules = DeathLossRules::ForPreset(DeathLossPreset::Standard));
};

} // namespace subspace
