#include "core/persistence/SaveGameManager.h"

#include <filesystem>
#include <fstream>
#include <sstream>

namespace subspace {

static const std::string kSaveExtension = ".save";

SaveGameManager& SaveGameManager::Instance()
{
    static SaveGameManager instance;
    return instance;
}

bool SaveGameManager::SaveGame(const SaveGameData& data, const std::string& fileName)
{
    namespace fs = std::filesystem;
    fs::create_directories(_saveDirectory);

    std::string path = _saveDirectory + "/" + fileName + kSaveExtension;
    std::ofstream file(path);
    if (!file.is_open()) return false;

    // Header
    file << "[HEADER]\n";
    file << "saveName=" << data.saveName << "\n";
    file << "saveTime=" << data.saveTime << "\n";
    file << "version=" << data.version << "\n";
    file << "galaxySeed=" << data.galaxySeed << "\n";

    // Game state
    file << "[GAMESTATE]\n";
    for (const auto& [key, value] : data.gameState) {
        file << key << "=" << value << "\n";
    }

    // Entities
    for (const auto& entity : data.entities) {
        file << "[ENTITY]\n";
        file << "id=" << entity.entityId << "\n";
        file << "name=" << entity.entityName << "\n";
        file << "active=" << (entity.isActive ? "true" : "false") << "\n";

        for (const auto& comp : entity.components) {
            file << "[COMPONENT]\n";
            file << "type=" << comp.componentType << "\n";
            for (const auto& [key, value] : comp.data) {
                file << key << "=" << value << "\n";
            }
        }
    }

    return file.good();
}

bool SaveGameManager::LoadGame(const std::string& fileName, SaveGameData& outData)
{
    std::string path = _saveDirectory + "/" + fileName + kSaveExtension;
    std::ifstream file(path);
    if (!file.is_open()) return false;

    outData = SaveGameData{};

    enum class Section { None, Header, GameState, Entity, Component };
    Section section = Section::None;

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;

        // Detect section markers
        if (line == "[HEADER]") { section = Section::Header; continue; }
        if (line == "[GAMESTATE]") { section = Section::GameState; continue; }
        if (line == "[ENTITY]") {
            section = Section::Entity;
            outData.entities.emplace_back();
            continue;
        }
        if (line == "[COMPONENT]") {
            section = Section::Component;
            if (!outData.entities.empty()) {
                outData.entities.back().components.emplace_back();
            }
            continue;
        }

        // Parse key=value
        auto pos = line.find('=');
        if (pos == std::string::npos) continue;
        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);

        switch (section) {
        case Section::Header:
            if (key == "saveName") outData.saveName = value;
            else if (key == "saveTime") outData.saveTime = value;
            else if (key == "version") outData.version = value;
            else if (key == "galaxySeed") {
                try { outData.galaxySeed = std::stoi(value); }
                catch (...) { outData.galaxySeed = 0; }
            }
            break;
        case Section::GameState:
            outData.gameState[key] = value;
            break;
        case Section::Entity:
            if (outData.entities.empty()) break;
            if (key == "id") {
                try { outData.entities.back().entityId = std::stoull(value); }
                catch (...) { outData.entities.back().entityId = 0; }
            }
            else if (key == "name") outData.entities.back().entityName = value;
            else if (key == "active") outData.entities.back().isActive = (value == "true");
            break;
        case Section::Component:
            if (outData.entities.empty() || outData.entities.back().components.empty()) break;
            if (key == "type") outData.entities.back().components.back().componentType = value;
            else outData.entities.back().components.back().data[key] = value;
            break;
        default:
            break;
        }
    }

    return true;
}

std::vector<SaveGameInfo> SaveGameManager::ListSaveGames() const
{
    namespace fs = std::filesystem;
    std::vector<SaveGameInfo> result;

    if (!fs::exists(_saveDirectory)) return result;

    for (const auto& entry : fs::directory_iterator(_saveDirectory)) {
        if (!entry.is_regular_file()) continue;
        if (entry.path().extension() != kSaveExtension) continue;

        // Read just the header to populate info
        std::ifstream file(entry.path());
        if (!file.is_open()) continue;

        SaveGameInfo info;
        info.fileName = entry.path().stem().string();

        std::string line;
        bool inHeader = false;
        while (std::getline(file, line)) {
            if (line == "[HEADER]") { inHeader = true; continue; }
            if (!line.empty() && line[0] == '[') break;
            if (!inHeader) continue;

            auto pos = line.find('=');
            if (pos == std::string::npos) continue;
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);

            if (key == "saveName") info.saveName = value;
            else if (key == "saveTime") info.saveTime = value;
            else if (key == "version") info.version = value;
        }

        result.push_back(std::move(info));
    }

    return result;
}

bool SaveGameManager::DeleteSave(const std::string& fileName)
{
    namespace fs = std::filesystem;
    std::string path = _saveDirectory + "/" + fileName + kSaveExtension;
    return fs::remove(path);
}

bool SaveGameManager::QuickSave(const SaveGameData& data)
{
    return SaveGame(data, "quicksave");
}

std::string SaveGameManager::GetSaveDirectory() const
{
    return _saveDirectory;
}

void SaveGameManager::SetSaveDirectory(const std::string& dir)
{
    _saveDirectory = dir;
}

} // namespace subspace
