#pragma once

#include <string>
#include <vector>

namespace subspace {

class DeveloperCommandAutocomplete {
public:
    void SetCommands(std::vector<std::string> commands);
    std::vector<std::string> Suggest(const std::string& input, std::size_t limit = 10) const;
    std::string CompleteCommonPrefix(const std::string& input) const;

private:
    std::vector<std::string> _commands;
};

} // namespace subspace
