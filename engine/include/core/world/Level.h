#pragma once

#include "core/ecs/EntityManager.h"

#include <functional>
#include <string>

namespace subspace {

/// Represents a named, loadable scene (sector, main menu, cutscene, etc.).
///
/// A Level wraps an EntityManager so that each scene has its own isolated
/// entity namespace.  The engine swaps active levels on sector transitions
/// (save current → unload → load new → resume).
///
/// Usage:
/// @code
///   Level level("sector_7");
///   level.SetOnLoad([&](EntityManager& em) {
///       // spawn entities for sector 7
///   });
///   level.Load();
///
///   // per-frame:
///   if (level.IsLoaded()) {
///       EntityManager& em = level.GetEntityManager();
///       // update systems ...
///   }
///
///   level.Unload();
/// @endcode
class Level {
public:
    explicit Level(const std::string& name = "");

    // ---- Identity ----------------------------------------------------------
    const std::string& GetName() const noexcept { return _name; }
    void SetName(const std::string& n) { _name = n; }

    // ---- Lifecycle callbacks -----------------------------------------------

    /// Register a callback invoked by Load() to populate the EntityManager.
    void SetOnLoad(std::function<void(EntityManager&)> cb) {
        _onLoad = std::move(cb);
    }

    /// Register a callback invoked by Unload() to clean up game state.
    void SetOnUnload(std::function<void(EntityManager&)> cb) {
        _onUnload = std::move(cb);
    }

    // ---- Load / Unload -----------------------------------------------------

    /// Populate the level.
    /// Invokes the OnLoad callback (if set) then marks the level as loaded.
    void Load();

    /// Clear the level.
    /// Invokes the OnUnload callback (if set), then destroys all entities.
    void Unload();

    /// Whether Load() has been called and Unload() has not yet been called.
    bool IsLoaded() const noexcept { return _loaded; }

    // ---- Entity access -----------------------------------------------------

    EntityManager&       GetEntityManager()       noexcept { return _em; }
    const EntityManager& GetEntityManager() const noexcept { return _em; }

private:
    std::string    _name;
    EntityManager  _em;
    bool           _loaded = false;

    std::function<void(EntityManager&)> _onLoad;
    std::function<void(EntityManager&)> _onUnload;
};

} // namespace subspace
