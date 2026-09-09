#include "rpg/DeathLossSystem.h"

#include <algorithm>

namespace subspace {

DeathLossRules DeathLossRules::ForPreset(DeathLossPreset preset) {
    DeathLossRules rules;
    rules.preset = preset;

    switch (preset) {
        case DeathLossPreset::Relaxed:
            rules.insuredHullPayoutFraction = 0.90f;
            rules.uninsuredModuleRecoveryFraction = 0.60f;
            rules.carriedCargoRecoveryFraction = 0.90f;
            rules.creditRecoveryFeeFraction = 0.02f;
            rules.maximumRecoveryFee = 50000.0f;
            rules.emergencyDebtLimit = 50000.0f;
            break;

        case DeathLossPreset::Standard:
            break;

        case DeathLossPreset::Hardcore:
            rules.insuredHullPayoutFraction = 0.45f;
            rules.uninsuredModuleRecoveryFraction = 0.10f;
            rules.carriedCargoRecoveryFraction = 0.35f;
            rules.creditRecoveryFeeFraction = 0.10f;
            rules.maximumRecoveryFee = 1000000.0f;
            rules.emergencyDebtLimit = 250000.0f;
            rules.allowEmergencyRecoveryContract = false;
            break;
    }

    return rules;
}

DeathOutcome DeathLossSystem::Resolve(const DeathContext& context,
                                      const DeathLossRules& rules)
{
    DeathOutcome out;
    out.industryRetained = rules.infrastructureSurvivesPlayerDeath || !context.hasSurvivingIndustry;

    if (!context.shipDestroyed) {
        out.respawnAnchor = RespawnAnchorType::CurrentShipRescue;
        out.resultingCredits = context.liquidCredits;
        out.summary = "Player recovered aboard the surviving ship; no hull-loss settlement required.";
        return out;
    }

    out.insurancePayout = std::max(0.0f, context.insuredHullValue) *
                          std::clamp(rules.insuredHullPayoutFraction, 0.0f, 1.0f);
    out.recoverableCargoValue = std::max(0.0f, context.carriedCargoValue) *
                                std::clamp(rules.carriedCargoRecoveryFraction, 0.0f, 1.0f);
    out.recoverableModuleValue = std::max(0.0f, context.uninsuredModuleValue) *
                                 std::clamp(rules.uninsuredModuleRecoveryFraction, 0.0f, 1.0f);
    out.wreckSpawned = rules.spawnRecoverableWreck &&
                       (out.recoverableCargoValue > 0.0f || out.recoverableModuleValue > 0.0f);

    const float feeBase = std::max(0.0f, context.currentHullValue + context.carriedCargoValue);
    out.recoveryFee = std::min(std::max(0.0f, rules.maximumRecoveryFee),
                               feeBase * std::max(0.0f, rules.creditRecoveryFeeFraction));
    out.resultingCredits = context.liquidCredits + out.insurancePayout - out.recoveryFee;

    if (context.hasOwnedOrbitalHub) {
        out.respawnAnchor = RespawnAnchorType::OrbitalHub;
    } else if (context.hasFriendlyStation) {
        out.respawnAnchor = RespawnAnchorType::FriendlyStation;
    } else {
        out.respawnAnchor = RespawnAnchorType::EmergencyContract;
    }

    if (out.resultingCredits < 0.0f) {
        if (rules.allowEmergencyRecoveryContract &&
            out.resultingCredits >= -std::max(0.0f, rules.emergencyDebtLimit)) {
            out.lossCondition = LossCondition::RecoveryDebt;
            out.respawnAnchor = RespawnAnchorType::EmergencyContract;
            out.emergencyShipGranted = true;
        } else if (!context.hasOwnedOrbitalHub &&
                   !context.hasFriendlyStation &&
                   !context.hasSurvivingIndustry) {
            out.lossCondition = LossCondition::CampaignCollapse;
        } else {
            out.lossCondition = LossCondition::RecoveryDebt;
        }
    }

    if (out.lossCondition == LossCondition::CampaignCollapse) {
        out.summary = "No viable recovery anchor or surviving industrial foothold remains.";
    } else if (out.lossCondition == LossCondition::RecoveryDebt) {
        out.summary = "Loss creates a recovery-debt loop; surviving infrastructure and wreck recovery can fund the rebuild.";
    } else {
        out.summary = "Hull loss settled through insurance/recovery; surviving infrastructure remains strategically useful.";
    }

    return out;
}

} // namespace subspace
