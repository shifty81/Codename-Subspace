#include "core/config/ConfigurationManager.h"

#include <algorithm>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace subspace {

ConfigurationManager& ConfigurationManager::Instance()
{
    static ConfigurationManager instance;
    return instance;
}

void ConfigurationManager::ResetToDefaults()
{
    _config = GameConfiguration{};
}

bool ConfigurationManager::ValidateConfiguration()
{
    bool valid = true;

    // Graphics validation
    auto& g = _config.graphics;
    if (g.resolutionWidth < 640 || g.resolutionWidth > 7680) valid = false;
    if (g.resolutionHeight < 480 || g.resolutionHeight > 4320) valid = false;
    if (g.targetFPS < 30 || g.targetFPS > 300) valid = false;

    // Audio validation
    auto& a = _config.audio;
    if (a.masterVolume < 0.0f || a.masterVolume > 1.0f) valid = false;
    if (a.musicVolume < 0.0f || a.musicVolume > 1.0f) valid = false;
    if (a.sfxVolume < 0.0f || a.sfxVolume > 1.0f) valid = false;
    if (a.voiceVolume < 0.0f || a.voiceVolume > 1.0f) valid = false;

    // Network validation
    auto& n = _config.network;
    if (n.serverPort < 1024 || n.serverPort > 65535) valid = false;
    if (n.maxPlayers < 1 || n.maxPlayers > 1000) valid = false;

    return valid;
}

static std::string Trim(const std::string& s)
{
    auto start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    auto end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

/// Helper to apply a key-value pair to the configuration.
static void ApplyKeyValue(GameConfiguration& cfg, const std::string& key, const std::string& value)
{
    // Graphics
    if (key == "graphics.resolutionWidth") cfg.graphics.resolutionWidth = std::stoi(value);
    else if (key == "graphics.resolutionHeight") cfg.graphics.resolutionHeight = std::stoi(value);
    else if (key == "graphics.vsync") cfg.graphics.vsync = (value == "true" || value == "1");
    else if (key == "graphics.targetFPS") cfg.graphics.targetFPS = std::stoi(value);
    else if (key == "graphics.antiAliasingLevel") cfg.graphics.antiAliasingLevel = std::stoi(value);
    else if (key == "graphics.renderDistance") cfg.graphics.renderDistance = std::stof(value);
    else if (key == "graphics.shadowsEnabled") cfg.graphics.shadowsEnabled = (value == "true" || value == "1");
    else if (key == "graphics.particlesEnabled") cfg.graphics.particlesEnabled = (value == "true" || value == "1");
    else if (key == "graphics.textureQuality") cfg.graphics.textureQuality = std::stoi(value);
    // Audio
    else if (key == "audio.masterVolume") cfg.audio.masterVolume = std::stof(value);
    else if (key == "audio.musicVolume") cfg.audio.musicVolume = std::stof(value);
    else if (key == "audio.sfxVolume") cfg.audio.sfxVolume = std::stof(value);
    else if (key == "audio.voiceVolume") cfg.audio.voiceVolume = std::stof(value);
    else if (key == "audio.isMuted") cfg.audio.isMuted = (value == "true" || value == "1");
    // Gameplay
    else if (key == "gameplay.playerName") cfg.gameplay.playerName = value;
    else if (key == "gameplay.autoSaveIntervalSeconds") cfg.gameplay.autoSaveIntervalSeconds = std::stoi(value);
    else if (key == "gameplay.difficulty") cfg.gameplay.difficulty = std::stoi(value);
    else if (key == "gameplay.showTutorials") cfg.gameplay.showTutorials = (value == "true" || value == "1");
    else if (key == "gameplay.showHints") cfg.gameplay.showHints = (value == "true" || value == "1");
    else if (key == "gameplay.language") cfg.gameplay.language = value;
    // Network
    else if (key == "network.serverAddress") cfg.network.serverAddress = value;
    else if (key == "network.serverPort") cfg.network.serverPort = std::stoi(value);
    else if (key == "network.connectionTimeoutSeconds") cfg.network.connectionTimeoutSeconds = std::stoi(value);
    else if (key == "network.isMultiplayerEnabled") cfg.network.isMultiplayerEnabled = (value == "true" || value == "1");
    else if (key == "network.maxPlayers") cfg.network.maxPlayers = std::stoi(value);
    else if (key == "network.tickRate") cfg.network.tickRate = std::stoi(value);
    // Development
    else if (key == "development.debugMode") cfg.development.debugMode = (value == "true" || value == "1");
    else if (key == "development.showProfiler") cfg.development.showProfiler = (value == "true" || value == "1");
    else if (key == "development.showMemoryTracker") cfg.development.showMemoryTracker = (value == "true" || value == "1");
    else if (key == "development.logLevelThreshold") cfg.development.logLevelThreshold = std::stoi(value);
    else if (key == "development.enableConsole") cfg.development.enableConsole = (value == "true" || value == "1");
    else if (key == "development.galaxySeed") cfg.development.galaxySeed = std::stoi(value);
}

bool ConfigurationManager::LoadConfiguration(const std::string& path)
{
    std::ifstream file(path);
    if (!file.is_open()) return false;

    std::string line;
    while (std::getline(file, line)) {
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') continue;

        auto delimPos = line.find('=');
        if (delimPos == std::string::npos) continue;

        std::string key = Trim(line.substr(0, delimPos));
        std::string value = Trim(line.substr(delimPos + 1));

        try {
            ApplyKeyValue(_config, key, value);
        } catch (const std::exception&) {
            // Skip malformed values
        }
    }

    return true;
}

static std::string BoolToString(bool v)
{
    return v ? "true" : "false";
}

bool ConfigurationManager::SaveConfiguration(const std::string& path)
{
    std::ofstream file(path);
    if (!file.is_open()) return false;

    const auto& g = _config.graphics;
    file << "graphics.resolutionWidth=" << g.resolutionWidth << "\n";
    file << "graphics.resolutionHeight=" << g.resolutionHeight << "\n";
    file << "graphics.vsync=" << BoolToString(g.vsync) << "\n";
    file << "graphics.targetFPS=" << g.targetFPS << "\n";
    file << "graphics.antiAliasingLevel=" << g.antiAliasingLevel << "\n";
    file << "graphics.renderDistance=" << g.renderDistance << "\n";
    file << "graphics.shadowsEnabled=" << BoolToString(g.shadowsEnabled) << "\n";
    file << "graphics.particlesEnabled=" << BoolToString(g.particlesEnabled) << "\n";
    file << "graphics.textureQuality=" << g.textureQuality << "\n";

    const auto& a = _config.audio;
    file << "audio.masterVolume=" << a.masterVolume << "\n";
    file << "audio.musicVolume=" << a.musicVolume << "\n";
    file << "audio.sfxVolume=" << a.sfxVolume << "\n";
    file << "audio.voiceVolume=" << a.voiceVolume << "\n";
    file << "audio.isMuted=" << BoolToString(a.isMuted) << "\n";

    const auto& gp = _config.gameplay;
    file << "gameplay.playerName=" << gp.playerName << "\n";
    file << "gameplay.autoSaveIntervalSeconds=" << gp.autoSaveIntervalSeconds << "\n";
    file << "gameplay.difficulty=" << gp.difficulty << "\n";
    file << "gameplay.showTutorials=" << BoolToString(gp.showTutorials) << "\n";
    file << "gameplay.showHints=" << BoolToString(gp.showHints) << "\n";
    file << "gameplay.language=" << gp.language << "\n";

    const auto& n = _config.network;
    file << "network.serverAddress=" << n.serverAddress << "\n";
    file << "network.serverPort=" << n.serverPort << "\n";
    file << "network.connectionTimeoutSeconds=" << n.connectionTimeoutSeconds << "\n";
    file << "network.isMultiplayerEnabled=" << BoolToString(n.isMultiplayerEnabled) << "\n";
    file << "network.maxPlayers=" << n.maxPlayers << "\n";
    file << "network.tickRate=" << n.tickRate << "\n";

    const auto& d = _config.development;
    file << "development.debugMode=" << BoolToString(d.debugMode) << "\n";
    file << "development.showProfiler=" << BoolToString(d.showProfiler) << "\n";
    file << "development.showMemoryTracker=" << BoolToString(d.showMemoryTracker) << "\n";
    file << "development.logLevelThreshold=" << d.logLevelThreshold << "\n";
    file << "development.enableConsole=" << BoolToString(d.enableConsole) << "\n";
    file << "development.galaxySeed=" << d.galaxySeed << "\n";

    return true;
}

const GameConfiguration& ConfigurationManager::GetConfig() const
{
    return _config;
}

GameConfiguration& ConfigurationManager::GetMutableConfig()
{
    return _config;
}

const GraphicsSettings& ConfigurationManager::GetGraphics() const
{
    return _config.graphics;
}

const AudioSettings& ConfigurationManager::GetAudio() const
{
    return _config.audio;
}

const GameplaySettings& ConfigurationManager::GetGameplay() const
{
    return _config.gameplay;
}

const NetworkSettings& ConfigurationManager::GetNetwork() const
{
    return _config.network;
}

const DevelopmentSettings& ConfigurationManager::GetDevelopment() const
{
    return _config.development;
}

} // namespace subspace
