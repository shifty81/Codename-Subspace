#pragma once

#include "expedition/ExpeditionContractBoard.h"

namespace subspace {

struct ExpeditionResolutionRules {
    float cargoLossOnFailure = 0.75f;
    float blackBoxResearchFraction = 0.25f;
    bool keepDiscoveriesOnFailure = true;
};

ExpeditionRewardPackage ResolveSuccessfulExtraction(const ExpeditionRunStateSnapshot& run,
                                                    const ExpeditionContract* contract = nullptr);
ExpeditionRewardPackage ResolveFailedExtraction(const ExpeditionRunStateSnapshot& run,
                                                const ExpeditionResolutionRules& rules = ExpeditionResolutionRules{});
std::string ExpeditionRewardPackageSummary(const ExpeditionRewardPackage& rewards);

} // namespace subspace
