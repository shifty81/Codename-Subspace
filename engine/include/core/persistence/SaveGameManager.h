#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace subspace {

/// Serialized data for a single component.
struct ComponentData {
    std::string componentType;
    std::unordered_map<std::string, std::string> data;
};

/// Serialized data for a single entity.
struct EntityData {
    uint64_t entityId = 0;
    std::string entityName;
    bool isActive = true;
    std::vector<ComponentData> components;
};

/// Complete save game payload.
struct SaveGameData {
    std::string saveName;
    std::string saveTime;
    std::string version = "1.0.0";
    int galaxySeed = 0;
    std::vector<EntityData> entities;
    std::unordered_map<std::string, std::string> gameState;
};

/// Lightweight metadata returned when listing saves.
struct SaveGameInfo {
    std::string fileName;
    std::string saveName;
    std::string saveTime;
    std::string version;
};

/// Manages serialization, deserialization and enumeration of save game files.
class SaveGameManager {
public:
    /// Get the singleton instance.
    static SaveGameManager& Instance();

    /// Serialize save data to a file.
    bool SaveGame(const SaveGameData& data, const std::string& fileName);

    /// Deserialize save data from a file.
    bool LoadGame(const std::string& fileName, SaveGameData& outData);

    /// List all .save files in the save directory.
    std::vector<SaveGameInfo> ListSaveGames() const;

    /// Delete a save file.
    bool DeleteSave(const std::string& fileName);

    /// Quick-save using the fixed "quicksave" filename.
    bool QuickSave(const SaveGameData& data);

    /// Get the current save directory path.
    std::string GetSaveDirectory() const;

    /// Set a custom save directory path.
    void SetSaveDirectory(const std::string& dir);

private:
    SaveGameManager() = default;
    SaveGameManager(const SaveGameManager&) = delete;
    SaveGameManager& operator=(const SaveGameManager&) = delete;

    std::string _saveDirectory = "saves";
};

} // namespace subspace
