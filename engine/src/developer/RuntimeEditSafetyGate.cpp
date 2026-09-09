#include "developer/safety/RuntimeEditSafetyGate.h"

namespace subspace {

RuntimeEditSafetyDecision RuntimeEditSafetyGate::Evaluate(const RuntimeEditCommand& command, const RuntimeEditSafetyContext& context) const {
    RuntimeEditSafetyDecision decision;

    if (!context.developerToolsEnabled) {
        decision.allowed = false;
        decision.message = "Developer tools are disabled.";
        return decision;
    }

    if (context.releaseBuild) {
        decision.allowed = false;
        decision.message = "Runtime editing is blocked in release builds.";
        return decision;
    }

    if (!context.sessionActive && command.name.rfind("dev.mode", 0) != 0) {
        decision.allowed = false;
        decision.message = "No active developer runtime edit session.";
        return decision;
    }

    if ((command.name == "asset.write" || command.name == "asset.save" || command.name == "ship.promote_to_blueprint") &&
        !context.allowAssetWrites && !context.allowBlueprintPromotion) {
        decision.allowed = false;
        decision.message = "Persistent asset writes are disabled until explicitly enabled.";
        return decision;
    }

    if ((command.name == "entity.delete" || command.name == "entity.component.remove") && !context.allowDestructiveEntityOps) {
        decision.allowed = false;
        decision.message = "Destructive entity operations require explicit opt-in.";
        return decision;
    }

    if (command.name == "asset.reload.all") {
        decision.warnings.push_back("asset.reload.all can be expensive; prefer targeted reload while editing.");
    }

    decision.allowed = true;
    decision.message = "Runtime edit command allowed.";
    return decision;
}

} // namespace subspace
