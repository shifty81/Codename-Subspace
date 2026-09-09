#include "scripting/ScriptingSystem.h"

#include <algorithm>
#include <sstream>
#include <stdexcept>

namespace subspace {

// --- ScriptingEngine ---

ScriptingEngine::ScriptingEngine() = default;

void ScriptingEngine::RegisterFunction(const std::string& name, ScriptFunction func) {
    functions_[name] = std::move(func);
}

bool ScriptingEngine::UnregisterFunction(const std::string& name) {
    return functions_.erase(name) > 0;
}

bool ScriptingEngine::HasFunction(const std::string& name) const {
    return functions_.find(name) != functions_.end();
}

size_t ScriptingEngine::GetFunctionCount() const {
    return functions_.size();
}

ScriptResult ScriptingEngine::CallFunction(const std::string& name,
                                           const std::vector<std::string>& args) {
    ScriptResult result;

    auto it = functions_.find(name);
    if (it == functions_.end()) {
        result.error = "Function not found: " + name;
        return result;
    }

    try {
        result.output = it->second(args);
        result.success = true;
        log_.push_back(result.output);
    } catch (const std::exception& e) {
        result.error = std::string("Exception in function '") + name + "': " + e.what();
    } catch (...) {
        result.error = "Unknown exception in function '" + name + "'";
    }

    return result;
}

ScriptResult ScriptingEngine::ExecuteScript(const std::string& script) {
    ScriptResult result;
    std::string combinedOutput;
    std::istringstream stream(script);
    std::string line;

    while (std::getline(stream, line)) {
        // Skip empty lines and comment lines
        if (line.empty() || line[0] == '#')
            continue;

        // Parse: functionName arg1 arg2 ...
        std::istringstream lineStream(line);
        std::string funcName;
        lineStream >> funcName;

        if (funcName.empty())
            continue;

        std::vector<std::string> args;
        std::string arg;
        while (lineStream >> arg)
            args.push_back(arg);

        ScriptResult callResult = CallFunction(funcName, args);
        if (!callResult.success) {
            result.error = callResult.error;
            return result;
        }

        if (!combinedOutput.empty() && !callResult.output.empty())
            combinedOutput += "\n";
        combinedOutput += callResult.output;
    }

    result.success = true;
    result.output = combinedOutput;
    return result;
}

void ScriptingEngine::SetGlobal(const std::string& name, const std::string& value) {
    globals_[name] = value;
}

std::string ScriptingEngine::GetGlobal(const std::string& name) const {
    auto it = globals_.find(name);
    if (it != globals_.end())
        return it->second;
    return "";
}

bool ScriptingEngine::HasGlobal(const std::string& name) const {
    return globals_.find(name) != globals_.end();
}

std::vector<std::string> ScriptingEngine::GetRegisteredFunctions() const {
    std::vector<std::string> names;
    names.reserve(functions_.size());
    for (const auto& pair : functions_)
        names.push_back(pair.first);
    return names;
}

const std::vector<std::string>& ScriptingEngine::GetLog() const {
    return log_;
}

void ScriptingEngine::ClearLog() {
    log_.clear();
}

// --- ModManager ---

ModManager::ModManager(ScriptingEngine& engine, const std::string& modsDirectory)
    : engine_(engine), modsDirectory_(modsDirectory) {}

const std::string& ModManager::GetModsDirectory() const {
    return modsDirectory_;
}

std::vector<ModInfo> ModManager::DiscoverMods() const {
    // In this abstraction, return all registered mods for testing without filesystem.
    std::vector<ModInfo> result;
    result.reserve(mods_.size());
    for (const auto& pair : mods_)
        result.push_back(pair.second);
    return result;
}

void ModManager::RegisterMod(const ModInfo& mod) {
    mods_[mod.id] = mod;
}

const std::unordered_map<std::string, ModInfo>& ModManager::GetRegisteredMods() const {
    return mods_;
}

std::vector<std::string> ModManager::ResolveDependencies() const {
    std::unordered_set<std::string> visited;
    std::unordered_set<std::string> visiting;
    std::vector<std::string> order;

    for (const auto& pair : mods_) {
        if (visited.find(pair.first) == visited.end()) {
            if (!TopologicalSort(pair.first, visited, visiting, order))
                return {}; // Circular dependency detected
        }
    }

    return order;
}

bool ModManager::TopologicalSort(const std::string& modId,
                                 std::unordered_set<std::string>& visited,
                                 std::unordered_set<std::string>& visiting,
                                 std::vector<std::string>& order) const {
    if (visiting.find(modId) != visiting.end())
        return false; // Circular dependency

    if (visited.find(modId) != visited.end())
        return true; // Already processed

    visiting.insert(modId);

    auto it = mods_.find(modId);
    if (it != mods_.end()) {
        for (const auto& dep : it->second.dependencies) {
            if (mods_.find(dep) == mods_.end())
                return false; // Missing dependency
            if (!TopologicalSort(dep, visited, visiting, order))
                return false;
        }
    }

    visiting.erase(modId);
    visited.insert(modId);
    order.push_back(modId);
    return true;
}

bool ModManager::LoadMod(const std::string& modId) {
    auto it = mods_.find(modId);
    if (it == mods_.end())
        return false;

    if (it->second.loaded)
        return true;

    // Check all dependencies are loaded
    for (const auto& dep : it->second.dependencies) {
        if (!IsModLoaded(dep)) {
            if (!LoadMod(dep))
                return false;
        }
    }

    it->second.loaded = true;
    loadOrder_.push_back(modId);
    return true;
}

bool ModManager::UnloadMod(const std::string& modId) {
    auto it = mods_.find(modId);
    if (it == mods_.end())
        return false;

    if (!it->second.loaded)
        return false;

    it->second.loaded = false;
    loadOrder_.erase(
        std::remove(loadOrder_.begin(), loadOrder_.end(), modId),
        loadOrder_.end());
    return true;
}

bool ModManager::LoadAllMods() {
    std::vector<std::string> order = ResolveDependencies();
    if (order.empty() && !mods_.empty())
        return false; // Dependency resolution failed

    for (const auto& modId : order) {
        if (!LoadMod(modId))
            return false;
    }

    return true;
}

bool ModManager::IsModLoaded(const std::string& modId) const {
    auto it = mods_.find(modId);
    if (it == mods_.end())
        return false;
    return it->second.loaded;
}

size_t ModManager::GetLoadedModCount() const {
    return loadOrder_.size();
}

const std::vector<std::string>& ModManager::GetLoadOrder() const {
    return loadOrder_;
}

const ModInfo* ModManager::GetModInfo(const std::string& modId) const {
    auto it = mods_.find(modId);
    if (it != mods_.end())
        return &it->second;
    return nullptr;
}

bool ModManager::ReloadAllMods() {
    // Unload all in reverse order
    for (auto it = loadOrder_.rbegin(); it != loadOrder_.rend(); ++it) {
        auto modIt = mods_.find(*it);
        if (modIt != mods_.end())
            modIt->second.loaded = false;
    }
    loadOrder_.clear();

    return LoadAllMods();
}

} // namespace subspace
