using System.Text.Json;

namespace AvorionLike.Core.Configuration;

/// <summary>
/// Central configuration for all game settings
/// </summary>
public class GameConfiguration
{
    // Graphics Settings
    public GraphicsSettings Graphics { get; set; } = new();
    
    // Audio Settings
    public AudioSettings Audio { get; set; } = new();
    
    // Gameplay Settings
    public GameplaySettings Gameplay { get; set; } = new();
    
    // Network Settings
    public NetworkSettings Network { get; set; } = new();
    
    // Development Settings
    public DevelopmentSettings Development { get; set; } = new();

    /// <summary>
    /// Load configuration from JSON file
    /// </summary>
    public static GameConfiguration LoadFromFile(string path)
    {
        try
        {
            if (!File.Exists(path))
            {
                Console.WriteLine($"Configuration file not found at {path}, using defaults");
                return new GameConfiguration();
            }

            var json = File.ReadAllText(path);
            var config = JsonSerializer.Deserialize<GameConfiguration>(json);
            return config ?? new GameConfiguration();
        }
        catch (Exception ex)
        {
            Console.WriteLine($"Error loading configuration: {ex.Message}");
            return new GameConfiguration();
        }
    }

    /// <summary>
    /// Save configuration to JSON file
    /// </summary>
    public void SaveToFile(string path)
    {
        try
        {
            var options = new JsonSerializerOptions 
            { 
                WriteIndented = true 
            };
            var json = JsonSerializer.Serialize(this, options);
            File.WriteAllText(path, json);
            Console.WriteLine($"Configuration saved to {path}");
        }
        catch (Exception ex)
        {
            Console.WriteLine($"Error saving configuration: {ex.Message}");
        }
    }
}

/// <summary>
/// Graphics-related settings
/// </summary>
public class GraphicsSettings
{
    public int ResolutionWidth { get; set; } = 1920;
    public int ResolutionHeight { get; set; } = 1080;
    public bool Fullscreen { get; set; } = false;
    public bool VSync { get; set; } = true;
    public int TargetFrameRate { get; set; } = 60;
    public int AntiAliasing { get; set; } = 4; // MSAA samples
    public float RenderDistance { get; set; } = 10000f;
    public bool EnableShadows { get; set; } = true;
    public bool EnableParticles { get; set; } = true;
    public int TextureQuality { get; set; } = 2; // 0=Low, 1=Medium, 2=High
}

/// <summary>
/// Audio-related settings
/// </summary>
public class AudioSettings
{
    public float MasterVolume { get; set; } = 1.0f;
    public float MusicVolume { get; set; } = 0.7f;
    public float SfxVolume { get; set; } = 0.8f;
    public float VoiceVolume { get; set; } = 0.9f;
    public bool Muted { get; set; } = false;
}

/// <summary>
/// Gameplay-related settings
/// </summary>
public class GameplaySettings
{
    public string PlayerName { get; set; } = "Player";
    public int AutoSaveIntervalSeconds { get; set; } = 300;
    public bool EnableAutoSave { get; set; } = true;
    public int Difficulty { get; set; } = 1; // 0=Easy, 1=Normal, 2=Hard
    public bool ShowTutorials { get; set; } = true;
    public bool EnableHints { get; set; } = true;
    public string Language { get; set; } = "en-US";
}

/// <summary>
/// Network/Multiplayer settings
/// </summary>
public class NetworkSettings
{
    public string ServerAddress { get; set; } = "127.0.0.1";
    public int ServerPort { get; set; } = 27015;
    public int ConnectionTimeoutSeconds { get; set; } = 30;
    public bool EnableMultiplayer { get; set; } = true;
    public int MaxPlayers { get; set; } = 50;
    public int TickRate { get; set; } = 20; // Server update rate
}

/// <summary>
/// Development/Debug settings
/// </summary>
public class DevelopmentSettings
{
    public bool EnableDebugMode { get; set; } = false;
    public bool ShowDebugOverlay { get; set; } = false;
    public bool EnablePerformanceProfiler { get; set; } = false;
    public bool EnableMemoryTracker { get; set; } = false;
    public bool LogToFile { get; set; } = true;
    public string LogLevel { get; set; } = "Info"; // Debug, Info, Warning, Error, Critical
    public bool EnableConsole { get; set; } = true;
    public int GalaxySeed { get; set; } = 12345;
}
