#pragma once

#include <string>
#include <vector>

namespace subspace {

struct DeveloperSessionSnapshot {
    std::string label;
    std::string mode;
    std::size_t dirtyEditCount = 0;
    std::vector<std::string> watchedAssets;
    std::vector<std::string> recentCommands;
};

class DeveloperSessionPersistence {
public:
    static std::string Serialize(const DeveloperSessionSnapshot& snapshot);
    static DeveloperSessionSnapshot ParseLoose(const std::string& text);
};

} // namespace subspace
