#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace subspace {

using RuntimeEditArgs = std::unordered_map<std::string, std::string>;

struct RuntimeEditCommand {
    std::string name;
    RuntimeEditArgs args;
    std::string source;
    std::string description;
    std::uint64_t sequence = 0;

    bool HasArg(const std::string& key) const;
    std::string GetArg(const std::string& key, const std::string& fallback = {}) const;
    void SetArg(const std::string& key, const std::string& value);
};

struct RuntimeEditResult {
    bool handled = false;
    bool success = false;
    bool undoable = false;
    std::string message;
    RuntimeEditCommand command;
    std::vector<std::string> warnings;

    static RuntimeEditResult Success(RuntimeEditCommand command, std::string message = {}, bool undoable = true);
    static RuntimeEditResult Failure(RuntimeEditCommand command, std::string message = {});
    static RuntimeEditResult Ignored(RuntimeEditCommand command, std::string message = {});
};

} // namespace subspace
