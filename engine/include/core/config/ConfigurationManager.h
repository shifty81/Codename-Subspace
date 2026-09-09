#pragma once

#include <string>

namespace subspace {

struct GraphicsSettings {
    int resolutionWidth = 1920;
    int resolutionHeight = 1080;
    bool vsync = true;
    int targetFPS = 60;
    int antiAliasingLevel = 4;
    float renderDistance = 10000.0f;
    bool shadowsEnabled = true;
    bool particlesEnabled = true;
    int textureQuality = 1;
};

struct AudioSettings {
    float masterVolume = 0.8f;
    float musicVolume = 0.6f;
    float sfxVolume = 0.7f;
    float voiceVolume = 1.0f;
    bool isMuted = false;
};

struct GameplaySettings {
    std::string playerName = "Player";
    int autoSaveIntervalSeconds = 300;
    int difficulty = 1;
    bool showTutorials = true;
    bool showHints = true;
    std::string language = "en";
};

struct NetworkSettings {
    std::string serverAddress = "127.0.0.1";
    int serverPort = 27015;
    int connectionTimeoutSeconds = 30;
    bool isMultiplayerEnabled = false;
    int maxPlayers = 50;
    int tickRate = 20;
};

struct DevelopmentSettings {
    bool debugMode = false;
    bool showProfiler = false;
    bool showMemoryTracker = false;
    int logLevelThreshold = 1;
    bool enableConsole = false;
    int galaxySeed = 12345;
};

struct GameConfiguration {
    GraphicsSettings graphics;
    AudioSettings audio;
    GameplaySettings gameplay;
    NetworkSettings network;
    DevelopmentSettings development;
};

/// Manages game configuration loading, saving, and validation (singleton).
class ConfigurationManager {
public:
    static ConfigurationManager& Instance();

    /// Load configuration from a key-value file at the given path.
    bool LoadConfiguration(const std::string& path);

    /// Save current configuration to a key-value file at the given path.
    bool SaveConfiguration(const std::string& path);

    /// Reset all settings to their default values.
    void ResetToDefaults();

    /// Validate that all configuration values are within acceptable ranges.
    bool ValidateConfiguration();

    const GameConfiguration& GetConfig() const;
    GameConfiguration& GetMutableConfig();

    const GraphicsSettings& GetGraphics() const;
    const AudioSettings& GetAudio() const;
    const GameplaySettings& GetGameplay() const;
    const NetworkSettings& GetNetwork() const;
    const DevelopmentSettings& GetDevelopment() const;

private:
    ConfigurationManager() = default;
    ConfigurationManager(const ConfigurationManager&) = delete;
    ConfigurationManager& operator=(const ConfigurationManager&) = delete;

    GameConfiguration _config;
};

} // namespace subspace
