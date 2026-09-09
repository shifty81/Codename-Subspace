#include "roguelite/RogueliteDirector.h"

#include <algorithm>
#include <iomanip>
#include <sstream>

namespace subspace {
namespace {
ExpeditionRunModifier MakeModifier(const std::string& id, const std::string& name, float risk, float reward) {
    ExpeditionRunModifier modifier;
    modifier.id = id;
    modifier.displayName = name;
    modifier.riskMultiplier = risk;
    modifier.rewardMultiplier = reward;
    return modifier;
}
}

RogueliteDirectorState CreateRogueliteDirector(std::uint32_t seed) {
    RogueliteDirectorState director;
    director.save = CreateDefaultHomeSystemSave(seed);
    director.availableRuns = GenerateRunOffers(director.save, 3);
    return director;
}

std::vector<RogueliteRunOffer> GenerateRunOffers(const HomeSystemSaveSnapshot& save, int count) {
    std::vector<RogueliteRunOffer> offers;
    const int capacity = EstimateShipyardBuildCapacity(save.shipyard);
    const int offerCount = std::max(1, count);
    for (int i = 0; i < offerCount; ++i) {
        ExpeditionRunConfig config;
        config.runId = "expedition-" + std::to_string(i + 1);
        config.seed = save.home.seed + 1000u + static_cast<std::uint32_t>(i * 97);
        config.depth = std::max(1, capacity + i);
        config.targetMinutes = 18 + i * 8;
        config.objective = (i % 3 == 0) ? ExpeditionObjectiveType::MiningSurvey
                         : (i % 3 == 1) ? ExpeditionObjectiveType::SalvageRecovery
                                         : ExpeditionObjectiveType::AnomalyScan;
        if (i == 1) config.modifiers.push_back(MakeModifier("pirate-pressure", "Pirate Pressure", 1.35f, 1.25f));
        if (i == 2) config.modifiers.push_back(MakeModifier("unstable-star", "Unstable Star", 1.5f, 1.45f));

        RogueliteRunOffer offer;
        offer.config = config;
        offer.displayName = ExpeditionObjectiveTypeName(config.objective) + " Depth " + std::to_string(config.depth);
        offer.riskLabel = EstimateExpeditionRisk(config) >= 1.5f ? "High" : "Standard";
        offer.rewardLabel = EstimateExpeditionRewardMultiplier(config) >= 1.4f ? "Rich" : "Useful";
        offers.push_back(offer);
    }
    return offers;
}

void ApplyExtractedRunRewards(RogueliteDirectorState& director, const ExpeditionRunStateSnapshot& run) {
    ++director.completedRuns;
    director.totalExtractedCredits += run.pendingRewards.credits;
    director.save.shipyard.researchData += run.pendingRewards.researchData;
    director.save.shipyard.blueprintProgress += run.pendingRewards.blueprintProgress;
    for (const auto& cargo : run.pendingRewards.cargo) {
        AddHomeInventory(director.save.factory, cargo.commodity, cargo.units);
    }
    director.save.lastRun = run;
    director.availableRuns = GenerateRunOffers(director.save, 3);
}

void ApplyFailedRunConsequences(RogueliteDirectorState& director, const ExpeditionRunStateSnapshot& run) {
    ++director.failedRuns;
    director.save.shipyard.researchData += 1; // black-box analysis consolation progression
    director.save.lastRun = run;
    director.availableRuns = GenerateRunOffers(director.save, 3);
}

std::string RogueliteRunOfferSummary(const RogueliteRunOffer& offer) {
    std::ostringstream out;
    out << offer.displayName << " risk=" << offer.riskLabel << " reward=" << offer.rewardLabel;
    return out.str();
}

std::string RogueliteDirectorSummary(const RogueliteDirectorState& director) {
    std::ostringstream out;
    out << "RogueliteDirector completed=" << director.completedRuns
        << " failed=" << director.failedRuns
        << " credits=" << director.totalExtractedCredits
        << " offers=" << director.availableRuns.size()
        << " | " << HomeSystemSaveSummary(director.save);
    return out.str();
}

} // namespace subspace
