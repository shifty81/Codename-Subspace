using NLua;
using AvorionLike.Core.Logging;

namespace AvorionLike.Core.Scripting;

/// <summary>
/// Manages Lua scripting for modding support
/// </summary>
public class ScriptingEngine
{
    private readonly Lua _luaState;
    private readonly Dictionary<string, object> _registeredObjects = new();
    private readonly Logger _logger;
    private LuaAPI? _luaAPI;

    public LuaAPI? API => _luaAPI;

    public ScriptingEngine()
    {
        _luaState = new Lua();
        _logger = Logger.Instance;
        InitializeStandardLibraries();
    }

    /// <summary>
    /// Initialize the Lua API wrapper
    /// </summary>
    public void InitializeAPI(GameEngine engine)
    {
        _luaAPI = new LuaAPI(engine);
        RegisterObject("API", _luaAPI);
        _logger.Info("ScriptingEngine", "Lua API initialized and registered");
    }

    private void InitializeStandardLibraries()
    {
        // Lua standard libraries are already loaded by NLua
        // Register custom game functions
        _luaState.DoString(@"
            function log(message)
                print('[LUA] ' .. tostring(message))
            end
            
            -- Helper function to access API safely
            function SafeAPICall(funcName, ...)
                if API == nil then
                    log('ERROR: API not initialized!')
                    return nil
                end
                local success, result = pcall(API[funcName], API, ...)
                if not success then
                    log('ERROR calling ' .. funcName .. ': ' .. tostring(result))
                    return nil
                end
                return result
            end
        ");
        
        _logger.Info("ScriptingEngine", "Standard Lua libraries initialized");
    }

    /// <summary>
    /// Execute a Lua script
    /// </summary>
    public object[]? ExecuteScript(string script)
    {
        try
        {
            _logger.Debug("ScriptingEngine", "Executing Lua script");
            return _luaState.DoString(script);
        }
        catch (Exception ex)
        {
            _logger.Error("ScriptingEngine", $"Lua script error: {ex.Message}");
            Console.WriteLine($"Lua script error: {ex.Message}");
            return null;
        }
    }

    /// <summary>
    /// Execute a Lua script file
    /// </summary>
    public object[]? ExecuteFile(string filePath)
    {
        try
        {
            _logger.Info("ScriptingEngine", $"Executing Lua file: {filePath}");
            return _luaState.DoFile(filePath);
        }
        catch (Exception ex)
        {
            _logger.Error("ScriptingEngine", $"Lua file error: {ex.Message}");
            Console.WriteLine($"Lua file error: {ex.Message}");
            return null;
        }
    }

    /// <summary>
    /// Register a C# object to be accessible from Lua
    /// </summary>
    public void RegisterObject(string name, object obj)
    {
        _registeredObjects[name] = obj;
        _luaState[name] = obj;
    }

    /// <summary>
    /// Call a Lua function
    /// </summary>
    public object? CallFunction(string functionName, params object[] args)
    {
        try
        {
            var function = _luaState[functionName] as LuaFunction;
            return function?.Call(args)?[0];
        }
        catch (Exception ex)
        {
            Console.WriteLine($"Error calling Lua function {functionName}: {ex.Message}");
            return null;
        }
    }

    /// <summary>
    /// Get a Lua global variable
    /// </summary>
    public object? GetGlobal(string name)
    {
        return _luaState[name];
    }

    /// <summary>
    /// Set a Lua global variable
    /// </summary>
    public void SetGlobal(string name, object value)
    {
        _luaState[name] = value;
    }

    /// <summary>
    /// Load and execute a mod script
    /// </summary>
    public bool LoadMod(string modPath)
    {
        if (!File.Exists(modPath))
        {
            Console.WriteLine($"Mod file not found: {modPath}");
            return false;
        }

        try
        {
            ExecuteFile(modPath);
            Console.WriteLine($"Mod loaded successfully: {modPath}");
            return true;
        }
        catch (Exception ex)
        {
            Console.WriteLine($"Failed to load mod {modPath}: {ex.Message}");
            return false;
        }
    }

    /// <summary>
    /// Clean up Lua state
    /// </summary>
    public void Dispose()
    {
        _luaState?.Dispose();
    }
}
