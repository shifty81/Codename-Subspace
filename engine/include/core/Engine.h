#pragma once

#include "core/ecs/EntityManager.h"
#include "procedural/GalaxyGenerator.h"
#include "ui/UIRenderer.h"

#include <chrono>
#include <cstdint>
#include <string>

namespace subspace {

// Forward declarations — systems owned via EntityManager::RegisterSystem()
class PhysicsSystem;
class CombatSystem;
class NavigationSystem;
class QuestSystem;
class TutorialSystem;
class PowerSystem;
class AIDecisionSystem;
class MiningSystem;
class UISystem;

/// Engine state machine.
enum class EngineState { Uninitialized, Running, Paused, ShuttingDown, Stopped };

/// Core game engine — owns the ECS world, all registered systems, and the
/// main update loop.  This is the single entry-point for Codename: Subspace.
///
/// Usage:
///   subspace::Engine engine;
///   engine.Initialize();
///   engine.Run();              // blocking loop
///   engine.Shutdown();
///
/// Or for external loop control:
///   engine.Initialize();
///   while (engine.GetState() == EngineState::Running) {
///       engine.Tick();         // one frame
///   }
///   engine.Shutdown();
class Engine {
public:
    Engine();
    ~Engine();

    // Non-copyable, non-movable.
    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;

    // ----- lifecycle -----

    /// Initialize all subsystems.  Must be called before Run() or Tick().
    void Initialize();

    /// Enter the blocking main loop. Returns when RequestShutdown() is called.
    void Run();

    /// Advance the simulation by one frame.
    void Tick();

    /// Request an orderly shutdown (the current or next frame will exit).
    void RequestShutdown();

    /// Tear down all subsystems and release resources.
    void Shutdown();

    // ----- state -----

    /// Pause the simulation (systems stop updating, but events are still
    /// processed so the UI can keep working).
    void Pause();

    /// Resume from a paused state.
    void Resume();

    EngineState GetState() const { return _state; }
    bool IsRunning() const { return _state == EngineState::Running; }
    bool IsPaused()  const { return _state == EngineState::Paused; }

    // ----- accessors (valid after Initialize) -----

    EntityManager&       GetEntityManager()       { return _entityManager; }
    const EntityManager& GetEntityManager() const { return _entityManager; }

    GalaxyGenerator&       GetGalaxyGenerator()       { return _galaxyGenerator; }
    const GalaxyGenerator& GetGalaxyGenerator() const { return _galaxyGenerator; }

    UIRenderer&       GetUIRenderer()       { return _uiRenderer; }
    const UIRenderer& GetUIRenderer() const { return _uiRenderer; }

    // ----- metrics -----

    /// Frames processed since Initialize().
    uint64_t GetFrameCount()      const { return _frameCount; }

    /// Wall-clock seconds elapsed since Initialize().
    double   GetElapsedSeconds()  const { return _elapsedSeconds; }

    /// Delta-time used for the most recent Tick().
    float    GetLastDeltaTime()   const { return _lastDeltaTime; }

    /// Target fixed timestep in seconds (default 1/60).
    float GetFixedTimestep()                const { return _fixedTimestep; }
    void  SetFixedTimestep(float dt)              { _fixedTimestep = dt; }

    /// Maximum frames to run (0 = unlimited).  Useful for testing.
    void  SetMaxFrames(uint64_t max)              { _maxFrames = max; }

    // ----- version -----

    static const char* GetVersionString();

    /// Print engine version information to stdout.
    static void PrintVersionInfo();

private:
    void RegisterSystems();
    void RenderFrame();

    EngineState _state = EngineState::Uninitialized;

    EntityManager    _entityManager;
    GalaxyGenerator  _galaxyGenerator;
    UIRenderer       _uiRenderer;
    UISystem*        _uiSystem = nullptr;   // non-owning; owned by EntityManager

    // Timing
    using Clock = std::chrono::steady_clock;
    Clock::time_point _startTime;
    Clock::time_point _lastTickTime;
    float             _fixedTimestep  = 1.0f / 60.0f;
    float             _lastDeltaTime  = 0.0f;
    double            _elapsedSeconds = 0.0;
    uint64_t          _frameCount     = 0;
    uint64_t          _maxFrames      = 0;
};

/// Print engine version information (legacy free-function kept for
/// backwards compatibility with existing main.cpp).
void engineInfo();

} // namespace subspace
