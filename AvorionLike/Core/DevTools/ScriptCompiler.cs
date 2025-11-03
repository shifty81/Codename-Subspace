using AvorionLike.Core.Scripting;

namespace AvorionLike.Core.DevTools;

/// <summary>
/// Script Compiler - Runtime script compilation and execution system
/// Provides hot-reloading capabilities for Lua scripts
/// </summary>
public class ScriptCompiler
{
    private ScriptingEngine scriptingEngine;
    private Dictionary<string, ScriptInfo> loadedScripts = new();
    private List<string> compilationErrors = new();

    public IReadOnlyList<string> CompilationErrors => compilationErrors.AsReadOnly();
    public int LoadedScriptCount => loadedScripts.Count;

    public ScriptCompiler(ScriptingEngine scripting)
    {
        scriptingEngine = scripting;
    }

    /// <summary>
    /// Compile and execute a script from string
    /// </summary>
    public bool CompileAndExecute(string scriptContent, string scriptName = "RuntimeScript")
    {
        compilationErrors.Clear();

        try
        {
            // Execute the script
            scriptingEngine.ExecuteScript(scriptContent);

            // Track the script
            loadedScripts[scriptName] = new ScriptInfo
            {
                Name = scriptName,
                Content = scriptContent,
                LoadTime = DateTime.Now,
                ExecutionCount = 1
            };

            Console.WriteLine($"[Script Compiler] Successfully compiled and executed: {scriptName}");
            return true;
        }
        catch (Exception ex)
        {
            compilationErrors.Add($"Error in {scriptName}: {ex.Message}");
            Console.WriteLine($"[Script Compiler] Compilation error in {scriptName}: {ex.Message}");
            return false;
        }
    }

    /// <summary>
    /// Compile and execute a script from file
    /// </summary>
    public bool CompileFile(string filePath)
    {
        compilationErrors.Clear();

        if (!File.Exists(filePath))
        {
            compilationErrors.Add($"File not found: {filePath}");
            return false;
        }

        try
        {
            string content = File.ReadAllText(filePath);
            string scriptName = Path.GetFileName(filePath);
            
            return CompileAndExecute(content, scriptName);
        }
        catch (Exception ex)
        {
            compilationErrors.Add($"Error loading {filePath}: {ex.Message}");
            Console.WriteLine($"[Script Compiler] Error loading {filePath}: {ex.Message}");
            return false;
        }
    }

    /// <summary>
    /// Reload a previously loaded script
    /// </summary>
    public bool ReloadScript(string scriptName)
    {
        if (!loadedScripts.ContainsKey(scriptName))
        {
            compilationErrors.Add($"Script not found: {scriptName}");
            return false;
        }

        var scriptInfo = loadedScripts[scriptName];
        bool success = CompileAndExecute(scriptInfo.Content, scriptName);
        
        if (success)
        {
            scriptInfo.ExecutionCount++;
            scriptInfo.LoadTime = DateTime.Now;
            loadedScripts[scriptName] = scriptInfo;
        }

        return success;
    }

    /// <summary>
    /// Watch a directory for script changes and auto-reload
    /// </summary>
    public FileSystemWatcher WatchDirectory(string directoryPath, string filter = "*.lua")
    {
        var watcher = new FileSystemWatcher(directoryPath, filter)
        {
            NotifyFilter = NotifyFilters.LastWrite | NotifyFilters.FileName
        };

        watcher.Changed += (sender, e) =>
        {
            Console.WriteLine($"[Script Compiler] Detected change in {e.Name}, reloading...");
            System.Threading.Thread.Sleep(100); // Brief delay to ensure file is fully written
            CompileFile(e.FullPath);
        };

        watcher.Created += (sender, e) =>
        {
            Console.WriteLine($"[Script Compiler] New script detected: {e.Name}");
            CompileFile(e.FullPath);
        };

        watcher.EnableRaisingEvents = true;
        Console.WriteLine($"[Script Compiler] Watching directory: {directoryPath}");
        
        return watcher;
    }

    /// <summary>
    /// Validate script syntax without executing
    /// </summary>
    public bool ValidateScript(string scriptContent, out List<string> errors)
    {
        errors = new List<string>();

        try
        {
            // Lua syntax validation - basic check
            if (string.IsNullOrWhiteSpace(scriptContent))
            {
                errors.Add("Script content is empty");
                return false;
            }

            // In a full implementation, this could use a Lua parser for syntax checking
            // For now, we just do basic validation
            int functionCount = scriptContent.Split("function").Length - 1;
            int endCount = scriptContent.Split("end").Length - 1;

            if (functionCount != endCount)
            {
                errors.Add("Mismatched function/end blocks");
                return false;
            }

            return true;
        }
        catch (Exception ex)
        {
            errors.Add($"Validation error: {ex.Message}");
            return false;
        }
    }

    /// <summary>
    /// Get information about a loaded script
    /// </summary>
    public ScriptInfo? GetScriptInfo(string scriptName)
    {
        return loadedScripts.ContainsKey(scriptName) ? loadedScripts[scriptName] : null;
    }

    /// <summary>
    /// Get all loaded scripts
    /// </summary>
    public IReadOnlyDictionary<string, ScriptInfo> GetLoadedScripts()
    {
        return loadedScripts;
    }

    /// <summary>
    /// Unload a script
    /// </summary>
    public bool UnloadScript(string scriptName)
    {
        if (loadedScripts.Remove(scriptName))
        {
            Console.WriteLine($"[Script Compiler] Unloaded script: {scriptName}");
            return true;
        }
        return false;
    }

    /// <summary>
    /// Clear all loaded scripts
    /// </summary>
    public void ClearAll()
    {
        loadedScripts.Clear();
        compilationErrors.Clear();
        Console.WriteLine("[Script Compiler] Cleared all loaded scripts");
    }

    /// <summary>
    /// Generate a compilation report
    /// </summary>
    public string GenerateReport()
    {
        var report = new System.Text.StringBuilder();
        report.AppendLine("=== Script Compiler Report ===");
        report.AppendLine($"Loaded Scripts: {LoadedScriptCount}");
        report.AppendLine($"Compilation Errors: {compilationErrors.Count}");
        
        if (loadedScripts.Count > 0)
        {
            report.AppendLine();
            report.AppendLine("=== Loaded Scripts ===");
            foreach (var script in loadedScripts.Values.OrderBy(s => s.Name))
            {
                report.AppendLine($"{script.Name}:");
                report.AppendLine($"  Load Time: {script.LoadTime:HH:mm:ss}");
                report.AppendLine($"  Executions: {script.ExecutionCount}");
                report.AppendLine($"  Size: {script.Content.Length} bytes");
            }
        }

        if (compilationErrors.Count > 0)
        {
            report.AppendLine();
            report.AppendLine("=== Recent Errors ===");
            foreach (var error in compilationErrors.TakeLast(5))
            {
                report.AppendLine($"  {error}");
            }
        }

        return report.ToString();
    }

    public struct ScriptInfo
    {
        public string Name { get; set; }
        public string Content { get; set; }
        public DateTime LoadTime { get; set; }
        public int ExecutionCount { get; set; }
    }
}
