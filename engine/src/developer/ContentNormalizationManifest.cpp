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
    manifest.plannedMoves.push_back({"Assets/", "content/assets/legacy-root-assets/", "Normalize an uppercase legacy asset root only when present and provenance-audited.", false});
    manifest.plannedMoves.push_back({"assets/", "content/assets/runtime/", "Normalize a lowercase legacy asset root only when present and provenance-audited.", false});
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
