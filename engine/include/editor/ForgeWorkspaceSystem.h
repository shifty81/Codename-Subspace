#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace subspace {

struct ForgeWorkspaceCommand {
    std::string key;
    std::string label;
    std::string category;
    bool mutates = false;
    bool requiresConfirmation = false;
    std::string executable;
    std::vector<std::string> arguments;
};

struct ForgeWorkspaceSnapshot {
    bool valid = false;
    std::filesystem::path projectRoot;
    std::filesystem::path controlPath;
    std::string schema;
    std::string projectId;
    std::string projectName;
    std::string version;
    std::string build;
    std::string status;
    std::vector<ForgeWorkspaceCommand> commands;
};

/// Native editor thin client for the project-owned Forge/PCC contract.
///
/// This deliberately does not embed ForgePY. It consumes forge.project.v1 and
/// preserves each command's declared executable. This prevents the historical
/// failure where a Python host attempted to execute SubspaceTools.ps1 itself.
class ForgeWorkspaceSystem {
public:
    static ForgeWorkspaceSnapshot Discover(const std::filesystem::path& start);
    static ForgeWorkspaceSnapshot Load(const std::filesystem::path& projectControlPath);
    static const ForgeWorkspaceCommand* Find(const ForgeWorkspaceSnapshot& snapshot,
                                             const std::string& key);
    static bool InterpreterMatchesScript(const ForgeWorkspaceCommand& command,
                                         std::string* error = nullptr);
    static std::string CommandPreview(const ForgeWorkspaceCommand& command);
};

} // namespace subspace
