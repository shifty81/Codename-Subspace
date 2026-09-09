#include "expedition/ExpeditionContractBoard.h"

#include <sstream>

namespace subspace {
namespace {
ExpeditionContract MakeContract(const std::string& id,
                                const std::string& name,
                                ExpeditionObjectiveType objective,
                                std::uint32_t seed,
                                int depth,
                                int advance,
                                int complete) {
    ExpeditionContract contract;
    contract.id = id;
    contract.displayName = name;
    contract.config.runId = id;
    contract.config.seed = seed;
    contract.config.objective = objective;
    contract.config.depth = depth;
    contract.config.targetMinutes = 18 + depth * 4;
    contract.advanceCredits = advance;
    contract.completionCredits = complete;
    return contract;
}
}

ExpeditionContractBoardState GenerateExpeditionContractBoard(std::uint32_t seed,
                                                             const ShipyardProgressionState& shipyard,
                                                             int completedRuns) {
    ExpeditionContractBoardState board;
    const int depthBase = std::max(1, shipyard.shipyardLevel + completedRuns / 3);
    board.contracts.push_back(MakeContract("contract-mining-" + std::to_string(seed), "Survey mineral belt", ExpeditionObjectiveType::MiningSurvey, seed + 11u, depthBase, 50, 250 + depthBase * 75));
    board.contracts.push_back(MakeContract("contract-salvage-" + std::to_string(seed), "Recover derelict cargo", ExpeditionObjectiveType::SalvageRecovery, seed + 29u, depthBase + 1, 75, 350 + depthBase * 90));
    board.contracts.push_back(MakeContract("contract-anomaly-" + std::to_string(seed), "Scan subspace anomaly", ExpeditionObjectiveType::AnomalyScan, seed + 47u, depthBase + 2, 100, 500 + depthBase * 120));
    if (shipyard.researchData >= 5) {
        board.contracts.push_back(MakeContract("contract-blueprint-" + std::to_string(seed), "Blueprint hunt", ExpeditionObjectiveType::BlueprintHunt, seed + 83u, depthBase + 2, 150, 650 + depthBase * 150));
    }
    return board;
}

std::string ExpeditionContractSummary(const ExpeditionContract& contract) {
    std::ostringstream stream;
    stream << contract.displayName << " objective=" << ExpeditionObjectiveTypeName(contract.config.objective)
           << " depth=" << contract.config.depth << " advance=" << contract.advanceCredits
           << " complete=" << contract.completionCredits;
    return stream.str();
}

std::string ExpeditionContractBoardSummary(const ExpeditionContractBoardState& board) {
    std::ostringstream stream;
    stream << "ContractBoard offers=" << board.contracts.size() << " completed=" << board.completedContractIds.size()
           << " refreshes=" << board.boardRefreshes;
    return stream.str();
}

} // namespace subspace
