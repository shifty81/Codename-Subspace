namespace AvorionLike.Core;

/// <summary>
/// Provides version information for the Codename:Subspace game engine
/// </summary>
public static class VersionInfo
{
    /// <summary>
    /// The current version of the game engine
    /// </summary>
    public const string Version = "0.9.0";
    
    /// <summary>
    /// The version name or codename
    /// </summary>
    public const string VersionName = "Player UI Release";
    
    /// <summary>
    /// The release date of this version
    /// </summary>
    public const string ReleaseDate = "2025-11-05";
    
    /// <summary>
    /// The full version string including name
    /// </summary>
    public static string FullVersion => $"Codename:Subspace v{Version} - {VersionName}";
    
    /// <summary>
    /// The copyright notice
    /// </summary>
    public const string Copyright = "Copyright © 2025 AvorionLike Team";
    
    /// <summary>
    /// The license type
    /// </summary>
    public const string License = "MIT License";
    
    /// <summary>
    /// Minimum .NET version required
    /// </summary>
    public const string MinDotNetVersion = ".NET 9.0";
    
    /// <summary>
    /// Target framework
    /// </summary>
    public const string TargetFramework = "net9.0";
    
    /// <summary>
    /// Get a formatted version info string for display
    /// </summary>
    public static string GetVersionInfo()
    {
        return $"{FullVersion}\n" +
               $"Released: {ReleaseDate}\n" +
               $"{Copyright}\n" +
               $"{License}";
    }
    
    /// <summary>
    /// Get system requirements information
    /// </summary>
    public static string GetSystemRequirements()
    {
        return $"System Requirements:\n" +
               $"  • {MinDotNetVersion} SDK or later\n" +
               $"  • OpenGL 3.3+ compatible graphics card\n" +
               $"  • 4 GB RAM minimum (8 GB recommended)\n" +
               $"  • 500 MB available disk space\n" +
               $"  • Windows 10/11, Linux, or macOS";
    }
}
