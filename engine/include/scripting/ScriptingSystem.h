#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace subspace {

/// Result of a script execution.
struct ScriptResult {
    bool success = false;
    std::string output;
    std::string error;
};

/// Scripting engine that manages script execution and function registration.
/// Transport-agnostic: no Lua VM dependency; uses registered C++ callbacks.
class ScriptingEngine {
public:
    using ScriptFunction = std::function<std::string(const std::vector<std::string>&)>;

    ScriptingEngine();

    /// Register a C++ function callable by name from scripts.
    void RegisterFunction(const std::string& name, ScriptFunction func);

    /// Unregister a function.
    bool UnregisterFunction(const std::string& name);

    /// Check if a function is registered.
    bool HasFunction(const std::string& name) const;

    /// Get count of registered functions.
    size_t GetFunctionCount() const;

    /// Call a registered function by name with arguments.
    ScriptResult CallFunction(const std::string& name, const std::vector<std::string>& args = {});

    /// Execute a script string (simple line-by-line command processor).
    /// Each line is: functionName arg1 arg2 ...
    ScriptResult ExecuteScript(const std::string& script);

    /// Set a global variable.
    void SetGlobal(const std::string& name, const std::string& value);

    /// Get a global variable.
    std::string GetGlobal(const std::string& name) const;

    /// Check if a global exists.
    bool HasGlobal(const std::string& name) const;

    /// Get all registered function names.
    std::vector<std::string> GetRegisteredFunctions() const;

    /// Get execution log (all outputs from calls).
    const std::vector<std::string>& GetLog() const;

    /// Clear execution log.
    void ClearLog();

private:
    std::unordered_map<std::string, ScriptFunction> functions_;
    std::unordered_map<std::string, std::string> globals_;
    std::vector<std::string> log_;
};

/// Mod metadata.
struct ModInfo {
    std::string id;
    std::string name;
    std::string version = "1.0.0";
    std::string author;
    std::string description;
    std::string mainScript = "main.lua";
    std::vector<std::string> dependencies;
    std::string directoryPath;
    bool loaded = false;
};

/// Mod manager handling discovery, dependency resolution, and loading.
class ModManager {
public:
    ModManager(ScriptingEngine& engine, const std::string& modsDirectory = "");

    /// Get mods directory path.
    const std::string& GetModsDirectory() const;

    /// Discover mods in the mods directory (looks for mod.json in subdirs).
    /// In this abstraction, mods can be added manually for testing.
    std::vector<ModInfo> DiscoverMods() const;

    /// Register a mod (for testing without filesystem).
    void RegisterMod(const ModInfo& mod);

    /// Get all registered mods.
    const std::unordered_map<std::string, ModInfo>& GetRegisteredMods() const;

    /// Resolve dependencies and return load order. Returns empty if circular dependency detected.
    std::vector<std::string> ResolveDependencies() const;

    /// Load a mod by id (checks dependencies, marks as loaded).
    bool LoadMod(const std::string& modId);

    /// Unload a mod by id.
    bool UnloadMod(const std::string& modId);

    /// Load all registered mods in dependency order.
    bool LoadAllMods();

    /// Check if a mod is loaded.
    bool IsModLoaded(const std::string& modId) const;

    /// Get loaded mod count.
    size_t GetLoadedModCount() const;

    /// Get the load order (ids in order they were loaded).
    const std::vector<std::string>& GetLoadOrder() const;

    /// Get mod info by id. Returns nullptr if not found.
    const ModInfo* GetModInfo(const std::string& modId) const;

    /// Reload all mods.
    bool ReloadAllMods();

private:
    bool TopologicalSort(const std::string& modId,
                         std::unordered_set<std::string>& visited,
                         std::unordered_set<std::string>& visiting,
                         std::vector<std::string>& order) const;

    ScriptingEngine& engine_;
    std::string modsDirectory_;
    std::unordered_map<std::string, ModInfo> mods_;
    std::vector<std::string> loadOrder_;
};

} // namespace subspace
