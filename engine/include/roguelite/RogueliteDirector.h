#pragma once

#include "core/persistence/HomeSystemSaveGame.h"

#include <string>
#include <vector>

namespace subspace {

struct RogueliteRunOffer {
    ExpeditionRunConfig config;
    std::string displayName;
    std::string riskLabel;
    std::string rewardLabel;
};

struct RogueliteDirectorState {
    HomeSystemSaveSnapshot save;
    std::vector<RogueliteRunOffer> availableRuns;
    int completedRuns = 0;
    int failedRuns = 0;
    int totalExtractedCredits = 0;
};

RogueliteDirectorState CreateRogueliteDirector(std::uint32_t seed = 0x51B5ACEu);
std::vector<RogueliteRunOffer> GenerateRunOffers(const HomeSystemSaveSnapshot& save, int count);
void ApplyExtractedRunRewards(RogueliteDirectorState& director, const ExpeditionRunStateSnapshot& run);
void ApplyFailedRunConsequences(RogueliteDirectorState& director, const ExpeditionRunStateSnapshot& run);
std::string RogueliteRunOfferSummary(const RogueliteRunOffer& offer);
std::string RogueliteDirectorSummary(const RogueliteDirectorState& director);

} // namespace subspace
