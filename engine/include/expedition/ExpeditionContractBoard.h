#pragma once

#include "expedition/ExpeditionRun.h"
#include "home/HomeShipyardProgression.h"

#include <string>
#include <vector>

namespace subspace {

struct ExpeditionContract {
    std::string id;
    std::string displayName;
    ExpeditionRunConfig config;
    int advanceCredits = 0;
    int completionCredits = 0;
    std::vector<HomeInventoryStack> expectedCargo;
    bool legal = true;
};

struct ExpeditionContractBoardState {
    std::vector<ExpeditionContract> contracts;
    std::vector<std::string> completedContractIds;
    int boardRefreshes = 0;
};

ExpeditionContractBoardState GenerateExpeditionContractBoard(std::uint32_t seed,
                                                             const ShipyardProgressionState& shipyard,
                                                             int completedRuns);
std::string ExpeditionContractSummary(const ExpeditionContract& contract);
std::string ExpeditionContractBoardSummary(const ExpeditionContractBoardState& board);

} // namespace subspace
