#pragma once

#include <string>
#include <vector>

namespace subspace {

enum class RogueliteLoopStage {
    HomeBuild,
    ShipFit,
    RailTravel,
    ExpeditionFlight,
    Extraction,
    RewardProcessing
};

struct RogueliteLoopStep {
    RogueliteLoopStage stage = RogueliteLoopStage::HomeBuild;
    std::string label;
    std::string playerDecision;
    std::string output;
};

const char* RogueliteLoopStageName(RogueliteLoopStage stage);
std::vector<RogueliteLoopStep> BuildDefaultRogueliteLoopSteps();
std::string RogueliteLoopSummary(const std::vector<RogueliteLoopStep>& steps);

} // namespace subspace
