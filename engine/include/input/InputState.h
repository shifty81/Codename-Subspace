#pragma once

#include <array>
#include <cstddef>

namespace subspace {

/// Platform-independent gameplay actions. Native window backends translate
/// keyboard/mouse/gamepad input into this contract; gameplay systems never
/// depend directly on Win32, GLFW, Silk.NET, or another input library.
enum class InputAction : std::size_t {
    ThrustForward = 0,
    ThrustReverse,
    StrafeLeft,
    StrafeRight,
    TurnLeft,
    TurnRight,
    ToggleFlightMode,
    Boost,
    EmergencyBrake,
    ToggleDampening,
    ToggleCutaway,
    FirePrimary,
    FireMiningMissile,
    ToggleInterior,
    RequestDock,
    Pause,
    OpenGalaxyMap,
    OpenSystemMap,
    OpenPlanetSurvey,
    OpenPlanetaryManufacturing,
    OpenStationBuilder,
    OpenShipBuilder,
    OpenHangarFitting,
    OpenMarketContracts,
    OpenExploration,
    OpenFleetCorporation,
    MenuAccept,
    MenuNext,
    MenuPrevious,
    MenuBack,
    ToggleShipInspection,
    Undo,
    Redo,
    SaveBlueprint,
    // Editor-authoring shortcuts are separate from flight actions so the same
    // physical keys can remain familiar in both contexts without allowing a
    // structural Shipyard tool to leak propulsion or weapon commands.
    EditorToolSelect,
    EditorToolMove,
    EditorToolRotate,
    EditorToolScale,
    EditorFrameSelected,
    EditorFrameShip,
    EditorDeleteModule,
    EditorNudgeLeft,
    EditorNudgeRight,
    EditorNudgeForward,
    EditorNudgeAft,
    EditorNudgeUp,
    EditorNudgeDown,
    Count
};

struct InputActionState {
    float value = 0.0f;
    bool down = false;
    bool pressed = false;
    bool released = false;
};

/// Frame-stable input state used by native gameplay systems. Digital actions
/// use values 0/1; analog devices may provide values in [0, 1].
class InputState {
public:
    void SetAction(InputAction action, bool down);
    void SetActionValue(InputAction action, float value);

    bool IsDown(InputAction action) const;
    bool WasPressed(InputAction action) const;
    bool WasReleased(InputAction action) const;
    float GetValue(InputAction action) const;

    /// Clears one-frame edge flags while preserving held state.
    void EndFrame();

    /// Releases every action and clears all edge flags.
    void Clear();

private:
    static constexpr std::size_t kActionCount = static_cast<std::size_t>(InputAction::Count);
    static std::size_t Index(InputAction action) { return static_cast<std::size_t>(action); }

    std::array<InputActionState, kActionCount> _actions{};
};

} // namespace subspace
