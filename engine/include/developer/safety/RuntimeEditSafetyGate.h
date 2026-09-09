#pragma once

#include "developer/RuntimeEditCommand.h"

#include <string>
#include <vector>

namespace subspace {

struct RuntimeEditSafetyContext {
    bool developerToolsEnabled = true;
    bool releaseBuild = false;
    bool sessionActive = true;
    bool allowAssetWrites = false;
    bool allowDestructiveEntityOps = false;
    bool allowBlueprintPromotion = false;
};

struct RuntimeEditSafetyDecision {
    bool allowed = true;
    std::string message;
    std::vector<std::string> warnings;
};

class RuntimeEditSafetyGate {
public:
    RuntimeEditSafetyDecision Evaluate(const RuntimeEditCommand& command, const RuntimeEditSafetyContext& context) const;
};

} // namespace subspace
