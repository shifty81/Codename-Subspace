#include "core/persistence/RuntimeSaveGame.h"

#include <sstream>

namespace subspace {

std::string SerializeRuntimeSaveGameSnapshot(const RuntimeSaveGameSnapshot& snapshot)
{
    std::ostringstream out;
    out << "saveId=" << snapshot.saveId << "\n";
    out << "sectorId=" << snapshot.sectorId << "\n";
    out << "sectorSeed=" << snapshot.sectorSeed << "\n";
    out << "player=" << snapshot.playerX << "," << snapshot.playerY << "," << snapshot.playerAngle << "\n";
    out << "hull=" << snapshot.hull << "\n";
    out << "credits=" << snapshot.credits << "\n";
    for (const auto& item : snapshot.cargo) {
        out << "cargo=" << item.commodity << "," << item.units << "," << item.creditValue << "\n";
    }
    return out.str();
}

RuntimeSaveGameSnapshot DeserializeRuntimeSaveGameSnapshot(const std::string& text)
{
    RuntimeSaveGameSnapshot snapshot;
    std::istringstream in(text);
    std::string line;
    while (std::getline(in, line)) {
        const auto eq = line.find('=');
        if (eq == std::string::npos) {
            continue;
        }
        const std::string key = line.substr(0, eq);
        const std::string value = line.substr(eq + 1);
        if (key == "saveId") snapshot.saveId = value;
        else if (key == "sectorId") snapshot.sectorId = value;
        else if (key == "sectorSeed") snapshot.sectorSeed = static_cast<std::uint32_t>(std::stoul(value));
        else if (key == "hull") snapshot.hull = std::stof(value);
        else if (key == "credits") snapshot.credits = std::stoi(value);
        else if (key == "cargo") {
            std::istringstream itemStream(value);
            std::string commodity;
            std::string units;
            std::string credits;
            if (std::getline(itemStream, commodity, ',') && std::getline(itemStream, units, ',') && std::getline(itemStream, credits, ',')) {
                snapshot.cargo.push_back({commodity, std::stoi(units), std::stoi(credits)});
            }
        }
        else if (key == "player") {
            std::istringstream pos(value);
            std::string x, y, a;
            if (std::getline(pos, x, ',') && std::getline(pos, y, ',') && std::getline(pos, a, ',')) {
                snapshot.playerX = std::stof(x);
                snapshot.playerY = std::stof(y);
                snapshot.playerAngle = std::stof(a);
            }
        }
    }
    return snapshot;
}

std::string RuntimeSaveGameSummary(const RuntimeSaveGameSnapshot& snapshot)
{
    std::ostringstream out;
    out << snapshot.saveId << " sector=" << snapshot.sectorId << " credits=" << snapshot.credits << " cargo=" << snapshot.cargo.size();
    return out.str();
}

} // namespace subspace
