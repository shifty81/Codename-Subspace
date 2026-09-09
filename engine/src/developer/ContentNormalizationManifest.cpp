#include "developer/assets/ContentNormalizationManifest.h"

namespace subspace {

bool ContentNormalizationManifest::HasDestructiveMoves() const {
    for (const auto& move : plannedMoves) {
        if (move.destructive) {
            return true;
        }
    }
    return false;
}

ContentNormalizationManifest ContentNormalizationManifestBuilder::BuildDefaultPlan() const {
    ContentNormalizationManifest manifest;
    manifest.plannedMoves.push_back({"Assets/", "content/assets/legacy-root-assets/", "Normalize uppercase asset root into canonical content/assets.", false});
    manifest.plannedMoves.push_back({"assets/", "content/assets/runtime/", "Normalize lowercase asset root into canonical content/assets.", false});
    manifest.plannedMoves.push_back({"GameData/", "content/data/", "Move gameplay JSON/data under canonical content/data.", false});
    manifest.plannedMoves.push_back({"AvorionLike/", "reference/csharp-prototype/", "Quarantine C# prototype as reference lane.", false});
    manifest.rootFilesToReview = {
        "121212.md.txt",
        "blenderaddon1.md",
        "AvorionLike.sln",
        "Makefile",
        "Dockerfile"
    };
    manifest.warnings.push_back("This manifest is a dry-run planning object. Do not delete old roots until redirects and build references are verified.");
    manifest.warnings.push_back("C# prototype quarantine should be handled separately from C++ runtime stabilization.");
    return manifest;
}

} // namespace subspace
