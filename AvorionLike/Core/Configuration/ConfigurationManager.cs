namespace AvorionLike.Core.Configuration;

/// <summary>
/// Manages game configuration lifecycle
/// </summary>
public class ConfigurationManager
{
    private static ConfigurationManager? _instance;
    private GameConfiguration _configuration = null!;
    private readonly string _configPath;

    public static ConfigurationManager Instance
    {
        get
        {
            _instance ??= new ConfigurationManager();
            return _instance;
        }
    }

    public GameConfiguration Config => _configuration;

    private ConfigurationManager()
    {
        _configPath = Path.Combine(
            Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData),
            "Codename-Subspace",
            "config.json"
        );

        // Ensure directory exists
        var directory = Path.GetDirectoryName(_configPath);
        if (!string.IsNullOrEmpty(directory) && !Directory.Exists(directory))
        {
            Directory.CreateDirectory(directory);
        }

        LoadConfiguration();
    }

    /// <summary>
    /// Load configuration from file or create default
    /// </summary>
    public void LoadConfiguration()
    {
        _configuration = GameConfiguration.LoadFromFile(_configPath);
    }

    /// <summary>
    /// Save current configuration to file
    /// </summary>
    public void SaveConfiguration()
    {
        _configuration.SaveToFile(_configPath);
    }

    /// <summary>
    /// Reset configuration to defaults
    /// </summary>
    public void ResetToDefaults()
    {
        _configuration = new GameConfiguration();
        SaveConfiguration();
    }

    /// <summary>
    /// Get configuration file path
    /// </summary>
    public string GetConfigPath()
    {
        return _configPath;
    }

    /// <summary>
    /// Validate configuration settings
    /// </summary>
    public bool ValidateConfiguration(out List<string> errors)
    {
        errors = new List<string>();

        // Validate graphics settings
        if (_configuration.Graphics.ResolutionWidth < 640 || _configuration.Graphics.ResolutionWidth > 7680)
            errors.Add("Graphics.ResolutionWidth must be between 640 and 7680");
        
        if (_configuration.Graphics.ResolutionHeight < 480 || _configuration.Graphics.ResolutionHeight > 4320)
            errors.Add("Graphics.ResolutionHeight must be between 480 and 4320");
        
        if (_configuration.Graphics.TargetFrameRate < 30 || _configuration.Graphics.TargetFrameRate > 300)
            errors.Add("Graphics.TargetFrameRate must be between 30 and 300");

        // Validate audio settings
        if (_configuration.Audio.MasterVolume < 0f || _configuration.Audio.MasterVolume > 1f)
            errors.Add("Audio.MasterVolume must be between 0.0 and 1.0");

        // Validate network settings
        if (_configuration.Network.ServerPort < 1024 || _configuration.Network.ServerPort > 65535)
            errors.Add("Network.ServerPort must be between 1024 and 65535");
        
        if (_configuration.Network.MaxPlayers < 1 || _configuration.Network.MaxPlayers > 1000)
            errors.Add("Network.MaxPlayers must be between 1 and 1000");

        return errors.Count == 0;
    }

    /// <summary>
    /// Apply configuration changes to engine
    /// </summary>
    public void ApplyConfiguration()
    {
        // Validate first
        if (!ValidateConfiguration(out var errors))
        {
            foreach (var error in errors)
            {
                Console.WriteLine($"Configuration validation error: {error}");
            }
            return;
        }

        Console.WriteLine("Configuration applied successfully");
        // Note: Actual application of settings would be done by respective systems
    }
}
