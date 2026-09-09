#include "expedition/ExpeditionRewardResolver.h"

#include <algorithm>
#include <sstream>

namespace subspace {

ExpeditionRewardPackage ResolveSuccessfulExtraction(const ExpeditionRunStateSnapshot& run,
                                                    const ExpeditionContract* contract) {
    ExpeditionRewardPackage rewards = run.pendingRewards;
    const float multiplier = EstimateExpeditionRewardMultiplier(run.config);
    rewards.credits = static_cast<int>(static_cast<float>(std::max(100, rewards.credits)) * multiplier);
    rewards.researchData += std::max(1, run.config.depth / 2);
    rewards.blueprintProgress += (run.config.objective == ExpeditionObjectiveType::BlueprintHunt) ? 3 : 1;
    if (contract) {
        rewards.credits += contract->completionCredits;
    }
    return rewards;
}

ExpeditionRewardPackage ResolveFailedExtraction(const ExpeditionRunStateSnapshot& run,
                                                const ExpeditionResolutionRules& rules) {
    ExpeditionRewardPackage rewards;
    rewards.researchData = std::max(1, static_cast<int>(static_cast<float>(run.pendingRewards.researchData) * rules.blackBoxResearchFraction));
    rewards.blueprintProgress = run.pendingRewards.blueprintProgress > 0 ? 1 : 0;
    if (rules.keepDiscoveriesOnFailure) {
        rewards.discoveries = run.pendingRewards.discoveries;
    }
    const float keptCargo = std::max(0.0f, 1.0f - rules.cargoLossOnFailure);
    for (const auto& cargo : run.pendingRewards.cargo) {
        const int kept = static_cast<int>(static_cast<float>(cargo.units) * keptCargo);
        if (kept > 0) {
            rewards.cargo.push_back({cargo.commodity, kept, cargo.creditValue});
        }
    }
    return rewards;
}

std::string ExpeditionRewardPackageSummary(const ExpeditionRewardPackage& rewards) {
    std::ostringstream stream;
    stream << "Rewards credits=" << rewards.credits << " research=" << rewards.researchData
           << " blueprint=" << rewards.blueprintProgress << " cargoStacks=" << rewards.cargo.size()
           << " discoveries=" << rewards.discoveries.size();
    return stream.str();
}

} // namespace subspace
