#include "developer/build/DeveloperBuildPreflight.h"

#include <unordered_set>

namespace subspace {

void DeveloperBuildPreflightReport::AddIssue(std::string severity, std::string code, std::string message) {
    if (severity == "error") {
        ready = false;
    }
    issues.push_back({std::move(severity), std::move(code), std::move(message)});
}

bool DeveloperBuildPreflightReport::HasErrors() const {
    for (const auto& issue : issues) {
        if (issue.severity == "error") {
            return true;
        }
    }
    return false;
}

DeveloperBuildPreflightReport DeveloperBuildPreflight::EvaluateRootInventory(const std::vector<std::string>& rootEntries) const {
    DeveloperBuildPreflightReport report;
    const std::unordered_set<std::string> expected = {
        ".editorconfig", ".gitignore", "README.md", "LICENSE", "CHANGELOG.md", "CONTRIBUTING.md", "CREDITS.md",
        "MANIFEST.md", "CMakeLists.txt", "Makefile", "Dockerfile", "engine", "docs", "scripts", "tools", "content",
        "reference", "Assets", "assets", "GameData", "AvorionLike", "AvorionLike.sln"
    };

    for (const auto& entry : rootEntries) {
        if (expected.find(entry) == expected.end()) {
            report.AddIssue("warning", "root.unexpected", "Unexpected root entry should be moved, documented, or ignored: " + entry);
        }
    }

    bool hasEngine = false;
    for (const auto& entry : rootEntries) {
        if (entry == "engine") {
            hasEngine = true;
            break;
        }
    }
    if (!hasEngine) {
        report.AddIssue("error", "root.engine_missing", "Required engine/ root was not found.");
    }
    return report;
}

} // namespace subspace
