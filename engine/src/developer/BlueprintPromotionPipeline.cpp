#include "developer/promotion/BlueprintPromotionPipeline.h"

namespace subspace {

void BlueprintPromotionPipeline::SetCallback(PromoteCallback callback)
{
    _callback = std::move(callback);
}

BlueprintPromotionReport BlueprintPromotionPipeline::Promote(const BlueprintPromotionRequest& request) const
{
    auto validation = ValidateOnly(request);
    if (!validation.success) {
        return validation;
    }
    if (!_callback) {
        validation.message = "Blueprint promotion validated, but no writer callback is registered.";
        return validation;
    }
    return _callback(request);
}

BlueprintPromotionReport BlueprintPromotionPipeline::ValidateOnly(const BlueprintPromotionRequest& request)
{
    BlueprintPromotionReport report;
    report.outputBlueprintPath = request.outputBlueprintPath;
    if (request.runtimeId.empty()) {
        report.message = "Promotion failed: runtimeId is required.";
        return report;
    }
    if (request.outputBlueprintPath.empty()) {
        report.message = "Promotion failed: outputBlueprintPath is required.";
        return report;
    }
    if (!request.overwrite && request.outputBlueprintPath == request.sourceBlueprintPath) {
        report.message = "Promotion refused: output path equals source path and overwrite=false.";
        return report;
    }
    report.success = true;
    report.message = "Blueprint promotion request validated.";
    return report;
}

} // namespace subspace
