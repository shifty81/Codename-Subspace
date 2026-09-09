#pragma once

#include "developer/RuntimeEditCommand.h"

#include <functional>
#include <string>
#include <vector>

namespace subspace {

struct BlueprintPromotionRequest {
    std::string runtimeId;
    std::string sourceBlueprintPath;
    std::string outputBlueprintPath;
    std::vector<RuntimeEditCommand> edits;
    bool overwrite = false;
};

struct BlueprintPromotionReport {
    bool success = false;
    bool written = false;
    std::string outputBlueprintPath;
    std::string message;
    std::vector<std::string> warnings;
};

class BlueprintPromotionPipeline {
public:
    using PromoteCallback = std::function<BlueprintPromotionReport(const BlueprintPromotionRequest&)>;

    void SetCallback(PromoteCallback callback);
    BlueprintPromotionReport Promote(const BlueprintPromotionRequest& request) const;
    static BlueprintPromotionReport ValidateOnly(const BlueprintPromotionRequest& request);

private:
    PromoteCallback _callback;
};

} // namespace subspace
