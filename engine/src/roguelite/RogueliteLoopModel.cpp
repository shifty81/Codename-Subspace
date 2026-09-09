#include "roguelite/RogueliteLoopModel.h"

#include <sstream>

namespace subspace {

const char* RogueliteLoopStageName(RogueliteLoopStage stage) {
    switch (stage) {
        case RogueliteLoopStage::HomeBuild: return "HomeBuild";
        case RogueliteLoopStage::ShipFit: return "ShipFit";
        case RogueliteLoopStage::RailTravel: return "RailTravel";
        case RogueliteLoopStage::ExpeditionFlight: return "ExpeditionFlight";
        case RogueliteLoopStage::Extraction: return "Extraction";
        case RogueliteLoopStage::RewardProcessing: return "RewardProcessing";
        default: return "Unknown";
    }
}

std::vector<RogueliteLoopStep> BuildDefaultRogueliteLoopSteps() {
    return {
        {RogueliteLoopStage::HomeBuild, "Home automation", "What should the safe home system produce next?", "parts, fuel, power, research"},
        {RogueliteLoopStage::ShipFit, "Builder bay", "Which hot-swappable parts fit this mission?", "ship stats and route eligibility"},
        {RogueliteLoopStage::RailTravel, "Interstellar route", "Which on-rails path balances reward and risk?", "fuel burn, route events, cargo"},
        {RogueliteLoopStage::ExpeditionFlight, "Manual sector play", "Mine, salvage, fight, scan, or retreat?", "run rewards and damage"},
        {RogueliteLoopStage::Extraction, "Extraction", "Leave now or push deeper?", "success/failure package"},
        {RogueliteLoopStage::RewardProcessing, "Home processing", "Spend, refine, research, automate", "permanent progression"}
    };
}

std::string RogueliteLoopSummary(const std::vector<RogueliteLoopStep>& steps) {
    std::ostringstream ss;
    ss << "loop steps=" << steps.size();
    if (!steps.empty()) {
        ss << " first=" << RogueliteLoopStageName(steps.front().stage)
           << " last=" << RogueliteLoopStageName(steps.back().stage);
    }
    return ss.str();
}

} // namespace subspace
