#pragma once

#include "developer/DeveloperCommandBridge.h"

#include <string>
#include <vector>

namespace subspace {

struct DeveloperCommandPaletteEntry {
    std::string command;
    std::string label;
    std::string category;
};

class DeveloperCommandPalette {
public:
    void RebuildFromBridge(const DeveloperCommandBridge& bridge);
    std::vector<DeveloperCommandPaletteEntry> Search(const std::string& text, std::size_t maxResults = 20) const;
    const std::vector<DeveloperCommandPaletteEntry>& GetEntries() const { return _entries; }
    void Clear();

private:
    static std::string CategoryForCommand(const std::string& command);
    static bool ContainsCaseInsensitive(const std::string& value, const std::string& query);

    std::vector<DeveloperCommandPaletteEntry> _entries;
};

} // namespace subspace
