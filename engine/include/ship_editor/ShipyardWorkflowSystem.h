#pragma once

#include "ship_editor/ShipyardBuilderSystem.h"

#include <string>
#include <vector>

namespace subspace {

enum class ShipyardWorkflowStage {
    Start,
    Assemble,
    Attachments,
    Interior,
    Systems,
    Appearance,
    Validate,
    TestAndSave
};

struct ShipyardWorkflowStep {
    ShipyardWorkflowStage stage = ShipyardWorkflowStage::Start;
    std::string title;
    std::string instruction;
    std::string shortcut;
    bool complete = false;
    bool blocked = false;
};

class ShipyardWorkflowSystem {
public:
    static std::vector<ShipyardWorkflowStep> Build(const ShipyardBuilderRuntimeModel& model);
    static ShipyardWorkflowStage Recommended(const ShipyardBuilderRuntimeModel& model);
    static const char* StageName(ShipyardWorkflowStage stage);
};

} // namespace subspace
