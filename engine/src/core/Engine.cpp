#include "core/Engine.h"

#include "core/config/ConfigurationManager.h"
#include "core/events/EventSystem.h"
#include "core/events/GameEvents.h"
#include "core/logging/Logger.h"
#include "core/physics/PhysicsSystem.h"
#include "combat/CombatSystem.h"
#include "navigation/NavigationSystem.h"
#include "quest/QuestSystem.h"
#include "tutorial/TutorialSystem.h"
#include "power/PowerSystem.h"
#include "ai/AIDecisionSystem.h"
#include "mining/MiningSystem.h"
#include "rendering/ParticleSystem.h"
#include "achievement/AchievementSystem.h"
#include "ui/UISystem.h"

#ifdef SUBSPACE_USE_GLFW
#include "rendering/GLFWWindow.h"
#endif

#include <algorithm>
#include <iostream>
#include <thread>

namespace subspace {

static constexpr const char* kVersion = "SubspaceEngine v0.2.0";
static constexpr const char* kLogCategory = "Engine";

// ---------------------------------------------------------------------------
// Construction / destruction
// ---------------------------------------------------------------------------

Engine::Engine() = default;

Engine::~Engine()
{
    if (_state != EngineState::Stopped && _state != EngineState::Uninitialized) {
        Shutdown();
    }
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

void Engine::Initialize()
{
    if (_state != EngineState::Uninitialized) return;

    Logger::Instance().Info(kLogCategory, "Initializing engine...");

    // Load configuration (best-effort; defaults are fine).
    auto& config = ConfigurationManager::Instance();
    config.ValidateConfiguration();

    // Seed the galaxy generator from config.
    _galaxyGenerator = GalaxyGenerator(config.GetDevelopment().galaxySeed);

    // Register all gameplay systems into the ECS.
    RegisterSystems();

    // Create the platform window when GLFW is available.
#ifdef SUBSPACE_USE_GLFW
    _window = std::make_unique<GLFWWindow>();
    if (_window->Create(1280, 720, "Codename: Subspace")) {
        Logger::Instance().Info(kLogCategory, "GLFW window created (1280x720).");
    } else {
        Logger::Instance().Warning(kLogCategory,
            "Failed to create GLFW window — running headless.");
        _window.reset();
    }
#endif

    // Mark timestamps.
    _startTime    = Clock::now();
    _lastTickTime = _startTime;

    _state = EngineState::Running;

    // Notify listeners.
    GameEvent evt;
    evt.eventType = GameEvents::GameStarted;
    EventSystem::Instance().Publish(GameEvents::GameStarted, evt);

    Logger::Instance().Info(kLogCategory, "Engine initialized — entering Running state.");
}

void Engine::Run()
{
    if (_state != EngineState::Running && _state != EngineState::Paused) {
        Logger::Instance().Warning(kLogCategory,
            "Run() called but engine is not in Running/Paused state.");
        return;
    }

    Logger::Instance().Info(kLogCategory, "Entering main loop...");

    while (_state == EngineState::Running || _state == EngineState::Paused) {
#ifdef SUBSPACE_USE_GLFW
        if (_window) {
            _window->PollEvents();
            if (_window->ShouldClose()) {
                RequestShutdown();
                break;
            }
        }
#endif
        Tick();

        // Honour frame cap for testing.
        if (_maxFrames > 0 && _frameCount >= _maxFrames) {
            RequestShutdown();
        }
    }

    Logger::Instance().Info(kLogCategory, "Main loop exited.");
}

void Engine::Tick()
{
    if (_state == EngineState::ShuttingDown || _state == EngineState::Stopped) return;

    // Compute delta-time.
    auto now = Clock::now();
    float dt = std::chrono::duration<float>(now - _lastTickTime).count();
    _lastTickTime = now;

    // Clamp to avoid spiral-of-death on long hitches.
    dt = std::min(dt, _fixedTimestep * 4.0f);

    _lastDeltaTime = dt;
    _elapsedSeconds = std::chrono::duration<double>(now - _startTime).count();

    // 1. Process queued events.
    EventSystem::Instance().ProcessQueuedEvents();

    // 2. Update all systems (skipped while paused).
    if (_state == EngineState::Running) {
        _entityManager.UpdateSystems(dt);
    }

    // 3. Render the frame (UI draw commands are collected even while paused
    //    so that menus remain visible).
    RenderFrame();

    ++_frameCount;
}

void Engine::RequestShutdown()
{
    if (_state == EngineState::Running || _state == EngineState::Paused) {
        Logger::Instance().Info(kLogCategory, "Shutdown requested.");
        _state = EngineState::ShuttingDown;
    }
}

void Engine::Shutdown()
{
    if (_state == EngineState::Stopped) return;

    Logger::Instance().Info(kLogCategory, "Shutting down...");

#ifdef SUBSPACE_USE_GLFW
    if (_window) {
        _window->Destroy();
        _window.reset();
    }
#endif

    _entityManager.Shutdown();

    EventSystem::Instance().ClearAllListeners();

    _state = EngineState::Stopped;
    Logger::Instance().Info(kLogCategory, "Engine shut down.");
}

// ---------------------------------------------------------------------------
// State helpers
// ---------------------------------------------------------------------------

void Engine::Pause()
{
    if (_state == EngineState::Running) {
        _state = EngineState::Paused;

        GameEvent evt;
        evt.eventType = GameEvents::GamePaused;
        EventSystem::Instance().Publish(GameEvents::GamePaused, evt);

        Logger::Instance().Info(kLogCategory, "Engine paused.");
    }
}

void Engine::Resume()
{
    if (_state == EngineState::Paused) {
        _state = EngineState::Running;

        // Reset the tick clock so the first frame after un-pause doesn't
        // have a huge delta.
        _lastTickTime = Clock::now();

        GameEvent evt;
        evt.eventType = GameEvents::GameResumed;
        EventSystem::Instance().Publish(GameEvents::GameResumed, evt);

        Logger::Instance().Info(kLogCategory, "Engine resumed.");
    }
}

// ---------------------------------------------------------------------------
// System registration
// ---------------------------------------------------------------------------

void Engine::RegisterSystems()
{
    Logger::Instance().Debug(kLogCategory, "Registering systems...");

    _entityManager.RegisterSystem(std::make_unique<PhysicsSystem>(_entityManager));
    _entityManager.RegisterSystem(std::make_unique<CombatSystem>());
    _entityManager.RegisterSystem(std::make_unique<NavigationSystem>());
    _entityManager.RegisterSystem(std::make_unique<PowerSystem>());
    _entityManager.RegisterSystem(std::make_unique<AIDecisionSystem>());
    _entityManager.RegisterSystem(std::make_unique<MiningSystem>());
    _entityManager.RegisterSystem(std::make_unique<QuestSystem>());
    _entityManager.RegisterSystem(std::make_unique<TutorialSystem>());
    _entityManager.RegisterSystem(std::make_unique<ParticleSystem>());
    _entityManager.RegisterSystem(std::make_unique<AchievementSystem>());

    auto uiSystemPtr = std::make_unique<UISystem>();
    _uiSystem = uiSystemPtr.get();
    _entityManager.RegisterSystem(std::move(uiSystemPtr));

    Logger::Instance().Debug(kLogCategory, "All systems registered.");
}

// ---------------------------------------------------------------------------
// Rendering
// ---------------------------------------------------------------------------

void Engine::RenderFrame()
{
    _uiRenderer.BeginFrame(
        _uiSystem ? _uiSystem->GetScreenWidth()  : 1920.0f,
        _uiSystem ? _uiSystem->GetScreenHeight() : 1080.0f);

    if (_uiSystem) {
        _uiSystem->Render(_uiRenderer);
    }

    _uiRenderer.EndFrame();

#ifdef SUBSPACE_USE_GLFW
    if (_window) {
        _window->BeginRender();
        _window->RenderDrawCommands(_uiRenderer);
        _window->EndRender();
    }
#endif
}

// ---------------------------------------------------------------------------
// Version helpers
// ---------------------------------------------------------------------------

const char* Engine::GetVersionString()
{
    return kVersion;
}

void Engine::PrintVersionInfo()
{
    std::cout << kVersion << std::endl;
}

void engineInfo()
{
    Engine::PrintVersionInfo();
}

} // namespace subspace
