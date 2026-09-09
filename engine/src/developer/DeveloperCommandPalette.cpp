#include "developer/ui/DeveloperCommandPalette.h"

#include <algorithm>
#include <cctype>

namespace subspace {

void DeveloperCommandPalette::RebuildFromBridge(const DeveloperCommandBridge& bridge) {
    _entries.clear();
    for (const auto& command : bridge.GetSupportedCommands()) {
        _entries.push_back({command, command, CategoryForCommand(command)});
    }
    std::sort(_entries.begin(), _entries.end(), [](const auto& a, const auto& b) {
        if (a.category == b.category) {
            return a.command < b.command;
        }
        return a.category < b.category;
    });
}

std::vector<DeveloperCommandPaletteEntry> DeveloperCommandPalette::Search(const std::string& text, std::size_t maxResults) const {
    std::vector<DeveloperCommandPaletteEntry> result;
    for (const auto& entry : _entries) {
        if (text.empty() || ContainsCaseInsensitive(entry.command, text) || ContainsCaseInsensitive(entry.label, text) || ContainsCaseInsensitive(entry.category, text)) {
            result.push_back(entry);
            if (result.size() >= maxResults) {
                break;
            }
        }
    }
    return result;
}

void DeveloperCommandPalette::Clear() {
    _entries.clear();
}

std::string DeveloperCommandPalette::CategoryForCommand(const std::string& command) {
    const auto dot = command.find('.');
    if (dot == std::string::npos) {
        return "misc";
    }
    return command.substr(0, dot);
}

bool DeveloperCommandPalette::ContainsCaseInsensitive(const std::string& value, const std::string& query) {
    auto lower = [](std::string text) {
        std::transform(text.begin(), text.end(), text.begin(), [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
        return text;
    };
    return lower(value).find(lower(query)) != std::string::npos;
}

} // namespace subspace
