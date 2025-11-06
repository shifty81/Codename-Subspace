using AvorionLike.Core.Logging;
using System.Text.Json;

namespace AvorionLike.Core.Scripting;

/// <summary>
/// Manages mod discovery, loading, and dependency resolution
/// </summary>
public class ModManager
{
    private readonly ScriptingEngine _scriptingEngine;
    private readonly Logger _logger;
    private readonly Dictionary<string, ModInfo> _loadedMods = new();
    private readonly List<string> _modLoadOrder = new();
    private string _modsDirectory;

    public IReadOnlyDictionary<string, ModInfo> LoadedMods => _loadedMods;
    public IReadOnlyList<string> ModLoadOrder => _modLoadOrder;

    public ModManager(ScriptingEngine scriptingEngine, string? modsDirectory = null)
    {
        _scriptingEngine = scriptingEngine;
        _logger = Logger.Instance;
        
        // Default mods directory in AppData
        _modsDirectory = modsDirectory ?? Path.Combine(
            Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData),
            "Codename-Subspace",
            "Mods"
        );

        EnsureModsDirectoryExists();
    }

    /// <summary>
    /// Ensure mods directory exists
    /// </summary>
    private void EnsureModsDirectoryExists()
    {
        if (!Directory.Exists(_modsDirectory))
        {
            Directory.CreateDirectory(_modsDirectory);
            _logger.Info("ModManager", $"Created mods directory: {_modsDirectory}");
        }
    }

    /// <summary>
    /// Discover all mods in the mods directory
    /// </summary>
    public List<ModInfo> DiscoverMods()
    {
        var discoveredMods = new List<ModInfo>();

        try
        {
            var modDirectories = Directory.GetDirectories(_modsDirectory);
            
            foreach (var modDir in modDirectories)
            {
                var modInfoPath = Path.Combine(modDir, "mod.json");
                
                if (File.Exists(modInfoPath))
                {
                    var modInfo = LoadModInfo(modInfoPath);
                    if (modInfo != null)
                    {
                        modInfo.DirectoryPath = modDir;
                        discoveredMods.Add(modInfo);
                        _logger.Info("ModManager", $"Discovered mod: {modInfo.Name} v{modInfo.Version}");
                    }
                }
            }
        }
        catch (Exception ex)
        {
            _logger.Error("ModManager", $"Error discovering mods: {ex.Message}");
        }

        return discoveredMods;
    }

    /// <summary>
    /// Load mod metadata from mod.json
    /// </summary>
    private ModInfo? LoadModInfo(string modInfoPath)
    {
        try
        {
            var json = File.ReadAllText(modInfoPath);
            var modInfo = JsonSerializer.Deserialize<ModInfo>(json, new JsonSerializerOptions
            {
                PropertyNameCaseInsensitive = true
            });
            return modInfo;
        }
        catch (Exception ex)
        {
            _logger.Error("ModManager", $"Failed to load mod info from {modInfoPath}: {ex.Message}");
            return null;
        }
    }

    /// <summary>
    /// Load all discovered mods in dependency order
    /// </summary>
    public bool LoadAllMods()
    {
        _logger.Info("ModManager", "Starting mod loading process...");
        
        var discoveredMods = DiscoverMods();
        
        if (discoveredMods.Count == 0)
        {
            _logger.Info("ModManager", "No mods found to load");
            return true;
        }

        // Sort mods by dependencies
        var sortedMods = ResolveDependencies(discoveredMods);
        
        if (sortedMods == null)
        {
            _logger.Error("ModManager", "Failed to resolve mod dependencies");
            return false;
        }

        // Load mods in order
        var successCount = 0;
        foreach (var mod in sortedMods)
        {
            if (LoadMod(mod))
            {
                successCount++;
            }
        }

        _logger.Info("ModManager", $"Loaded {successCount}/{sortedMods.Count} mods successfully");
        return successCount == sortedMods.Count;
    }

    /// <summary>
    /// Load a single mod
    /// </summary>
    public bool LoadMod(ModInfo modInfo)
    {
        if (_loadedMods.ContainsKey(modInfo.Id))
        {
            _logger.Warning("ModManager", $"Mod {modInfo.Name} is already loaded");
            return true;
        }

        try
        {
            _logger.Info("ModManager", $"Loading mod: {modInfo.Name} v{modInfo.Version}");

            // Check dependencies
            if (!CheckDependencies(modInfo))
            {
                _logger.Error("ModManager", $"Missing dependencies for mod: {modInfo.Name}");
                return false;
            }

            // Load main script
            var mainScriptPath = Path.Combine(modInfo.DirectoryPath!, modInfo.MainScript);
            
            if (!File.Exists(mainScriptPath))
            {
                _logger.Error("ModManager", $"Main script not found: {mainScriptPath}");
                return false;
            }

            // Execute mod script
            var result = _scriptingEngine.ExecuteFile(mainScriptPath);
            
            if (result == null)
            {
                _logger.Error("ModManager", $"Failed to execute mod script: {modInfo.Name}");
                return false;
            }

            // Mark as loaded
            _loadedMods[modInfo.Id] = modInfo;
            _modLoadOrder.Add(modInfo.Id);

            _logger.Info("ModManager", $"Successfully loaded mod: {modInfo.Name}");
            Console.WriteLine($"✓ Loaded mod: {modInfo.Name} v{modInfo.Version} by {modInfo.Author}");
            
            return true;
        }
        catch (Exception ex)
        {
            _logger.Error("ModManager", $"Error loading mod {modInfo.Name}: {ex.Message}");
            return false;
        }
    }

    /// <summary>
    /// Check if all dependencies are loaded
    /// </summary>
    private bool CheckDependencies(ModInfo modInfo)
    {
        if (modInfo.Dependencies == null || modInfo.Dependencies.Count == 0)
            return true;

        foreach (var dep in modInfo.Dependencies)
        {
            if (!_loadedMods.ContainsKey(dep))
            {
                _logger.Warning("ModManager", $"Missing dependency: {dep} for mod {modInfo.Name}");
                return false;
            }
        }

        return true;
    }

    /// <summary>
    /// Resolve mod dependencies and return load order
    /// </summary>
    private List<ModInfo>? ResolveDependencies(List<ModInfo> mods)
    {
        var sortedMods = new List<ModInfo>();
        var visited = new HashSet<string>();
        var visiting = new HashSet<string>();
        var modDict = mods.ToDictionary(m => m.Id);

        bool Visit(ModInfo mod)
        {
            if (visited.Contains(mod.Id))
                return true;

            if (visiting.Contains(mod.Id))
            {
                _logger.Error("ModManager", $"Circular dependency detected involving mod: {mod.Name}");
                return false;
            }

            visiting.Add(mod.Id);

            // Visit dependencies first
            if (mod.Dependencies != null)
            {
                foreach (var depId in mod.Dependencies)
                {
                    if (modDict.TryGetValue(depId, out var depMod))
                    {
                        if (!Visit(depMod))
                            return false;
                    }
                    else
                    {
                        _logger.Warning("ModManager", $"Dependency {depId} not found for mod {mod.Name}");
                    }
                }
            }

            visiting.Remove(mod.Id);
            visited.Add(mod.Id);
            sortedMods.Add(mod);

            return true;
        }

        foreach (var mod in mods)
        {
            if (!Visit(mod))
                return null;
        }

        return sortedMods;
    }

    /// <summary>
    /// Unload a mod
    /// </summary>
    public bool UnloadMod(string modId)
    {
        if (!_loadedMods.ContainsKey(modId))
        {
            _logger.Warning("ModManager", $"Mod {modId} is not loaded");
            return false;
        }

        _loadedMods.Remove(modId);
        _modLoadOrder.Remove(modId);
        
        _logger.Info("ModManager", $"Unloaded mod: {modId}");
        return true;
    }

    /// <summary>
    /// Reload all mods
    /// </summary>
    public bool ReloadAllMods()
    {
        _logger.Info("ModManager", "Reloading all mods...");
        
        _loadedMods.Clear();
        _modLoadOrder.Clear();
        
        return LoadAllMods();
    }

    /// <summary>
    /// Get mod information
    /// </summary>
    public ModInfo? GetModInfo(string modId)
    {
        return _loadedMods.TryGetValue(modId, out var modInfo) ? modInfo : null;
    }

    /// <summary>
    /// Create a sample mod template
    /// </summary>
    public void CreateSampleMod(string modName)
    {
        try
        {
            var modId = modName.ToLower().Replace(" ", "_");
            var modDir = Path.Combine(_modsDirectory, modId);
            
            if (Directory.Exists(modDir))
            {
                _logger.Warning("ModManager", $"Mod directory already exists: {modDir}");
                return;
            }

            Directory.CreateDirectory(modDir);

            // Create mod.json
            var modInfo = new ModInfo
            {
                Id = modId,
                Name = modName,
                Version = "1.0.0",
                Author = "ModAuthor",
                Description = $"A sample mod: {modName}",
                MainScript = "main.lua",
                Dependencies = new List<string>()
            };

            var json = JsonSerializer.Serialize(modInfo, new JsonSerializerOptions
            {
                WriteIndented = true
            });
            File.WriteAllText(Path.Combine(modDir, "mod.json"), json);

            // Create main.lua using string interpolation
            var luaTemplate = $@"-- {modName} Mod
-- Main script file

log('Loading {modName} mod...')

-- Your mod initialization code here
function OnModLoad()
    log('{modName} mod loaded successfully!')
end

-- Call initialization
OnModLoad()
";
            File.WriteAllText(Path.Combine(modDir, "main.lua"), luaTemplate);

            _logger.Info("ModManager", $"Created sample mod: {modName} at {modDir}");
            Console.WriteLine($"✓ Created sample mod template at: {modDir}");
        }
        catch (Exception ex)
        {
            _logger.Error("ModManager", $"Failed to create sample mod: {ex.Message}");
        }
    }
}

/// <summary>
/// Mod metadata structure
/// </summary>
public class ModInfo
{
    public string Id { get; set; } = "";
    public string Name { get; set; } = "";
    public string Version { get; set; } = "1.0.0";
    public string Author { get; set; } = "";
    public string Description { get; set; } = "";
    public string MainScript { get; set; } = "main.lua";
    public List<string>? Dependencies { get; set; }
    public string? DirectoryPath { get; set; }
}
