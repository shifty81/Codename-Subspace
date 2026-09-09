#include "developer/DeveloperSessionPersistence.h"

#include <sstream>

namespace subspace {

std::string DeveloperSessionPersistence::Serialize(const DeveloperSessionSnapshot& snapshot)
{
    std::ostringstream out;
    out << "label=" << snapshot.label << "\n";
    out << "mode=" << snapshot.mode << "\n";
    out << "dirtyEditCount=" << snapshot.dirtyEditCount << "\n";
    for (const auto& asset : snapshot.watchedAssets) {
        out << "watched=" << asset << "\n";
    }
    for (const auto& command : snapshot.recentCommands) {
        out << "command=" << command << "\n";
    }
    return out.str();
}

DeveloperSessionSnapshot DeveloperSessionPersistence::ParseLoose(const std::string& text)
{
    DeveloperSessionSnapshot snapshot;
    std::istringstream in(text);
    std::string line;
    while (std::getline(in, line)) {
        auto pos = line.find('=');
        if (pos == std::string::npos) {
            continue;
        }
        auto key = line.substr(0, pos);
        auto value = line.substr(pos + 1);
        if (key == "label") snapshot.label = value;
        else if (key == "mode") snapshot.mode = value;
        else if (key == "watched") snapshot.watchedAssets.push_back(value);
        else if (key == "command") snapshot.recentCommands.push_back(value);
        else if (key == "dirtyEditCount") snapshot.dirtyEditCount = static_cast<std::size_t>(std::stoull(value.empty() ? "0" : value));
    }
    return snapshot;
}

} // namespace subspace
