#include "developer/ui/DeveloperCommandAutocomplete.h"

#include <algorithm>

namespace subspace {

void DeveloperCommandAutocomplete::SetCommands(std::vector<std::string> commands)
{
    std::sort(commands.begin(), commands.end());
    commands.erase(std::unique(commands.begin(), commands.end()), commands.end());
    _commands = std::move(commands);
}

std::vector<std::string> DeveloperCommandAutocomplete::Suggest(const std::string& input, std::size_t limit) const
{
    std::vector<std::string> result;
    for (const auto& command : _commands) {
        if (command.rfind(input, 0) == 0) {
            result.push_back(command);
            if (result.size() >= limit) {
                break;
            }
        }
    }
    return result;
}

std::string DeveloperCommandAutocomplete::CompleteCommonPrefix(const std::string& input) const
{
    auto suggestions = Suggest(input, _commands.size());
    if (suggestions.empty()) {
        return input;
    }
    std::string prefix = suggestions.front();
    for (const auto& suggestion : suggestions) {
        std::size_t i = 0;
        while (i < prefix.size() && i < suggestion.size() && prefix[i] == suggestion[i]) {
            ++i;
        }
        prefix.resize(i);
    }
    return prefix.size() > input.size() ? prefix : input;
}

} // namespace subspace
